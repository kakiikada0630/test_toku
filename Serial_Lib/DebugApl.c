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
} sysctrl_t;

static sysctrl_t sys_t={0};

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
	
	//SPI�R�}���h�T�C�Y�m�F
	for( cmd_size=0 ; cmd_size<MAX_CMD_SIZE; cmd_size++ )
	{
		// �A�h���X���܂߁A�R�}���h��0�ƂȂ邱�Ƃ͂Ȃ����߁A
		// 0�ƂȂ�܂ł͗L���R�}���h���l�܂��Ă�����̂Ɖ��߂���B
		if(*(buf16+cmd_size) == 0 ){ break; }
	}

	if( ( 0 != cmd_size%2 ) || ( cmd_size < BST_CMD_SIZE ) )
	{
		return;
	}

	buck_cmd_size = (cmd_size - BST_CMD_SIZE)/2;

	//--------------------------------------------------
	//Boost�ւ̖��߂̌���
	//--------------------------------------------------
	for(int i=0 ; i<BST_CMD_SIZE ; i++)
	{
		unsigned short address;
		unsigned short data;

		address = (((*buf16) & 0x0078) >> 3);               	// SPI�̓r�b�O�G���f�B�A���ő��M���Ă��Ă���B
		data    = (((*buf16) & 0x0003) << 8) + ((*buf16) >> 8);	// �v���O������̓��g���G���f�B�A���Ƃ��Ĉ�������
																// �f�[�^���C������B
		if( address == 0x04 ) // Register Ox04
		{
			#define VAL_DIFF   103  // ( 125 -  22)
			#define VOL_DIFF 53300  // (64.8 -11.5)*1000  ���P�ʂ�mV�ɏC��
			short BoostVsetpoint = data & 0x007f;
			
			param->BOOST_VOL = VOL_DIFF*BoostVsetpoint/VAL_DIFF + 22;  // ���W�X�^�l(BoostVsetpoint)����d�����v�Z
			break;
		}
		buf16++;
	}

	//--------------------------------------------------
	//Buck(StringA/B)�ւ̖��߂̌���
	//--------------------------------------------------
	buf16 = (unsigned short *)(buf+BLOCK_SPI+BST_CMD_SIZE*2); // Buck�R�}���h�̐擪�ʒu�ֈړ�
	unsigned int Ch1adj = 0;
	unsigned int Ch2adj = 0;

	for(int i=0 ; i<buck_cmd_size ; i++)
	{
		unsigned short address;
		unsigned short data;

		address = (((*buf16) & 0x007E) >> 1);   // SPI�̓r�b�O�G���f�B�A���ő��M���Ă��Ă���B
		data    = ( (*buf16)           >> 8);	// �v���O������̓��g���G���f�B�A���Ƃ��Ĉ�������
												// �f�[�^���C������B
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
	#define V_DACFS  2480  // mV�ɕϊ��ς�
	#define R_CS     100   // m���ɕϊ��ς�(�����_����̂���)
	#define SAITEKIKA (V_DACFS*1000)/R_CS
	
	param->A_CUR = (Ch1adj * SAITEKIKA)/(1024 * 14 );
	param->B_CUR = (Ch2adj * SAITEKIKA)/(1024 * 14 );

	//--------------------------------------------------
	//Buck(StringC/D)�ւ̖��߂̌���
	//--------------------------------------------------
	buf16 = (unsigned short *)(buf+BLOCK_SPI+BST_CMD_SIZE*2+buck_cmd_size*2); // Buck�R�}���h�̐擪�ʒu�ֈړ�
	Ch1adj = 0;
	Ch2adj = 0;

	for(int i=0 ; i<buck_cmd_size ; i++)
	{
		unsigned short address;
		unsigned short data;

		address = (((*buf16) & 0x007E) >> 1);   // SPI�̓r�b�O�G���f�B�A���ő��M���Ă��Ă���B
		data    = ( (*buf16)           >> 8);	// �v���O������̓��g���G���f�B�A���Ƃ��Ĉ�������
												// �f�[�^���C������B
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
	

	while(1)
	{
		unsigned int cmd;
		unsigned int data_size;
		unsigned int dev_add;
		unsigned int reg_add;
		unsigned int data;

		cmd = *buf8;
 		
		// Write����
		if     ( cmd == 0x87 ){ data_size = 1; }
		else if( cmd == 0x99 ){ data_size = 2; }
		else if( cmd == 0x1E ){ data_size = 3; }
		else if( cmd == 0xAA ){ data_size = 4; }
		else if( cmd == 0x2D ){ data_size =12; }
		else if( cmd == 0x33 ){ data_size =16; }
		else if( cmd == 0xB4 ){ data_size =32; }

		// Read����
		else if( cmd == 0x4B ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0xCC ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0xD2 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0x55 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0xE1 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0x66 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else if( cmd == 0x78 ){ buf8+= 5; continue; }   // REG_ADD= 1Byte, INIT=1Byte, DevID=1Byte, CRC=2Byte
		else
		{
			break;
		}
		
		// �f�o�C�XID�̎擾
		buf8 = buf8+1;
		dev_add = *buf8;

		// ���W�X�^�A�h���X�擾
		buf8 = buf8+1;
		reg_add = *buf8;
		buf8 = buf8+1;

		for(int i=0 ; i<data_size ; i++)
		{
			data = *buf8;

			// LMM�o��PWM��WIDTH�l�擾
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
		}
		
		// CRC��������A���̃R�}���h��
		buf8 = buf8+2;
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

void Analize( unsigned char* buf, int size, struct Parameter *param )
{
	unsigned int* tick_pnt = (unsigned int*)(buf+BLOCK_TICK);
	param->TICK = *tick_pnt;

	Analize_PWM (buf, size, param);
	Analize_SPI (buf, size, param);
	Analize_UART(buf, size, param);
	Analize_DAC (buf, size, param);
}

void WriteLog( unsigned char* buf, int size, struct Parameter *param )
{
	//-----------------------------------------
	//���O�����̂܂܏����o��
	//-----------------------------------------
	unsigned char out[2000]={0};
	unsigned char *buf8 = buf+BLOCK_PWM;

	if( (sys_t.output_org == NULL) || (sys_t.output_analized == NULL) )
	{
		return;
	}

	fprintf(sys_t.output_org,"[%7d]",param->TICK );

	for( int i=0 ; i<BLOCK_SIZE-BLOCK_PWM ; i++ )
	{
		sprintf( out+i*2, "%02x", *(buf8+i) );
	}

	fprintf(sys_t.output_org,"%s\n",out );

	//-----------------------------------------
	//��͌�̐��l�������o��
	//-----------------------------------------
	fprintf(sys_t.output_analized,"[%7d],",param->TICK );
	sprintf( out, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
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
					param->DAC_LED3     );
		fprintf(sys_t.output_analized,"%s",out);

	//-----------------------------------------
}


void FileOpenInt()
{
	// �t�@�C���o�͐ݒ�
	char org_fname[256]     ={0};
	char analized_fname[256]={0};
	
	sprintf( org_fname     , "%s_org.csv", sys_t.label );
	sprintf( analized_fname, "%s_ana.csv", sys_t.label );

	sys_t.output_org = fopen(org_fname, "w");  // �t�@�C�����������ݗp�ɃI�[�v��(�J��)
	if (sys_t.output_org == NULL) {            // �I�[�v���Ɏ��s�����ꍇ
		printf("cannot open\n");         // �G���[���b�Z�[�W���o����
		return;                          // �ُ�I��
	}

	sys_t.output_analized = fopen(analized_fname, "w");  // �t�@�C�����������ݗp�ɃI�[�v��(�J��)
	if (sys_t.output_analized == NULL) {          // �I�[�v���Ɏ��s�����ꍇ
		printf("cannot open\n");            // �G���[���b�Z�[�W���o����
		fclose(sys_t.output_org);
		sys_t.output_org = NULL;
		return;                            // �ُ�I��
	}
	
	unsigned char out[300]={0};
	fprintf(sys_t.output_analized,"BoostVol=(mV)\n",out);
	fprintf(sys_t.output_analized,"x_CUR=(mA)\n",out);
	fprintf(sys_t.output_analized,"xx_PWM=(0.01%/LSB)\n",out);
	fprintf(sys_t.output_analized,"DAC_xxx=(��)\n",out);
	sprintf( out, "[Tick(us)],A1_PWM,A2_PWM,B1_PWM,B2_PWM,B3_PWM,DISCHARGE_PWM,UDIM21_PWM,UDIM22_PWM,C1_PWM,C2_PWM,C3_PWM,C4_PWM,C5_PWM,C6_PWM,C7_PWM,C8_PWM,C9_PWM,C10_PWM,C11_PWM,C12_PWM,D1_PWM,D2_PWM,D3_PWM,D4_PWM,D5_PWM,D6_PWM,D7_PWM,D8_PWM,BoostVol,A_CUR,B_CUR,C_CUR,D_CUR,DAC_LCD,DAC_LED1,DAC_LED2,DAC_LED3\n");	fprintf(sys_t.output_analized,"%s",out);
}

void FileCloseInt()
{
	fclose(sys_t.output_org     );
	sys_t.output_org = NULL;
	
	fclose(sys_t.output_analized);
	sys_t.output_analized = NULL;
}


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
			Sleep(4);
		}
		
		if( FALSE == sys_t.file_open )
		{
			if( (NULL != sys_t.output_org) || (NULL != sys_t.output_analized) )
			{
				FileOpenInt();
			}
		}
	}

	ExitThread(TRUE);
	return 0;
}



//---------------------------------------------------
// �O�����J�֐�
//---------------------------------------------------
DLLAPI void OpenSerial(char* com_name)
{
	sys_t.obj = serial_create(com_name,921600);
	if ( sys_t.obj == NULL ) {
		fprintf(stderr,"�I�u�W�F�N�g�����Ɏ��s");
		return;
	}
	
	InitializeCriticalSection(&sys_t.cs_data_get);

	// ����̋N��
	Sleep(100);
	serial_send(sys_t.obj,"vbu on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"ig1 on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"bin on\n",sizeof("bin on\n"));

	// �X���b�h�������s�J�n
	sys_t.thread_active = TRUE;
	sys_t.thread_handle = CreateThread(NULL,0,execute_thread,NULL,0,&sys_t.thread_id);
}

DLLAPI void CloseSerial()
{
	DWORD thread_state;

	// ����̃V���b�g�_�E��
	Sleep(100);
	serial_send(sys_t.obj,"bin on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"ig1 on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"vbu on\n",sizeof("bin on\n"));

	// �X���b�h�������s�I��
	sys_t.file_open = FALSE;
	Sleep(100);
	sys_t.thread_active = FALSE;
	do {
		Sleep(1);
		GetExitCodeThread(sys_t.thread_handle,&thread_state);
	} while (thread_state == STILL_ACTIVE);

	DeleteCriticalSection(&sys_t.cs_data_get);
	sys_t.data_get = FALSE;

	// �V���A�������̏I��
	serial_delete(sys_t.obj);
}

DLLAPI void WriteCmd( char* cmd )
{
	serial_send(sys_t.obj,cmd,strlen(cmd));
	//printf("%d: %s",strlen(cmd), cmd);
}

DLLAPI void FileOpen( char *plabel )
{
	memset(sys_t.label, 0, FILE_LABEL_SIZE);
	sprintf(sys_t.label, "%s", plabel);
	sys_t.file_open = TRUE;
}

DLLAPI void FileClose()
{
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


//---------------------------------------------------
// main�֐�
//---------------------------------------------------

#ifndef BUILD_DLL
int main (int argc, char *argv[])
{
	unsigned char buf[1024]; 
	int len;

	sys_t.obj = serial_create(argv[1],921600);
	if ( sys_t.obj == NULL ) {
		fprintf(stderr,"�I�u�W�F�N�g�����Ɏ��s");
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
	serial_send(sys_t.obj,"bin on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"ig1 on\n",sizeof("bin on\n"));
	Sleep(100);
	serial_send(sys_t.obj,"vbu on\n",sizeof("bin on\n"));

	FileCloseInt();
	serial_delete(sys_t.obj);
	return EXIT_SUCCESS;
}

#endif