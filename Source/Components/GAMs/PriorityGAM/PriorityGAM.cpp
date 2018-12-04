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
    prevSignalMem = NULL;
    sortedIndices = NULL;
    numberOfSignalToBeSent = 0u;
    totalSize = 0u;
    chunckCounter = 0u;
    indexList = NULL;
    currentIdx = 0u;
    currentChangePos = 0u;
    numberOfChunks = 0u;
}

PriorityGAM::~PriorityGAM() {
    if (prevSignalMem != NULL) {
        delete[] prevSignalMem;
    }

    if (indexList != NULL) {
        delete[] indexList;
    }
}

bool PriorityGAM::Setup() {
    bool ret = (numberOfOutputSignals == (numberOfInputSignals + 1u));
    if(!ret){
        REPORT_ERROR(ErrorManagement::InitialisationError, "The number of output signals must be one more than the number of the input signals");
    }

    totalSize = 0u;
    StreamString dataSourceName;

    for (uint32 i = 0u; (i < numberOfInputSignals) && (ret); i++) {
        uint32 byteSize;
        ret = GetSignalByteSize(InputSignals, i, byteSize);
        if (ret) {
            totalSize += byteSize;
        }
        if (ret) {
            uint32 outByteSize;
            ret = GetSignalByteSize(OutputSignals, i, outByteSize);
            if (ret) {
                ret = (outByteSize == byteSize);
                if (!ret) {
                    REPORT_ERROR(ErrorManagement::InitialisationError, "The signal (%d) has a different size than its specular output", i);
                }
            }
        }
        if(ret){
            StreamString dsName;
            ret = GetSignalDataSourceName(OutputSignals, i, dsName);
            if (ret) {
                if (i == 0u) {
                    dataSourceName = dsName;
                }
                else {
                    ret = (dsName == dataSourceName);
                    if (!ret) {
                        REPORT_ERROR(ErrorManagement::InitialisationError,
                                     "The data source of the first signal (%s) differs from the one of the signal %d (%s)", dataSourceName.Buffer(), i,
                                     dsName.Buffer());
                    }
                }
            }

        }
    }


    if (ret) {
        prevSignalMem = new uint8[totalSize];
        indexList = new uint32[numberOfInputSignals];

        for (uint32 i = 0u; i < numberOfInputSignals; i++) {
            indexList[i] = i;
        }
        //save the last output signal that must contain
        //the N indices of the signals to be sent
        sortedIndices = (uint32*) GetOutputSignalMemory(numberOfOutputSignals - 1u);

        ret = GetSignalNumberOfElements(OutputSignals, numberOfOutputSignals - 1u, numberOfSignalToBeSent);
    }
    if (ret) {
        numberOfChunks = (numberOfInputSignals / numberOfSignalToBeSent);
        if ((numberOfInputSignals % numberOfSignalToBeSent) > 0u) {
            numberOfChunks++;
        }
    }
    return ret;
}

bool PriorityGAM::Execute() {

    MemoryOperationsHelper::Copy(GetOutputSignalsMemory(), GetInputSignalsMemory(), totalSize);


    if (chunckCounter >= (numberOfChunks)) {
        //insertion sort if changed
        for (uint32 i = 0u; i < numberOfInputSignals; i++) {
            uint32 signalIdx;
            uint32 index = (currentIdx + i) % numberOfInputSignals;
            signalIdx = indexList[index];
            //if a variable changes
            uint32 byteSize;
            GetSignalByteSize(InputSignals, signalIdx, byteSize);
            uint32 offset = ((uintp) inputSignalsMemoryIndexer[signalIdx] - (uintp) GetInputSignalsMemory());
            if (MemoryOperationsHelper::Compare(inputSignalsMemoryIndexer[signalIdx], prevSignalMem + offset, byteSize) != 0) {
                for (uint32 j = i; j > currentChangePos; j--) {
                    uint32 jIndex = (currentIdx + j) % numberOfInputSignals;
                    uint32 prevJIndex = (jIndex == 0u) ? (numberOfInputSignals - 1u) : (jIndex - 1u);
                    uint32 temp = indexList[prevJIndex];
                    indexList[prevJIndex] = indexList[jIndex];
                    indexList[jIndex] = temp;
                }
                currentChangePos++;
            }
        }
    }
    else {
        chunckCounter++;
    }

    uint32 sizeToCopy = numberOfSignalToBeSent;
    bool needTwoCopies = false;
    if ((currentIdx + sizeToCopy) > numberOfInputSignals) {
        sizeToCopy = (numberOfInputSignals - currentIdx);
        needTwoCopies = true;
    }
    MemoryOperationsHelper::Copy(sortedIndices, &indexList[currentIdx], sizeToCopy * sizeof(uint32));
    if (needTwoCopies) {
        uint32 sortedIdx = sizeToCopy;
        sizeToCopy = (currentIdx + sizeToCopy) - numberOfInputSignals;
        MemoryOperationsHelper::Copy(&sortedIndices[sortedIdx], indexList, sizeToCopy * sizeof(uint32));
    }


    currentIdx += numberOfSignalToBeSent;
    currentIdx %= numberOfInputSignals;
    if (currentChangePos > numberOfSignalToBeSent) {
        currentChangePos -= numberOfSignalToBeSent;
    }
    else {
        currentChangePos = 0u;
    }

    if (currentChangePos > (numberOfInputSignals - numberOfSignalToBeSent)) {
        //if almost all the signal are changing do a cycle of FIFO again
        currentChangePos = 0u;
        chunckCounter = 0u;
    }

    MemoryOperationsHelper::Copy(prevSignalMem, GetInputSignalsMemory(), totalSize);

    return true;
}

CLASS_REGISTER(PriorityGAM, "1.0")
}
