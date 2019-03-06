/**
 * @file DiodeReceiver2.cpp
 * @brief Source file for class DiodeReceiver2
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
 * the class DiodeReceiver2 (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "DiodeReceiver2.h"
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

void DiodeReceiver2CycleLoop(DiodeReceiver2 &arg) {
    while (arg.quit == 0) {
        if (arg.threadSetContext == 0) {
            ca_context_create (ca_enable_preemptive_callback);
            for (uint32 n = 0u; (n < arg.numberOfVariables); n++) {
                ca_create_channel(&arg.pvs[n].pvName[0], NULL, NULL, 20u, &arg.pvs[n].pvChid);
                (void) ca_pend_io(0.1);
            }
            arg.threadSetContext = 1;
        }
        else {
            //printf("\nSync!!\n");
            if (arg.syncSem.FastLock()) {
                MemoryOperationsHelper::Copy(arg.memory[1], arg.memory[0], arg.totalMemorySize);
                MemoryOperationsHelper::Copy(arg.changeFlag[1], arg.changeFlag[0], arg.numberOfVariables);
                MemoryOperationsHelper::Set(arg.changeFlag[0], 0, arg.numberOfVariables);
                arg.syncSem.FastUnLock();
            }
            for (uint32 n = 0u; (n < arg.numberOfVariables); n++) {
                if ((arg.changeFlag[1])[n] == 1) {
                    if (ca_array_put(arg.pvs[n].pvType, arg.pvs[n].numberOfElements, arg.pvs[n].pvChid, arg.memory[1] + arg.pvs[n].offset) != ECA_NORMAL) {
                        printf("ca_put failed for PV: %s\n", arg.pvs[n].pvName);
                    }
                    (void) ca_pend_io(0.1);
                }
            }
            uint32 elapsed = (uint32)(((HighResolutionTimer::Counter() - arg.lastCounter) * 1000) * HighResolutionTimer::Period());
            if (elapsed < arg.msecPeriod) {
                Sleep::MSec(arg.msecPeriod - elapsed);
            }
            arg.lastCounter = HighResolutionTimer::Counter();
        }
    }

    for (uint32 n = 0u; (n < arg.numberOfVariables); n++) {
        (void) ca_clear_channel(arg.pvs[n].pvChid);
    }

    ca_detach_context();
    ca_context_destroy();

    arg.quit = 2;
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

DiodeReceiver2::DiodeReceiver2() :
        MultiClientService(embeddedMethod),
        embeddedMethod(*this, &DiodeReceiver2::ServerCycle) {
    // Auto-generated constructor stub for DiodeReceiver2
    // TODO Verify if manual additions are needed
    serverPort = 0u;
    acceptTimeout = TTInfiniteWait;
    syncSem.Create();
    pvs = NULL;
    numberOfVariables = 0u;
    mainCpuMask = 0x1u;
    memory[0] = NULL;
    memory[1] = NULL;
    memoryPrec = NULL;
    threadSetContext = 0u;
    msecPeriod = 1000;
    changeFlag[0] = NULL;
    changeFlag[1] = NULL;
    lastCounter = 0ull;
    quit = 0u;
    totalMemorySize = 0u;

}

DiodeReceiver2::~DiodeReceiver2() {
    // Auto-generated destructor stub for DiodeReceiver2
    // TODO Verify if manual additions are needed

    if (pvs != NULL) {
        delete[] pvs;
    }
    if (memory[0] != NULL) {
        HeapManager::Free((void*&) memory[0]);
    }
    if (memory[1] != NULL) {
        HeapManager::Free((void*&) memory[1]);
    }
    if (memoryPrec != NULL) {
        HeapManager::Free((void*&) memoryPrec);
    }
    if (changeFlag[0] != NULL) {
        HeapManager::Free((void*&) changeFlag[0]);
    }
    if (changeFlag[1] != NULL) {
        HeapManager::Free((void*&) changeFlag[1]);
    }
}

bool DiodeReceiver2::Initialise(StructuredDataI &data) {
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
                ret = data.Read("MsecPeriod", msecPeriod);
                if (!ret) {
                    REPORT_ERROR(ErrorManagement::InitialisationError, "Please define MsecPeriod");
                }
            }
            if (ret) {
                if (!data.Read("MainCpuMask", mainCpuMask)) {
                    mainCpuMask = 0x1;
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
#define MAX_ARR_LEN 100

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

                //i need the initialisation also here
                /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
                ret = (ca_context_create(ca_enable_preemptive_callback) == ECA_NORMAL);
                if (!ret) {
                    REPORT_ERROR(ErrorManagement::FatalError, "ca_enable_preemptive_callback failed");
                }
                else {
                    totalMemorySize = 0u;
                    for (uint32 n = 0u; (n < numberOfVariables); n++) {
                        /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
                        ret = (ca_create_channel(&pvs[n].pvName[0], NULL, NULL, 20u, &pvs[n].pvChid) == ECA_NORMAL);
                        ca_pend_io(0.1);
                        if (!ret) {
                            REPORT_ERROR(ErrorManagement::FatalError, "ca_create_channel failed for PV with name %s", pvs[n].pvName);
                        }
                        else {
                            cainfo(pvs[n].pvChid, pvs[n].pvName, pvs[n].pvType, pvs[n].numberOfElements);
                            ca_pend_io(0.1);

                            pvs[n].byteSize = 0;
                            pvs[n].at = voidAnyType;

                            if (pvs[n].numberOfElements > MAX_ARR_LEN) {
                                pvs[n].numberOfElements = 0u;
                            }
                            if (pvs[n].numberOfElements > 0u) {
                                const char8* epicsTypeName = dbf_type_to_text(pvs[n].pvType);
                                //printf("%s: nElems=%d, type=%s\n", pvs[n].pvName, pvs[n].numberOfElements, epicsTypeName);
                                if (StringHelper::Compare(epicsTypeName, "DBF_DOUBLE") == 0u) {
                                    pvs[n].byteSize = (sizeof(float64)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(Float64Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
                                    pvs[n].byteSize = (sizeof(float32)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(Float32Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
                                    pvs[n].byteSize = (sizeof(int32)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(SignedInteger32Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
                                    pvs[n].byteSize = (sizeof(uint32)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(UnsignedInteger32Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
                                    pvs[n].byteSize = (sizeof(int16)) * pvs[n].numberOfElements;

                                    pvs[n].at = AnyType(SignedInteger16Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
                                    pvs[n].byteSize = (sizeof(uint16)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(UnsignedInteger16Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
                                    pvs[n].byteSize = (sizeof(int8)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(SignedInteger8Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
                                    pvs[n].byteSize = (sizeof(uint8)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(UnsignedInteger8Bit, 0u, (void*) NULL);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_STRING") == 0) {

                                    TypeDescriptor td;
                                    td.numberOfBits = MAX_STRING_SIZE * 8u;
                                    td.isStructuredData = false;
                                    td.type = CArray;
                                    td.isConstant = false;

                                    pvs[n].byteSize = (sizeof(char8)) * MAX_STRING_SIZE * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(td, 0u, (void*) NULL);
                                }
                                else {
                                    epicsTypeName = DBF_DOUBLE;
                                    pvs[n].byteSize = (sizeof(float64)) * pvs[n].numberOfElements;
                                    pvs[n].at = AnyType(Float64Bit, 0u, (void*) NULL);
                                }
                                pvs[n].offset = totalMemorySize;
                                totalMemorySize += pvs[n].byteSize;

                                if (pvs[n].numberOfElements > 1u) {
                                    pvs[n].at.SetNumberOfDimensions(1u);
                                    pvs[n].at.SetNumberOfElements(0u, pvs[n].numberOfElements);
                                }
                            }
                        }
                    }

                    memory[0] = (uint8*) HeapManager::Malloc(totalMemorySize);
                    memory[1] = (uint8*) HeapManager::Malloc(totalMemorySize);

                    memoryPrec = (uint8*) HeapManager::Malloc(totalMemorySize);
                    changeFlag[0] = (uint8*) HeapManager::Malloc(numberOfVariables);
                    changeFlag[1] = (uint8*) HeapManager::Malloc(numberOfVariables);

                    MemoryOperationsHelper::Set(memory[0], 0, totalMemorySize);
                    MemoryOperationsHelper::Set(memory[1], 0, totalMemorySize);

                    MemoryOperationsHelper::Set(memoryPrec, 0, totalMemorySize);
                    MemoryOperationsHelper::Set(changeFlag[0], 0, numberOfVariables);
                    MemoryOperationsHelper::Set(changeFlag[1], 0, numberOfVariables);

                    for (uint32 n = 0u; (n < numberOfVariables); n++) {
                        pvs[n].at.SetDataPointer(memory[0] + pvs[n].offset);
                    }
                }
            }
        }

        if (ret) {
            for (uint32 n = 0u; (n < numberOfVariables); n++) {
                (void) ca_clear_channel(pvs[n].pvChid);
            }

            ca_detach_context();
            ca_context_destroy();
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

ErrorManagement::ErrorType DiodeReceiver2::Start() {
    lastCounter = HighResolutionTimer::Counter();
    Threads::BeginThread((ThreadFunctionType) DiodeReceiver2CycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, mainCpuMask);
    while (threadSetContext == 0) {
        Sleep::Sec(1);
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

ErrorManagement::ErrorType DiodeReceiver2::Stop() {
    quit = 1;
    while (quit == 1) {
        Sleep::Sec(1);
    }
    return MultiClientService::Stop();
}

ErrorManagement::ErrorType DiodeReceiver2::ClientService(TCPSocket * const commClient) {

    ErrorManagement::ErrorType err;

    //do the work
    //get the full message
    HttpProtocol protocol(*commClient);

    //discard the header
    err = !(protocol.ReadHeader());

    //uint64 tic=HighResolutionTimer::Counter();

    StreamString line;

    char8 buff[1024];
    uint32 state = 0;

    StreamString payload;
    StreamString varName;
    StreamString varValue;
    StreamString varTs;
    StreamString localcfg;
    uint32 readVariables = 0u;

    if (err.ErrorsCleared()) {

        uint32 chunkSize = 0u;
        do {
            commClient->GetLine(line, false);
            line.SetSize(line.Size() - 1);
            //REPORT_ERROR(ErrorManagement::Information, "received=%s", line.Buffer());
            StreamString toConv = "0x";
            toConv += line;
            TypeConvert(chunkSize, toConv);
            //REPORT_ERROR(ErrorManagement::Information, "chunkSize=%d", chunkSize);
            line.SetSize(0ull);
            if (err.ErrorsCleared()) {
                uint32 sizeRead = 0u;

                while (sizeRead < chunkSize) {
                    MemoryOperationsHelper::Set(buff, '\0', 1024);
                    uint32 sizeToRead = chunkSize - sizeRead;
                    if (sizeToRead > 1023) {
                        sizeToRead = 1023;
                    }
                    commClient->Read(buff, sizeToRead);
                    //printf("%s\n", buff);

                    payload += buff;
                    sizeRead += sizeToRead;

                }
                if (chunkSize > 0) {
                    uint32 size = 2;
                    commClient->Read(buff, size);

                    while (1) {

                        payload.Seek(0u);
                        //REPORT_ERROR(ErrorManagement::Information, "initPayload=%s", payload.Buffer());

                        char8 terminator;

                        if (state == 0) {
                            varName.SetSize(0);
                            payload.GetToken(varName, "\n", terminator, "\r\n");

                            if (terminator == '\n') {
                                uint32 varNameSize = varName.Size();
                                //remove = {
                                varName.SetSize(varNameSize - 4u);
                                varName = varName.Buffer() + 1;
                                //REPORT_ERROR(ErrorManagement::Information, "%d %s", state, varName.Buffer());
                                payload = payload.Buffer() + payload.Position();
                                payload.Seek(0ull);
                                //REPORT_ERROR(ErrorManagement::Information, "newPayload=%s", payload.Buffer());

                                state = 1;
                            }
                            else {
                                break;
                            }
                        }
                        if (state == 1) {
                            varValue.SetSize(0);
                            payload.GetToken(varValue, "\n", terminator, "\r\n");
                            if (terminator == '\n') {
                                uint32 varValueSize = varValue.Size();
                                //remove = {
                                varValue.SetSize(varValueSize - 1u);
                                varValue = varValue.Buffer() + StringHelper::Length("\"Value\": ");
                                //REPORT_ERROR(ErrorManagement::Information, "%d %s", state, varValue.Buffer());
                                payload = payload.Buffer() + payload.Position();
                                payload.Seek(0ull);
                                //REPORT_ERROR(ErrorManagement::Information, "newPayload=%s", payload.Buffer());
                                state = 2;
                            }
                            else {
                                //payload = varValue;
                                break;
                            }
                        }
                        if (state == 2) {
                            varTs.SetSize(0);
                            payload.GetToken(varTs, "\n", terminator, "\r\n");
                            if (terminator == '\n') {
                                //remove = {
                                varTs = varTs.Buffer() + StringHelper::Length("\"Timestamp\": ");
                                //REPORT_ERROR(ErrorManagement::Information, "%d %s", state, varTs.Buffer());
                                payload = payload.Buffer() + payload.Position();
                                payload.Seek(0ull);
                                //REPORT_ERROR(ErrorManagement::Information, "newPayload=%s", payload.Buffer());
                                state = 3;
                            }
                            else {
                                //payload = varTs;
                                break;
                            }
                        }
                        if (state == 3) {

                            //REPORT_ERROR(ErrorManagement::Information, "%s, %s, %s", varName.Buffer(), varValue.Buffer(), varTs.Buffer());
                            uint32 range_1 = numberOfVariables;
                            uint32 range = numberOfVariables / 2;
                            uint32 index = range;
                            //bool done = false;
                            while (range_1 > 0) {
                                int32 res = StringHelper::Compare(pvs[index].pvName, varName.Buffer());
                                if (res == 0) {
                                    readVariables++;
                                    bool ok = true;
                                    ConfigurationDatabase cdb;
                                    if (pvs[index].numberOfElements > 0) {
                                        if (pvs[index].at.GetNumberOfDimensions() > 0u) {
                                            localcfg = "\"Value\": ";
                                            localcfg += varValue.Buffer();
                                            localcfg.Seek(0);
                                            JsonParser parser(localcfg, cdb);
                                            ok = parser.Parse();
                                            if (!ok) {
                                                REPORT_ERROR(ErrorManagement::FatalError, "Failed parse");
                                            }
                                            else {
                                                cdb.MoveToRoot();
                                            }
                                        }
                                        if (ok) {
                                            if (syncSem.FastLock()) {
                                                if (pvs[index].at.GetNumberOfDimensions() > 0u) {
                                                    cdb.MoveToRoot();
                                                    ok = cdb.Read("Value", pvs[index].at);
                                                }
                                                else {
                                                    ok = TypeConvert(pvs[index].at, varValue.Buffer());
                                                }
                                                if (ok) {
                                                    if (MemoryOperationsHelper::Compare(memory[0] + pvs[index].offset, memoryPrec + pvs[index].offset,
                                                                                        pvs[index].byteSize) != 0) {
                                                        (changeFlag[0])[index] = 1;
                                                        MemoryOperationsHelper::Copy(memoryPrec + pvs[index].offset, memory[0] + pvs[index].offset,
                                                                                     pvs[index].byteSize);

                                                    }
                                                }
                                                syncSem.FastUnLock();
                                            }
                                            if (!ok) {
                                                StreamString errStr;
                                                errStr.Printf("Fail to read the %s Value %s", varName.Buffer(), varValue.Buffer());
                                                printf("%s\n", errStr.Buffer());
                                            }
                                        }
                                    }
                                    break;
                                }
                                else if (res == 2) {
                                    uint32 rem = range % 2;
                                    range_1 = range;
                                    if (rem == 1) {
                                        index--;
                                    }
                                    range = range_1 / 2;
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
                                    range = range_1 / 2;
                                    index += (range + 1);
                                }
                            }
                            state = 4;

                        }
                        if (state == 4) {
                            StreamString endBlock;
                            payload.GetToken(endBlock, "\n", terminator, "\r\n");
                            //printf("payload\n %s\nterminator=%c\n", payload.Buffer(), terminator);

                            if (terminator == '\n') {
                                payload = payload.Buffer() + payload.Position();
                                payload.Seek(0ull);

                                state = 0;
                            }
                            else {
                                break;
                            }
                        }
                    }
                }
            }
        }
        while (chunkSize > 0u);
        commClient->GetLine(line, false);

    }
    else {
        REPORT_ERROR(ErrorManagement::Information, "Error in ReadHeader");
    }
    return err;

}

ErrorManagement::ErrorType DiodeReceiver2::ServerCycle(MARTe::ExecutionInfo &information) {
    ErrorManagement::ErrorType err;

    if (information.GetStage() == MARTe::ExecutionInfo::StartupStage) {

    }
    if (information.GetStage() == MARTe::ExecutionInfo::MainStage) {

        /*lint -e{593} -e{429} the newClient pointer will be freed within the thread*/
        if (information.GetStageSpecific() == MARTe::ExecutionInfo::WaitRequestStageSpecific) {
            /*lint -e{429} the newClient pointer will be freed within the thread*/
            TCPSocket *newClient = new TCPSocket();
            if (err.ErrorsCleared()) {
                REPORT_ERROR(ErrorManagement::Information, "Waiting new connection");
                if (server.WaitConnection(acceptTimeout, newClient) == NULL) {
                    err = MARTe::ErrorManagement::Timeout;
                    delete newClient;
                }
                else {
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

CLASS_REGISTER(DiodeReceiver2, "1.0")
}

