gcc -shared -DBUILD_DLL fifo.c DebugApl.c Serial.c -o DebgApl.dll
gcc -DUSE_MAIN fifo.c DebugApl.c Serial.c -o analizer.exe

pause
