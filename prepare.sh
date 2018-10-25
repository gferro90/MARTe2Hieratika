#!/bin/sh

export MARTe2_DIR=/home/pc/MARTe2Project/GIT/MARTe2
export MARTe2_Components_DIR=/home/pc/MARTe2Project/GIT/MARTe2-components
export ROOT_DIR=$MARTe2_DIR
export MAKEDEFAULTDIR=$ROOT_DIR
export ENVIRONMENT_BM_L1Portability_DIR=Environment
export LD_LIBRARY_PATH=/home/pc/MARTe2Project/GIT/MARTe2/Build/x86-linux/Core/:$MARTe2_Components_DIR/Build/x86-linux/Components/DataSources:$MARTe2_Components_DIR/Build/x86-linux/Components/GAMs:$MARTe2_Components_DIR/Build/x86-linux/Components/Interfaces
export OUTPUT_DIR=Build
export TARGET=x86-linux

