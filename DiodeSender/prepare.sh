#!/bin/sh
export MARTe2_DIR=/home/ifmif/MARTe2Project/GIT/MARTe2
export MARTe2_Components_DIR=/home/ifmif/MARTe2Project/GIT/MARTe2-components
export ROOT_DIR=/home/ifmif/MARTe2Project/GIT/MARTe2-components
export MAKEDEFAULTDIR=$MARTe2_DIR/MakeDefaults
export LD_LIBRARY_PATH=/home/ifmif/MARTe2Project/GIT/MARTe2/Build/x86-linux/Core/:/home/ifmif/EPICS/base-3.15.6/lib/linux-x86_64/
#export EPICS_BASE=/home/ifmif/EPICS/base-3.15.6
export EPICS_BASE=/home/ifmif/EPICS/base-3.15.6
export EPICS_HOST_ARCH=linux-x86_64
export OUTPUT_DIR=Build
export TARGET=x86-linux
export OPEN62541_INCLUDE=/home/ifmif/open62541/build
export OPEN62541_LIB=/home/ifmif/open62541/build/bin
export EPICS_CA_MAX_ARRAY_BYTES=1000000
