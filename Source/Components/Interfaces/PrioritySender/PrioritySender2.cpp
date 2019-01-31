/**
 * @file PrioritySender2.cpp
 * @brief Source file for class PrioritySender2
 * @date 01 dic 2018
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
 * the class PrioritySender2 (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "PrioritySender2.h"
#include "HttpChunkedStream.h"
#include "StreamStructuredData.h"
#include "JsonPrinter.h"
#include "AdvancedErrorManagement.h"
#include "HttpProtocol.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

void PrioritySenderCycleLoop(PrioritySender2 &arg) {
    while (arg.quit == 0u) {
        uint32 nThreadsFinishedTmp;
        if (arg.syncSem.FastLock()) {
            nThreadsFinishedTmp = arg.nThreadsFinished;
            arg.syncSem.FastUnLock();
        }

        //all the threads sent the variables
        if (nThreadsFinishedTmp == arg.numberOfPoolThreads) {
            arg.nThreadsFinished = 0u;
            arg.dataSource->Synchronise(arg.memory, arg.changeFlag);
            //if not, just send using FIFO mechanism
            if (arg.chunckCounter >= (arg.numberOfChunks)) {
                //insertion sort if changed
                for (uint32 i = 0u; i < arg.numberOfVariables; i++) {
                    //the index in the indexList
                    uint32 index = (arg.currentIdx + i) % arg.numberOfVariables;
                    //the Pv index
                    uint32 signalIdx = arg.indexList[index];
                    //swap to the first positions if the signal has changed
                    if (arg.changeFlag[signalIdx] == 1u) {
                        for (uint32 j = i; j > arg.currentChangePos; j--) {
                            uint32 jIndex = (arg.currentIdx + j) % arg.numberOfVariables;
                            uint32 prevJIndex = (jIndex == 0u) ? (arg.numberOfVariables - 1u) : (jIndex - 1u);
                            uint32 temp = arg.indexList[prevJIndex];
                            arg.indexList[prevJIndex] = arg.indexList[jIndex];
                            arg.indexList[jIndex] = temp;
                        }
                        arg.currentChangePos++;
                    }
                }
            }
            else {
                arg.chunckCounter++;
            }
            printf("current changes %d %d\n", arg.currentChangePos, arg.currentIdx);

            //update the first position for the new cycle, in order to send the ones that
            //cannot be sent in this cycle
            if (arg.currentChangePos > (arg.numberOfSignalToBeSent * arg.numberOfPoolThreads)) {
                arg.currentChangePos -= (arg.numberOfSignalToBeSent * arg.numberOfPoolThreads);
            }
            else {
                arg.currentChangePos = 0u;
            }

            //too many variable have changed...just make another FIFO trip
            if (arg.currentChangePos > (arg.numberOfVariables - (arg.numberOfSignalToBeSent * arg.numberOfPoolThreads))) {
                //if almost all the signal are changing do a cycle of FIFO again
                arg.currentChangePos = 0u;
                arg.chunckCounter = 0u;
            }

            arg.eventSem.Post();

        }
        //wait the cycle time
        uint32 elapsed = (uint32)((float32)((HighResolutionTimer::Counter() - arg.lastTickCounter) * 1000u * HighResolutionTimer::Period()));

        if (elapsed < arg.msecPeriod) {
            Sleep::MSec(arg.msecPeriod - elapsed);
        }
        arg.lastTickCounter = HighResolutionTimer::Counter();

    }
    while (arg.nThreadsFinished != arg.numberOfPoolThreads) {
        Sleep::Sec(1u);
    }
    arg.eventSem.Post();
    arg.quit = 2u;
}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

PrioritySender2::PrioritySender2() :
        MultiThreadService(embeddedMethod),
        embeddedMethod(*this, &PrioritySender2::ThreadCycle) {
    // Auto-generated constructor stub for PrioritySender2
    // TODO Verify if manual additions are needed
    dataSource = NULL;
    memory = NULL;
    indexList = NULL;
    changeFlag = NULL;
    totalMemorySize = 0ull;
    numberOfVariables = 0u;
    eventSem.Create();
    eventSem.Reset();
    syncSem.Create();
    currentIdx = 0u;
    currentChangePos = 0u;
    numberOfSignalToBeSent = 0u;
    nThreadsFinished = 0u;
    chunckCounter = 0u;
    numberOfChunks = 0u;
    serverPort = 0u;
    serverInitialPort = 0u;
    connectionTimeout = TTInfiniteWait;
    mainCpuMask = 0xffu;
    quit = 0u;
    lastTickCounter = 0u;
    msecPeriod = 0u;
}

PrioritySender2::~PrioritySender2() {
    // Auto-generated destructor stub for PrioritySender2
    // TODO Verify if manual additions are needed
    if (memory != NULL) {
        HeapManager::Free((void*&) memory);
    }
    if (changeFlag != NULL) {
        HeapManager::Free((void*&) changeFlag);
    }
    if (indexList != NULL) {
        HeapManager::Free((void*&) indexList);
    }
    eventSem.Close();
}

bool PrioritySender2::Initialise(StructuredDataI &data) {
    bool ret = MultiThreadService::Initialise(data);
    if (ret) {
        ret = data.Read("NumberOfSignalPerThread", numberOfSignalToBeSent);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define NumberOfSignalPerThread");
        }
        if (ret) {
            ret = data.Read("ServerIp", serverIpAddress);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerIp");
            }
        }
        if (ret) {
            ret = data.Read("ServerInitialPort", serverInitialPort);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerInitialPort");
            }
            else {
                serverPort = serverInitialPort;
            }
        }
        if (ret) {
            ret = data.Read("MainCpuMask", mainCpuMask);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define MainCpuMask");
            }
        }
        if (ret) {
            ret = data.Read("MsecPeriod", msecPeriod);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define MsecPeriod");
            }
        }
        if (ret) {
            uint32 timeoutTemp;
            if (data.Read("ConnectionTimeout", timeoutTemp)) {
                connectionTimeout = timeoutTemp;
            }
        }
    }
    return ret;
}

bool PrioritySender2::SetDataSource(EpicsParserAndSubscriber &dataSourceIn) {

    dataSource = &dataSourceIn;
    while (!dataSource->InitialisationDone()) {
        Sleep::Sec(1u);
    }
    totalMemorySize = dataSource->GetTotalMemorySize();
    memory = (uint8*) HeapManager::Malloc(totalMemorySize);
    numberOfVariables = dataSource->GetNumberOfVariables();
    changeFlag = (uint8*) HeapManager::Malloc(numberOfVariables);
    uint32 sentPerCycle = (numberOfPoolThreads * numberOfSignalToBeSent);
    bool ret = (numberOfVariables >= sentPerCycle);
    if (ret) {
        indexList = (uint32*) HeapManager::Malloc(numberOfVariables * sizeof(uint32));
        MemoryOperationsHelper::Set(memory, 0, totalMemorySize);
        for (uint32 i = 0u; i < numberOfVariables; i++) {
            indexList[i] = i;
        }
        numberOfChunks = (numberOfVariables / sentPerCycle);
        if ((numberOfVariables % sentPerCycle) > 0u) {
            numberOfChunks++;
        }
        nThreadsFinished = 0u;
    }
    else {
        REPORT_ERROR(ErrorManagement::InitialisationError, "The number of variables (%d) must be > than the variables sent per cycle (%d)", numberOfVariables,
                     sentPerCycle);
    }
    return ret;
}

ErrorManagement::ErrorType PrioritySender2::Start() {
    lastTickCounter = HighResolutionTimer::Counter();
    ErrorManagement::ErrorType err = MultiThreadService::Start();
    Threads::BeginThread((ThreadFunctionType) PrioritySenderCycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, mainCpuMask);

    return err;
}

ErrorManagement::ErrorType PrioritySender2::ThreadCycle(ExecutionInfo & info) {
    ErrorManagement::ErrorType err;

    if (info.GetStage() == MARTe::ExecutionInfo::StartupStage) {
        HttpChunkedStream *newClient = new HttpChunkedStream();
        newClient->Open();
        while (!(newClient->Connect(serverIpAddress.Buffer(), serverPort, connectionTimeout)) && (quit == 0u)) {
            Sleep::Sec(1);
        }
        if (err.ErrorsCleared()) {
            newClient->SetBlocking(true);
            //always use the buffer
            newClient->SetCalibReadParam(0u);

            info.SetThreadSpecificContext(reinterpret_cast<void*>(newClient));
        }
    }
    else if (info.GetStage() == MARTe::ExecutionInfo::MainStage) {
        if (quit == 0u) {
            if (syncSem.FastLock()) {
                eventSem.Reset();
                nThreadsFinished++;
                syncSem.FastUnLock();
            }
            //lock on the index list
            eventSem.Wait(TTInfiniteWait);

            HttpChunkedStream *client = reinterpret_cast<HttpChunkedStream *>(info.GetThreadSpecificContext());

            if (client != NULL) {

                uint32 listIndex;
                if (syncSem.FastLock()) {
                    listIndex = currentIdx;
                    currentIdx += numberOfSignalToBeSent;
                    currentIdx %= numberOfVariables;
                    syncSem.FastUnLock();
                }

                client->SetChunkMode(false);
                HttpProtocol hprotocol(*client);
                if (!hprotocol.MoveAbsolute("OutputOptions")) {
                    hprotocol.CreateAbsolute("OutputOptions");
                }

                StreamString param;
                param.Printf("%s:%d", serverIpAddress.Buffer(), serverPort);
                hprotocol.Write("Host", param.Buffer());
                hprotocol.Write("Connection", "keep-alive");
                hprotocol.Write("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
                hprotocol.Write("Accept-Encoding", "gzip, deflate, br");
                hprotocol.Write("Cache-Control", "no-cache");

                hprotocol.Write("Transfer-Encoding", "chunked");
                hprotocol.Write("Content-Type", "text/html");

                StreamString hstream;
                err = !hprotocol.WriteHeader(false, HttpDefinition::HSHCPut, &hstream, "/archiver");
                if (err.ErrorsCleared()) {
                    client->Flush();
                }
                if (err.ErrorsCleared()) {

                    client->SetChunkMode(true);

                    StreamStructuredData < JsonPrinter > sdata;
                    sdata.SetStream(*client);
                    uint32 sentCounter = 0u;
                    for (uint32 i = 0u; (i < numberOfSignalToBeSent) && (err.ErrorsCleared()); i++) {
                        PvDescriptor *pvDes = dataSource->GetPvDescriptors();
                        uint32 signalIndex = indexList[listIndex];
                        if (pvDes[signalIndex].numberOfElements > 0) {

                            sentCounter++;
                            uint64 offset = (pvDes[signalIndex]).offset;

                            StreamString signalName = pvDes[signalIndex].pvName;

                            void*signalPtr = &memory[offset];

                            AnyType signalAt(pvDes[signalIndex].td, 0u, signalPtr);
                            if (pvDes[signalIndex].numberOfElements > 1u) {
                                signalAt.SetNumberOfDimensions(1u);
                                signalAt.SetNumberOfElements(0u, pvDes[signalIndex].numberOfElements);
                            }

                            err = !(sdata.CreateRelative(signalName.Buffer()));
                            if (err.ErrorsCleared()) {
                                err = !(sdata.Write("Value", signalAt));
                            }
                            if (err.ErrorsCleared()) {
                                //ca_pend_event(0.01);

                                char8 timestamp[128];
                                MemoryOperationsHelper::Set(timestamp, 0, 128);
                                epicsTimeToStrftime(timestamp, 128, "%Y-%m-%d %H:%M:%S.%06f",
                                                    (epicsTimeStamp *) (&memory[offset + pvDes[signalIndex].numberOfElements * pvDes[signalIndex].memorySize]));

                                StreamString ts;
                                ts += timestamp;
                                if (ts.Size() == 0ull) {
                                    ts = "\"Undefined\"";
                                }
                                err = !(sdata.Write("TimeStamp", ts.Buffer()));
                            }
                            if (err.ErrorsCleared()) {
                                err = !(sdata.MoveToRoot());
                            }
                        }
                        listIndex++;
                        listIndex %= numberOfVariables;
                    }
                    if (err.ErrorsCleared()) {
                        client->Flush();
                    }
                    if (err.ErrorsCleared()) {
                        client->FinalChunk();
                    }
                }
            }
        }
    }
    else {
        HttpChunkedStream *client = reinterpret_cast<HttpChunkedStream *>(info.GetThreadSpecificContext());
        if (client != NULL) {
            client->Close();
            delete client;
            info.SetThreadSpecificContext(NULL);
        }
        serverPort = serverInitialPort;
    }
    return err;
}

void PrioritySender2::Quit() {
    quit = 1u;
    while (quit != 2u) {
        Sleep::Sec(1u);

    }
    Stop();
}

CLASS_REGISTER(PrioritySender2, "1.0")
}

