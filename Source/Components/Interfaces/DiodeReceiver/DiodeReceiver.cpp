/**
 * @file DiodeReceiver.cpp
 * @brief Source file for class DiodeReceiver
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

 * @details This source file contains the definition of all the methods for
 * the class DiodeReceiver (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "DiodeReceiver.h"
#include "AdvancedErrorManagement.h"
#include "JsonParser.h"
#include "MultiClientEmbeddedThread.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
#define MAX_ARR_LEN 20000
#define INDEX_INIT_ID 0xFFFFFFFFu
#define INDEX_NOT_FOUND_ID 0xFFFFFFFEu
#define NUMBER_OF_TYPES 11u
static TypeDescriptor descriptors[NUMBER_OF_TYPES] = { SignedInteger8Bit, UnsignedInteger8Bit, SignedInteger16Bit, UnsignedInteger16Bit, SignedInteger32Bit,
        UnsignedInteger32Bit, SignedInteger64Bit, UnsignedInteger64Bit, Float32Bit, Float64Bit, TypeDescriptor(false, CArray, MAX_STRING_SIZE * 8u) };

static bool GetVariable(File &xmlFile,
                        StreamString &variable) {
    StreamString token;
    char8 term;
    bool ret = xmlFile.GetToken(token, "<", term, " \n");

    token.SetSize(0ull);
    while (variable == "" && (ret)) {
        token.SetSize(0ull);
        ret = xmlFile.GetToken(token, ">", term, " \n");
        //printf("token2=%s\n", token.Buffer());
        token.SetSize(0ull);
        ret &= xmlFile.GetToken(variable, "<", term, " \n");
    }
    ret &= xmlFile.GetToken(token, ">", term, "");

    return ret;
}

static int cainfo(chid &pvChid,
                  char8 *pvName,
                  chtype &type,
                  uint32 &numberOfElements) {
    uint32 nElems = 0u;
    enum channel_state state;
    const char8 *stateStrings[] = { "never connected", "previously connected", "connected", "closed" };

    state = ca_state(pvChid);
    nElems = ca_element_count(pvChid);
    type = ca_field_type(pvChid);

    if (state != 2) {
        nElems = 0u;
        printf("The variable %s is not connected, state=%s\n", pvName, stateStrings[state]);
    }

    numberOfElements = nElems;

    return 0;
}

void DiodeReceiverCycleLoop(DiodeReceiver &arg) {

    uint32 threadId;
    uint32 beg = 0u;
    uint32 end = arg.numberOfVariables;
    bool ret = true;

    if (arg.syncSem.FastLock()) {
        threadId = arg.threadCnt;
        arg.threadCnt++;
        arg.syncSem.FastUnLock();
    }
    ret = (ca_context_create(ca_enable_preemptive_callback) == ECA_NORMAL);

    if (ret) {
        arg.totalMemorySize = 0u;
        uint32 nThreads = arg.numberOfInitThreads;
        uint32 numberOfVarsPerThread = (arg.numberOfVariables / nThreads);

        beg = threadId * numberOfVarsPerThread;
        end = (threadId + 1) * numberOfVarsPerThread;

        if ((arg.numberOfVariables - end) < nThreads) {
            end = arg.numberOfVariables;
        }
        printf("Starting Thread from %d to %d\n", beg, end);

        for (uint32 n = beg; (n < end); n++) {
            /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
            ret = (ca_create_channel(arg.pvs[n].pvName, NULL, NULL, 20u, &arg.pvs[n].pvChid) == ECA_NORMAL);
            ca_pend_io(1);
            if (ret) {
                arg.pvs[n].numberOfElements = 0u;
                cainfo(arg.pvs[n].pvChid, arg.pvs[n].pvName, arg.pvs[n].pvType, arg.pvs[n].numberOfElements);
                ca_pend_io(1);

                arg.pvs[n].totalSize = 0;
                arg.pvs[n].at = voidAnyType;
                if (arg.pvs[n].numberOfElements > arg.maxArraySize) {
                    arg.pvs[n].numberOfElements = 0u;
                }
                if (arg.pvs[n].numberOfElements > 0u) {
                    const char8* epicsTypeName = dbf_type_to_text(arg.pvs[n].pvType);
                    //printf("%s: nElems=%d, type=%s\n", pvs[n].pvName, pvs[n].numberOfElements, epicsTypeName);
                    if (StringHelper::Compare(epicsTypeName, "DBF_DOUBLE") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(float64)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(Float64Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(float32)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(Float32Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(int32)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(SignedInteger32Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(uint32)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(UnsignedInteger32Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(int16)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(SignedInteger16Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(uint16)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(UnsignedInteger16Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(int8)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(SignedInteger8Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
                        arg.pvs[n].totalSize = (sizeof(uint8)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(UnsignedInteger8Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_STRING") == 0) {

                        TypeDescriptor td;
                        td.numberOfBits = MAX_STRING_SIZE * 8u;
                        td.isStructuredData = false;
                        td.type = CArray;
                        td.isConstant = false;

                        arg.pvs[n].totalSize = (sizeof(char8)) * MAX_STRING_SIZE * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(td, 0u, (void*) NULL);
                    }
                    else {
                        arg.pvs[n].totalSize = (sizeof(float64)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(Float64Bit, 0u, (void*) NULL);

                        arg.pvs[n].pvType = DBF_DOUBLE;

                    }
                    if (arg.pvs[n].numberOfElements > 1u) {
                        arg.pvs[n].at.SetNumberOfDimensions(1u);
                        arg.pvs[n].at.SetNumberOfElements(0u, arg.pvs[n].numberOfElements);
                    }

                }
            }
        }

    }
    else {
        printf("ca_enable_preemptive_callback failed\n");
    }
    arg.eventSem.Reset();
    Atomic::Increment(&arg.threadSetContext);
    arg.eventSem.Wait(TTInfiniteWait);

    while (arg.quit == 0) {
        //sync here
        if (threadId == 0u) {
            printf("Synchronizing\n");
            arg.Synchronise(arg.memory2, arg.changeFlag2);
        }
        for (uint32 index = beg; index < end; index++) {

            if (arg.changeFlag2[index] == 1) {
                if (ca_array_put(((arg.pvs[index].pvType) | (0x8000u)), arg.pvs[index].numberOfElements, arg.pvs[index].pvChid,
                                 arg.memory2 + arg.pvs[index].offset) != ECA_NORMAL) {
                    printf("ca_put failed for PV: %s\n", arg.pvs[index].pvName);
                }
                (void) ca_pend_io(0.1);
            }
        }

        //wait the cycle time
        uint32 elapsed = (uint32)(((float32)(HighResolutionTimer::Counter() - arg.lastTickCounter[threadId]) * 1000u * HighResolutionTimer::Period()));

        if (elapsed < arg.msecPeriod) {
            Sleep::MSec(arg.msecPeriod - elapsed);
        }
        arg.lastTickCounter[threadId] = HighResolutionTimer::Counter();
    }

    for (uint32 n = beg; (n < end); n++) {
        (void) ca_clear_channel(arg.pvs[n].pvChid);
    }

    ca_detach_context();
    ca_context_destroy();

    printf("Init thread terminated\n");

}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

DiodeReceiver::DiodeReceiver() :
        MultiClientService(embeddedMethod),
        embeddedMethod(*this, &DiodeReceiver::ServerCycle) {
// Auto-generated constructor stub for DiodeReceiver
// TODO Verify if manual additions are needed
    serverPort = 0u;
    acceptTimeout = TTInfiniteWait;
    syncSem.Create();
    pvs = NULL;
    numberOfVariables = 0u;
    numberOfCpus = 4u;
    memory = NULL;
    memory2 = NULL;
    memoryPrec = NULL;
    threadSetContext = 0u;
    changeFlag = NULL;
    changeFlag2 = NULL;
    quit = 0;
    totalMemorySize = 0u;
    eventSem.Create();
    eventSem.Reset();
    threadCnt = 0u;
    pvMapping = NULL;
    maxNumberOfVariables = 0xFFFFFFFFu;
    lastTickCounter = NULL;
    msecPeriod = 0u;
    currentCpuMask = 1u;
    maxArraySize = MAX_ARR_LEN;
}

DiodeReceiver::~DiodeReceiver() {
// Auto-generated destructor stub for DiodeReceiver
// TODO Verify if manual additions are needed

    if (pvs != NULL) {
        delete[] pvs;
    }
    if (memory != NULL) {
        HeapManager::Free((void*&) memory);
    }
    if (memory2 != NULL) {
        HeapManager::Free((void*&) memory2);
    }
    if (memoryPrec != NULL) {
        HeapManager::Free((void*&) memory);
    }

    if (changeFlag != NULL) {
        HeapManager::Free((void*&) changeFlag);
    }
    if (changeFlag2 != NULL) {
        HeapManager::Free((void*&) changeFlag2);
    }
    if (pvMapping != NULL) {
        delete[] pvMapping;
    }
    if (lastTickCounter != NULL) {
        delete[] lastTickCounter;
    }
}

bool DiodeReceiver::Initialise(StructuredDataI &data) {
    bool ret = MultiClientService::Initialise(data);
    if (ret) {

        ret = data.Read("ServerPort", serverPort);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerPort");
        }
        if (ret) {
            StreamString xmlFilePath;
            ret = data.Read("InputFilePath", xmlFilePath);
            StreamString firstVariableName;

            if (ret) {
                ret = data.Read("FirstVariableName", firstVariableName);
                if (!ret) {
                    REPORT_ERROR(ErrorManagement::InitialisationError, "Please define FirstVariableName");
                }
            }
            else {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define InputFilePath");
            }

            if (ret) {
                ret = data.Read("NumberOfCpus", numberOfCpus);
                if (!ret) {
                    REPORT_ERROR(ErrorManagement::InitialisationError, "Please define NumberOfCpus");
                }
            }

            if (ret) {

                if (!data.Read("NumberOfInitThreads", numberOfInitThreads)) {
                    numberOfInitThreads = 1u;
                }
                if (!data.Read("MaxNumberOfVariables", maxNumberOfVariables)) {
                    maxNumberOfVariables = 0xFFFFFFFFu;
                }
                if (!data.Read("MsecPeriod", msecPeriod)) {
                    msecPeriod = 1000u;
                }
                uint32 readTimeoutTmp = 10000;
                if (!data.Read("ReadTimeout", readTimeoutTmp)) {
                    readTimeoutTmp = 10000;
                }
                readTimeout = readTimeoutTmp;
                //open the xml file
                File xmlFile;
                if (!xmlFile.Open(xmlFilePath.Buffer(), File::ACCESS_MODE_R)) {
                    printf("Failed opening file %s\n", xmlFilePath.Buffer());
                    return false;
                }

                xmlFile.Seek(0ull);

                StreamString variable;
                bool start = false;
                //get the number of variables
                numberOfVariables = 0u;
                xmlFile.Seek(0ull);
                while (GetVariable(xmlFile, variable)) {

                    if (variable == firstVariableName) {
                        start = true;
                    }
                    if (start) {
                        //printf("variable=%s\n", variable.Buffer());
                        numberOfVariables++;
                        if (numberOfVariables >= maxNumberOfVariables) {
                            break;
                        }
                    }
                    variable.SetSize(0ull);
                }

                //create the pv descriptors
                pvs = new PvRecDescriptor[numberOfVariables];
                variable.SetSize(0ull);

                xmlFile.Seek(0ull);
                uint32 counter = 0u;

                variable.SetSize(0ull);
                start = false;
                while (GetVariable(xmlFile, variable)) {

                    if (!start) {
                        if (variable == firstVariableName) {
                            start = true;
                        }
                    }
                    if (start) {
                        pvs[counter].at = voidAnyType;
                        StringHelper::Copy(pvs[counter].pvName, variable.Buffer());
                        for (uint32 j = counter; j > 0u; j--) {
                            if (StringHelper::Compare(pvs[j].pvName, pvs[j - 1].pvName) == 1) {
                                char8 pvName[PV_NAME_MAX_SIZE_REC];
                                StringHelper::Copy(pvName, pvs[j - 1].pvName);
                                StringHelper::Copy(pvs[j - 1].pvName, pvs[j].pvName);
                                StringHelper::Copy(pvs[j].pvName, pvName);
                            }
                            else {
                                break;
                            }
                        }
                        counter++;
                        if (counter >= maxNumberOfVariables) {
                            break;
                        }
                    }

                    variable.SetSize(0ull);
                }
                xmlFile.Close();

            }
        }

        if (ret) {
            uint32 acceptTimeoutTmp;
            if (data.Read("AcceptTimeout", acceptTimeoutTmp)) {
                acceptTimeout = acceptTimeoutTmp;
            }
        }

        if (ret) {
            if (!data.Read("MaxArraySize", maxArraySize)) {
                maxArraySize = MAX_ARR_LEN;
            }
        }
    }
    return ret;
}

ErrorManagement::ErrorType DiodeReceiver::Start() {
    uint32 cpuMask = ((1 << numberOfCpus) - 1u);

    for (uint32 i = 0u; i < numberOfInitThreads; i++) {
        Threads::BeginThread((ThreadFunctionType) DiodeReceiverCycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, cpuMask);
        //Threads::SetPriority(tid, Threads::RealTimePriorityClass, 15);
    }
    while ((uint32) threadSetContext < numberOfInitThreads) {
        Sleep::Sec(1);
    }

    totalMemorySize = 0u;
    for (uint32 n = 0u; n < numberOfVariables; n++) {
        pvs[n].offset = totalMemorySize;
        totalMemorySize += (pvs[n].totalSize + sizeof(uint64));

    }

    memory = (uint8*) HeapManager::Malloc(totalMemorySize);
    memory2 = (uint8*) HeapManager::Malloc(totalMemorySize);
    memoryPrec = (uint8*) HeapManager::Malloc(totalMemorySize);
    changeFlag = (uint8*) HeapManager::Malloc(numberOfVariables);
    changeFlag2 = (uint8*) HeapManager::Malloc(numberOfVariables);
    pvMapping = new uint32[numberOfVariables];

    MemoryOperationsHelper::Set(memory, 0, totalMemorySize);
    MemoryOperationsHelper::Set(memory2, 0, totalMemorySize);
    MemoryOperationsHelper::Set(memoryPrec, 0, totalMemorySize);
    MemoryOperationsHelper::Set(changeFlag, 0, numberOfVariables);
    MemoryOperationsHelper::Set(changeFlag2, 0, numberOfVariables);
    for (uint32 n = 0u; (n < numberOfVariables); n++) {
        pvMapping[n] = INDEX_INIT_ID;
        pvs[n].at.SetDataPointer(memory + pvs[n].offset);

    }

    ErrorManagement::ErrorType err;
    server.Open();

    if (err.ErrorsCleared()) {
        err = !(server.Listen(serverPort, 255));

        if (err.ErrorsCleared()) {
            err = MultiClientService::Start();
        }
    }
    if (err.ErrorsCleared()) {
        lastTickCounter = new uint64[numberOfInitThreads];
        for (uint32 i = 0u; i < numberOfInitThreads; i++) {
            lastTickCounter[i] = HighResolutionTimer::Counter();
        }
        eventSem.Post();
    }
    return err;
}

ErrorManagement::ErrorType DiodeReceiver::Stop() {
    Atomic::Increment(&quit);
    eventSem.Post();
    REPORT_ERROR(ErrorManagement::Information, "called DiodeReceiver::Stop");
    //Sleep::Sec(readTimeout.GetTimeoutMSec()+5);
    ErrorManagement::ErrorType err= MultiClientService::Stop();
    REPORT_ERROR(ErrorManagement::Information, "Stopped MultiClientService");
    return err;
}

ErrorManagement::ErrorType DiodeReceiver::AddThread() {

    ErrorManagement::ErrorType err;
    err.illegalOperation = (threadPool.Size() >= maxNumberOfThreads);
    if (err.ErrorsCleared()) {
        ReferenceT < MultiClientEmbeddedThread > thread(new (NULL) MultiClientEmbeddedThread(method, *this));
        err.fatalError = !thread.IsValid();
        if (err.ErrorsCleared()) {
            thread->SetPriorityClass(GetPriorityClass());
            thread->SetPriorityLevel(GetPriorityLevel());
            thread->SetCPUMask(currentCpuMask);
            currentCpuMask <<= 1u;
            if (currentCpuMask >= (1u << numberOfCpus)) {
                currentCpuMask = 1u;
            }
            thread->SetTimeout(GetTimeout());
            StreamString tname;
            if (GetName() != NULL) {
                (void) tname.Printf("%s_%d", GetName(), HighResolutionTimer::Counter32());
            }
            else {
                (void) tname.Printf("Thread_%d", HighResolutionTimer::Counter32());
            }
            thread->SetName(tname.Buffer());
            err = thread->Start();
        }

        if (err.ErrorsCleared()) {
            err.fatalError = !threadPool.Insert(thread);
        }

    }
    return err;
}

ErrorManagement::ErrorType DiodeReceiver::ClientService(TCPSocket * const commClient) {

    ErrorManagement::ErrorType err;
    if (quit == 0) {
        //do the work
        //get the full message
        HttpProtocol protocol(*commClient);

        //discard the header
        err = !(protocol.ReadHeader());

        StreamString payload;
        StreamString varName;
        StreamString varValue;
        StreamString varTs;
        StreamString varIndex;

        uint32 receivedIndex = 0u;
        uint32 receivedSize = 0u;
        uint8 receivedTypeId = 0u;
        uint32 index = INDEX_INIT_ID;
        uint32 receivedOffset = 0u;

        bool isChunked = true;
        uint32 contentLength = 0u;

        if (err.ErrorsCleared()) {
            if (protocol.MoveAbsolute("InputOptions")) {
                if (protocol.Read("Content-Length", contentLength)) {
                    isChunked = false;
                    if (contentLength == 0u) {
                        protocol.SetKeepAlive(false);
                    }
                }
                protocol.MoveToAncestor(1u);
            }
        }

        if (err.ErrorsCleared()) {

            uint32 chunkSize = (isChunked) ? (0u) : (32u);
            //REPORT_ERROR(ErrorManagement::Information, "Chunked %d %d", (uint32) isChunked, contentLength);

            do {
                err = ReadNewChunk(commClient, payload, isChunked, chunkSize, contentLength);
                if (err.ErrorsCleared()) {
                    if (chunkSize > 0) {
                        if (isChunked) {
                            //read the \r\n
                            uint32 size = 2;
                            char8 buff[2];
                            err = !(commClient->Read(buff, size));
                        }
                        bool ok = err.ErrorsCleared();

                        //consume all the complete variables in this payload
                        while (ok) {
                            payload.Seek(0);
                            varName.SetSize(0);
                            uint32 processedSize = 0u;
                            const char8 *dataPtr = NULL;
                            bool controlOk = true;
                            ok = ReadVarNameAndIndex(payload, varName, receivedIndex, receivedTypeId, receivedSize, receivedOffset, processedSize, dataPtr);

                            if (ok) {
                                ok = GetLocalIndex(payload, varName, receivedIndex, receivedSize, receivedOffset, index, processedSize, controlOk);
                            }

                            if (ok) {
                                ReadVarValueAndSkip(payload, dataPtr, index, processedSize, receivedTypeId, receivedOffset, receivedSize, controlOk);
                            }
                        }
                    }
                }
            }
            while ((chunkSize > 0u) && (err.ErrorsCleared()));
            if (err.ErrorsCleared()) {
                if (isChunked) {
                    StreamString line;
                    err = !(commClient->GetLine(line, false));
                }
            }
        }

        if (err.ErrorsCleared()) {
            err = SendOkReplyMessage(protocol, commClient);
        }
        else {
            SendErrorReplyMessage(protocol, commClient);
        }
        Sleep::MSec(10);
    }

    return err;

}

ErrorManagement::ErrorType DiodeReceiver::ServerCycle(MARTe::ExecutionInfo & information) {
    ErrorManagement::ErrorType err;

    if (information.GetStage() == MARTe::ExecutionInfo::StartupStage) {

    }
    else if (information.GetStage() == MARTe::ExecutionInfo::MainStage) {
        if (quit == 0) {
            if (information.GetStageSpecific() == MARTe::ExecutionInfo::WaitRequestStageSpecific) {
                TCPSocket *newClient = new TCPSocket();
                if (err.ErrorsCleared()) {
                    REPORT_ERROR(ErrorManagement::Information, "Waiting new connection");
                    if (server.WaitConnection(acceptTimeout, newClient) == NULL) {
                        err = MARTe::ErrorManagement::Timeout;
                        delete newClient;
                    }
                    else {
                        REPORT_ERROR(ErrorManagement::Information, "Connection established!");
                        newClient->SetCalibReadParam(0xFFFFFFFFu);
                        newClient->SetTimeout(readTimeout);
                        information.SetThreadSpecificContext(reinterpret_cast<void*>(newClient));
                        err = MARTe::ErrorManagement::NoError;
                    }
                }
            }
            if (information.GetStageSpecific() == MARTe::ExecutionInfo::ServiceRequestStageSpecific) {
                TCPSocket *newClient = reinterpret_cast<TCPSocket *>(information.GetThreadSpecificContext());
                err = ClientService(newClient);
                //if error the client is deleted here
                if (!err.ErrorsCleared()) {
                    information.SetThreadSpecificContext(reinterpret_cast<void*>(NULL));
                }
            }
        }
    }
    else {
        REPORT_ERROR(ErrorManagement::Information, "MultiClientService Stopped");
        TCPSocket *newClient = reinterpret_cast<TCPSocket *>(information.GetThreadSpecificContext());
        if (newClient != NULL) {
            HttpProtocol protocol(*newClient);
            protocol.SetKeepAlive(false);
            err = SendOkReplyMessage(protocol, newClient);
        }
        err = ErrorManagement::Completed;
    }

    return err;
}

bool DiodeReceiver::Synchronise(uint8 *memoryOut,
                                uint8 *changedFlags) {
    if (syncSem.FastLock()) {
        MemoryOperationsHelper::Copy(memoryOut, memory, totalMemorySize);
        MemoryOperationsHelper::Copy(changedFlags, changeFlag, numberOfVariables);
        MemoryOperationsHelper::Set(changeFlag, 0, numberOfVariables);
        syncSem.FastUnLock();
    }
    return true;

}

uint32 DiodeReceiver::GetNumberOfVariables() {
    return numberOfVariables;
}

PvRecDescriptor * DiodeReceiver::GetPvDescriptors() {
    return pvs;
}

uint64 DiodeReceiver::GetTotalMemorySize() {
    return totalMemorySize;
}

bool DiodeReceiver::InitialisationDone() {
    return ((uint32) threadSetContext >= numberOfInitThreads);
}

bool DiodeReceiver::GetLocalVariableIndex(const char8 *varName,
                                          uint32 &index) {

    uint32 range_1 = numberOfVariables;
    uint32 range = (numberOfVariables / 2);
    index = range;
//bool done = false;
    bool ok = (varName != NULL);
    if (ok) {
        ok = false;
        while ((range_1 > 0) && (!ok)) {
            int32 res = 0;
            if (index >= numberOfVariables) {
                res = 2;
            }
            else {
                res = StringHelper::Compare(pvs[index].pvName, varName);
            }
            if (res == 0) {
                ok = true;
            }
            else if (res == 2) {
                uint32 rem = range % 2;
                range_1 = range;
                if (rem == 1) {
                    index--;
                }
                range = (range_1 / 2);
                index -= range;
            }
            else if (res == 1) {
                //this means that they are equal
                uint32 rem_1 = range_1 % 2;
                range_1 = range;
                if (rem_1 == 0) {
                    range_1--;
                    //one less
                }
                range = (range_1 / 2);
                index += (range + 1);
            }
        }
    }
    return ok;
}

ErrorManagement::ErrorType DiodeReceiver::ReadNewChunk(TCPSocket * const commClient,
                                                       StreamString &payload,
                                                       bool isChunked,
                                                       uint32 &chunkSize,
                                                       uint32 &contentLength) {
    char8 buff[1024];
    StreamString line;
    ErrorManagement::ErrorType err;

    if (isChunked) {
        //get the line
        err = !(commClient->GetLine(line, false));
        if (err.ErrorsCleared()) {

            if (line.Size() > 0ull) {
                char8 lastChar = line[line.Size() - 1];
                if (lastChar == '\r') {
                    //remove the \r
                    line.SetSize(line.Size() - 1);
                }
                else {
                    REPORT_ERROR(ErrorManagement::Information, "last char %c", lastChar);
                }
            }
            //get the chunk size
            StreamString toConv = "0x";
            toConv += line;
            TypeConvert(chunkSize, toConv);
            line.SetSize(0ull);
        }
    }
    else {
        if (contentLength < chunkSize) {
            chunkSize = contentLength;
        }
        contentLength -= chunkSize;
    }

    if (err.ErrorsCleared()) {
        uint32 sizeRead = 0u;
        //append to payload the chunk
        payload.Seek(payload.Size());
        while ((sizeRead < chunkSize) && (err.ErrorsCleared())) {
            MemoryOperationsHelper::Set(buff, '\0', 1024);
            uint32 sizeToRead = chunkSize - sizeRead;
            if (sizeToRead > 1023) {
                sizeToRead = 1023;
            }
            err = !commClient->Read(buff, sizeToRead);

            if (err.ErrorsCleared()) {
                payload.Write(buff, sizeToRead);
                sizeRead += sizeToRead;
            }

        }
    }
    return err;

}

bool DiodeReceiver::ReadVarNameAndIndex(StreamString &payload,
                                        StreamString &varName,
                                        uint32 &receivedIndex,
                                        uint8 &receivedTypeId,
                                        uint32 &receivedSize,
                                        uint32 &receivedOffset,
                                        uint32 &processedSize,
                                        const char8 * &dataPtr) {

    const char8 *pattern = "\": ";
    uint32 patternSize = StringHelper::Length(pattern);

    bool ok = (payload.Size() > 0u);
    if (ok) {
        //error... resync
        if ((payload.Buffer())[0] != '\"') {
            //this case we have to find a " that is not a pattern
            REPORT_ERROR(ErrorManagement::Information, "Payload not in sync: resync %d |%s|\n", payload.Size(), payload.Buffer());
            bool found = false;
            uint32 i = 0u;
            while ((i < payload.Size()) && (!found)) {
                found = ((payload.Buffer())[i] == '\"');
                if (found) {
                    uint32 currentSize = (payload.Size() - i);
                    //consume until the "
                    payload.Seek(0ull);
                    REPORT_ERROR(ErrorManagement::Information, "Payload not in sync: resync %d %d\n", i, currentSize);
                    MemoryOperationsHelper::Copy((void*) payload.Buffer(), payload.Buffer() + i, currentSize);
                    payload.SetSize(currentSize);
                    payload += "";

                    ok = (payload.Size() >= patternSize);
                    if (ok) {
                        //should be differen than the pattern!
                        found = (StringHelper::CompareN(payload.Buffer(), pattern, patternSize) != 0);
                        //exit if ok is true, but if false continue from the next
                        i = 0u;
                    }
                    else
                        break; // in this case we need a refill
                }
                i++;
            }
            //not found and no need to refill... consume the payload
            //found && ok --> go on
            //found && !ok --> need a refill
            //!found && ok --> need a refill
            //!found && !ok --> need a refill

            if (ok && (!found)) {
                payload.Seek(0ull);
                payload.SetSize(0ull);
                payload += "";
                //set to refill
                ok = false;
            }
        }
    }

    if (ok) {
        //find the pattern here
        bool found = false;
        uint32 i = 1u;
        while ((i < payload.Size()) && (!found)) {
            found = ((payload.Buffer())[i] == '\"');
            if (found) {
                // see if there is enough space
                uint32 currentSize = (payload.Size() - i);
                ok = (currentSize >= patternSize);
                if (ok) {
                    found = (StringHelper::CompareN(payload.Buffer() + i, pattern, patternSize) == 0);
                    if (!found) {
                        //cannot find a " in the name... resync
                        REPORT_ERROR(ErrorManagement::Information, "The var name cannot contain \": resync %d %d\n", i, currentSize);
                        payload.Seek(0ull);
                        MemoryOperationsHelper::Copy((void*) payload.Buffer(), payload.Buffer() + i, currentSize);
                        payload.SetSize(currentSize);
                        payload += "";
                        i = 0u;
                    }
                    else {
                        dataPtr = (payload.Buffer() + i);
                    }
                }
                else
                    break; // in this case we need a refill
            }
            i++;
        }

        //found and ok --> go on
        //found and !ok --> need to refill
        //!found --> need to refill

        //need a refill if not found
        if (!found) {
            ok = false;
        }
    }
    if (ok) {
        uint32 nameSize = 0u;
        processedSize = 0u;
        nameSize = (uint32)(dataPtr - payload.Buffer() - 1u);
        ok = (nameSize <= PV_NAME_MAX_SIZE_REC);

        if (ok) {
            // skip the "
            const uint32 maxSize = (PV_NAME_MAX_SIZE_REC + 1);
            char8 buffer[maxSize];
            MemoryOperationsHelper::Set(buffer, 0, maxSize);
            MemoryOperationsHelper::Copy(buffer, payload.Buffer() + 1u, nameSize);
            StreamString varNameTemp = buffer;
            varNameTemp.Seek(0ull);
            varName.SetSize(0ull);
            char8 term;
            varNameTemp.GetToken(varName, ".", term);
            if (term != '.') {
                varName = varNameTemp;
            }
            varNameTemp.Seek(0ull);

            dataPtr += patternSize;
            processedSize += (nameSize + patternSize);

            ok = ((payload.Size() - processedSize) >= (3 * sizeof(uint32)) + sizeof(uint8));
            if (ok) {
                MemoryOperationsHelper::Copy(&receivedIndex, dataPtr, sizeof(uint32));
                dataPtr += sizeof(uint32);
                MemoryOperationsHelper::Copy(&receivedTypeId, dataPtr, sizeof(uint8));
                dataPtr += sizeof(uint8);
                MemoryOperationsHelper::Copy(&receivedSize, dataPtr, sizeof(uint32));
                dataPtr += sizeof(uint32);
                MemoryOperationsHelper::Copy(&receivedOffset, dataPtr, sizeof(uint32));
                if ((receivedIndex >= numberOfVariables) || (receivedSize >= (8u * maxArraySize))) {
                    REPORT_ERROR(ErrorManagement::Information, "receivedIndex %d, receivedSize %d\n", receivedIndex, receivedSize);

                    payload.Seek(0ull);
                    payload.SetSize(0ull);
                    payload += "";
                    //set to refill
                    ok = false;
                }
            }
        }
        else {
            //something wrong...jump everything and resync
            if (nameSize > payload.Size()) {
                nameSize = payload.Size();
            }
            uint32 curSize = (payload.Size() - nameSize);
            REPORT_ERROR(ErrorManagement::Information, "variable name size greater than PV_NAME_MAX_SIZE_REC: skip and resync %d %d\n", nameSize,
                         payload.Size());
            payload.Seek(0);
            MemoryOperationsHelper::Copy((void*) payload.Buffer(), payload.Buffer() + nameSize, curSize);
            payload.SetSize(curSize);
            payload += "";
        }
    }
    return ok;
}

bool DiodeReceiver::GetLocalIndex(StreamString &payload,
                                  StreamString &varName,
                                  uint32 receivedIndex,
                                  uint32 receivedSize,
                                  uint32 receivedOffset,
                                  uint32 &index,
                                  uint32 &processedSize,
                                  bool &controlOk) {
    bool ok = true;

    if (syncSem.FastLock()) {
        index = pvMapping[receivedIndex];
        syncSem.FastUnLock();
    }
    if (index == INDEX_INIT_ID) {
        if (GetLocalVariableIndex(varName.Buffer(), index)) {
            if (syncSem.FastLock()) {
                pvMapping[receivedIndex] = index;
                syncSem.FastUnLock();
            }
        }
        else {
            index = INDEX_NOT_FOUND_ID;
            REPORT_ERROR(ErrorManagement::Information, "variable %s not found", varName.Buffer());
        }
    }
    if (index < numberOfVariables) {
        controlOk = ((pvs[index].totalSize > 0u) && ((receivedSize + receivedOffset) <= pvs[index].totalSize));
    }

    processedSize += (3 * (sizeof(uint32)) + receivedSize + sizeof(uint64) + 4u);
    ok = (payload.Size() >= processedSize);

    return ok;
}

void DiodeReceiver::ReadVarValueAndSkip(StreamString &payload,
                                        const char8 *dataPtr,
                                        uint32 index,
                                        uint32 processedSize,
                                        uint8 receivedTypeId,
                                        uint32 varOffset,
                                        uint32 receivedSize,
                                        bool controlOk) {

    if (index < numberOfVariables) {
        dataPtr += sizeof(uint32);
        if (syncSem.FastLock()) {

            void *ptr = (void*) (memory + pvs[index].offset);
            void *ptr_1 = (void*) (memoryPrec + pvs[index].offset);
            if (controlOk) {
                MemoryOperationsHelper::Copy(ptr + varOffset, dataPtr, receivedSize);
                MemoryOperationsHelper::Copy(ptr + pvs[index].totalSize, dataPtr + receivedSize, sizeof(uint64));
            }
            else {
                if (varOffset == 0u) {
                    //convert!!
                    if (receivedTypeId < NUMBER_OF_TYPES) {
                        REPORT_ERROR(ErrorManagement::Warning, "Attempting to convert");
                        AnyType temp = AnyType(descriptors[receivedTypeId], 0u, (void*) dataPtr);
                        TypeConvert(pvs[index].at, temp);
                    }
                }
            }
            if (MemoryOperationsHelper::Compare(ptr, ptr_1, pvs[index].totalSize) != 0) {
                (changeFlag)[index] = 1;
                MemoryOperationsHelper::Copy(ptr_1, ptr, pvs[index].totalSize);
            }
            syncSem.FastUnLock();
        }
    }
    if (processedSize > payload.Size()) {
        REPORT_ERROR(ErrorManagement::Information, "processedSize %! payloadSize %!", processedSize, payload.Size());

        //something wrong... adjust
        processedSize = payload.Size();
    }
    uint32 currentSize = (payload.Size() - processedSize);
    payload.Seek(0);
    MemoryOperationsHelper::Copy((void*) payload.Buffer(), payload.Buffer() + processedSize, currentSize);
    payload.SetSize(currentSize);
    payload += "";
}

ErrorManagement::ErrorType DiodeReceiver::SendOkReplyMessage(HttpProtocol &protocol,
                                                             TCPSocket * const commClient) {
    ErrorManagement::ErrorType err;
    protocol.CompleteReadOperation(NULL, 0u);
    StreamString hstream = "<html>"
            "<body>"
            "<h1>OK!</h1>"
            "</body>"
            "</html>";
    hstream.Seek(0);
    protocol.CreateAbsolute("OutputOptions");
    protocol.Write("Content-Length", hstream.Size());
    protocol.Write("Content-Type", "text/html");
    protocol.WriteHeader(true, HttpDefinition::HSHCReplyOK, &hstream, NULL);
    if (!protocol.KeepAlive()) {
        REPORT_ERROR(ErrorManagement::Information, "KA close: close the connection");
        err = !(commClient->Close());
        if (err.ErrorsCleared()) {
            err = ErrorManagement::Completed;
        }
        delete commClient;
    }
    return err;
}

void DiodeReceiver::SendErrorReplyMessage(HttpProtocol &protocol,
                                          TCPSocket * const commClient) {

    protocol.CompleteReadOperation(NULL, 0u);
    REPORT_ERROR(ErrorManagement::Information, "Error: close the connection");
    StreamString hstream = "<html>"
            "<body>"
            "<h1>ERROR!</h1>"
            "</body>"
            "</html>";
    hstream.Seek(0);
    protocol.CreateAbsolute("OutputOptions");
    protocol.Write("Content-Length", hstream.Size());
    protocol.Write("Content-Type", "text/html");
    protocol.SetKeepAlive(false);
    protocol.WriteHeader(true, HttpDefinition::HSHCReplyBadRequest, &hstream, NULL);
    commClient->Close();
    delete commClient;
}

CLASS_REGISTER(DiodeReceiver, "1.0")
}

