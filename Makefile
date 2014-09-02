protomonitor: global.c proc_manager.c protomonitor.c stat_manager.c proto_io.c global.h proc_manager.h stat_manager.h proto_io.h
	gcc -I ~/sysdig/userspace/libscap/ -g protomonitor.c global.c proc_manager.c stat_manager.c proto_io.c -o protomonitor ~/sysdig/build/userspace/libscap/libscap.a ~/sysdig/build/zlib-prefix/src/zlib/libz.a

