#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"
#include "Serial.h"

/* 1つのシリアル通信に関するデータ構造 */
struct _TAG_SERIAL {
	// 通信関係
	HANDLE handle;
	HANDLE handle_plusb;
	
	// スレッドに関して
	HANDLE thread_handle;
	DWORD thread_id;
	BOOL thread_active;
	CRITICAL_SECTION cs_send;
	CRITICAL_SECTION cs_recv;
	
	// FIFO
	fifo_t *q_recv;
	fifo_t *q_send;
	
	// その他
	char *msg;
};

/*安定感電源用コマンド列*/
enum HM310T_CMD{
	HM310T_CMD_ON = 0,
	HM310T_CMD_OFF,
	HM310T_CMD_SETVOL,
	HM310T_CMD_NUM
};

#define HM310T_CMD_LENGTH 8

const char HM310T[HM310T_CMD_NUM][HM310T_CMD_LENGTH] =
{
	//SleveAdd,funcID ,CMD_ID   ,Value    ,CRC
	{ 0x01    ,0x06   ,0x00,0x01,0x00,0x01,0x00,0x00 },
	{ 0x01    ,0x06   ,0x00,0x01,0x00,0x00,0x00,0x00 },
	{ 0x01    ,0x06   ,0x00,0x30,0x00,0x00,0x00,0x00 },
};

/* プロトタイプ宣言 */
DWORD WINAPI	serial_thread ( LPVOID param );
HANDLE			serial_open   ( char   *pname, unsigned int baud);
BOOL			is_vcc_ctrlcmd( char   *send_buf, DWORD *send_size);
unsigned int    crc16         ( int    n, unsigned char c[]);

/* 1次バッファ */
#define SERIAL_TMP_BUFSIZE	4096

/* 安定化電源用のCRC生成関数 */
unsigned int crc16(int n, unsigned char c[])
{
#define CRC16POLY2  0xA001U  /* 左右逆転 */
	int i, j;
	unsigned long r;
	unsigned long ret;

	r = 0xFFFFU;
	for (i = 0; i < n; i++) {
		r ^= c[i];
		for (j = 0; j < 8; j++)
			if (r & 1) r = (r >> 1) ^ CRC16POLY2;
			else       r >>= 1;
	}
	
	r = r & 0xFFFFU;
	
	ret = ((r&0xFF00U)>>8) + ((r&0x00FF)<<8);
	
	return ret;
}


/* シリアル通信を開始 */
serial_t serial_create(char *p_dbg_name, unsigned int p_dbg_baud, char *p_plusb_name, unsigned int p_plusb_baud)
{
	serial_t obj;

	// インスタンスメモリ確保
	obj = (serial_t) malloc(sizeof(struct _TAG_SERIAL));
	if ( obj == NULL ) return NULL;
	ZeroMemory(obj,sizeof(struct _TAG_SERIAL));

	// デバッグボード用COMポートの初期化
	obj->handle = serial_open( p_dbg_name, p_dbg_baud );
	if ( obj->handle == INVALID_HANDLE_VALUE ) {
		printf("DebugBoard Serial Open Error.\n");
		CloseHandle(obj->handle);
		free(obj);
		return NULL;
	}

	// +B制御用安定化電源用COMポートの初期化
	//   ※+B安定化電源用のCOMポートはOpenできなくても処理は継続できるようにする。
	obj->handle_plusb = serial_open( p_plusb_name, p_plusb_baud );
	if ( obj->handle_plusb == INVALID_HANDLE_VALUE ) {
		printf("PlusB Serial Open Error.\n");
	}

	// FIFOメモリ確保
	obj->q_send = fifo_create();
	obj->q_recv = fifo_create();
	if ( obj->q_send == NULL || obj->q_recv == NULL ) {
		CloseHandle(obj->handle);
		CloseHandle(obj->handle_plusb);
		fifo_delete(obj->q_send);
		fifo_delete(obj->q_recv);
		free(obj);
		return NULL;
	}

	// スレッド開始
	InitializeCriticalSection(&obj->cs_recv);
	InitializeCriticalSection(&obj->cs_send);
	obj->thread_active = TRUE;
	obj->thread_handle = CreateThread(NULL,0,serial_thread,(LPVOID *)obj,0,&obj->thread_id);
	if ( obj->thread_handle == NULL ) {
		DeleteCriticalSection(&obj->cs_recv);
		DeleteCriticalSection(&obj->cs_send);
		CloseHandle(obj->handle);
		CloseHandle(obj->handle_plusb);
		fifo_delete(obj->q_send);
		fifo_delete(obj->q_recv);
		free(obj);
		return NULL;
	}

	return obj;
}

HANDLE serial_open(char *pname, unsigned int baud)
{
	HANDLE         seral_haldle;
	COMMTIMEOUTS   timeout;
	DCB            dcb;

	printf("%s\n",pname);
	
	// COMポートのハンドルを取得
	seral_haldle = CreateFile(pname,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if ( seral_haldle == INVALID_HANDLE_VALUE ) {
		return INVALID_HANDLE_VALUE;
	}
	
	// COMポートの通信設定
	GetCommState(seral_haldle, &dcb);
	dcb.BaudRate = baud;
	dcb.fBinary  = TRUE;
	dcb.ByteSize = 8;
	dcb.fParity  = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	if ( SetCommState(seral_haldle, &dcb) == FALSE ) {
		CloseHandle(seral_haldle);
		return INVALID_HANDLE_VALUE;
	}

	// COMポートの初期化
	PurgeComm(seral_haldle,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );
	
	
	// COMポートのタイムアウト設定
	ZeroMemory(&timeout,sizeof(COMMTIMEOUTS));
	timeout.ReadIntervalTimeout = MAXDWORD;
	if ( SetCommTimeouts(seral_haldle, &timeout) == FALSE ) {
		CloseHandle(seral_haldle);
		return INVALID_HANDLE_VALUE;
	}

	return seral_haldle;
}

/* シリアル通信を停止 */
void serial_delete(serial_t obj)
{
	DWORD thread_state;
	
	// スレッドを停止
	obj->thread_active = FALSE;
	do {
		Sleep(1);
		GetExitCodeThread(obj->thread_handle,&thread_state);
	} while (thread_state == STILL_ACTIVE);
	DeleteCriticalSection(&obj->cs_send);
	DeleteCriticalSection(&obj->cs_recv);
	
	// 通信ポートを閉じる
	CloseHandle(obj->handle);
	
	// メモリー領域の解放
	fifo_delete(obj->q_send);
	fifo_delete(obj->q_recv);
	free(obj);
}

/* シリアル通信スレッド */
DWORD WINAPI serial_thread(LPVOID param)
{
	serial_t obj = (serial_t) param;
	BYTE recv_buf[SERIAL_TMP_BUFSIZE]={0};
	BYTE send_buf[SERIAL_TMP_BUFSIZE]={0};
	DWORD recv_len;
	DWORD send_len,send_size;
	BOOL ret;
	BOOL recv_hold = FALSE;
	BOOL send_hold = FALSE;
	HANDLE         serial_handle;
	BOOL vcc_ctrl  = FALSE;

	
	while ( obj->thread_active ) {
		// 受信
		if ( recv_hold == FALSE ) {
			ret = ReadFile(obj->handle, recv_buf, sizeof(recv_buf), &recv_len, NULL);
			if ( ret == FALSE ) {
				obj->msg = "ReadFile failed.";
				break;
			}
			if ( recv_len )
			{
				recv_buf[recv_len]=0;
				recv_hold = TRUE;
			}
		} else if ( TryEnterCriticalSection(&obj->cs_recv) ) {
			recv_len -= fifo_write(obj->q_recv, recv_buf, recv_len);
			LeaveCriticalSection(&obj->cs_recv);
			if ( recv_len != 0 )	obj->msg = "q_recv is fully filled. (>_<)";
			recv_hold = FALSE;
		}
		
		// 送信
		if ( send_hold ) {
			
			// 送信内容から送信先を選択
			vcc_ctrl = is_vcc_ctrlcmd( send_buf, &send_size );
			
			if( TRUE == vcc_ctrl )	
			{
				serial_handle = obj->handle_plusb;
				printf("cmd=%x %x %x %x %x %x %x %x\n", send_buf[0], send_buf[1], send_buf[2], send_buf[3], send_buf[4], send_buf[5], send_buf[6], send_buf[7]);
			}
			else
			{ 
				serial_handle = obj->handle;
				printf("%s\n", send_buf);
			}
			
			if( serial_handle == INVALID_HANDLE_VALUE )
			{
				// シリアル通信路Openできていないため終了する。
				send_hold = FALSE;
				continue;
			}
			
			// データ送信
			ret = WriteFile(serial_handle, send_buf, send_size, &send_len, NULL);
			if ( ret == FALSE ) {
				obj->msg = "WriteFile failed.";
				break;
			}
			if ( send_size != send_len )	obj->msg = "WriteFile spilled some of q_send.";

			// 送信後のステータス初期化
			send_hold = FALSE;
			memset(send_buf, 0, SERIAL_TMP_BUFSIZE);
		} else if ( TryEnterCriticalSection(&(obj->cs_send)) ) {
			send_size = fifo_read(obj->q_send, send_buf, SERIAL_TMP_BUFSIZE);
			LeaveCriticalSection(&(obj->cs_send));
			
			if( send_size <= 0 )
			{
				Sleep(5);
			}
			else
			{
				send_hold = TRUE;
			}
		}
	}
	
	obj->thread_active = FALSE;
	ExitThread(TRUE);

	return 0;
}


BOOL is_vcc_ctrlcmd( char *send_buf, DWORD *send_size )
{
	char cmd[100];
	char kind[100];
	char val[100];
	int  data;
	unsigned int crc;

	sscanf( send_buf, "%s %s %s", cmd, kind, val );
	
	// 電源制御命令ではない場合、何もせずFALSEを返す。
	if( 0 != strncmp( cmd, "vcc", 3 ) )
	{
		return FALSE;
	}
	
	// 電源制御命令の場合、電源に送信するデータに変換する
	if( 0 == strncmp( kind, "on", 2 ) )
	{
		// 電源ONコマンドの場合
		memcpy( send_buf, HM310T[HM310T_CMD_ON], HM310T_CMD_LENGTH );
	}
	else if( 0 == strncmp( kind, "off", 3 ) )
	{
		// 電源OFFコマンドの場合
		memcpy( send_buf, HM310T[HM310T_CMD_OFF], HM310T_CMD_LENGTH );
	}
	else if( 0 == strncmp( kind, "setvol", 6 ) )
	{
		// 電源電圧設定コマンドの場合
		memcpy( send_buf, HM310T[HM310T_CMD_SETVOL], HM310T_CMD_LENGTH );

		// 電源電圧をコマンド列に埋め込む
		data = atoi( val );
		send_buf[4] = data/256;
		send_buf[5] = data%256;
	}

	// CRCを計算する
	crc = crc16(6,send_buf);
	send_buf[6] = crc/256;
	send_buf[7] = crc%256;
	
	*send_size = HM310T_CMD_LENGTH;

	return TRUE;
}


/* 送信する */
unsigned int serial_send(serial_t obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	EnterCriticalSection(&obj->cs_send);
	ret = fifo_write(obj->q_send, buf, size);
	LeaveCriticalSection(&obj->cs_send);
	return ret;
}

/* 受信する */
unsigned int serial_recv(serial_t obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	EnterCriticalSection(&obj->cs_recv);
	ret = fifo_read(obj->q_recv, buf, size);
	LeaveCriticalSection(&obj->cs_recv);
	return ret;
}

/* 受信する */
unsigned int serial_recv_block(serial_t obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	EnterCriticalSection(&obj->cs_recv);
	ret = fifo_read_block(obj->q_recv, buf, size);
	LeaveCriticalSection(&obj->cs_recv);
	return ret;
}


/* 受信したバイト数を取得 */
unsigned int serial_recv_length(serial_t obj)
{
	return fifo_length(obj->q_recv);
}