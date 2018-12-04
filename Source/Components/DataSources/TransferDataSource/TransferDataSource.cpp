/**
 * @file TransferDataSource.cpp
 * @brief Source file for class TransferDataSource
 * @date 21 nov 2018
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
 * the class TransferDataSource (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "AdvancedErrorManagement.h"
#include "TransferDataSource.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

TransferDataSource::TransferDataSource() {
    // Auto-generated constructor stub for TransferDataSource
    // TODO Verify if manual additions are needed
    memory = NULL;
    totalMemorySize = 0u;
    offsets = NULL;
}

TransferDataSource::~TransferDataSource() {
    // Auto-generated destructor stub for TransferDataSource
    // TODO Verify if manual additions are needed
    if (memory != NULL) {
        uint32 numberOfMemoryBuffers = GetNumberOfStatefulMemoryBuffers();
        for (uint32 i = 0u; i < numberOfMemoryBuffers; i++) {
            MemoryOperationsHelper::Free(&memory[i]);
        }
    }
    if (offsets != NULL) {
        delete[] offsets;
    }
}

bool TransferDataSource::Initialise(StructuredDataI &data) {
    bool ret = DataSourceI::Initialise(data);
    if (ret) {
        totalMemorySize = 0u;
        ret = data.MoveRelative("MemoryConfiguration");
        if (ret) {
            uint32 numberOfSignals = data.GetNumberOfChildren();
            offsets = new uint32[numberOfSignals];
            for (uint32 i = 0u; (i < numberOfSignals) && (ret); i++) {
                ret = data.MoveToChild(i);
                if (ret) {
                    uint32 numberOfElements = 1u;
                    if (!data.Read("NumberOfElements", numberOfElements)) {
                        numberOfElements = 1u;
                    }
                    StreamString type;
                    ret = data.read("Type", type);
                    if (ret) {
                        TypeDescriptor td = TypeDescriptor::GetTypeDescriptorFromTypeName(type.Buffer);
                        offsets[i] = totalMemorySize;
                        totalMemorySize += (td.numberOfBits / 8u) * numberOfElements;
                        ret = data.MoveToAncestor(1u);
                    }
                    else {
                        REPORT_ERROR(ErrorManagement::InitialisationError, "Please specify the type of the signal %s", data.GetName());
                    }
                }
            }
        }
        if (ret) {
            ret = data.MoveToAncestor(1u);
        }
        if (ret) {
            memoryConfig = data;
        }
    }
    return ret;
}

bool TransferDataSource::AllocateMemory() {
    uint32 numberOfMemoryBuffers = GetNumberOfStatefulMemoryBuffers();
    memory = new uint8*[numberOfMemoryBuffers];
    for (uint32 i = 0u; i < numberOfMemoryBuffers; i++) {
        memory[i] = (uint8*) MemoryOperationsHelper::Malloc(totalMemorySize);
    }
    return true;
}

bool TransferDataSource::SetConfiguredDatabase(StructuredDataI & data) {
    bool ret = data.SetConfiguredDatabase(data);
    if (ret) {
        ret = (GetNumberOfSignals() == 1u);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::FatalError, "Only one signal is allowed");
        }
    }
    return ret;
}

bool TransferDataSource::GetSignalMemoryBuffer(const uint32 signalIdx,
                                               const uint32 bufferIdx,
                                               void *&signalAddress) {
    signalAddress = (void*) (&memory[bufferIdx]);
}

}

