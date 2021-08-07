import tkinter as tk
import threading
import time
import ctypes

#-----------------------------------
# 環境に合わせて変える!!!
#-----------------------------------
COM_DBG   = b"\\\.\COM7"
COM_PLUSB = b"\\\.\COM8"

#-----------------------------------

class PARAM(ctypes.Structure):
	_fields_ = [ 
		("TICK"         , ctypes.c_int32),
		("PWM_A1"       , ctypes.c_int32),
		("PWM_A2"       , ctypes.c_int32),
		("PWM_B1"       , ctypes.c_int32),
		("PWM_B2"       , ctypes.c_int32),
		("PWM_B3"       , ctypes.c_int32),
		("PWM_DISCHARGE", ctypes.c_int32),
		("PWM_UDIM21"   , ctypes.c_int32),
		("PWM_UDIM22"   , ctypes.c_int32),
		("PWM_C1"       , ctypes.c_int32),
		("PWM_C2"       , ctypes.c_int32),
		("PWM_C3"       , ctypes.c_int32),
		("PWM_C4"       , ctypes.c_int32),
		("PWM_C5"       , ctypes.c_int32),
		("PWM_C6"       , ctypes.c_int32),
		("PWM_C7"       , ctypes.c_int32),
		("PWM_C8"       , ctypes.c_int32),
		("PWM_C9"       , ctypes.c_int32),
		("PWM_C10"      , ctypes.c_int32),
		("PWM_C11"      , ctypes.c_int32),
		("PWM_C12"      , ctypes.c_int32),
		("PWM_D1"       , ctypes.c_int32),
		("PWM_D2"       , ctypes.c_int32),
		("PWM_D3"       , ctypes.c_int32),
		("PWM_D4"       , ctypes.c_int32),
		("PWM_D5"       , ctypes.c_int32),
		("PWM_D6"       , ctypes.c_int32),
		("PWM_D7"       , ctypes.c_int32),
		("PWM_D8"       , ctypes.c_int32),
		("BOOST_VOL"    , ctypes.c_int32),
		("A_CUR"        , ctypes.c_int32),
		("B_CUR"        , ctypes.c_int32),
		("C_CUR"        , ctypes.c_int32),
		("D_CUR"        , ctypes.c_int32),
		("DAC_LCM"      , ctypes.c_int32),
		("DAC_LED1"     , ctypes.c_int32),
		("DAC_LED2"     , ctypes.c_int32),
		("DAC_LED3"     , ctypes.c_int32),
		("VBU"          , ctypes.c_int32),
		("IG1"          , ctypes.c_int32),
		("TURNS"        , ctypes.c_int32),
		("HLBKUP"       , ctypes.c_int32)]

DebufApl = ctypes.WinDLL("./DebgApl_server.dll")

DebufApl.FileOpen.argtypes   = (ctypes.POINTER(ctypes.c_char),)

DebufApl.OpenSerial.argtypes = (ctypes.POINTER(ctypes.c_char,), ctypes.POINTER(ctypes.c_char,),)

DebufApl.WriteCmd.argtypes   = (ctypes.POINTER(ctypes.c_char),)

DebufApl.GetParam.argtypes   = (ctypes.POINTER(PARAM),)

DebufApl.GetMiconVer.argtypes   = (ctypes.POINTER(ctypes.c_char),)

DebufApl.GetVer.argtypes   = (ctypes.POINTER(ctypes.c_char),)



vbu_state = 0
ig1_state = 0
turn_state = 0
hlbkup_state = 0
logging_state = 0

def time_count():
    global vbu_state
    global ig1_state
    global turn_state
    global hlbkup_state
    param = PARAM()

    micon_ver = b'               ';
    lib_ver   = b'               ';

    while True:
        DebufApl.GetParam(param)
        Static1.config(text=param.TICK)

        DebufApi.GetMiconVer( micon_ver )
        Static3.config(text=micon_ver)

        DebufApi.GetVer(lib_ver)
        Static5.config(text=lib_ver)

        lbl_LCMDec_val .config(text=param.DAC_LCM)
        lbl_LED1Dec_val.config(text=param.DAC_LED1)
        lbl_LED2Dec_val.config(text=param.DAC_LED2)
        lbl_LED3Dec_val.config(text=param.DAC_LED3)

        lbl_Discharge_val.config(text=param.PWM_DISCHARGE/100)
        lbl_A1_val.config(text=param.PWM_A1/100)
        lbl_A2_val.config(text=param.PWM_A2/100)
        lbl_B1_val.config(text=param.PWM_B1/100)
        lbl_B2_val.config(text=param.PWM_B2/100)
        lbl_B3_val.config(text=param.PWM_B3/100)
        lbl_UDIM21_val.config(text=param.PWM_UDIM21/100)
        lbl_UDIM22_val.config(text=param.PWM_UDIM22/100)
        lbl_C1_val.config(text=param.PWM_C1/100)
        lbl_C2_val.config(text=param.PWM_C2/100)
        lbl_C3_val.config(text=param.PWM_C3/100)
        lbl_C4_val.config(text=param.PWM_C4/100)
        lbl_C5_val.config(text=param.PWM_C5/100)
        lbl_C6_val.config(text=param.PWM_C6/100)
        lbl_C7_val.config(text=param.PWM_C7/100)
        lbl_C8_val.config(text=param.PWM_C8/100)
        lbl_C9_val.config(text=param.PWM_C9/100)
        lbl_C10_val.config(text=param.PWM_C10/100)
        lbl_C11_val.config(text=param.PWM_C11/100)
        lbl_C12_val.config(text=param.PWM_C12/100)
        lbl_D1_val.config(text=param.PWM_D1/100)
        lbl_D2_val.config(text=param.PWM_D2/100)
        lbl_D3_val.config(text=param.PWM_D3/100)
        lbl_D4_val.config(text=param.PWM_D4/100)
        lbl_D5_val.config(text=param.PWM_D5/100)
        lbl_D6_val.config(text=param.PWM_D6/100)
        lbl_D7_val.config(text=param.PWM_D7/100)
        lbl_D8_val.config(text=param.PWM_D8/100)
        lbl_CurA_val.config(text=param.A_CUR)
        lbl_CurB_val.config(text=param.B_CUR)
        lbl_CurC_val.config(text=param.C_CUR)
        lbl_CurD_val.config(text=param.D_CUR)
        lbl_Boost_val.config(text=param.BOOST_VOL)

        if 0 != param.VBU:
            Btn_Vbu.config( text="VBU on",bg="skyblue")
            vbu_state = 1
        else:
            Btn_Vbu.config( text="VBU off",bg="SystemButtonFace")
            vbu_state = 0

        if 0 != param.IG1:
            Btn_Ig1.config( text="IG1 on",bg="skyblue")
            ig1_state = 1
        else:
            Btn_Ig1.config( text="IG1 off",bg="SystemButtonFace")
            ig1_state = 0

        if 0 != param.TURNS:
            Btn_Turn.config( text="Turn on",bg="skyblue")
            turn_state = 1
        else:
            Btn_Turn.config( text="Turn off",bg="SystemButtonFace")
            turn_state = 0

        if 0 != param.HLBKUP:
            Btn_Hlbkup.config( text="HL on",bg="skyblue")
            hlbkup_state = 1
        else:
            Btn_Hlbkup.config( text="HL off",bg="SystemButtonFace")
            hlbkup_state = 0


def lcm_dec():
    moji = b"dac 0 " + (txt_LCMDec.get()).encode('utf-8') + b'.'
    var_lcm.set( txt_LCMDec.get() )
    #print( moji )
    DebufApl.WriteCmd(moji)

def lcm_dec_slide(*arg):
    moji = b"dac 0 " + str( var_lcm.get() ).encode('utf-8') + b'.'
    txt_LCMDec.delete(0,tk.END)
    txt_LCMDec.insert(tk.END,str( var_lcm.get() ))
    #print( moji )
    DebufApl.WriteCmd(moji)

def led1_dec():
    moji = b"dac 1 " + (txt_LED1Dec.get()).encode('utf-8') + b'.'
    var_led1.set( txt_LED1Dec.get() )
    #print( moji )
    DebufApl.WriteCmd(moji)

def led1_dec_slide(*arg):
    moji = b"dac 1 " + str( var_led1.get() ).encode('utf-8') + b'.'
    txt_LED1Dec.delete(0,tk.END)
    txt_LED1Dec.insert(tk.END,str( var_led1.get() ))
    #print( moji )
    DebufApl.WriteCmd(moji)

def led2_dec():
    moji = b"dac 2 " + (txt_LED2Dec.get()).encode('utf-8') + b'.'
    var_led2.set( txt_LED2Dec.get() )
    #print( moji )
    DebufApl.WriteCmd(moji)

def led2_dec_slide(*arg):
    moji = b"dac 2 " + str( var_led2.get() ).encode('utf-8') + b'.'
    txt_LED2Dec.delete(0,tk.END)
    txt_LED2Dec.insert(tk.END,str( var_led2.get() ))
    #print( moji )
    DebufApl.WriteCmd(moji)

def led3_dec():
    moji = b"dac 3 " + (txt_LED3Dec.get()).encode('utf-8') + b'.'
    var_led3.set( txt_LED3Dec.get() )
    #print( moji )
    DebufApl.WriteCmd(moji)

def led3_dec_slide(*arg):
    moji = b"dac 3 " + str( var_led3.get() ).encode('utf-8') + b'.'
    txt_LED3Dec.delete(0,tk.END)
    txt_LED3Dec.insert(tk.END,str( var_led3.get() ))
    #print( moji )
    DebufApl.WriteCmd(moji)

def cmd_exe():
    moji = (txt_Cmd.get()).encode('utf-8') + b'.'
    #print( moji )
    DebufApl.WriteCmd(moji)

def cmd_fukkatsu():
    moji = b"bin on " + b'.'
    DebufApl.WriteCmd(moji)

def cmd_vbu():
    global vbu_state
    global Btn_Vbu
    if 0 == vbu_state:
        moji = b"vbu on " + b'.'
        DebufApl.WriteCmd(moji)
    else:
        moji = b"vbu off " + b'.'
        DebufApl.WriteCmd(moji)

def cmd_ig1():
    global ig1_state
    if 0 == ig1_state:
        moji = b"ig1 on " + b'.'
        DebufApl.WriteCmd(moji)
    else:
        moji = b"ig1 off " + b'.'
        DebufApl.WriteCmd(moji)

def cmd_turn():
    global turn_state
    if 0 == turn_state:
        moji = b"turn on " + b'.'
        DebufApl.WriteCmd(moji)
    else:
        moji = b"turn off " + b'.'
        DebufApl.WriteCmd(moji)

def cmd_hlbkup():
    global hlbkup_state
    if 0 == hlbkup_state:
        moji = b"hlbkup on " + b'.'
        DebufApl.WriteCmd(moji)
    else:
        moji = b"hlbkup off " + b'.'
        DebufApl.WriteCmd(moji)

def cmd_logging():
    global logging_state
    if 0 == logging_state:
        moji = (txt_rogging.get()).encode('utf-8')
        DebufApl.FileOpen ( moji )
        Btn_rogging.config( text="ログ取得中" ,bg="skyblue" )
        logging_state = 1
    else:
        DebufApl.FileClose  ()
        Btn_rogging.config( text="ログ取得 off" ,bg="skyblue" )
        logging_state = 0

frame=1
stop_flag=False
thread=None
semaphore = threading.Semaphore(1)
root=tk.Tk()

#---------------------------------------------------

#---------------------------------------------------
# シリアル開始開始
#---------------------------------------------------
DebufApl.OpenSerial(COM_DBG, COM_PLUSB)

#---------------------------------------------------
# 温度関連
#---------------------------------------------------
# LCM温度用ボタン等
Btn_LCMDec =tk.Button(root,text="LCM温度(℃)",command=lcm_dec).grid(row=0, column=0)
txt_LCMDec = tk.Entry(text=b'LCM_DEC',width=10)
txt_LCMDec.grid(row=0, column=1)
txt_LCMDec.insert(tk.END,"30")
lbl_LCMDec_val = tk.Label(text=b'test',width=10,anchor="w")
lbl_LCMDec_val.grid(row=0, column=2)
var_lcm = tk.IntVar( master=root,value=30)
sc_lcm  = tk.Scale( root, variable=var_lcm, orient='horizontal', length=170, from_=-40, to=155, command=lcm_dec_slide)
sc_lcm.grid(row=1, column=0)

# LED1温度用ボタン等
Btn_LED1Dec=tk.Button(root,text="LED1温度(℃)",command=led1_dec).grid(row=2, column=0)
txt_LED1Dec = tk.Entry(text=b'LED1_DEC',width=10)
txt_LED1Dec.grid(row=2, column=1)
txt_LED1Dec.insert(tk.END,"30")
lbl_LED1Dec_val = tk.Label(text=b'test',width=10,anchor="w")
lbl_LED1Dec_val.grid(row=2, column=2)
var_led1 = tk.IntVar( master=root, value=30)
sc_led1  = tk.Scale( root, variable=var_led1, orient='horizontal', length=170, from_=-40, to=155, command=led1_dec_slide)
sc_led1.grid(row=3, column=0)

# LED2温度用ボタン等
Btn_LED2Dec=tk.Button(root,text="LED2温度(℃)",command=led2_dec).grid(row=0, column=3)
txt_LED2Dec = tk.Entry(text=b'LED2_DEC',width=10)
txt_LED2Dec.grid(row=0, column=4)
txt_LED2Dec.insert(tk.END,"30")
lbl_LED2Dec_val = tk.Label(text=b'test',width=10,anchor="w")
lbl_LED2Dec_val.grid(row=0, column=5)
var_led2 = tk.IntVar( master=root, value=30)
sc_led2  = tk.Scale( root, variable=var_led2, orient='horizontal', length=170, from_=-40, to=155, command=led2_dec_slide)
sc_led2.grid(row=1, column=3)

# LED3温度用ボタン等
Btn_LED3Dec=tk.Button(root,text="LED3温度(℃)",command=led3_dec).grid(row=2, column=3)
txt_LED3Dec = tk.Entry(text=b'LED3_DEC',width=10)
txt_LED3Dec.grid(row=2, column=4)
txt_LED3Dec.insert(tk.END,"30")
lbl_LED3Dec_val = tk.Label(text=b'test',width=10,anchor="w")
lbl_LED3Dec_val.grid(row=2, column=5)
var_led3 = tk.IntVar( master=root, value=30)
sc_led3  = tk.Scale( root, variable=var_led3, orient='horizontal', length=170, from_=-40, to=155, command=led3_dec_slide)
sc_led3.grid(row=3, column=3)

#TICK
Static1 = tk.Label(text=u'test',width=10,anchor="w")
Static1.grid(row=0, column=6)

#---------------------------------------------------
# 灯火関連
#---------------------------------------------------
lbl_Discharge = tk.Label(text='放電',width=10,anchor="e")
lbl_Discharge.grid(row=6, column=0)
lbl_Discharge_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_Discharge_val.grid(row=6, column=1)

lbl_A1 = tk.Label(text='A1',width=10,anchor="e")
lbl_A1.grid(row=7, column=0)
lbl_A1_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_A1_val.grid(row=7, column=1)

lbl_A2 = tk.Label(text='A2',width=10,anchor="e")
lbl_A2.grid(row=8, column=0)
lbl_A2_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_A2_val.grid(row=8, column=1)

lbl_B1 = tk.Label(text='B1',width=10,anchor="e")
lbl_B1.grid(row=10, column=0)
lbl_B1_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_B1_val.grid(row=10, column=1)

lbl_B2 = tk.Label(text='B2',width=10,anchor="e")
lbl_B2.grid(row=11, column=0)
lbl_B2_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_B2_val.grid(row=11, column=1)

lbl_B3 = tk.Label(text='B3',width=10,anchor="e")
lbl_B3.grid(row=12, column=0)
lbl_B3_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_B3_val.grid(row=12, column=1)

lbl_UDIM21 = tk.Label(text='UDIM21',width=10,anchor="e")
lbl_UDIM21.grid(row=14, column=0)
lbl_UDIM21_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_UDIM21_val.grid(row=14, column=1)

lbl_UDIM22 = tk.Label(text='UDIM22',width=10,anchor="e")
lbl_UDIM22.grid(row=15, column=0)
lbl_UDIM22_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_UDIM22_val.grid(row=15, column=1)


lbl_C1 = tk.Label(text='C1',width=10,anchor="e")
lbl_C1.grid(row=6, column=2)
lbl_C1_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C1_val.grid(row=6, column=3)

lbl_C2 = tk.Label(text='C2',width=10,anchor="e")
lbl_C2.grid(row=7, column=2)
lbl_C2_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C2_val.grid(row=7, column=3)

lbl_C3 = tk.Label(text='C3',width=10,anchor="e")
lbl_C3.grid(row=8, column=2)
lbl_C3_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C3_val.grid(row=8, column=3)

lbl_C4 = tk.Label(text='C4',width=10,anchor="e")
lbl_C4.grid(row=9, column=2)
lbl_C4_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C4_val.grid(row=9, column=3)

lbl_C5 = tk.Label(text='C5',width=10,anchor="e")
lbl_C5.grid(row=10, column=2)
lbl_C5_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C5_val.grid(row=10, column=3)

lbl_C6 = tk.Label(text='C6',width=10,anchor="e")
lbl_C6.grid(row=11, column=2)
lbl_C6_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C6_val.grid(row=11, column=3)

lbl_C7 = tk.Label(text='C7',width=10,anchor="e")
lbl_C7.grid(row=12, column=2)
lbl_C7_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C7_val.grid(row=12, column=3)

lbl_C8 = tk.Label(text='C8',width=10,anchor="e")
lbl_C8.grid(row=13, column=2)
lbl_C8_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C8_val.grid(row=13, column=3)

lbl_C9 = tk.Label(text='C9',width=10,anchor="e")
lbl_C9.grid(row=14, column=2)
lbl_C9_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C9_val.grid(row=14, column=3)

lbl_C10 = tk.Label(text='C10',width=10,anchor="e")
lbl_C10.grid(row=15, column=2)
lbl_C10_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C10_val.grid(row=15, column=3)

lbl_C11 = tk.Label(text='C11',width=10,anchor="e")
lbl_C11.grid(row=16, column=2)
lbl_C11_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C11_val.grid(row=16, column=3)

lbl_C12 = tk.Label(text='C12',width=10,anchor="e")
lbl_C12.grid(row=18, column=2)
lbl_C12_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_C12_val.grid(row=18, column=3)



lbl_D1 = tk.Label(text='D1',width=10,anchor="e")
lbl_D1.grid(row=6, column=4)
lbl_D1_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D1_val.grid(row=6, column=5)

lbl_D2 = tk.Label(text='D2',width=10,anchor="e")
lbl_D2.grid(row=7, column=4)
lbl_D2_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D2_val.grid(row=7, column=5)

lbl_D3 = tk.Label(text='D3',width=10,anchor="e")
lbl_D3.grid(row=8, column=4)
lbl_D3_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D3_val.grid(row=8, column=5)

lbl_D4 = tk.Label(text='D4',width=10,anchor="e")
lbl_D4.grid(row=9, column=4)
lbl_D4_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D4_val.grid(row=9, column=5)

lbl_D5 = tk.Label(text='D5',width=10,anchor="e")
lbl_D5.grid(row=10, column=4)
lbl_D5_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D5_val.grid(row=10, column=5)

lbl_D6 = tk.Label(text='D6',width=10,anchor="e")
lbl_D6.grid(row=11, column=4)
lbl_D6_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D6_val.grid(row=11, column=5)

lbl_D7 = tk.Label(text='D7',width=10,anchor="e")
lbl_D7.grid(row=12, column=4)
lbl_D7_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D7_val.grid(row=12, column=5)

lbl_D8 = tk.Label(text='D8',width=10,anchor="e")
lbl_D8.grid(row=13, column=4)
lbl_D8_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_D8_val.grid(row=13, column=5)


#---------------------------------------------------
# 電流/電圧関連
#---------------------------------------------------
lbl_CurA = tk.Label(text='CurA(mA)',width=10,anchor="e")
lbl_CurA.grid(row=20, column=0)
lbl_CurA_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_CurA_val.grid(row=20, column=1)

lbl_CurB = tk.Label(text='CurB(mA)',width=10,anchor="e")
lbl_CurB.grid(row=20, column=2)
lbl_CurB_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_CurB_val.grid(row=20, column=3)

lbl_CurC = tk.Label(text='CurC(mA)',width=10,anchor="e")
lbl_CurC.grid(row=21, column=0)
lbl_CurC_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_CurC_val.grid(row=21, column=1)

lbl_CurD = tk.Label(text='CurD(mA)',width=10,anchor="e")
lbl_CurD.grid(row=21, column=2)
lbl_CurD_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_CurD_val.grid(row=21, column=3)

lbl_Boost = tk.Label(text='BoostVol(mV)',width=12,anchor="e")
lbl_Boost.grid(row=20, column=4)
lbl_Boost_val = tk.Label(text=b'0',width=10,anchor="w")
lbl_Boost_val.grid(row=20, column=5)

#---------------------------------------------------
# その他コマンド関連
#---------------------------------------------------
Btn_Cmd =tk.Button(root,text="CMD",command=cmd_exe).grid(row=22, column=0)
txt_Cmd = tk.Entry(text=b'CMD',width=10)
txt_Cmd.grid(row=22, column=1)

#バージョン情報
Static2 = tk.Label(text=u'micon_ver',width=10,anchor="w")
Static2.grid(row=25, column=0)
Static3 = tk.Label(text=u'micon_ver',width=10,anchor="w")
Static3.grid(row=25, column=1)
Static4 = tk.Label(text=u'lib_ver',width=10,anchor="w")
Static4.grid(row=26, column=0)
Static5 = tk.Label(text=u'lib_ver',width=10,anchor="w")
Static5.grid(row=26, column=1)

# 復活の呪文
Btn_Fukkatsu =tk.Button(root,text="復活の呪文",command=cmd_fukkatsu).grid(row=22, column=3)

# その他ボタン
Btn_Vbu      =tk.Button(root,text="VBU off"       ,command=cmd_vbu)
Btn_Vbu.grid(row=23, column=3)
Btn_Ig1      =tk.Button(root,text="IG1 off"       ,command=cmd_ig1)
Btn_Ig1.grid(row=23, column=4)
Btn_Turn      =tk.Button(root,text="Turn off"       ,command=cmd_turn)
Btn_Turn.grid(row=24, column=3)
Btn_Hlbkup    =tk.Button(root,text="H/L Bkup off"       ,command=cmd_hlbkup)
Btn_Hlbkup.grid(row=24, column=4)
Btn_rogging    =tk.Button(root,text="ログ取得 off"       ,command=cmd_logging)
Btn_rogging.grid(row=25, column=3)
txt_rogging = tk.Entry(text=b'LOG',width=20)
txt_rogging.grid(row=25, column=4)
txt_rogging.insert(tk.END,"Python")

#---------------------------------------------------
thread = threading.Thread(target=time_count)
thread.start()

root.mainloop()
# 終了時にスレッドを停止する処理
stop_flag=True
thread.join()
