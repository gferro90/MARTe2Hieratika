#!/bin/sh

export MARTe2_DIR=/home/pc/MARTe2Project/GIT/MARTe2
export MARTe2_Components_DIR=/home/pc/MARTe2Project/GIT/MARTe2-components
export ROOT_DIR=$MARTe2_DIR
export MAKEDEFAULTDIR=$ROOT_DIR
export ENVIRONMENT_BM_L1Portability_DIR=Environment
export EPICS_BASE=/home/pc/EPICS/base-3.15.6
export EPICS_HOST_ARCH=linux-x86_64
export PATH=$PATH:$EPICS_BASE/bin/linux-x86_64
export LD_LIBRARY_PATH=/home/pc/MARTe2Project/GIT/MARTe2/Build/x86-linux/Core/:$MARTe2_Components_DIR/Build/x86-linux/Components/DataSources:$MARTe2_Components_DIR/Build/x86-linux/Components/GAMs:$MARTe2_Components_DIR/Build/x86-linux/Components/Interfaces:$EPICS_BASE/lib/$EPICS_HOST_ARCH
export OUTPUT_DIR=Build
export TARGET=x86-linux

