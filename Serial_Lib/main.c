#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include "serial.h"
#include "fifo.h"

FILE *output_org;
FILE *output_analized;

static unsigned int TICK          = 0;  // % * 100
static unsigned int PWM_A1        = 0;  // % * 100
static unsigned int PWM_A2        = 0;  // % * 100
static unsigned int PWM_B1        = 0;  // % * 100
static unsigned int PWM_B2        = 0;  // % * 100
static unsigned int PWM_B3        = 0;  // % * 100
static unsigned int PWM_DISCHARGE = 0;  // % * 100
static unsigned int PWM_UDIM21    = 0;  // % * 100
static unsigned int PWM_UDIM22    = 0;  // % * 100
static unsigned int PWM_C1        = 0;  // % * 100
static unsigned int PWM_C2        = 0;  // % * 100
static unsigned int PWM_C3        = 0;  // % * 100
static unsigned int PWM_C4        = 0;  // % * 100
static unsigned int PWM_C5        = 0;  // % * 100
static unsigned int PWM_C6        = 0;  // % * 100
static unsigned int PWM_C7        = 0;  // % * 100
static unsigned int PWM_C8        = 0;  // % * 100
static unsigned int PWM_C9        = 0;  // % * 100
static unsigned int PWM_C10       = 0;  // % * 100
static unsigned int PWM_C11       = 0;  // % * 100
static unsigned int PWM_C12       = 0;  // % * 100
static unsigned int PWM_D1        = 0;  // % * 100
static unsigned int PWM_D2        = 0;  // % * 100
static unsigned int PWM_D3        = 0;  // % * 100
static unsigned int PWM_D4        = 0;  // % * 100
static unsigned int PWM_D5        = 0;  // % * 100
static unsigned int PWM_D6        = 0;  // % * 100
static unsigned int PWM_D7        = 0;  // % * 100
static unsigned int PWM_D8        = 0;  // % * 100
static unsigned int BOOST_VOL     = 0;  // mV
static unsigned int A_CUR         = 0;  // mA
static unsigned int B_CUR         = 0;  // mA
static unsigned int C_CUR         = 0;  // mA
static unsigned int D_CUR         = 0;  // mA



void FileOpen( char *pname, int size )
{
	// ファイル出力設定
	char org_fname[256]     ={0};
	char analized_fname[256]={0};
	
	sprintf( org_fname     , "%s_org.csv", pname );
	sprintf( analized_fname, "%s_ana.csv", pname );

	output_org = fopen(org_fname, "w");  // ファイルを書き込み用にオープン(開く)
	if (output_org == NULL) {            // オープンに失敗した場合
		printf("cannot open\n");         // エラーメッセージを出して
		exit(1);                         // 異常終了
	}

	output_analized = fopen(analized_fname, "w");  // ファイルを書き込み用にオープン(開く)
	if (output_analized == NULL) {          // オープンに失敗した場合
		printf("cannot open\n");            // エラーメッセージを出して
		fclose(output_org);
		exit(1);                            // 異常終了
	}
	
	unsigned char out[300]={0};
	fprintf(output_analized,"BoostVol=(mV)\n",out);
	fprintf(output_analized,"x_CUR=(mA)\n",out);
	fprintf(output_analized,"xx_PWM=(0.01%LSB)\n",out);
	sprintf( out, "[Tick(us)]\tA1_PWM\tA2_PWM\tB1_PWM\tB2_PWM\tB3_PWM\tDISCHARGE_PWM\tUDIM21_PWM\tUDIM22_PWM\tC1_PWM\tC2_PWM\tC3_PWM\tC4_PWM\tC5_PWM\tC6_PWM\tC7_PWM\tC8_PWM\tC9_PWM\tC10_PWM\tC11_PWM\tC12_PWM\tD1_PWM\tD2_PWM\tD3_PWM\tD4_PWM\tD5_PWM\tD6_PWM\tD7_PWM\tD8_PWM\tBoostVol\tA_CUR\tB_CUR\tC_CUR\tD_CUR\n");	fprintf(output_analized,"%s",out);
}


void FileClose()
{
	fclose(output_org     );
	fclose(output_analized);
}

void Analize_PWM( unsigned char* buf, int size )
{
	unsigned short* buf16 = (unsigned short *)(buf+BLOCK_PWM);
	
	PWM_DISCHARGE = *(buf16+0);
	PWM_A1        = *(buf16+1);
	PWM_A2        = *(buf16+2);
	PWM_B1        = *(buf16+3);
	PWM_B2        = *(buf16+4);
	PWM_B3        = *(buf16+5);
	PWM_UDIM21    = *(buf16+6);
	PWM_UDIM22    = *(buf16+7);
}


void Analize_SPI( unsigned char* buf, int size )
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

	buck_cmd_size = (cmd_size - BST_CMD_SIZE)/2;

	//--------------------------------------------------
	//Boostへの命令の検索
	//--------------------------------------------------
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
			
			BOOST_VOL = VOL_DIFF*BoostVsetpoint/VAL_DIFF + 22;  // レジスタ値(BoostVsetpoint)から電圧を計算
			break;
		}
		buf16++;
	}

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
	
	A_CUR = (Ch1adj * SAITEKIKA)/(1024 * 14 );
	B_CUR = (Ch2adj * SAITEKIKA)/(1024 * 14 );

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

	C_CUR = (Ch1adj * SAITEKIKA)/(1024 * 14 );
	D_CUR = (Ch2adj * SAITEKIKA)/(1024 * 14 );
}

void Analize_UART( unsigned char* buf, int size )
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
			break;
		}
		
		// デバイスIDの取得
		buf8 = buf8+1;
		dev_add = *buf8;

		// レジスタアドレス取得
		buf8 = buf8+1;
		reg_add = *buf8;
		buf8 = buf8+1;

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
		}
		
		// CRCを回避し、次のコマンドへ
		buf8 = buf8+2;
	}
	
	PWM_C1 = StringC[ 0]*10000/1023;
	PWM_C2 = StringC[ 1]*10000/1023;
	PWM_C3 = StringC[ 2]*10000/1023;
	PWM_C4 = StringC[ 3]*10000/1023;
	PWM_C5 = StringC[ 4]*10000/1023;
	PWM_C6 = StringC[ 5]*10000/1023;
	PWM_C7 = StringC[ 6]*10000/1023;
	PWM_C8 = StringC[ 7]*10000/1023;
	PWM_C9 = StringC[ 8]*10000/1023;
	PWM_C10= StringC[ 9]*10000/1023;
	PWM_C11= StringC[10]*10000/1023;
	PWM_C12= StringC[11]*10000/1023;
	PWM_D1 = StringD[ 0]*10000/1023;
	PWM_D2 = StringD[ 1]*10000/1023;
	PWM_D3 = StringD[ 2]*10000/1023;
	PWM_D4 = StringD[ 3]*10000/1023;
	PWM_D5 = StringD[ 4]*10000/1023;
	PWM_D6 = StringD[ 5]*10000/1023;
	PWM_D7 = StringD[ 6]*10000/1023;
	PWM_D8 = StringD[ 7]*10000/1023;
}

void Analize( unsigned char* buf, int size )
{
	unsigned int* tick_pnt = (unsigned int*)(buf+BLOCK_TICK);
	TICK = *tick_pnt;

	Analize_PWM (buf, size);
	Analize_SPI (buf, size);
	Analize_UART(buf, size);
}

void WriteLog( unsigned char* buf, int size )
{
	//-----------------------------------------
	//ログをそのまま書き出し
	//-----------------------------------------
	unsigned char out[2000]={0};
	unsigned char *buf8 = buf+BLOCK_PWM;
	
	fprintf(output_org,"[%7d]",TICK );

	for( int i=0 ; i<BLOCK_SIZE-BLOCK_PWM ; i++ )
	{
		sprintf( out+i*2, "%02x", *(buf8+i) );
	}

	fprintf(output_org,"%s\n",out );

	//-----------------------------------------
	//解析後の数値を書き出し
	//-----------------------------------------
	fprintf(output_analized,"[%7d]\t",TICK );
	sprintf( out, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
					PWM_A1       ,
					PWM_A2       ,
					PWM_B1       ,
					PWM_B2       ,
					PWM_B3       ,
					PWM_DISCHARGE,
					PWM_UDIM21   ,
					PWM_UDIM22   ,
					PWM_C1       ,
					PWM_C2       ,
					PWM_C3       ,
					PWM_C4       ,
					PWM_C5       ,
					PWM_C6       ,
					PWM_C7       ,
					PWM_C8       ,
					PWM_C9       ,
					PWM_C10      ,
					PWM_C11      ,
					PWM_C12      ,
					PWM_D1       ,
					PWM_D2       ,
					PWM_D3       ,
					PWM_D4       ,
					PWM_D5       ,
					PWM_D6       ,
					PWM_D7       ,
					PWM_D8       ,
					BOOST_VOL    ,
					A_CUR        ,
					B_CUR        ,
					C_CUR        ,
					D_CUR        );
		fprintf(output_analized,"%s",out);

	//-----------------------------------------
}

int main (void)
{
  serial_t obj = serial_create("COM5",921600);
  unsigned char buf[1024]; 
  int len;

  if ( obj == NULL ) {
    fprintf(stderr,"オブジェクト生成に失敗");
    return EXIT_FAILURE;
  }

	FileOpen("data",sizeof("data"));

	sleep(1);
	serial_send(obj,"bin on\n",sizeof("bin on\n"));

  while (1) {
	memset(buf, 0, sizeof(buf));
    len = serial_recv_block(obj,buf,sizeof(buf));
    if (len){
		//printf("[%5d]\n",len); 
		//fwrite(buf,len,1,stdout);
		Analize ( buf, len );
		WriteLog( buf, len );
		unsigned int *tick = (unsigned int*)(buf+8);
		//fprintf(output_analized,"[%7d]\n",*tick);
		fprintf(stdout,"[%7d]\n",*tick);
		//fwrite(buf,len,1,output_org);
		
	}
	else
	{
		Sleep(4);
	}
    //if ( kbhit() )  break;
  }

  serial_delete(obj);

  return EXIT_SUCCESS;
}