/**
 * @file PriorityGAM.cpp
 * @brief Source file for class PriorityGAM
 * @date 06 nov 2018
 * @author pc
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class PriorityGAM (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "AdvancedErrorManagement.h"
#include "PriorityGAM.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

PriorityGAM::PriorityGAM() :
        GAM() {
    cycleTax = 0u;
    changeTax = 0u;
    prevSignalMem = NULL;
    byteSize = NULL;
    offset = NULL;
    sortedIndices = NULL;
    numberOfSignalToBeSent = 0u;
    priorityList = NULL;
    priorityListBuffer = NULL;
    totalSize = 0u;
    firstTime = 0u;
}

PriorityGAM::~PriorityGAM() {
    if (prevSignalMem != NULL) {
        delete[] prevSignalMem;
    }
    if (byteSize != NULL) {
        delete[] byteSize;
    }
    if (offset != NULL) {
        delete[] offset;
    }
    if (priorityList != NULL) {
        delete[] priorityList;
    }
    if (priorityListBuffer != NULL) {
        delete[] priorityListBuffer;
    }
}

bool PriorityGAM::Initialise(StructuredDataI & data) {
    bool ret = GAM::Initialise(data);
    if (ret) {
        ret = data.Read("CycleStep", cycleTax);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define the CycleStep parameter");
        }
        else {
            ret = data.Read("ChangeStep", changeTax);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define the ChangeStep parameter");
            }
        }

    }
    return ret;
}

bool PriorityGAM::Setup() {
    bool ret = true;
    totalSize = 0u;

    byteSize = new uint32[numberOfInputSignals];
    offset = new uint32[numberOfInputSignals];

    priorityList = new uint32[numberOfInputSignals];
    priorityListBuffer = new uint32[numberOfInputSignals];
    for (uint32 i = 0u; (i < numberOfInputSignals) && (ret); i++) {
        ret = GetSignalByteSize(InputSignals, i, byteSize[i]);
        offset[i] = totalSize;
        if (ret) {
            totalSize += byteSize[i];
        }
    }

    if (ret) {
        prevSignalMem = new uint8[totalSize];
        //save the last output signal that must contain
        //the N indices of the signals to be sent
        sortedIndices = (uint32*) GetOutputSignalMemory(numberOfOutputSignals - 1u);

        ret = GetSignalNumberOfElements(OutputSignals, numberOfOutputSignals - 1u, numberOfSignalToBeSent);
    }
    return ret;
}

bool PriorityGAM::Execute() {
    if (firstTime>0u) {
        MemoryOperationsHelper::Copy(GetOutputSignalsMemory(), GetInputSignalsMemory(), totalSize);

        //Maintain an ordered list of the indexes
        //of the signals to be sent by priority

        //phase 1... move the first N signals to the end and reset them
        REPORT_ERROR(ErrorManagement::Information, "Copying %d to buffer", numberOfSignalToBeSent);
        MemoryOperationsHelper::Copy(priorityListBuffer, priorityList, numberOfSignalToBeSent * sizeof(uint32));

        uint32 remainedSignals = (numberOfInputSignals - numberOfSignalToBeSent);
        //REPORT_ERROR(ErrorManagement::Information, "Moving %d to top", remainedSignals);
        MemoryOperationsHelper::Move(priorityList, &priorityList[numberOfSignalToBeSent], remainedSignals * sizeof(uint32));
        //REPORT_ERROR(ErrorManagement::Information, "Copying %d from buffer to end", numberOfSignalToBeSent);
        MemoryOperationsHelper::Copy(&priorityList[numberOfSignalToBeSent], priorityListBuffer, numberOfSignalToBeSent * sizeof(uint32));



        for (uint32 i = 0u; i < numberOfSignalToBeSent; i++) {
            sortedIndices[i]+=numberOfSignalToBeSent;
            sortedIndices[i]%=numberOfInputSignals;
            //REPORT_ERROR(ErrorManagement::Information, "sortedIndices[%d]=%d", i, sortedIndices[i]);

        }
        //phase 2... reset the last signals and add the cycle tax to the others
        for (uint32 i = 0u; i < numberOfInputSignals; i++) {
            if (i < remainedSignals) {
                priorityList[i] += cycleTax;
            }
            else {
                priorityList[i] = cycleTax;
            }
            //phase 3... check if change and insertion sort
            //REPORT_ERROR(ErrorManagement::Information, "Comparing %d, offset=%d", i, offset[i]);

            if (MemoryOperationsHelper::Compare(((uint8*) GetInputSignalsMemory()) + offset[i], prevSignalMem + offset[i], byteSize[i]) != 0) {
                priorityList[i] += changeTax;
                for (uint32 j = i; j >= 1; j--) {
                    if (priorityList[j] < priorityList[j - 1u]) {
                        uint32 temp = priorityList[j - 1u];
                        priorityList[j - 1u] = priorityList[j];
                        priorityList[j] = temp;
                    }
                }
            }
        }
    }
    else {
        for (uint32 i = 0u; (i < numberOfSignalToBeSent); i++) {
            sortedIndices[i]=i;
        }
        firstTime++;
    }
    MemoryOperationsHelper::Copy(prevSignalMem, GetInputSignalsMemory(), totalSize);

    return true;
}

CLASS_REGISTER(PriorityGAM, "1.0")
}
