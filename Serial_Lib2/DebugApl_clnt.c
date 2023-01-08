#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>

#include "DebugApl.h"

HANDLE hPipe = INVALID_HANDLE_VALUE;

//---------------------------------------------------
// 外部公開関数
//---------------------------------------------------
DLLAPI void OpenSerial(char* com_dbg, char* com_plusb)
{
}

DLLAPI void CloseSerial()
{
}

DLLAPI void WriteCmd( char* cmd )
{
	DWORD dwBytesWritten;

	//パイプ作成
	hPipe = CreateFile("\\\\.\\pipe\\DebugApl",
                          GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING, 
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
	
	if (hPipe == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Couldn't create NamedPipe.");
		return;
	}

	//クライアントにデータ送信
	WriteFile(hPipe, cmd, strlen(cmd), &dwBytesWritten, NULL);

	//パイプクローズ
	CloseHandle(hPipe);
}

DLLAPI void FileOpen( char *plabel )
{
}

DLLAPI void FileClose()
{
}

DLLAPI void GetParam( struct Parameter *param )
{
}


//---------------------------------------------------
// main関数
//---------------------------------------------------

#ifndef BUILD_DLL
int main (int argc, char *argv[])
{
	unsigned char buf[1024]; 
	int len;

	sys_t.obj = serial_create(argv[1],921600);
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
