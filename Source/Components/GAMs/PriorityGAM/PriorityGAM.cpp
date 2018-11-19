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
    sortedIndices = NULL;
    numberOfSignalToBeSent = 0u;
    totalSize = 0u;
    firstTime = 0u;
}

PriorityGAM::~PriorityGAM() {
    if (prevSignalMem != NULL) {
        delete[] prevSignalMem;
    }
}

bool PriorityGAM::Setup() {
    bool ret = true;
    totalSize = 0u;

    for (uint32 i = 0u; (i < numberOfInputSignals) && (ret); i++) {
        uint32 byteSize;
        ret = GetSignalByteSize(InputSignals, i, byteSize);
        if (ret) {
            totalSize += byteSize;
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
    if (firstTime > 0u) {
        MemoryOperationsHelper::Copy(GetOutputSignalsMemory(), GetInputSignalsMemory(), totalSize);

        //Maintain an ordered list of the indexes
        //of the signals to be sent by priority

        for (uint32 i = 0u; i < numberOfSignalToBeSent; i++) {
            sortedIndices[i] += numberOfSignalToBeSent;
            sortedIndices[i] %= numberOfInputSignals;
            //REPORT_ERROR(ErrorManagement::Information, "sortedIndices[%d]=%d", i, sortedIndices[i]);

        }
        uint32 nChunks = (numberOfInputSignals / numberOfSignalToBeSent) + 1u;
        if (firstTime >= (nChunks)) {
            uint32 currentPos = 1u;
            for (uint32 i = 0u; i < numberOfInputSignals; i++) {
                uint32 signalIdx;
                if (i < numberOfSignalToBeSent) {
                    signalIdx = sortedIndices[i];
                }
                else {
                    signalIdx = (sortedIndices[i % numberOfSignalToBeSent] + numberOfSignalToBeSent) % numberOfInputSignals;
                }
                //if a variable changes
                uint32 byteSize;
                GetSignalByteSize(InputSignals, signalIdx, byteSize);
                uint32 offset = ((uint64) inputSignalsMemoryIndexer[signalIdx] - (uint64) GetInputSignalsMemory());
                if (MemoryOperationsHelper::Compare(inputSignalsMemoryIndexer[signalIdx], prevSignalMem + offset, byteSize) != 0) {
                    for (uint32 j = i; j >= currentPos; j--) {
                        uint32 temp = sortedIndices[j - 1u];
                        sortedIndices[j - 1u] = sortedIndices[j];
                        sortedIndices[j] = temp;
                    }
                    currentPos++;
                }
            }
        }
        else {
            firstTime++;
        }

    }
    else {
        for (uint32 i = 0u; (i < numberOfSignalToBeSent); i++) {
            sortedIndices[i] = i;
        }
        firstTime++;
    }
    MemoryOperationsHelper::Copy(prevSignalMem, GetInputSignalsMemory(), totalSize);

    return true;
}

CLASS_REGISTER(PriorityGAM, "1.0")
}
