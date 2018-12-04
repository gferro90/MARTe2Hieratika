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
#include "HttpChunkedStream.h"
#include "StreamStructuredData.h"
#include "JsonPrinter.h"
#include "AdvancedErrorManagement.h"
#include "HttpProtocol.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

static void CycleLoop(PrioritySender &arg) {
    while (arg.quit == 0u) {
        uint32 nThreadsFinishedTmp;
        if (arg.syncSem.FastLock()) {
            nThreadsFinishedTmp = arg.nThreadsFinished;
            arg.nThreadsFinished = 0u;
            arg.syncSem.FastUnLock();
        }

        if (nThreadsFinishedTmp == arg.numberOfPoolThreads) {
            arg.dataSource->Synchronise(arg.memory, arg.changeFlag);
            if (arg.chunckCounter >= (arg.numberOfChunks)) {
                //insertion sort if changed
                for (uint32 i = 0u; i < arg.numberOfVariables; i++) {

                    uint32 index = (arg.currentIdx + i) % arg.numberOfVariables;
                    uint32 signalIdx = arg.indexList[index];
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
            if (arg.currentChangePos > (arg.numberOfSignalToBeSent * arg.numberOfPoolThreads)) {
                arg.currentChangePos -= (arg.numberOfSignalToBeSent * arg.numberOfPoolThreads);
            }
            else {
                arg.currentChangePos = 0u;
            }

            if (arg.currentChangePos > (arg.numberOfVariables - arg.numberOfSignalToBeSent)) {
                //if almost all the signal are changing do a cycle of FIFO again
                arg.currentChangePos = 0u;
                arg.chunckCounter = 0u;
            }

        }
        uint32 elapsed = (uint32)((float32)((HighResolutionTimer::Counter() - arg.lastTickCounter)*1000u * HighResolutionTimer::Period()));

        printf("elapsed %d %lld %lld\n", elapsed, HighResolutionTimer::Counter(), arg.lastTickCounter );
        if (elapsed < arg.msecPeriod) {
            Sleep::MSec(arg.msecPeriod-elapsed);
        }
        arg.lastTickCounter = HighResolutionTimer::Counter();

//        Sleep::Sec(1);
        arg.eventSem.Post();

    }
    arg.quit == 2u;
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
    connectionTimeout = TTInfiniteWait;
    mainCpuMask = 0xffu;
    quit = 0u;
    lastTickCounter = 0u;
    msecPeriod = 0u;
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
            ret = data.Read("ServerInitialPort", serverPort);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerInitialPort");
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

    dataSource = &dataSourceIn;
    while(!dataSource->InitialisationDone()){
        Sleep::Sec(1u);
    }    totalMemorySize = dataSource->GetTotalMemorySize();
    memory = (uint8*)HeapManager::Malloc(totalMemorySize);
    numberOfVariables = dataSource->GetNumberOfVariables();
    changeFlag = HeapManager::Malloc(numberOfVariables);
    uint32 sentPerCycle = (numberOfPoolThreads * numberOfSignalToBeSent);
    bool ret = (numberOfVariables >= sentPerCycle);
    if (ret) {
        indexList = HeapManager::Malloc(numberOfVariables * sizeof(uint32));
        MemoryOperationsHelper::Set(memory, 0, totalMemorySize);
        for (uint32 i = 0u; i < numberOfVariables; i++) {
            indexList[i] = i;
        }
        numberOfChunks = (numberOfVariables / numberOfSignalToBeSent);
        if ((numberOfVariables % numberOfSignalToBeSent) > 0u) {
            numberOfChunks++;
        }
        nThreadsFinished=numberOfPoolThreads;
    }
    else {
        REPORT_ERROR(ErrorManagement::InitialisationError, "The number of variables (%d) must be > than the variables sent per cycle (%d)", numberOfVariables,
                     sentPerCycle);
    }
    return ret;
}

ErrorManagement::ErrorType PrioritySender::Start() {
    lastTickCounter = HighResolutionTimer::Counter();
    REPORT_ERROR(ErrorManagement::InitialisationError, "lastTickCounter %d", lastTickCounter);
    ErrorManagement::ErrorType err = MultiThreadService::Start();
    Threads::BeginThread((ThreadFunctionType) CycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, mainCpuMask);

    return err;
}

ErrorManagement::ErrorType PrioritySender::ThreadCycle(ExecutionInfo & info) {
    ErrorManagement::ErrorType err;

    if (info.GetStage() == MARTe::ExecutionInfo::StartupStage) {
        HttpChunkedStream *newClient = new HttpChunkedStream();
        newClient->Open();
        if (syncSem.FastLock()) {
            while (!(newClient->Connect(serverIpAddress.Buffer(), serverPort, connectionTimeout))){
                Sleep::MSec(100);
            }
            serverPort++;
            syncSem.FastUnLock();
        }
        if (err.ErrorsCleared()) {
            newClient->SetBlocking(true);
            //always use the buffer
            newClient->SetCalibReadParam(0u);

            info.SetThreadSpecificContext(reinterpret_cast<void*>(newClient));
        }
    }
    else if (info.GetStage() == MARTe::ExecutionInfo::MainStage) {
        //lock on the index list
        eventSem.ResetWait(TTInfiniteWait);
        //Sleep::Sec(1);

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
            REPORT_ERROR(ErrorManagement::Information, "Here2");

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
            hprotocol.WriteHeader(false, HttpDefinition::HSHCPut, &hstream, "/archiver");
            client->Flush();
            client->SetChunkMode(true);

            StreamStructuredData < JsonPrinter > sdata;
            sdata.SetStream(*client);

            for (uint32 i = 0u; i < numberOfSignalToBeSent; i++) {
                PvDescriptor *pvDes = dataSource->GetPvDescriptors();
                uint32 signalIndex = indexList[listIndex];
                REPORT_ERROR(ErrorManagement::Information, "signalIndex %d",signalIndex);

                uint64 offset = (pvDes[signalIndex]).offset;

                StreamString signalName = pvDes[signalIndex].pvName;
                REPORT_ERROR(ErrorManagement::Information, "offset %d",offset);

                void*signalPtr = &memory[offset];

                TypeDescriptor td;
                StreamString typeStr = dbf_type_to_text(pvDes[signalIndex].pvType);
                REPORT_ERROR(ErrorManagement::Information, "typeStr %s",typeStr.Buffer());

                if (typeStr == "DBF_DOUBLE") {
                    td = Float64Bit;
                }
                else if (typeStr == "DBF_FLOAT") {
                    td = Float32Bit;
                }
                else if (typeStr == "DBF_CHAR") {
                    td = SignedInteger8Bit;
                }
                else if (typeStr == "DBF_UCHAR") {
                    td = UnsignedInteger8Bit;
                }
                else if (typeStr == "DBF_SHORT") {
                    td = SignedInteger16Bit;
                }
                else if (typeStr == "DBF_USHORT") {
                    td = UnsignedInteger16Bit;
                }
                else if (typeStr == "DBF_LONG") {
                    td = SignedInteger32Bit;
                }
                else if (typeStr == "DBF_LONG") {
                    td = UnsignedInteger32Bit;
                }
                else {
                    td = Float64Bit;
                }

                AnyType signalAt(td, 0u, signalPtr);
                if (pvDes[signalIndex].numberOfElements > 1u) {
                    signalAt.SetNumberOfDimensions(1u);
                    signalAt.SetNumberOfElements(0u, pvDes[signalIndex].numberOfElements);
                }

                //StreamString x;
                //x.Printf("%J!", signalAt);
                //REPORT_ERROR(ErrorManagement::Information, "Print %s", x.Buffer());
                sdata.CreateRelative(signalName.Buffer());
                sdata.Write("Value", signalAt);
                REPORT_ERROR(ErrorManagement::Information, "Here 0");

                //ca_pend_event(0.01);
                REPORT_ERROR(ErrorManagement::Information, "Here 1");
                printf("%d %d\n", signalIndex, numberOfVariables);

                char8 timestamp[128];
                MemoryOperationsHelper::Set(timestamp, 0, 128);
                epicsTimeToStrftime(timestamp, 128, "%Y-%m-%d %H:%M:%S.%06f",
                                    (epicsTimeStamp *) (&memory[offset + pvDes[signalIndex].numberOfElements * pvDes[signalIndex].memorySize]));

                printf("%s\n", timestamp);
                REPORT_ERROR(ErrorManagement::Information, "Here 2");

                StreamString ts;
                /*if(timestamp[0]!='\"'){
                    ts += "\"";
                }*/
                ts += timestamp;
                /*if(timestamp[0]!='\"'){
                    ts += "\"";
                }*/
                if (ts.Size() == 0ull) {
                    ts = "\"Undefined\"";
                }
                sdata.Write("TimeStamp", ts.Buffer());
                sdata.MoveToRoot();
                listIndex++;
                listIndex %= numberOfVariables;
            }
            if (syncSem.FastLock()) {
                client->Flush();
                client->FinalChunk();

                nThreadsFinished++;
                syncSem.FastUnLock();
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

    }
    return err;
}

void PrioritySender::Quit() {
    quit = 1u;
    Stop();
    while (quit != 2u) {
        Sleep::Sec(1u);
    }
}

CLASS_REGISTER(PrioritySender, "1.0")
}

