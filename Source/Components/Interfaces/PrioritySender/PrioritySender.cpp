/**
 * @file PrioritySender.cpp
 * @brief Source file for class PrioritySender
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
 * the class PrioritySender (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "PrioritySender.h"
#include "StreamStructuredData.h"
#include "JsonPrinter.h"
#include "AdvancedErrorManagement.h"
#include "HttpProtocol.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

void PrioritySenderCycleLoop(PrioritySender &arg) {
    arg.eventSem.Post();
    while (arg.quit == 0) {
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
            //wait the cycle time
            uint32 elapsed = (uint32)((float32)((HighResolutionTimer::Counter() - arg.lastTickCounter) * 1000u * HighResolutionTimer::Period()));

            if (elapsed < arg.msecPeriod) {
                Sleep::MSec(arg.msecPeriod - elapsed);
            }
            arg.lastTickCounter = HighResolutionTimer::Counter();
        }
        else {
            Sleep::MSec(10);
        }

    }

    while (arg.nThreadsFinished != arg.numberOfPoolThreads) {
        Sleep::Sec(1u);
    }
    arg.eventSem.Post();
}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

PrioritySender::PrioritySender() :
        MultiThreadService(embeddedMethod),
        embeddedMethod(*this, &PrioritySender::ThreadCycle) {
    // Auto-generated constructor stub for PrioritySender
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
    quit = 0;
    lastTickCounter = 0u;
    msecPeriod = 0u;
    initialised = false;
}

PrioritySender::~PrioritySender() {
    // Auto-generated destructor stub for PrioritySender
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

bool PrioritySender::Initialise(StructuredDataI &data) {
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

bool PrioritySender::SetDataSource(EpicsParserAndSubscriber &dataSourceIn) {

    bool ret = false;
    dataSource = &dataSourceIn;
    if (dataSource != NULL) {
        while (!dataSource->InitialisationDone()) {
            Sleep::Sec(1u);
        }
        totalMemorySize = dataSource->GetTotalMemorySize();
        memory = (uint8*) HeapManager::Malloc(totalMemorySize);
        numberOfVariables = dataSource->GetNumberOfVariables();
        changeFlag = (uint8*) HeapManager::Malloc(numberOfVariables);
        uint32 sentPerCycle = (numberOfPoolThreads * numberOfSignalToBeSent);
        ret = (numberOfVariables >= sentPerCycle);
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
            REPORT_ERROR(ErrorManagement::InitialisationError, "The number of variables (%d) must be > than the variables sent per cycle (%d)",
                         numberOfVariables, sentPerCycle);
        }
    }
    return ret;
}

ErrorManagement::ErrorType PrioritySender::Start() {
    lastTickCounter = HighResolutionTimer::Counter();
    ErrorManagement::ErrorType err = MultiThreadService::Start();
    Threads::BeginThread((ThreadFunctionType) PrioritySenderCycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, mainCpuMask);

    return err;
}

ErrorManagement::ErrorType PrioritySender::ThreadCycle(ExecutionInfo & info) {
    ErrorManagement::ErrorType err;

    if (info.GetStage() == MARTe::ExecutionInfo::StartupStage) {

        //be sure the main thread started
        if (!initialised) {
            eventSem.Wait(TTInfiniteWait);
            initialised = true;
        }

        HttpChunkedStream *newClient = new HttpChunkedStream();
        newClient->Open();
        REPORT_ERROR(ErrorManagement::Information, "Client connecting...");
        while (!(newClient->Connect(serverIpAddress.Buffer(), serverPort, connectionTimeout)) && (quit == 0)) {
            Sleep::Sec(1);
        }
        REPORT_ERROR(ErrorManagement::Information, "Connection established!");

        if (quit == 0) {
            if (err.ErrorsCleared()) {
                newClient->SetBlocking(true);
                //always use the buffer
                newClient->SetCalibReadParam(0u);

                info.SetThreadSpecificContext(reinterpret_cast<void*>(newClient));
            }
        }
        else {
            HttpChunkedStream *client = reinterpret_cast<HttpChunkedStream *>(info.GetThreadSpecificContext());
            if (client != NULL) {
                client->Close();
                delete client;
                info.SetThreadSpecificContext(NULL);
            }
            nThreadsFinished = numberOfPoolThreads;
        }

    }
    else if (info.GetStage() == MARTe::ExecutionInfo::MainStage) {
        if (quit == 0) {
            if (syncSem.FastLock()) {
                eventSem.Reset();
                nThreadsFinished++;
                syncSem.FastUnLock();
            }
            //lock on the index list
            eventSem.Wait(TTInfiniteWait);
            HttpChunkedStream *client = reinterpret_cast<HttpChunkedStream *>(info.GetThreadSpecificContext());

            if (client != NULL) {

                client->SetChunkMode(false);
                err = SendVariables(*client);

            }
        }

    }
    else {
        REPORT_ERROR(ErrorManagement::FatalError, "Error: close the connection");
        HttpChunkedStream *client = reinterpret_cast<HttpChunkedStream *>(info.GetThreadSpecificContext());
        if (info.GetStage() == MARTe::ExecutionInfo::BadTerminationStage) {
            REPORT_ERROR(ErrorManagement::FatalError, "Send final message");

            //send a connection-close message
            if (client != NULL) {
                SendCloseConnectionMessage(*client);
            }
        }

        if (client != NULL) {
            client->Close();
            delete client;
            client = NULL;
            info.SetThreadSpecificContext(NULL);
        }

        serverPort = serverInitialPort;
    }
    return err;
}

ErrorManagement::ErrorType PrioritySender::Stop() {
    Atomic::Increment(&quit);
    eventSem.Wait(TTInfiniteWait);
    return MultiThreadService::Stop();
}

ErrorManagement::ErrorType PrioritySender::SendVariables(HttpChunkedStream &client) {
    ErrorManagement::ErrorType err;
    HttpProtocol hprotocol(client);
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
    err = !hprotocol.WriteHeader(false, HttpDefinition::HSHCPut, &hstream, "/");
    if (err.ErrorsCleared()) {
        client.Flush();
    }
    if (err.ErrorsCleared()) {

        uint32 listIndex;
        if (syncSem.FastLock()) {
            listIndex = currentIdx;
            currentIdx += numberOfSignalToBeSent;
            currentIdx %= numberOfVariables;
            syncSem.FastUnLock();
        }

        client.SetChunkMode(true);
        uint32 sentCounter = 0u;
        PvDescriptor *pvDes = dataSource->GetPvDescriptors();
        if (pvDes != NULL) {
            for (uint32 i = 0u; (i < numberOfSignalToBeSent) && (err.ErrorsCleared()); i++) {
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

                    // err = !(sdata.CreateRelative(signalName.Buffer()));
                    StreamString var = "\"";
                    var += signalName.Buffer();
                    var += "\": ";
                    uint32 varSize = var.Size();
                    client.Write(var.Buffer(), varSize);
                    uint32 indexSize = sizeof(uint32);
                    client.Write((uint8*) (&signalIndex), indexSize);
                    uint32 totalSize = pvDes[signalIndex].memorySize * pvDes[signalIndex].numberOfElements;
                    client.Write((uint8*) signalPtr, totalSize);
                    uint32 timestampSize = sizeof(uint64);
                    uint32 tsIndex = (pvDes[signalIndex].numberOfElements * pvDes[signalIndex].memorySize);
                    uint8* timeStampPtr = (uint8*) (&memory[offset + tsIndex]);
                    client.Write((uint8*) timeStampPtr, timestampSize);
                    uint32 termSize = 2u;
                    client.Write("\n\r", termSize);
                }
                listIndex++;
                listIndex %= numberOfVariables;
            }
        }
        if (err.ErrorsCleared()) {
            err = !client.Flush();
        }
        if (err.ErrorsCleared()) {
            err = !client.FinalChunk();
        }
        if (err.ErrorsCleared()) {
            err = !hprotocol.ReadHeader();
        }
        if (err.ErrorsCleared()) {
            StreamString hstream;
            hprotocol.CompleteReadOperation(&hstream, 10000u);
        }
        if (err.ErrorsCleared()) {
            if (!hprotocol.KeepAlive()) {
                REPORT_ERROR(ErrorManagement::FatalError, "Connection complete!");
                err = ErrorManagement::Completed;
            }
        }
    }
    return err;
}

ErrorManagement::ErrorType PrioritySender::SendCloseConnectionMessage(HttpChunkedStream &client) {

    ErrorManagement::ErrorType err;
    uint32 listIndex;
    if (syncSem.FastLock()) {
        listIndex = currentIdx;
        currentIdx += numberOfSignalToBeSent;
        currentIdx %= numberOfVariables;
        syncSem.FastUnLock();
    }

    client.SetChunkMode(false);
    HttpProtocol hprotocol(client);
    if (!hprotocol.MoveAbsolute("OutputOptions")) {
        hprotocol.CreateAbsolute("OutputOptions");
    }
    StreamString param;
    param.Printf("%s:%d", serverIpAddress.Buffer(), serverPort);
    hprotocol.Write("Host", param.Buffer());
    hprotocol.Write("Connection", "close");
    hprotocol.Write("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    hprotocol.Write("Accept-Encoding", "gzip, deflate, br");
    hprotocol.Write("Cache-Control", "no-cache");
    hprotocol.Write("Transfer-Encoding", "chunked");
    hprotocol.Write("Content-Type", "text/html");
    hprotocol.SetKeepAlive(false);
    StreamString hstream;
    err = !hprotocol.WriteHeader(false, HttpDefinition::HSHCPut, &hstream, "/");
    if (err.ErrorsCleared()) {
        client.Flush();
    }
    if (err.ErrorsCleared()) {
        client.SetChunkMode(true);
        err = !client.FinalChunk();
    }
    return err;
}

CLASS_REGISTER(PrioritySender, "1.0")
}

