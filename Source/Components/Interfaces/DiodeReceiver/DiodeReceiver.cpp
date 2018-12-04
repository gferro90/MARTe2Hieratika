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
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

DiodeReceiver::DiodeReceiver() :
        MultiThreadService(embeddedMethod),
        embeddedMethod(*this, &DiodeReceiver::ThreadCycle) {
    // Auto-generated constructor stub for DiodeReceiver
    // TODO Verify if manual additions are needed
    serverPort = 0u;
    acceptTimeout = TTInfiniteWait;
    outputFile = NULL;
    openedFile = NULL;

    syncSem.Create();
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

}

bool DiodeReceiver::Initialise(StructuredDataI &data) {
    bool ret = MultiThreadService::Initialise(data);
    if (ret) {
        ret = data.Read("ServerIpAddress", serverIp);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerIpAddress");
        }

        if (ret) {
            ret = data.Read("ServerInitialPort", serverPort);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please define ServerInitialPort");
            }
        }
        if (ret) {
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
        while(!server->WaitConnection(acceptTimeout, client)){
            //REPORT_ERROR(ErrorManagement::Information, "Waiting new connection");
            Sleep::MSec(100);
        }
        REPORT_ERROR(ErrorManagement::Information, "Connection Accepted");
        delete server;
        //manage only one connection per port
        client->SetBlocking(true);
        //always use the buffer
        client->SetCalibReadParam(0u);

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

        info.SetThreadSpecificContext(client);

    }
    else if (info.GetStage() == MARTe::ExecutionInfo::MainStage) {
        TCPSocket *client = reinterpret_cast<TCPSocket *>(info.GetThreadSpecificContext());

        uint32 fileIdx = 0u;
        for (uint32 i = 0u; i < numberOfPoolThreads; i++) {
            if (openedFile[i] == client) {
                fileIdx = i;
                break;
            }
        }

        //do the work
        //get the full message
        HttpProtocol protocol(*client);
        REPORT_ERROR(ErrorManagement::Information, "Reading header");

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
                        //printf("%s\n", buff);

                    }

                }
            }
            while (chunkSize > 0u);
            client->GetLine(line, false);
            //printf("%s\n", line.Buffer());

            payload += "}";
            //printf("%s\n", payload.Buffer());
            outputFile[fileIdx].Printf("%s\n", payload.Buffer());
            outputFile[fileIdx].Flush();
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

    }
    return err;

}
CLASS_REGISTER(DiodeReceiver, "1.0")
}

