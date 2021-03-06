#############################################################
#
# Copyright 2015 F4E | European Joint Undertaking for ITER 
#  and the Development of Fusion Energy ('Fusion for Energy')
# 
# Licensed under the EUPL, Version 1.1 or - as soon they 
# will be approved by the European Commission - subsequent  
# versions of the EUPL (the "Licence"); 
# You may not use this work except in compliance with the 
# Licence. 
# You may obtain a copy of the Licence at: 
#  
# http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in 
# writing, software distributed under the Licence is 
# distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
# express or implied. 
# See the Licence for the specific language governing 
# permissions and limitations under the Licence. 
#
#############################################################

SPB=MARTe2HieratikaInterface.x MARTe2HieratikaMessageDispatcher.x

PACKAGE=Components
ROOT_DIR=../../..
MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)


LIBRARIES_STATIC=$(BUILD_DIR)/MARTe2HieratikaInterface/MARTe2HieratikaInterfaceTest$(LIBEXT)
LIBRARIES_STATIC+=$(BUILD_DIR)/MARTe2HieratikaMessageDispatcher/MARTe2HieratikaMessageDispatcherTest$(LIBEXT)

all: $(OBJS)  $(SUBPROJ)  \
    $(BUILD_DIR)/InterfacesTest$(LIBEXT) \
    $(BUILD_DIR)/InterfacesTest$(DLLEXT)
	echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)


