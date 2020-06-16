ca/client/tcpiiu.cpp:    type&=~(0x8000u);
ca/client/oldChannelNotify.cpp:    type&=~(0x8000u);
ca/client/comQueSend.cpp: 	if((dataType&(0x8000u))!=0){
ca/client/comQueSend.cpp:            dataType&=~(0x8000u);
ca/client/comQueSend.cpp:    dataType&=~(0x8000u);
ca/client/comQueSend.cpp: 	    /*if((dataTypeOrig&(0x8000u))!=0){
ca/client/comQueSend.cpp: 	    /*if((dataTypeOrig&(0x8000u))!=0){
ca/client/comQueSend.cpp: 	    /*if((dataTypeOrig&(0x8000u))!=0){
ca/client/comQueSend.cpp:    if((dataTypeOrig&(0x8000u))!=0){
ca/client/nciu.cpp:    type&=~(0x8000u);
ioc/rsrv/camessage.c
../configure/os/CONFIG.Common.linuxCommon

-Replace the file in EPICS
-RE-compile
ca_array_put(pv_type| (0x8000u), numberOfElements, pvChid, pv_memory)
-pv_memory=[pv_value|timestamp] (timestamp=64 bit)

