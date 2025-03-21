#ifndef DEBUG_APL_H
#define DEBUG_APL_H

#ifdef BUILD_DLL
#define DLLAPI __declspec(dllexport)
#elif  USE_MAIN
#define DLLAPI
#else
#define DLLAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif



struct Parameter
{
	int TICK          ;  // % * 100
	int PWM_A1        ;  // % * 100
	int PWM_A2        ;  // % * 100
	int PWM_B1        ;  // % * 100
	int PWM_B2        ;  // % * 100
	int PWM_B3        ;  // % * 100
	int PWM_DISCHARGE ;  // % * 100
	int PWM_UDIM21    ;  // % * 100
	int PWM_UDIM22    ;  // % * 100
	int PWM_C1        ;  // % * 100
	int PWM_C2        ;  // % * 100
	int PWM_C3        ;  // % * 100
	int PWM_C4        ;  // % * 100
	int PWM_C5        ;  // % * 100
	int PWM_C6        ;  // % * 100
	int PWM_C7        ;  // % * 100
	int PWM_C8        ;  // % * 100
	int PWM_C9        ;  // % * 100
	int PWM_C10       ;  // % * 100
	int PWM_C11       ;  // % * 100
	int PWM_C12       ;  // % * 100
	int PWM_D1        ;  // % * 100
	int PWM_D2        ;  // % * 100
	int PWM_D3        ;  // % * 100
	int PWM_D4        ;  // % * 100
	int PWM_D5        ;  // % * 100
	int PWM_D6        ;  // % * 100
	int PWM_D7        ;  // % * 100
	int PWM_D8        ;  // % * 100
	int BOOST_VOL     ;  // mV
	int A_CUR         ;  // mA
	int B_CUR         ;  // mA
	int C_CUR         ;  // mA
	int D_CUR         ;  // mA
	int DAC_LCD       ;  // �� * 100
	int DAC_LED1      ;  // �� * 100
	int DAC_LED2      ;  // �� * 100
	int DAC_LED3      ;  // �� * 100
};


DLLAPI void BkupFileOpen( char *plabel );

DLLAPI void BkupFileClose();

DLLAPI void InportCmdFile(char* cmd_path, int ch);

DLLAPI void SetCh( int ch );

DLLAPI void OpenSerial(char* com_name, int baud);

DLLAPI void CloseSerial();

DLLAPI void WriteCmd( char* cmd );

DLLAPI void GetParam( struct Parameter *param );

#ifdef __cplusplus
}
#endif

#endif
