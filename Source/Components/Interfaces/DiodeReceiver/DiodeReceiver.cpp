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

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "DiodeReceiver.h"
#include "AdvancedErrorManagement.h"
#include "HttpProtocol.h"
#include "JsonParser.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
#define MAX_ARR_LEN 100

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
    uint32 nElems;
    enum channel_state state;
    const char8 *stateStrings[] = { "never connected", "previously connected", "connected", "closed" };

    state = ca_state(pvChid);
    nElems = ca_element_count(pvChid);
    type = ca_field_type(pvChid);

    if (state != 2) {
        printf("The variable %s is not connected, state=%s\n", pvName, stateStrings[state]);
    }

    numberOfElements = nElems;

    return 0;
}

void DiodeReceiverCycleLoop(DiodeReceiver &arg) {

    uint32 threadId;
    if (arg.syncSem.FastLock()) {
        threadId = arg.threadCnt;
        arg.threadCnt++;
        arg.syncSem.FastUnLock();
    }
    bool ret = true;
    if (threadId == 0u) {
        ret = (ca_context_create(ca_enable_preemptive_callback) == ECA_NORMAL);
        arg.eventSem.Post();
    }
    else {
        arg.eventSem.Wait();
    }

    if (!ret) {
        //REPORT_ERROR(ErrorManagement::FatalError, "ca_enable_preemptive_callback failed");
    }
    else {
        arg.totalMemorySize = 0u;
        uint32 nThreads = arg.numberOfInitThreads;
        uint32 numberOfVarsPerThread = (arg.numberOfVariables / nThreads);

        uint32 beg = threadId * numberOfVarsPerThread;
        uint32 end = (threadId + 1) * numberOfVarsPerThread;
        if ((arg.numberOfVariables - end) < nThreads) {
            end = arg.numberOfVariables;
        }
        for (uint32 n = beg; (n < end); n++) {
            /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
            ret = (ca_create_channel(arg.pvs[n].pvName, NULL, NULL, 20u, &arg.pvs[n].pvChid) == ECA_NORMAL);
            ca_pend_io(0.1);
            if (!ret) {
                //REPORT_ERROR_STATIC(ErrorManagement::FatalError, "ca_create_channel failed for PV with name %s", arg.pvs[n].pvName);
            }
            else {
                cainfo(arg.pvs[n].pvChid, arg.pvs[n].pvName, arg.pvs[n].pvType, arg.pvs[n].numberOfElements);
                ca_pend_io(0.1);

                arg.pvs[n].byteSize = 0;
                arg.pvs[n].at = voidAnyType;

                if (arg.pvs[n].numberOfElements > MAX_ARR_LEN) {
                    arg.pvs[n].numberOfElements = 0u;
                }
                if (arg.pvs[n].numberOfElements > 0u) {
                    const char8* epicsTypeName = dbf_type_to_text(arg.pvs[n].pvType);
                    //printf("%s: nElems=%d, type=%s\n", pvs[n].pvName, pvs[n].numberOfElements, epicsTypeName);
                    if (StringHelper::Compare(epicsTypeName, "DBF_DOUBLE") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(float64)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(Float64Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(float32)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(Float32Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(int32)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(SignedInteger32Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(uint32)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(UnsignedInteger32Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(int16)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(SignedInteger16Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(uint16)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(UnsignedInteger16Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(int8)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(SignedInteger8Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
                        arg.pvs[n].byteSize = (sizeof(uint8)) * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(UnsignedInteger8Bit, 0u, (void*) NULL);
                    }
                    else if (StringHelper::Compare(epicsTypeName, "DBF_STRING") == 0) {

                        TypeDescriptor td;
                        td.numberOfBits = MAX_STRING_SIZE * 8u;
                        td.isStructuredData = false;
                        td.type = CArray;
                        td.isConstant = false;

                        arg.pvs[n].byteSize = (sizeof(char8)) * MAX_STRING_SIZE * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(td, 0u, (void*) NULL);
                    }
                    else {
                        TypeDescriptor td;
                        td.numberOfBits = MAX_STRING_SIZE * 8u;
                        td.isStructuredData = false;
                        td.type = CArray;
                        td.isConstant = false;

                        arg.pvs[n].byteSize = (sizeof(char8)) * MAX_STRING_SIZE * arg.pvs[n].numberOfElements;
                        arg.pvs[n].at = AnyType(td, 0u, (void*) NULL);
                        arg.pvs[n].pvType = DBF_STRING;
                        /*
                         epicsTypeName = DBF_DOUBLE;
                         pvs[n].byteSize = (sizeof(float64)) * pvs[n].numberOfElements;
                         pvs[n].at = AnyType(Float64Bit, 0u, (void*) NULL);*/
                    }
                    if (arg.pvs[n].numberOfElements > 1u) {
                        arg.pvs[n].at.SetNumberOfDimensions(1u);
                        arg.pvs[n].at.SetNumberOfElements(0u, arg.pvs[n].numberOfElements);
                    }
                }
            }
        }

    }

    Atomic::Increment(&arg.threadSetContext);
    arg.eventSem.ResetWait(TTInfiniteWait);

    if (threadId == 0u) {
        for (uint32 n = 0u; (n < arg.numberOfVariables); n++) {
            (void) ca_clear_channel(arg.pvs[n].pvChid);
        }

        ca_detach_context();
        ca_context_destroy();
    }

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
    mainCpuMask = 0x1u;
    memory = NULL;
    memoryPrec = NULL;
    threadSetContext = 0u;
    changeFlag = NULL;
    lastCounter = 0ull;
    quit = 0;
    totalMemorySize = 0u;
    eventSem.Create();
    eventSem.Reset();
    threadCnt = 0u;
    pvMapping = NULL;
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
    if (memoryPrec != NULL) {
        HeapManager::Free((void*&) memory);
    }

    if (changeFlag != NULL) {
        HeapManager::Free((void*&) changeFlag);
    }

    if (pvMapping != NULL) {
        delete[] pvMapping;
    }

}

bool DiodeReceiver::Initialise(StructuredDataI &data) {
    bool ret = MultiClientService::Initialise(data);
    if (ret) {

        ret = data.Read("ServerInitialPort", serverPort);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerInitialPort");
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
                if (!data.Read("MainCpuMask", mainCpuMask)) {
                    mainCpuMask = 0xFF;
                }
                if (!data.Read("NumberOfInitThreads", numberOfInitThreads)) {
                    numberOfInitThreads = 1u;
                }

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
                        printf("variable=%s\n", variable.Buffer());
                        numberOfVariables++;
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
                        pvs[counter].pvType = DBF_DOUBLE;
                        pvs[counter].prevBuff = NULL;
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

    }
    return ret;
}

ErrorManagement::ErrorType DiodeReceiver::Start() {
    lastCounter = HighResolutionTimer::Counter();
    for (uint32 i = 0u; i < numberOfInitThreads; i++) {
        Threads::BeginThread((ThreadFunctionType) DiodeReceiverCycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, mainCpuMask);
    }
    while (threadSetContext < numberOfInitThreads) {
        Sleep::Sec(1);
    }

    for (uint32 n = 0u; n < numberOfVariables; n++) {
        pvs[n].offset = totalMemorySize;
        totalMemorySize += (pvs[n].byteSize + sizeof(uint64));
    }

    memory = (uint8*) HeapManager::Malloc(totalMemorySize);
    memoryPrec = (uint8*) HeapManager::Malloc(totalMemorySize);
    changeFlag = (uint8*) HeapManager::Malloc(numberOfVariables);
    pvMapping = new uint32[numberOfVariables];
    for (uint32 i = 0u; i < numberOfVariables; i++) {
        pvMapping[i] = 0xFFFFFFFF;
    }
    MemoryOperationsHelper::Set(memory, 0, totalMemorySize);
    MemoryOperationsHelper::Set(memoryPrec, 0, totalMemorySize);
    MemoryOperationsHelper::Set(changeFlag, 0, numberOfVariables);
    for (uint32 n = 0u; (n < numberOfVariables); n++) {
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

    return err;
}

ErrorManagement::ErrorType DiodeReceiver::Stop() {
    Atomic::Increment(&quit);
    eventSem.Post();
    return MultiClientService::Stop();
}

ErrorManagement::ErrorType DiodeReceiver::ClientService(TCPSocket * const commClient) {

    ErrorManagement::ErrorType err;
    if (quit == 0) {
        //do the work
        //get the full message
        HttpProtocol protocol(*commClient);

        //discard the header
        err = !(protocol.ReadHeader());

        StreamString line;

        char8 buff[1024];
        uint32 state = 0;

        StreamString payload;
        StreamString varName;
        StreamString varValue;
        StreamString varTs;
        StreamString varIndex;

        uint32 readVariables = 0u;
        uint32 receivedIndex = 0u;
        const char8 *pattern = "\": ";
        uint32 patternSize = StringHelper::Length(pattern);

        bool isChunked = true;
        uint32 contentLength = 0u;

        if (err.ErrorsCleared()) {
            if (protocol.MoveAbsolute("InputOptions")) {
                if (protocol.Read("Content-Length", contentLength)) {
                    isChunked = false;
                    if(contentLength==0u){
                        protocol.SetKeepAlive(false);
                    }
                }
                protocol.MoveToAncestor(1u);
            }
        }

        if (err.ErrorsCleared()) {

            uint32 chunkSize = (isChunked) ? (0u) : (32u);
            do {
                if (isChunked) {
                    //get the line
                    err = !(commClient->GetLine(line, false));
                    //remove the \r
                    line.SetSize(line.Size() - 1);
                    //get the chunk size
                    StreamString toConv = "0x";
                    toConv += line;
                    TypeConvert(chunkSize, toConv);
                    line.SetSize(0ull);
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

                if (err.ErrorsCleared()) {
                    if (chunkSize > 0) {
                        if (isChunked) {
                            //read the \r\n
                            uint32 size = 2;
                            err = !(commClient->Read(buff, size));
                        }
                        bool ok = err.ErrorsCleared();
                        while (ok) {

                            payload.Seek(0);
                            varName.SetSize(0);

                            const char8 *dataPtr = StringHelper::SearchString(payload.Buffer(), pattern);

                            ok = (dataPtr != NULL);
                            uint32 nameSize = 0u;
                            uint32 processedSize = 0u;

                            if (ok) {
                                // skip the "
                                varName = payload.Buffer() + 1;
                                nameSize = (uint32)(dataPtr - payload.Buffer() - 1u);
                                varName.SetSize(nameSize);
                                uint32 indexSize = sizeof(uint32);
                                dataPtr += patternSize;

                                processedSize += (nameSize + patternSize);

                                ok = ((payload.Size() - processedSize) >= indexSize);
                                if (ok) {
                                    MemoryOperationsHelper::Copy(&receivedIndex, dataPtr, indexSize);
                                }
                            }

                            if (ok) {
                                if (pvMapping[receivedIndex] == 0xFFFFFFFF) {
                                    uint32 index=0u;
                                    ok=GetLocalVariableIndex(varName.Buffer(), receivedIndex, index);
                                    if(ok){
                                        pvMapping[receivedIndex]=index;
                                    }
                                }
                            }

                            if (ok) {
                                uint32 index = pvMapping[receivedIndex];
                                processedSize += (sizeof(uint32) + pvs[index].byteSize + sizeof(uint64) + 3u);

                                ok = (payload.Size() >= processedSize);
                                if (ok) {
                                    dataPtr += sizeof(uint32);
                                    if (syncSem.FastLock()) {

                                        void *ptr = (void*) (memory + pvs[index].offset);
                                        void *ptr_1 = (void*) (memoryPrec + pvs[index].offset);
                                        MemoryOperationsHelper::Copy(ptr, dataPtr, (pvs[index].byteSize + sizeof(uint64)));
                                        if (MemoryOperationsHelper::Compare(ptr, ptr_1, pvs[index].byteSize) != 0) {
                                            (changeFlag)[index] = 1;
                                            MemoryOperationsHelper::Copy(ptr_1, ptr, pvs[index].byteSize);
                                        }
                                        syncSem.FastUnLock();
                                    }
                                    uint32 currentSize = (payload.Size() - processedSize);
                                    payload.Seek(0);
                                    if (currentSize > 0u) {
                                        MemoryOperationsHelper::Copy(payload.Buffer(), payload.Buffer() + processedSize, currentSize);
                                    }
                                    payload.SetSize(currentSize);
                                }
                            }
                        }
                    }
                }
            }
            while (chunkSize > 0u);
            if (isChunked) {
                err = !(commClient->GetLine(line, false));
            }
        }

        if (err.ErrorsCleared()) {
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
        }
        else {
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

    }

    return err;

}

ErrorManagement::ErrorType DiodeReceiver::ServerCycle(MARTe::ExecutionInfo & information) {
    ErrorManagement::ErrorType err;

    if (information.GetStage() == MARTe::ExecutionInfo::StartupStage) {

    }
    if (information.GetStage() == MARTe::ExecutionInfo::MainStage) {

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
                    information.SetThreadSpecificContext(reinterpret_cast<void*>(newClient));
                    err = MARTe::ErrorManagement::NoError;
                }
            }
        }
        if (information.GetStageSpecific() == MARTe::ExecutionInfo::ServiceRequestStageSpecific) {
            TCPSocket *newClient = reinterpret_cast<TCPSocket *>(information.GetThreadSpecificContext());
            err = ClientService(newClient);
        }
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
    return (threadSetContext >= numberOfInitThreads);
}

bool DiodeReceiver::GetLocalVariableIndex(const char8 *varName, uint32 receivedIndex, uint32 &index){
    uint32 range_1 = numberOfVariables;
    uint32 range = (numberOfVariables / 2);
    index = range;
    //bool done = false;
    bool ok = false;
    while ((range_1 > 0) && (!ok)) {
        int32 res = StringHelper::Compare(pvs[index].pvName, varName);
        if (res == 0) {
            pvMapping[receivedIndex] = index;
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
    return ok;
}


CLASS_REGISTER(DiodeReceiver, "1.0")
}

