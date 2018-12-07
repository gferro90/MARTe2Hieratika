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
    int32 dbfType;
    int32 dbrType;
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

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

DiodeReceiver::DiodeReceiver() :
        MultiThreadService(embeddedMethod),
        embeddedMethod(*this, &DiodeReceiver::ThreadCycle) {
    // Auto-generated constructor stub for DiodeReceiver
    // TODO Verify if manual additions are needed
    serverPort = 0u;
    acceptTimeout = TTInfiniteWait;
    outputFile = NULL;
    openedFile = NULL;
    serverInitialPort = 0u;
    syncSem.Create();
    pvs = NULL;
    numberOfVariables = 0u;
    test = 0u;
}

DiodeReceiver::~DiodeReceiver() {
    // Auto-generated destructor stub for DiodeReceiver
    // TODO Verify if manual additions are needed
    if (outputFile != NULL) {
        for (uint32 i = 0u; i < numberOfPoolThreads; i++) {
            outputFile[i].Close();
        }
        delete[] outputFile;
    }

    if (openedFile != NULL) {
        delete[] openedFile;
    }

    if (pvs != NULL) {
        for (uint32 n = 0u; (n < numberOfVariables); n++) {
            if (pvs[n].at.GetDataPointer() != NULL) {
                void* ptr = pvs[n].at.GetDataPointer();
                HeapManager::Free((void*&) ptr);
            }
            if (pvs[n].prevBuff != NULL) {
                HeapManager::Free((void*&) pvs[n].prevBuff);
            }
        }
        delete[] pvs;
    }
}

bool DiodeReceiver::Initialise(StructuredDataI &data) {
    bool ret = MultiThreadService::Initialise(data);
    if (ret) {

        ret = data.Read("ServerInitialPort", serverInitialPort);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerInitialPort");
        }
        if (ret) {
            serverPort = serverInitialPort;

            if (!data.Read("IsTest", test)) {
                test = 0u;
            }

            if (test > 0u) {
                StreamString outputFilePath;
                ret = data.Read("OutputFilePath", outputFilePath);
                if (!ret) {
                    REPORT_ERROR(ErrorManagement::InitialisationError, "Please define OutputFilePath");
                }
                outputFile = new File[numberOfPoolThreads];
                openedFile = new TCPSocket*[numberOfPoolThreads];

                for (uint32 i = 0u; i < numberOfPoolThreads; i++) {
                    openedFile[i] = NULL;
                    StreamString fileName = outputFilePath;
                    fileName.Printf("_%d", i);
                    outputFile[i].Open(fileName.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT | BasicFile::FLAG_TRUNC);
                }
            }
            else {
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

                                const char8* epicsTypeName = dbf_type_to_text(pvs[n].pvType);
                                printf("%s: nElems=%d, type=%s\n", pvs[n].pvName, pvs[n].numberOfElements, epicsTypeName);
                                if (StringHelper::Compare(epicsTypeName, "DBF_DOUBLE") == 0u) {
                                    pvs[n].byteSize = (sizeof(float64)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);
                                    pvs[n].at = AnyType(Float64Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
                                    pvs[n].byteSize = (sizeof(float32)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(Float32Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
                                    pvs[n].byteSize = (sizeof(int32)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(SignedInteger32Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
                                    pvs[n].byteSize = (sizeof(uint32)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(UnsignedInteger32Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
                                    pvs[n].byteSize = (sizeof(int16)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(SignedInteger16Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
                                    pvs[n].byteSize = (sizeof(uint16)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(UnsignedInteger16Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
                                    pvs[n].byteSize = (sizeof(int8)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(SignedInteger8Bit, 0u, signalAddress);
                                }
                                else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
                                    pvs[n].byteSize = (sizeof(uint8)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(UnsignedInteger8Bit, 0u, signalAddress);
                                }
                                else {
                                    pvs[n].byteSize = (sizeof(float64)) * pvs[n].numberOfElements;
                                    void *signalAddress = HeapManager::Malloc(pvs[n].byteSize);

                                    pvs[n].at = AnyType(Float64Bit, 0u, signalAddress);
                                }
                                pvs[n].prevBuff = HeapManager::Malloc(pvs[n].byteSize);
                                MemoryOperationsHelper::Set(pvs[n].at.GetDataPointer(), 0, pvs[n].byteSize);
                                MemoryOperationsHelper::Set(pvs[n].prevBuff, 0, pvs[n].byteSize);

                                if (pvs[n].numberOfElements > 1u) {
                                    pvs[n].at.SetNumberOfDimensions(1u);
                                    pvs[n].at.SetNumberOfElements(0u, pvs[n].numberOfElements);
                                }
                            }
                        }
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

ErrorManagement::ErrorType DiodeReceiver::ThreadCycle(ExecutionInfo & info) {

    ErrorManagement::ErrorType err;

    if (info.GetStage() == MARTe::ExecutionInfo::StartupStage) {
        TCPSocket *server = new TCPSocket;
        TCPSocket *client = new TCPSocket;
        //listen and accept connection on the port
        server->Open();

        if (syncSem.FastLock()) {
            server->Listen(serverPort, 255);
            serverPort++;
            syncSem.FastUnLock();
        }
        REPORT_ERROR(ErrorManagement::Information, "Waiting new connection");
        while (!server->WaitConnection(acceptTimeout, client)) {
            //REPORT_ERROR(ErrorManagement::Information, "Waiting new connection");
            Sleep::MSec(100);
        }
        REPORT_ERROR(ErrorManagement::Information, "Connection Accepted");
        delete server;
        //manage only one connection per port
        client->SetBlocking(true);
        //always use the buffer
        client->SetCalibReadParam(0u);
        if (test > 0u) {
            if (syncSem.FastLock()) {

                for (uint32 i = 0u; i < numberOfPoolThreads; i++) {
                    if (openedFile[i] == NULL) {
                        openedFile[i] = client;
                        syncSem.FastUnLock();
                        break;
                    }
                }
                syncSem.FastUnLock();
            }
        }
        else{
            ca_context_create(ca_enable_preemptive_callback);
        }

        info.SetThreadSpecificContext(client);

    }
    else if (info.GetStage() == MARTe::ExecutionInfo::MainStage) {
        TCPSocket *client = reinterpret_cast<TCPSocket *>(info.GetThreadSpecificContext());
        uint32 fileIdx = 0u;
        if (test > 0u) {

            for (uint32 i = 0u; i < numberOfPoolThreads; i++) {
                if (openedFile[i] == client) {
                    fileIdx = i;
                    break;
                }
            }
        }

        //do the work
        //get the full message
        HttpProtocol protocol(*client);

        //discard the header
        err = !(protocol.ReadHeader());
        StreamString line;

        char8 buff[1024];

        //read the chunked payload
        StreamString payload = "\"Payload\": {";
        if (err.ErrorsCleared()) {

            uint32 chunkSize = 0u;
            do {
                client->GetLine(line, false);
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
                        client->Read(buff, sizeToRead);
                        //printf("%s\n", buff);

                        payload += buff;
                        sizeRead += sizeToRead;

                    }
                    if (chunkSize != 0) {
                        uint32 size = 2;
                        client->Read(buff, size);
                    }
                }
            }
            while (chunkSize > 0u);
            client->GetLine(line, false);

            payload += "}";
            if (test > 0u) {
                outputFile[fileIdx].Printf("%s\n", payload.Buffer());
                outputFile[fileIdx].Flush();
            }
            else {
                printf("%s\n", payload.Buffer());
                ConfigurationDatabase cdb;
                //parse the json content
                if (err.ErrorsCleared()) {
                    payload.Seek(0ull);
                    JsonParser parser(payload, cdb);
                    err = !(parser.Parse());
                    cdb.MoveAbsolute("Payload");
                }
                if (err.ErrorsCleared()) {
                    uint32 numberOfRecvars = cdb.GetNumberOfChildren();
                    for (uint32 n = 0u; (n < numberOfRecvars); n++) {
                        StreamString signalName = cdb.GetChildName(n);
                        for (uint32 m = 0u; (m < numberOfVariables); m++) {
                            if (StringHelper::Compare(pvs[n].pvName, signalName.Buffer()) == 0) {
                                ca_create_channel(&pvs[n].pvName[0], NULL, NULL, 20u, &pvs[n].pvChid);
                                (void) ca_pend_io(0.1);
                                cdb.MoveToChild(n);
                                err = !cdb.Read("Value", pvs[n].at);
                                if (!err.ErrorsCleared()) {
                                    REPORT_ERROR(ErrorManagement::FatalError, "Fail to read the %s Value", signalName.Buffer());
                                }
                                else {
                                    if (MemoryOperationsHelper::Compare(pvs[n].prevBuff, pvs[n].at.GetDataPointer(), pvs[n].byteSize) != 0) {
                                        MemoryOperationsHelper::Copy(pvs[n].prevBuff, pvs[n].at.GetDataPointer(), pvs[n].byteSize);
                                        printf("caput %s", pvs[n].pvName);
                                        err = !(ca_array_put(pvs[n].pvType, pvs[n].numberOfElements, pvs[n].pvChid, pvs[n].at.GetDataPointer()) == ECA_NORMAL);
                                        (void) ca_pend_io(0.1);
                                        if (!err.ErrorsCleared()) {
                                            REPORT_ERROR(ErrorManagement::FatalError, "ca_put failed for PV: %s", pvs[n].pvName);
                                        }
                                    }
                                }
                                (void) ca_clear_channel(pvs[n].pvChid);
                                cdb.MoveToAncestor(1u);
                                break;
                            }
                        }
                    }
                }
                else {
                    REPORT_ERROR(ErrorManagement::FatalError, "Failed Parse!");
                }

            }
        }
        else {
            REPORT_ERROR(ErrorManagement::Information, "Error in ReadHeader");

        }
    }
    else {
        TCPSocket *client = reinterpret_cast<TCPSocket *>(info.GetThreadSpecificContext());
        if (client != NULL) {
            client->Close();
            delete client;
            info.SetThreadSpecificContext(NULL);
        }

        if(test==0){
            ca_detach_context();
            ca_context_destroy();

        }

        serverPort = serverInitialPort;
    }
    return err;

}
CLASS_REGISTER(DiodeReceiver, "1.0")
}

