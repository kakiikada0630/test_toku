#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Serial.h"
#include "fifo.h"

#include "DebugApl.h"


struct Parameter PARAM;


typedef struct _TAG_SYSCTRL
{
#define FILE_LABEL_SIZE 100

	BOOL file_open;
	BOOL thread_active;
	BOOL data_get;

	CRITICAL_SECTION cs_data_get;

	char label[FILE_LABEL_SIZE];
	FILE *output_org;
	FILE *output_analized;
	serial_t obj;

	HANDLE thread_handle;
	DWORD thread_id;

	HANDLE thread_server_handle;
	DWORD  thread_server_id;
} sysctrl_t;

static sysctrl_t sys_t={0};
FILE *log_file;


static unsigned char MICON_VERSION[VER_SIZE] = {0};

void Analize_PWM( unsigned char* buf, int size, struct Parameter *param )
{
	unsigned short* buf16 = (unsigned short *)(buf+BLOCK_PWM);
	
	param->PWM_DISCHARGE = *(buf16+0);
	param->PWM_A1        = *(buf16+1);
	param->PWM_A2        = *(buf16+2);
	param->PWM_B1        = *(buf16+3);
	param->PWM_B2        = *(buf16+4);
	param->PWM_B3        = *(buf16+5);
	param->PWM_UDIM21    = *(buf16+6);
	param->PWM_UDIM22    = *(buf16+7);
}


void Analize_SPI( unsigned char* buf, int size, struct Parameter *param )
{
	#define MAX_CMD_SIZE  (BLOCK_UART - BLOCK_SPI)/2
	#define BST_CMD_SIZE  6

	unsigned short *buf16         = (unsigned short *)(buf+BLOCK_SPI);
	unsigned int   cmd_size       = 0;
	unsigned int   buck_cmd_size  = 0;
	
	//SPIコマンドサイズ確認
	for( cmd_size=0 ; cmd_size<MAX_CMD_SIZE; cmd_size++ )
	{
		// アドレス部含め、コマンドが0となることはないため、
		// 0となるまでは有効コマンドが詰まっているものと解釈する。
		if(*(buf16+cmd_size) == 0 ){ break; }
	}

	if( ( 0 != cmd_size%2 ) || ( cmd_size < BST_CMD_SIZE ) )
	{
		return;
	}

	buck_cmd_size = (cmd_size - BST_CMD_SIZE)/2;

	//--------------------------------------------------
	//Boostへの命令の検索
	//--------------------------------------------------
	unsigned int   boost_vol  = 0;
	for(int i=0 ; i<BST_CMD_SIZE ; i++)
	{
		unsigned short address;
		unsigned short data;

		address = (((*buf16) & 0x0078) >> 3);               	// SPIはビッグエンディアンで送信してきている。
		data    = (((*buf16) & 0x0003) << 8) + ((*buf16) >> 8);	// プログラム上はリトルエンディアンとして扱うため
																// データを修正する。
		if( address == 0x04 ) // Register Ox04
		{
			#define VAL_DIFF   103  // ( 125 -  22)
			#define VOL_DIFF 53300  // (64.8 -11.5)*1000  ※単位をmVに修正
			short BoostVsetpoint = data & 0x007f;
			
			boost_vol = VOL_DIFF*BoostVsetpoint/VAL_DIFF + 22;  // レジスタ値(BoostVsetpoint)から電圧を計算
			break;
		}
		buf16++;
	}

	param->BOOST_VOL = boost_vol;

	//--------------------------------------------------
	//Buck(StringA/B)への命令の検索
	//--------------------------------------------------
	buf16 = (unsigned short *)(buf+BLOCK_SPI+BST_CMD_SIZE*2); // Buckコマンドの先頭位置へ移動
	unsigned int Ch1adj = 0;
	unsigned int Ch2adj = 0;

	for(int i=0 ; i<buck_cmd_size ; i++)
	{
		unsigned short address;
		unsigned short data;

		address = (((*buf16) & 0x007E) >> 1);   // SPIはビッグエンディアンで送信してきている。
		data    = ( (*buf16)           >> 8);	// プログラム上はリトルエンディアンとして扱うため
												// データを修正する。
		if( 0x09 == address)          // CH1ADJH
		{
			Ch1adj += data << 2;
		}
		else if( 0x08 == address )    // CH1ADJL
		{
			Ch1adj += data & 0x0003;
		}
		else if( 0x0B == address )    // CH2ADJH
		{
			Ch2adj += data << 2;
		}
		else if( 0x0A == address )    // CH2ADJL
		{
			Ch2adj += data & 0x0003;
		}
		buf16++;
	}
	#define V_DACFS  2480  // mVに変換済み
	#define R_CS     100   // mΩに変換済み(小数点回避のため)
	#define SAITEKIKA (V_DACFS*1000)/R_CS
	
	param->A_CUR = (Ch1adj * SAITEKIKA)/(1024 * 14 );
	param->B_CUR = (Ch2adj * SAITEKIKA)/(1024 * 14 );

	//--------------------------------------------------
	//Buck(StringC/D)への命令の検索
	//--------------------------------------------------
	buf16 = (unsigned short *)(buf+BLOCK_SPI+BST_CMD_SIZE*2+buck_cmd_size*2); // Buckコマンドの先頭位置へ移動
	Ch1adj = 0;
	Ch2adj = 0;

	for(int i=0 ; i<buck_cmd_size ; i++)
	{
		unsigned short address;
		unsigned short data;

		address = (((*buf16) & 0x007E) >> 1);   // SPIはビッグエンディアンで送信してきている。
		data    = ( (*buf16)           >> 8);	// プログラム上はリトルエンディアンとして扱うため
												// データを修正する。
		if( 0x09 == address)          // CH1ADJH
		{
			Ch1adj += data << 2;
		}
		else if( 0x08 == address )    // CH1ADJL
		{
			Ch1adj += data & 0x0003;
		}
		else if( 0x0B == address )    // CH2ADJH
		{
			Ch2adj += data << 2;
		}
		else if( 0x0A == address )    // CH2ADJL
		{
			Ch2adj += data & 0x0003;
		}
		
		buf16++;
	}

	param->C_CUR = (Ch1adj * SAITEKIKA)/(1024 * 14 );
	param->D_CUR = (Ch2adj * SAITEKIKA)/(1024 * 14 );
}

void Analize_UART( unsigned char* buf, int size, struct Parameter *param )
{
	unsigned char *buf8 = buf+BLOCK_UART;
	
	unsigned int StringC[12] = {0};
	unsigned int StringD[ 8] = {0};
	unsigned int count=0;
	

	while(1)
	{
		unsigned int cmd;
		unsigned int data_size;
		unsigned int dev_add;
		unsigned int reg_add;
		unsigned int data;

		if( count > size )
		{
			break;
		}
		
		cmd = *buf8;
 		
		// Write命令
		if     ( cmd == 0x87 ){ data_size = 1; }
		else if( cmd == 0x99 ){ data_size = 2; }
		else if( cmd == 0x1E ){ data_size = 3; }
		else if( cmd == 0xAA ){ data_size = 4; }
		else if( cmd == 0x2D ){ data_size =12; }
		else if( cmd == 0x33 ){ data_size =16; }
		else if( cmd == 0xB4 ){ data_size =32; }

		// Read命令
		else if( cmd == 0x4B ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0xCC ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0xD2 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0x55 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0xE1 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0x66 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0x78 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else
		{
			count++;
			buf8++;
			continue;
		}
		
		// デバイスIDの取得
		buf8 = buf8+1;
		dev_add = *buf8;
		count++;

		// レジスタアドレス取得
		buf8 = buf8+1;
		reg_add = *buf8;
		buf8 = buf8+1;

		count+=2;
		
		for(int i=0 ; i<data_size ; i++)
		{
			data = *buf8;

			// LMM出力PWMのWIDTH値取得
			if     ( 0x30 == reg_add ){ if(0x20==dev_add){ StringC[11]+=(data<<2); } if(0x61==dev_add){StringD[7]+=(data<<2);} }
			else if( 0x31 == reg_add ){ if(0x20==dev_add){ StringC[10]+=(data<<2); } if(0x61==dev_add){StringD[6]+=(data<<2);} }
			else if( 0x32 == reg_add ){ if(0x20==dev_add){ StringC[ 9]+=(data<<2); } if(0x61==dev_add){StringD[5]+=(data<<2);} }
			else if( 0x33 == reg_add ){ if(0x20==dev_add){ StringC[ 8]+=(data<<2); } if(0x61==dev_add){StringD[4]+=(data<<2);} }
			else if( 0x34 == reg_add ){ if(0x20==dev_add){ StringC[ 7]+=(data<<2); } if(0x61==dev_add){StringD[3]+=(data<<2);} }
			else if( 0x35 == reg_add ){ if(0x20==dev_add){ StringC[ 6]+=(data<<2); } if(0x61==dev_add){StringD[2]+=(data<<2);} }
			else if( 0x36 == reg_add ){ if(0x20==dev_add){ StringC[ 5]+=(data<<2); } if(0x61==dev_add){StringD[1]+=(data<<2);} }
			else if( 0x37 == reg_add ){ if(0x20==dev_add){ StringC[ 4]+=(data<<2); } if(0x61==dev_add){StringD[0]+=(data<<2);} }
			else if( 0x38 == reg_add ){ if(0x20==dev_add){ StringC[ 3]+=(data<<2); } }
			else if( 0x39 == reg_add ){ if(0x20==dev_add){ StringC[ 2]+=(data<<2); } }
			else if( 0x3A == reg_add ){ if(0x20==dev_add){ StringC[ 1]+=(data<<2); } }
			else if( 0x3B == reg_add ){ if(0x20==dev_add){ StringC[ 0]+=(data<<2); } }
			else if( 0x3C == reg_add ){ if(0x20==dev_add){ StringC[11]+=(data &0x03); StringC[10]+=((data &0x0C)>>2); StringC[9]+=((data &0x30)>>4); }
										if(0x61==dev_add){ StringD[ 7]+=(data &0x03); StringD[ 6]+=((data &0x0C)>>2); StringD[5]+=((data &0x30)>>4); } }
			else if( 0x3D == reg_add ){ if(0x20==dev_add){ StringC[ 8]+=(data &0x03); StringC[ 7]+=((data &0x0C)>>2); StringC[6]+=((data &0x30)>>4); }
										if(0x61==dev_add){ StringD[ 4]+=(data &0x03); StringD[ 3]+=((data &0x0C)>>2); StringD[2]+=((data &0x30)>>4); } }
			else if( 0x3E == reg_add ){ if(0x20==dev_add){ StringC[ 5]+=(data &0x03); StringC[ 4]+=((data &0x0C)>>2); StringC[3]+=((data &0x30)>>4); }
										if(0x61==dev_add){ StringD[ 1]+=(data &0x03); StringD[ 0]+=((data &0x0C)>>2);                                } }
			else if( 0x3F == reg_add ){ if(0x20==dev_add){ StringC[ 2]+=(data &0x03); StringC[ 1]+=((data &0x0C)>>2); StringC[0]+=((data &0x30)>>4); } }
			else {}

			reg_add = reg_add+1;
			buf8    = buf8+1;
			count   = count+1;
		}
		
		// CRCを回避し、次のコマンドへ
		buf8 = buf8+2;
		count= count+2;
	}
	
	param->PWM_C1 = StringC[ 0]*10000/1023;
	param->PWM_C2 = StringC[ 1]*10000/1023;
	param->PWM_C3 = StringC[ 2]*10000/1023;
	param->PWM_C4 = StringC[ 3]*10000/1023;
	param->PWM_C5 = StringC[ 4]*10000/1023;
	param->PWM_C6 = StringC[ 5]*10000/1023;
	param->PWM_C7 = StringC[ 6]*10000/1023;
	param->PWM_C8 = StringC[ 7]*10000/1023;
	param->PWM_C9 = StringC[ 8]*10000/1023;
	param->PWM_C10= StringC[ 9]*10000/1023;
	param->PWM_C11= StringC[10]*10000/1023;
	param->PWM_C12= StringC[11]*10000/1023;
	param->PWM_D1 = StringD[ 0]*10000/1023;
	param->PWM_D2 = StringD[ 1]*10000/1023;
	param->PWM_D3 = StringD[ 2]*10000/1023;
	param->PWM_D4 = StringD[ 3]*10000/1023;
	param->PWM_D5 = StringD[ 4]*10000/1023;
	param->PWM_D6 = StringD[ 5]*10000/1023;
	param->PWM_D7 = StringD[ 6]*10000/1023;
	param->PWM_D8 = StringD[ 7]*10000/1023;
}

void Analize_DAC( unsigned char* buf, int size, struct Parameter *param )
{
	short *dac_pnt = (short *)(buf+BLOCK_DAC);
	
	param->DAC_LCD  = *(dac_pnt  );
	param->DAC_LED1 = *(dac_pnt+1);
	param->DAC_LED2 = *(dac_pnt+2);
	param->DAC_LED3 = *(dac_pnt+3);
}

void Analize_SW( unsigned char* buf, int size, struct Parameter *param )
{
	unsigned int sw_status = *((unsigned int *)(buf+BLOCK_SW));

	param->VBU    = ( 0 == (sw_status & SW_VBU_STATUS    ))? 0 : 1;
	param->IG1    = ( 0 == (sw_status & SW_IG1_STATUS    ))? 0 : 1;
	param->TURNS  = ( 0 == (sw_status & SW_TURNS_STATUS  ))? 0 : 1;
	param->HLBKUP = ( 0 == (sw_status & SW_HLBKUP_STATUS ))? 0 : 1;
}

void Analize( unsigned char* buf, int size, struct Parameter *param )
{
	unsigned int* tick_pnt = (unsigned int*)(buf+BLOCK_TICK);
	unsigned char ver[VER_SIZE+1] = VERSION;
	param->TICK = *tick_pnt;

	Analize_PWM (buf, size, param);
	Analize_SPI (buf, size, param);
	Analize_UART(buf, size, param);
	Analize_DAC (buf, size, param);
	Analize_SW  (buf, size, param);
	
	for( int i=0 ; i< VER_SIZE ; i++ )
	{
		MICON_VERSION[i] = *(buf+BLOCK_VER+i);
	}
}

void WriteLog( unsigned char* buf, int size, struct Parameter *param )
{
	//-----------------------------------------
	//ログをそのまま書き出し
	//-----------------------------------------
	static unsigned char  data[2000]={0};
	unsigned char  *out= data;
	unsigned char  *buf8  = 0;
	unsigned short *buf16 = (unsigned short*)(buf+BLOCK_PWM);
	unsigned int   *buf32;
	unsigned int   loop   = 0;

	if( (sys_t.output_org == NULL) || (sys_t.output_analized == NULL) )
	{
		return;
	}

	fprintf(sys_t.output_org,"[%7d],",param->TICK );

	// PWM出力
	loop = ( PWM_SIZE )/ 2;
	for( int i=0 ; i<loop  ; i++ )
	{
		sprintf( out+i*6, "%04x, ", *(buf16+i) );
	}
	out += loop*6; // PWM領域分のアドレスを進める

	// SPI
	loop = ( SPI_SIZE );
	buf8 = buf + BLOCK_SPI;
	
	for( int i=0 ; i<loop  ; i++ )
	{
		sprintf( out+i*2, "%02x", *(buf8+i) );
	}
	out += loop*2;
	sprintf( out, " ,");
	out+=2;

	// UART
	loop = ( UART_SIZE );
	buf8 = buf + BLOCK_UART;
	for( int i=0 ; i<loop  ; i++ )
	{
		sprintf( out+i*2, "%02x", *(buf8+i) );
	}
	out += loop*2;
	sprintf( out, " ,");
	out+=2;

	// LIN
	loop = ( LIN_SIZE );
	buf8 = buf + BLOCK_LIN;
	for( int i=0 ; i<loop  ; i++ )
	{
		sprintf( out+i*2, "%02x", *(buf8+i) );
	}
	out += loop*2;
	sprintf( out, " ,");
	out+=2;

	// CAN
	loop = (CAN_SIZE);
	buf8 = buf + BLOCK_CAN;
	for( int i=0 ; i<loop ; i++ )
	{
		sprintf( out+i*2, "%02x", *(buf8+i) );
	}
	out += loop*2;
	sprintf( out, " ,");
	out+=2;
	
	// DAC
	loop = ( DAC_SIZE )/2;
	buf16 = (unsigned short*)(buf + BLOCK_DAC);
	for( int i=0 ; i<loop  ; i++ )
	{
		sprintf( out+i*6, "%04x, ", *(buf16+i) );
	}
	out+=6*loop;

	// SW
	loop = SW_SIZE/4;
	buf32 = (unsigned int*)(buf + BLOCK_SW);
	for( int i=0 ; i<loop  ; i++ )
	{
		sprintf( out+i*10, "%08x, ", *(buf32+i) );
	}
	out+=10*loop;
	*out=0;
	
	fprintf(sys_t.output_org,"%s\n",data );

	//-----------------------------------------
	//解析後の数値を書き出し
	//-----------------------------------------
	out= data;
	fprintf(sys_t.output_analized,"[%7d],",param->TICK );
	sprintf( out, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\0",
					param->PWM_A1       ,
					param->PWM_A2       ,
					param->PWM_B1       ,
					param->PWM_B2       ,
					param->PWM_B3       ,
					param->PWM_DISCHARGE,
					param->PWM_UDIM21   ,
					param->PWM_UDIM22   ,
					param->PWM_C1       ,
					param->PWM_C2       ,
					param->PWM_C3       ,
					param->PWM_C4       ,
					param->PWM_C5       ,
					param->PWM_C6       ,
					param->PWM_C7       ,
					param->PWM_C8       ,
					param->PWM_C9       ,
					param->PWM_C10      ,
					param->PWM_C11      ,
					param->PWM_C12      ,
					param->PWM_D1       ,
					param->PWM_D2       ,
					param->PWM_D3       ,
					param->PWM_D4       ,
					param->PWM_D5       ,
					param->PWM_D6       ,
					param->PWM_D7       ,
					param->PWM_D8       ,
					param->BOOST_VOL    ,
					param->A_CUR        ,
					param->B_CUR        ,
					param->C_CUR        ,
					param->D_CUR        ,
					param->DAC_LCD      ,
					param->DAC_LED1     ,
					param->DAC_LED2     ,
					param->DAC_LED3     ,
					param->VBU          ,
					param->IG1          ,
					param->TURNS        ,
					param->HLBKUP       );
		fprintf(sys_t.output_analized,"%s",data);

	//-----------------------------------------
}


void FileOpenInt()
{
	// ファイル出力設定
	char org_fname[256]     ={0};
	char analized_fname[256]={0};
	unsigned char out[300]  ={0};
	
	sprintf( org_fname     , "%s_org.csv", sys_t.label );
	sprintf( analized_fname, "%s_ana.csv", sys_t.label );

	sys_t.output_org = fopen(org_fname, "w");  // ファイルを書き込み用にオープン(開く)
	if (sys_t.output_org == NULL) {            // オープンに失敗した場合
		printf("cannot open\n");         // エラーメッセージを出して
		return;                          // 異常終了
	}

	sys_t.output_analized = fopen(analized_fname, "w");  // ファイルを書き込み用にオープン(開く)
	if (sys_t.output_analized == NULL) {          // オープンに失敗した場合
		printf("cannot open\n");            // エラーメッセージを出して
		fclose(sys_t.output_org);
		sys_t.output_org = NULL;
		return;                            // 異常終了
	}
	sprintf( out, "[Tick(ms)],Discharge,A1,A2,B1,B2,B3,UDIM21,UDIM22,-,-,-,-,-,-,SPI,UART,LIN,CAN,lcm_Dec,led_Dec1,led_Dec2,led_Dec3,-,SW_Status\n");
	fprintf(sys_t.output_org,"%s",out);
	

	fprintf(sys_t.output_analized,"BoostVol=(mV)\n",out);
	fprintf(sys_t.output_analized,"x_CUR=(mA)\n",out);
	fprintf(sys_t.output_analized,"xx_PWM=(0.01%/LSB)\n",out);
	fprintf(sys_t.output_analized,"DAC_xxx=(℃)\n",out);
	sprintf( out, "[Tick(ms)],A1_PWM,A2_PWM,B1_PWM,B2_PWM,B3_PWM,DISCHARGE_PWM,UDIM21_PWM,UDIM22_PWM,C1_PWM,C2_PWM,C3_PWM,C4_PWM,C5_PWM,C6_PWM,C7_PWM,C8_PWM,C9_PWM,C10_PWM,C11_PWM,C12_PWM,D1_PWM,D2_PWM,D3_PWM,D4_PWM,D5_PWM,D6_PWM,D7_PWM,D8_PWM,BoostVol,A_CUR,B_CUR,C_CUR,D_CUR,DAC_LCD,DAC_LED1,DAC_LED2,DAC_LED3,VBU,IG1,TURN_Sync,HLBkup\n");
	fprintf(sys_t.output_analized,"%s",out);
}

void FileCloseInt()
{
	if( NULL != sys_t.output_org ){fclose(sys_t.output_org     );}
	sys_t.output_org = NULL;
	
	if( NULL != sys_t.output_analized){fclose(sys_t.output_analized);}
	sys_t.output_analized = NULL;
}

//---------------------------------------------------
// COMポートからのデータ受信監視スレッド
//---------------------------------------------------
DWORD WINAPI execute_thread(LPVOID param)
{
	unsigned char buf[1024]; 
	int len;

	while( sys_t.thread_active )
	{
		if( TRUE == sys_t.file_open )
		{
			if( (NULL == sys_t.output_org) || (NULL==sys_t.output_analized) )
			{
				FileOpenInt();
			}
		}

		memset(buf, 0, sizeof(buf));
		len = serial_recv_block(sys_t.obj,buf,sizeof(buf));
		if (len){
			Analize ( buf, len, &PARAM );
			
			TryEnterCriticalSection(&sys_t.cs_data_get);
			sys_t.data_get = FALSE;
			LeaveCriticalSection(&sys_t.cs_data_get);
			
			WriteLog( buf, len, &PARAM );
		}
		else
		{
			Sleep(1);
		}
		
		if( FALSE == sys_t.file_open )
		{
			if( (NULL != sys_t.output_org) || (NULL != sys_t.output_analized) )
			{
				FileCloseInt();
			}
		}
	}

	ExitThread(TRUE);
	return 0;
}

//---------------------------------------------------
// 外部公開用の名前付きパイプ作成及び監視スレッド
//---------------------------------------------------
DWORD WINAPI execute_serverthread(LPVOID param)
{
	unsigned char buf[1024]; 
	int len;

    HANDLE hPipe = INVALID_HANDLE_VALUE;
    hPipe = CreateNamedPipe("\\\\.\\pipe\\DebugApl", //lpName
                           PIPE_ACCESS_INBOUND,            // dwOpenMode
                           PIPE_TYPE_BYTE | PIPE_WAIT,     // dwPipeMode
                           1,                              // nMaxInstances
                           0,                              // nOutBufferSize
                           0,                              // nInBufferSize
                           100,                            // nDefaultTimeOut
                           NULL);                          // lpSecurityAttributes

    if (hPipe == INVALID_HANDLE_VALUE) {
        fprintf(log_file, "Couldn't create NamedPipe.\n");
        return 1;
    }

    //fprintf(log_file, "名前付きパイプ作成.\n");


	while(1)
	{
	    if (!ConnectNamedPipe(hPipe, NULL)) {
	        fprintf(log_file, "Couldn't connect to NamedPipe.\n");
	        CloseHandle(hPipe);
	        return 1;
	    }

	    //fprintf(log_file, "クライアント接続.\n");

		while( 1 )
		{
	        char szBuff[256];
	        DWORD dwBytesRead;
	        if (!ReadFile(hPipe, szBuff, sizeof(szBuff), &dwBytesRead, NULL)) {
	                //fprintf(log_file, "Couldn't read NamedPipe.\n");
	                break;
	        }
	        szBuff[dwBytesRead] = '\0';
			serial_send(sys_t.obj,szBuff,dwBytesRead);
	        fprintf(log_file, "[%s]cmd=%s", __func__, szBuff);
	        //Sleep(1);
		}
		FlushFileBuffers(hPipe);
		DisconnectNamedPipe(hPipe);
	}

    fprintf(log_file, "終了.\n");
    CloseHandle(hPipe);

	ExitThread(TRUE);
	return 0;
}

//---------------------------------------------------
// 外部公開関数
//---------------------------------------------------
DLLAPI void OpenSerial(char* com_dbg, char* com_plusb)
{
	//------- 動作ログ用 -------
	char log_fname[256]     ="LOG_FILE.txt";

	if( log_file == NULL ) { log_file = fopen(log_fname, "w"); }
	if( log_file != NULL ) { fprintf( log_file, "[%s] COM=%s\n", __func__, com_dbg ); }
	if( log_file != NULL ) { fprintf( log_file, "[%s] COM=%s\n", __func__, com_plusb ); }
	//------- 動作ログ用 -------

	//----------------------------------------
	// スレッド処理実行開始
	sys_t.thread_server_handle = CreateThread(NULL,0,execute_serverthread,NULL,0,&sys_t.thread_server_id);

	//----------------------------------------
	// デバッグボード, +B用安定化電源とのシリアル通信路開設
	sys_t.obj = serial_create(com_dbg,921600,com_plusb,9600);
	if ( sys_t.obj == NULL ) {
		if( log_file != NULL ) { fprintf(log_file,"[%s][err]オブジェクト生成に失敗\n", __func__);}
		return;
	}
	
	InitializeCriticalSection(&sys_t.cs_data_get);

	// 基板側の起動
	Sleep(100);
	serial_send(sys_t.obj,"bin on.",sizeof("bin on."));
	if( log_file != NULL ) { fprintf( log_file, "[%s] serial_send=%s\n", __func__, "bin on\\n" ); }
	// スレッド処理実行開始
	sys_t.thread_active = TRUE;
	sys_t.thread_handle = CreateThread(NULL,0,execute_thread,NULL,0,&sys_t.thread_id);
}

DLLAPI void CloseSerial()
{
	//------- 動作ログ用 -------
	if( log_file  != NULL ) { fprintf( log_file, "[%s]\n", __func__ ); }
	if( sys_t.obj == NULL ) 
	{
		if( log_file  != NULL ) { fprintf( log_file, "[%s][err] シリアルポートがOPENしてません。\n", __func__); return; }
	}
	//------- 動作ログ用 -------

	DWORD thread_state;

	// スレッド処理実行終了
	sys_t.file_open = FALSE;
	Sleep(100);
	
	if( TRUE == sys_t.thread_active )
	{
		sys_t.thread_active = FALSE;
		do {
			Sleep(1);
			GetExitCodeThread(sys_t.thread_handle,&thread_state);
		} while (thread_state == STILL_ACTIVE);

		DeleteCriticalSection(&sys_t.cs_data_get);
		sys_t.data_get = FALSE;
	}

	// シリアル処理の終了
	if(sys_t.obj != NULL)
	{
		serial_delete(sys_t.obj);
		sys_t.obj = NULL;
	}
}

DLLAPI void WriteCmd( char* cmd )
{
	//------- 動作ログ用 -------
	if( log_file  != NULL ) { fprintf( log_file, "[%s] cmd=%s\n", __func__, cmd);                      }
	if( sys_t.obj == NULL ) 
	{
		if( log_file != NULL ) { fprintf( log_file, "[%s][err] シリアルポートがOPENしてません。\n", __func__); return; }
	}
	//------- 動作ログ用 -------

	serial_send(sys_t.obj,cmd,strlen(cmd));
	//printf("%d: %s",strlen(cmd), cmd);
}

DLLAPI void FileOpen( char *plabel )
{
	//------- 動作ログ用 -------
	if( log_file  != NULL ) { fprintf( log_file, "[%s] label=%s\n", __func__, plabel); }
	//------- 動作ログ用 -------
	memset(sys_t.label, 0, FILE_LABEL_SIZE);
	sprintf(sys_t.label, "%s", plabel);
	sys_t.file_open = TRUE;
}

DLLAPI void FileClose()
{
	//------- 動作ログ用 -------
	if( log_file != NULL ) { fprintf( log_file, "[%s]\n", __func__); }
	//------- 動作ログ用 -------
	sys_t.file_open = FALSE;
}

DLLAPI void GetParam( struct Parameter *param )
{
	TryEnterCriticalSection(&sys_t.cs_data_get);
	sys_t.data_get = TRUE;
	LeaveCriticalSection(&sys_t.cs_data_get);

	while( sys_t.data_get )
	{
		Sleep(1);
	}
	
	if( param != NULL )
	{
		*param = PARAM;
	}
}

DLLAPI void GetMiconVer( char* ver )
{
	for( int i=0 ; i<VER_SIZE ; i++ )
	{
		ver[i] = (char)MICON_VERSION[i];
	}
}

DLLAPI void GetVer( char *ver )
{
	const unsigned char version[] = VERSION;
	
	for( int i=0 ; i<VER_SIZE ; i++ )
	{
		ver[i] = version[i];
	}
}

//---------------------------------------------------
// main関数
//---------------------------------------------------

#ifndef BUILD_DLL
int main (int argc, char *argv[])
{
	unsigned char buf[1024]; 
	int len;

	sys_t.obj = serial_create(argv[1],921600,argv[2],9600);
	if ( sys_t.obj == NULL ) {
		fprintf(stderr,"オブジェクト生成に失敗");
		return EXIT_FAILURE;
	}

	sprintf(sys_t.label, "%s", "data");

	FileOpenInt();

	Sleep(100);
	serial_send(sys_t.obj,"vbu on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"ig1 on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"bin on\n",sizeof("bin on\n"));

	while (1) {
		memset(buf, 0, sizeof(buf));
		memset(&PARAM, 0, sizeof(struct Parameter));
		len = serial_recv_block(sys_t.obj,buf,sizeof(buf));
		if (len){
			//printf("[%5d]\n",len); 
			//fwrite(buf,len,1,stdout);
			Analize ( buf, len, &PARAM );
			WriteLog( buf, len, &PARAM );
			unsigned int *tick = (unsigned int*)(buf+8);
			//fprintf(output_analized,"[%7d]\n",*tick);
			fprintf(stdout,"[%7d]\n",*tick);
			//fwrite(buf,len,1,output_org);
		}
		else
		{
			Sleep(4);
		}
		if ( kbhit() )  break;
	}

	Sleep(100);
	serial_send(sys_t.obj,"bin off\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"ig1 off\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"vbu off\n",sizeof("bin on\n"));

	FileCloseInt();
	serial_delete(sys_t.obj);
	return EXIT_SUCCESS;
}

#endif
