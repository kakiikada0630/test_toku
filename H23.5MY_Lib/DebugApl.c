#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Serial.h"
#include "fifo.h"

#include "DebugApl.h"


struct Parameter PARAM;
unsigned short crc16_table[256];

#define TITLE_SIZE   100
#define CH_NUM        10
#define CMD_NUM       10
#define CMD_MAX_SIZE 100

struct Cmd_str
{
	char title[TITLE_SIZE];
	char cmd[CMD_NUM][CMD_MAX_SIZE];
	int  size[CMD_NUM];
	int  cmd_num;
	int  cur_cmd_index;
} send_cmd[CH_NUM];


typedef struct _TAG_SYSCTRL
{
#define FILE_LABEL_SIZE 100

	BOOL file_open;
	BOOL thread_active;
	BOOL data_get;

	CRITICAL_SECTION cs_data_get;

	char label[FILE_LABEL_SIZE];
	FILE *output_org;
	serial_t obj;

	HANDLE thread_handle;
	DWORD thread_id;
} sysctrl_t;

static sysctrl_t sys_t={0};
FILE *log_file;

static int CMD_CH = 0;

//************************************************************
//************************************************************
//**                      �����֐�                          **
//************************************************************
//************************************************************

//--------------------------------------------
// �X�y�[�X�y�щ��s�̍폜
//--------------------------------------------
void trim_space( char* inout, int size )
{
	char buf[200]={0};
	
	int i=0, j=0;
	
	while( (*(inout+i) != '\0') && (*(inout+i) != '#') && (*(inout+i) != '\n') )
	{
		if( ( *(inout+i) != ' ' ) && ( *(inout+i) != '\t' ) )
		{
			*(buf+j) = *(inout+i);
			j++;
		}
	
		i++;
	}
	
	memset( inout, 0, size );
	memcpy( inout, buf, strnlen( buf, sizeof(buf) ) );
}


//--------------------------------------------
// �r�b�g���](LSB��MSB�̔��])
//--------------------------------------------
unsigned short swapbit(unsigned short x, int b) {
  unsigned r = 0;
  while (b--) {
    r <<= 1;
    r |= (x & 1);
    x >>= 1;
  }
  return r;
}

//--------------------------------------------
// CRC16-IBM�v�Z�e�[�u���̐���
//--------------------------------------------
void make_crc16_table(void) {
    for (unsigned short i = 0; i < 256; i++) {
        unsigned short c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xA001 ^ (c >> 1)) : (c >> 1);
        }
        crc16_table[i] = c;
    }
}

//--------------------------------------------
// CRC16-IBM�̌v�Z
//--------------------------------------------
unsigned short crc16(unsigned char *buf, size_t len) {
    unsigned short c = 0;
    for (size_t i = 0; i < len; i++) {
        c = crc16_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    }
    return c;
}

//--------------------------------------------
// ����M�f�[�^�̏����o��
//--------------------------------------------
void WriteBkup_CMD( unsigned char* buf, int size )
{
	//-----------------------------------------
	//���O�����̂܂܏����o��
	//-----------------------------------------
	unsigned char out[2000]={0};
	unsigned char *buf8 = buf+BLOCK_PWM;
	SYSTEMTIME stTime;

	GetLocalTime(&stTime);

	if( sys_t.output_org == NULL )
	{
		return;
	}

	fprintf(sys_t.output_org,"[%7d]",stTime.wMilliseconds+stTime.wSecond*1000 );

	for( int i=0 ; i<size ; i++ )
	{
		sprintf( out+(i*3), "%02x,", *(buf+i) );
	}

	fprintf(sys_t.output_org,"%s",out );

	//-----------------------------------------
}

void WriteBkup_Recv( unsigned char* buf, int size )
{
	//-----------------------------------------
	//���O�����̂܂܏����o��
	//-----------------------------------------
	unsigned char out[2000]={0};
	unsigned char *buf8 = buf+BLOCK_PWM;

	if( sys_t.output_org == NULL )
	{
		return;
	}

	for( int i=0 ; i<size ; i++ )
	{
		sprintf( out+(i*3), "%02x,", *(buf+i) );
	}

	fprintf(sys_t.output_org,"%s\n",out );

	//-----------------------------------------
}

//--------------------------------------------
// ����M�f�[�^�̏����o���t�@�C���I�[�v��
//--------------------------------------------
void FileOpenInt()
{
	// �t�@�C���o�͐ݒ�
	char org_fname[256]     ={0};
	
	sprintf( org_fname     , "%s_org.csv", sys_t.label );

	sys_t.output_org = fopen(org_fname, "w");  // �t�@�C�����������ݗp�ɃI�[�v��(�J��)
	if (sys_t.output_org == NULL) {            // �I�[�v���Ɏ��s�����ꍇ
		printf("cannot open\n");         // �G���[���b�Z�[�W���o����
		return;                          // �ُ�I��
	}
}

//--------------------------------------------
// ����M�f�[�^�̏����o���t�@�C���N���[�Y
//--------------------------------------------
void FileCloseInt()
{
	if( NULL != sys_t.output_org ){fclose(sys_t.output_org     );}
	sys_t.output_org = NULL;
}

//--------------------------------------------
// �f�[�^�̑���M���{�p�T�u�X���b�h���C������
//--------------------------------------------
DWORD WINAPI execute_thread(LPVOID param)
{
	unsigned char buf[1024]; 
	int len;
	unsigned int prev_time;
	unsigned int cur_time;
	SYSTEMTIME bufTime;

	GetLocalTime(&bufTime);
	prev_time = bufTime.wHour*3600000+bufTime.wMinute*60000+bufTime.wSecond*1000+bufTime.wMilliseconds;

	while( sys_t.thread_active )
	{
		//�t�@�C���I�[�v���󋵊m�F
		if( TRUE == sys_t.file_open )
		{
			if( NULL == sys_t.output_org )
			{
				FileOpenInt();
			}
		}

		//��M�f�[�^�擾����у��O�����o��
		memset(buf, 0, sizeof(buf));
		len = serial_recv(sys_t.obj,buf,sizeof(buf));
		WriteBkup_Recv( buf, len );

		//�R�}���h���M�y�у��O�����o��
		serial_send(sys_t.obj,send_cmd[CMD_CH].cmd[send_cmd[CMD_CH].cur_cmd_index], send_cmd[CMD_CH].size[send_cmd[CMD_CH].cur_cmd_index]);
		WriteBkup_CMD( send_cmd[CMD_CH].cmd[send_cmd[CMD_CH].cur_cmd_index], send_cmd[CMD_CH].size[send_cmd[CMD_CH].cur_cmd_index] );
		send_cmd[CMD_CH].cur_cmd_index = (send_cmd[CMD_CH].cur_cmd_index + 1)%send_cmd[CMD_CH].cmd_num;
		
		// �N���[�Y�󋵊m�F
		if( FALSE == sys_t.file_open )
		{
			if( NULL != sys_t.output_org )
			{
				FileCloseInt();
			}
		}
		
		while(1)
		{
			GetLocalTime(&bufTime);
			cur_time = bufTime.wHour*3600000+bufTime.wMinute*60000+bufTime.wSecond*1000+bufTime.wMilliseconds;
			
			if( 5 <= (cur_time-prev_time) )
			{
				//printf("%d\n",cur_time);
				prev_time = cur_time;
				break;
			}
			
			Sleep(1);
		}

	}

	ExitThread(TRUE);
	return 0;
}



//************************************************************
//************************************************************
//**                  �O�����J�֐�                          **
//************************************************************
//************************************************************

//--------------------------------------------
// �V���A���|�[�g�I�[�v��
//--------------------------------------------
DLLAPI void OpenSerial(char* com_name, int baud)
{
	//------- ���샍�O�p -------
	char log_fname[256]     ="LOG_FILE.txt";

	if( log_file == NULL ) { log_file = fopen(log_fname, "w"); }
	if( log_file != NULL ) { fprintf( log_file, "[%s] COM=%s\n", __func__, com_name ); }
	//------- ���샍�O�p -------

	sys_t.obj = serial_create(com_name,baud);
	if ( sys_t.obj == NULL ) {
		if( log_file != NULL ) { fprintf(log_file,"[%s][err]�I�u�W�F�N�g�����Ɏ��s\n", __func__);}
		return;
	}
	
	InitializeCriticalSection(&sys_t.cs_data_get);

	// ����̋N��
	Sleep(100);
	sys_t.thread_active = TRUE;
	sys_t.thread_handle = CreateThread(NULL,0,execute_thread,NULL,0,&sys_t.thread_id);
}

//--------------------------------------------
// �V���A���|�[�g�N���[�Y
//--------------------------------------------
DLLAPI void CloseSerial()
{
	//------- ���샍�O�p -------
	if( log_file  != NULL ) { fprintf( log_file, "[%s]\n", __func__ ); }
	if( sys_t.obj == NULL ) 
	{
		if( log_file  != NULL ) { fprintf( log_file, "[%s][err] �V���A���|�[�g��OPEN���Ă܂���B\n", __func__); return; }
	}
	//------- ���샍�O�p -------

	DWORD thread_state;

	// �X���b�h�������s�I��
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

	// �V���A�������̏I��
	if(sys_t.obj != NULL)
	{
		serial_delete(sys_t.obj);
		sys_t.obj = NULL;
	}
}


//--------------------------------------------
// �t�@�C���ɂ��R�}���h�o�^
//--------------------------------------------
DLLAPI void InportCmdFile(char* cmd_path, int ch)
{
	FILE*   cmd_file;
	struct  Cmd_str *bufcmd;
	char    line[200];
	char    buf [200];
	char    buf2[200];
	char*   wait="wait";

	int cmd_index  = 0;
	int write_index= 0;
	
	cmd_file = fopen(cmd_path, "r");  // �t�@�C����ǂݍ��ݗp�ɃI�[�v��(�J��)
	if (cmd_file == NULL) {            // �I�[�v���Ɏ��s�����ꍇ
		printf("cannot open\n");         // �G���[���b�Z�[�W���o����
		return;                          // �ُ�I��
	}
	
	bufcmd = &(send_cmd[ch]);
	
	bufcmd->cur_cmd_index  = 0;
	bufcmd->cmd_num        = 1;

	// CRC�e�[�u���̐���
	make_crc16_table();
	
	while(1)
	{
		int line_index = 0;
		int s_crc_index= 0;
		int crc;
		
		memset( line , 0, sizeof(line) );

		// 1���C���Ăяo���AEOF�������ꍇ�A�������I������
		// �Ȃ��A"#"�ȍ~�̓R�����g�s�ł���s�v�̂��ߓǂݎ̂Ă�B
		if( 0 >= fgets( line, sizeof(line), cmd_file ) )
		{
			bufcmd->size[cmd_index]= write_index;
			bufcmd->cmd_num        = cmd_index+1;
			break;
		}

		// �ǂ݂��������C������X�y�[�X����������
		trim_space( line, sizeof(line) );
		
		// ���C�����e��wait�ł���΁A�R�}���h�̓o�^�s���X�V����
		if( 0 == strncmp(wait,line, 4) )
		{
			bufcmd->size[cmd_index]= write_index;
			cmd_index++;
			write_index=0;
			continue;
		}
		
		//���C�����̕����T�C�Y��0�̏ꍇ�A�s�v�s�Ƃ��Ď��̍s�Ɉړ�
		if( 0 == strnlen(line,sizeof(line)) )
		{
			continue;
		}
		
		// �擪��Sync�t�B�[���h��o�^
		bufcmd->cmd[cmd_index][write_index]= 0x55;
		write_index++;
		
		s_crc_index = write_index;
		
		// 1���C������͂���
		while(1)
		{
			memset( buf , 0, sizeof(buf) );

			if( 0 > sscanf( (line+line_index), "%[^,],%s",buf,buf2) )
			{
				break;
			}

			// ���̃f�[�^�擾�J�n�����w�肷�邽�߁A�C���f�b�N�X��ǉ�����
			// �Ȃ��A",(�J���})"�������񂩂珜�O����Ă��邽�߁A������+1�����Z����B
			line_index += strnlen( buf, sizeof(buf) ) + 1;
			sscanf( buf, "%x", &bufcmd->cmd[cmd_index][write_index]);
			
			write_index++;
		}
		
		// CRC���v�Z����
		crc = crc16(  &bufcmd->cmd[cmd_index][s_crc_index]
					, write_index -s_crc_index 
					);
		
		// CRC�𔽓]��LSB����i�[����
		crc = swapbit( crc, 16);
		bufcmd->cmd[cmd_index][write_index] = crc & 0x00FF;
		write_index++;
		bufcmd->cmd[cmd_index][write_index] = (crc >> 8 ) & 0x00FF;
		write_index++;
	}
	
	if( NULL != cmd_file ){fclose(cmd_file     );}
	cmd_file = NULL;
}

//--------------------------------------------
// ���M�ΏۃR�}���h�`���l���̐؂�ւ�
//--------------------------------------------
DLLAPI void SetCh( int ch )
{
	if( ch < CH_NUM )
	{
		CMD_CH = ch;
		send_cmd[CMD_CH].cur_cmd_index = 0;
	}
}

//--------------------------------------------
// �o�b�N�A�b�v�t�@�C���̖��̓o�^�y�уI�[�v��
//--------------------------------------------
DLLAPI void BkupFileOpen( char *plabel )
{
	//------- ���샍�O�p -------
	if( log_file  != NULL ) { fprintf( log_file, "[%s] label=%s\n", __func__, plabel); }
	//------- ���샍�O�p -------
	memset(sys_t.label, 0, FILE_LABEL_SIZE);
	sprintf(sys_t.label, "%s", plabel);
	sys_t.file_open = TRUE;
}

//--------------------------------------------
// �o�b�N�A�b�v�t�@�C���̃N���[�Y
//--------------------------------------------
DLLAPI void BkupFileClose()
{
	//------- ���샍�O�p -------
	if( log_file != NULL ) { fprintf( log_file, "[%s]\n", __func__); }
	//------- ���샍�O�p -------
	sys_t.file_open = FALSE;
}

//--------------------------------------------
// �p�����[�^�擾
//--------------------------------------------
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


#ifndef BUILD_DLL

void main(void)
{
	int i, j, k;
	int read;
	
	BkupFileOpen("Python");
	
	InportCmdFile("cmd0.csv",0);
	InportCmdFile("cmd1.csv",1);
	InportCmdFile("cmd2.csv",2);

	for(k=0 ; k<3; k++)
	{
		for(i=0 ; i< send_cmd[k].cmd_num ; i++)
		{
			printf("ch=%d,cmd=%d :", k, i);

			for( j=0 ; j< send_cmd[k].size[i] ; j++ )
			{
				printf("%2x,",(unsigned char)send_cmd[k].cmd[i][j]);
			}
			
			printf("\n");
		}
	}

	OpenSerial("\\\\.\\COM4", CBR_128000);

	while(1)
	{
		printf("switch ch?(0-3) :");
		scanf("%d",&read);

		SetCh(read);
	}
}

#endif

