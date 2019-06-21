#!/bin/sh
cp tcpiiu.cpp $EPICS_BASE/src/ca/client
cp oldChannelNotify.cpp $EPICS_BASE/src/ca/client
cp comQueSend.cpp $EPICS_BASE/src/ca/client
cp nciu.cpp $EPICS_BASE/src/ca/client
cp camessage.c $EPICS_BASE/src/ioc/rsrv
cp CONFIG.Common.linuxCommon $EPICS_BASE/configure/os
cp recGbl.c $EPICS_BASE/src/ioc/db
cd $EPICS_BASE/src/
make
cd -

