protomonitor: protomonitor.c
   gcc -I ~/sysdig/userspace/libscap/ -g protomonitor.c global.c proc_manager.c stat_manager.c  -o protomonitor  ~/sysdig/build/userspace/libscap/libscap.a ~/sysdig/build/zlib-prefix/src/zlib/libz.a

