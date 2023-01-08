gcc -shared -DBUILD_DLL fifo.c DebugApl.c Serial.c -o DebgApl_server.dll
gcc -DUSE_MAIN fifo.c DebugApl.c Serial.c -o analizer.exe

gcc -shared -DBUILD_DLL DebugApl_clnt.c -o DebgApl.dll

pause
