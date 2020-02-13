#!/bin/sh
export EPICS_CA_MAX_ARRAY_BYTES=1000000
export MARTe2_DIR=/home/DiodeSender/MARTe2
export MARTe2_Hieratika_DIR=/home/DiodeSender/MARTe2Hieratika
export ROOT_DIR=$MARTe2_DIR
export MAKEDEFAULTDIR=$ROOT_DIR
export ENVIRONMENT_BM_L1Portability_DIR=Environment
export EPICS_BASE=/home/DiodeSender/base-3.15.6
export EPICS_HOST_ARCH=linux-x86_64
export PATH=$PATH:$EPICS_BASE/bin/linux-x86_64
export LD_LIBRARY_PATH=/home/DiodeSender/MARTe2/Build/x86-linux/Core/:$EPICS_BASE/lib/$EPICS_HOST_ARCH
export OUTPUT_DIR=Build
export TARGET=x86-linux
$MARTe2_Hieratika_DIR/Build/x86-linux/GTest/DiodeStandaloneApp.ex /home/DiodeSender/DiodeSender/DiodeSender.cfg


