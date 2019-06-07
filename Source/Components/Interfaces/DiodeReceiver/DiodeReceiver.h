/**
 * @file DiodeReceiver.h
 * @brief Header file for class DiodeReceiver
 * @date 04 dic 2018
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

 * @details This header file contains the declaration of the class DiodeReceiver
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef DIODERECEIVER_H_
#define DIODERECEIVER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <epicsStdlib.h>
#include <cadef.h>
#include <epicsGetopt.h>
#include "tool_lib.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MultiClientService.h"
#include "File.h"
#include "TCPSocket.h"
#include "EmbeddedServiceMethodBinderT.h"
#include "EventSem.h"
#include "HttpProtocol.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

const uint32 PV_NAME_MAX_SIZE_REC = 64u;

/**
 * Contains all the PV data
 */
struct PvRecDescriptor {
    /**
     * The channel identifier
     */
    chid pvChid;
    /**
     * The PV type
     */
    chtype pvType;
    /**
     * The number of elements > 0
     */
    uint32 numberOfElements;

    /**
     * The PV name
     */
    char8 pvName[PV_NAME_MAX_SIZE_REC];

    /**
     * The AnyType associated to this PV
     */
    AnyType at;

    /**
     * The total size of the variable
     */
    uint32 totalSize;

    /**
     * The offset of the variable in the memory buffer
     */
    uint32 offset;

};


/**
 * @brief
 */
class DiodeReceiver: public MultiClientService {
public:
    CLASS_REGISTER_DECLARATION()
    DiodeReceiver();
    virtual ~DiodeReceiver();

    virtual bool Initialise(StructuredDataI &data);

    virtual ErrorManagement::ErrorType Start();

    ErrorManagement::ErrorType ServerCycle(MARTe::ExecutionInfo &information);

    ErrorManagement::ErrorType ClientService(TCPSocket * const commClient);

    virtual ErrorManagement::ErrorType Stop();

    virtual bool Synchronise(uint8 *memoryOut,
                             uint8 *changedFlags);

    uint32 GetNumberOfVariables();

    PvRecDescriptor *GetPvDescriptors();

    uint64 GetTotalMemorySize();

    bool InitialisationDone();
    friend void DiodeReceiverCycleLoop(DiodeReceiver &arg);

    bool GetLocalVariableIndex(const char8 *varName,
                               uint32 &index);

    virtual ErrorManagement::ErrorType AddThread();

protected:

    ErrorManagement::ErrorType ReadNewChunk(TCPSocket * const commClient,
                                            StreamString &payload,
                                            bool isChunked,
                                            uint32 &chunkSize,
                                            uint32 &contentLength);

    bool ReadVarNameAndIndex(StreamString &payload,
                             StreamString &varName,
                             uint32 &receivedIndex,
                             uint32 &receivedSize,
                             uint32 &processedSize,
                             const char8 * &dataPtr);

    bool GetLocalIndex(StreamString &payload,
                       StreamString &varName,
                       uint32 receivedIndex,
                       uint32 receivedSize,
                       uint32 &index,
                       uint32 &processedSize);

    void ReadVarValueAndSkip(StreamString &payload,
                             const char8 *dataPtr,
                             uint32 index,
                             uint32 processedSize);

    ErrorManagement::ErrorType SendOkReplyMessage(HttpProtocol &protocol,
                                                  TCPSocket * const commClient);

    void SendErrorReplyMessage(HttpProtocol &protocol,
                               TCPSocket * const commClient);

    TCPSocket server;

    EmbeddedServiceMethodBinderT<DiodeReceiver> embeddedMethod;

    uint32 serverPort;

    TimeoutType acceptTimeout;

    FastPollingMutexSem syncSem;

    PvRecDescriptor *pvs;
    uint32 numberOfVariables;

    uint32 numberOfCpus;

    uint8 *memory;
    uint8 *memory2;
    uint8 *memoryPrec;

    uint32 *pvMapping;

    volatile int32 threadSetContext;

    uint8 *changeFlag;
    uint8 *changeFlag2;
    uint64 lastCounter;

    volatile int32 quit;
    uint32 totalMemorySize;

    EventSem eventSem;

    uint32 numberOfInitThreads;
    uint32 threadCnt;
    uint32 maxNumberOfVariables;
    uint64 *lastTickCounter;
    uint32 msecPeriod;
    uint32 currentCpuMask;

};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_INTERFACES_DIODERECEIVER_DIODERECEIVER_H_ */

