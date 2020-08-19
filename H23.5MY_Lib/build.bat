gcc -shared -DBUILD_DLL fifo.c DebugApl.c Serial.c -o DebugApp_H23_5MY.dll
gcc  -DUSE_MAIN fifo.c DebugApl.c Serial.c -o DebugApp_H23_5MY.exe

pause
