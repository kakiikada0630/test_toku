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
	int PWM0          ;  // % * 100
	int PWM1          ;  // % * 100
	int PWM2          ;  // % * 100
	int PWM3          ;  // % * 100
	int PWM4          ;  // % * 100
	int PWM5          ;  // % * 100
	int PWM6          ;  // % * 100
	int PWM7          ;  // % * 100
	int PWM8          ;  // % * 100
	int PWM9          ;  // % * 100
	int PWM10         ;  // % * 100
	int PWM11         ;  // % * 100
	int PWM12         ;  // % * 100
	int PWM13         ;  // % * 100
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
	int DAC0          ;  // Åé * 100
	int DAC1          ;  // Åé * 100
	int DAC2          ;  // Åé * 100
	int DAC3          ;  // Åé * 100
	int DAC4          ;  // Åé * 100
	int DAC5          ;  // Åé * 100
	int DAC6          ;  // Åé * 100
	int DAC7          ;  // Åé * 100
	int ADC0          ;  // Åé * 100
	int ADC1          ;  // Åé * 100
	int ADC2          ;  // Åé * 100
	int ADC3          ;  // Åé * 100
	int ADC4          ;  // Åé * 100
	int ADC5          ;  // Åé * 100
	int ADC6          ;  // Åé * 100
	int ADC7          ;  // Åé * 100
	int VBU           ;  // 1:ON, 0:OFF
	int IG1           ;  // 1:ON, 0:OFF
	int TURNS         ;  // 1:ON, 0:OFF
	int HLBKUP        ;  // 1:ON, 0:OFF
};


DLLAPI void FileOpen( char *plabel );

DLLAPI void FileClose();

DLLAPI void OpenSerial(char* com_dbg, char* com_plusb);

DLLAPI void CloseSerial();

DLLAPI void WriteCmd( char* cmd );

DLLAPI void GetParam( struct Parameter *param );

DLLAPI void GetMiconVer( char* ver );

DLLAPI void GetVer( char* ver );


#ifdef __cplusplus
}
#endif

#endif
