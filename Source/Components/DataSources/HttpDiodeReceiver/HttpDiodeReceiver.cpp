/**
 * @file HttpDiodeReceiver.cpp
 * @brief Source file for class HttpDiodeReceiver
 * @date 14 nov 2018
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
 * the class HttpDiodeReceiver (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "HttpDiodeReceiver.h"
#include "AdvancedErrorManagement.h"
#include "HttpProtocol.h"
#include "JsonParser.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

HttpDiodeReceiver::HttpDiodeReceiver() :
        MemoryDataSourceI(),
        EmbeddedServiceMethodBinderI(),
        executor(*this) {
    serverPort = 0u;
    acceptTimeout = TTInfiniteWait;
    signalIndices = NULL;
    nSignalsToReceive = 0u;
    isIndependentThread = 1u;
    firstTime = 1u;
}

HttpDiodeReceiver::~HttpDiodeReceiver() {
    if (isIndependentThread > 0u) {
        if (!executor.Stop()) {
            if (!executor.Stop()) {
                REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
            }
        }
    }
}

bool HttpDiodeReceiver::Initialise(StructuredDataI &data) {
    bool ret = MemoryDataSourceI::Initialise(data);
    if (ret) {
        ret = data.Read("ServerPort", serverPort);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please specify the ServerPort parameter");
        }
        else {
            if (data.Read("IsIndependentThread", isIndependentThread)) {
                isIndependentThread = 1u;
            }
            if (isIndependentThread > 0u) {
                uint32 cpuMask;
                if (data.Read("CPUMask", cpuMask)) {
                    cpuMask = 0xffu;
                }
                //executor.SetCPUMask(cpuMask);
                acceptTimeout = TTInfiniteWait;
                uint32 acceptTimeoutMSec;
                if (data.Read("AcceptTimeout", acceptTimeoutMSec)) {
                    acceptTimeout = acceptTimeoutMSec;
                }
            }
        }
    }
    return ret;
}

bool HttpDiodeReceiver::Synchronise() {
    return true;
}

bool HttpDiodeReceiver::SetConfiguredDatabase(StructuredDataI & data) {
    bool ret = MemoryDataSourceI::SetConfiguredDatabase(data);
    if (ret) {
        //listen and accept connection on the port
        ret = server.Open();
        if (ret) {
            ret = server.Listen(serverPort, 255);
        }
        //manage only one connection per port
        if (ret) {
            client.SetBlocking(true);
            //always use the buffer
            client.SetCalibReadParam(0u);
        }
    }
    return ret;
}

bool HttpDiodeReceiver::PrepareNextState(const char8 * const currentStateName,
                                         const char8 * const nextStateName) {
    bool ok = true;
    if (isIndependentThread > 0u) {
        if (executor.GetStatus() == EmbeddedThreadI::OffState) {
            ok = executor.Start();
        }
    }

    if (ok) {
        GetSignalMemoryBuffer(numberOfSignals - 1u, 0u, (void*&) signalIndices);
        GetSignalNumberOfElements(numberOfSignals - 1u, nSignalsToReceive);
    }

    return ok;
}

uint32 HttpDiodeReceiver::GetNumberOfStatefulMemoryBuffers() {
    return 2u;
}

uint32 HttpDiodeReceiver::GetCurrentStateBuffer() {
    ErrorManagement::ErrorType err;
    if (isIndependentThread == 0u) {
        if (firstTime > 0u) {
            REPORT_ERROR(ErrorManagement::Information, "Waiting for new connection");
            server.WaitConnection(acceptTimeout, &client);
            firstTime = 0u;
        }
        ExecutionInfo info;
        info.SetStage(ExecutionInfo::MainStage);
        err = Execute(info);
        if (!err.ErrorsCleared()) {
            //wait for a new connection here and executes in the next cycle
            client.Close();
            server.WaitConnection(acceptTimeout, &client);
        }
    }
    if (err.ErrorsCleared()) {
        if (mutex.FastLock()) {
            void *signalAddr1 = NULL;
            GetSignalMemoryBuffer(0u, 0u, signalAddr1);
            void *signalAddr2 = NULL;
            GetSignalMemoryBuffer(0u, 1u, signalAddr2);
            MemoryOperationsHelper::Copy(signalAddr1, signalAddr2, stateMemorySize);
            mutex.FastUnLock();
        }
    }
    return 0u;
}

ErrorManagement::ErrorType HttpDiodeReceiver::Execute(ExecutionInfo & info) {
    ErrorManagement::ErrorType err;

    if (info.GetStage() == ExecutionInfo::StartupStage) {
        //client.Close();
        REPORT_ERROR(ErrorManagement::Information, "Waiting for new connection");
        client.Close();
        server.WaitConnection(acceptTimeout, &client);
        REPORT_ERROR(ErrorManagement::Information, "Accepted new connection");

    }
    else if (info.GetStage() == ExecutionInfo::MainStage) {
        //get the full message
        HttpProtocol protocol(client);
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
                client.GetLine(line, false);
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
                        client.Read(buff, sizeToRead);
                        //printf("%s\n", buff);

                        payload += buff;
                        sizeRead += sizeToRead;

                    }
                    if (chunkSize != 0) {
                        uint32 size = 2;
                        client.Read(buff, size);
                        //printf("%s\n", buff);

                    }

                }
            }
            while (chunkSize > 0u);
            client.GetLine(line, false);
            //printf("%s\n", line.Buffer());

            payload+="}";
            printf("%s\n", payload.Buffer());
        }
        else {
            REPORT_ERROR(ErrorManagement::Information, "Error in ReadHeader");

        }

        ConfigurationDatabase cdb;
        //parse the json content
        if (err.ErrorsCleared()) {
            payload.Seek(0ull);
            JsonParser parser(payload, cdb);
            err = !(parser.Parse());
            cdb.MoveAbsolute("Payload");

        }

        //update the signals value
        if (err.ErrorsCleared()) {
            //uint32 numberOfChildren = cdb.GetNumberOfChildren();

            for (uint32 i = 0u; i < nSignalsToReceive; i++) {
                StreamString signalName = cdb.GetChildName(i);
                cdb.MoveToChild(i);
                uint32 signalIdx;
                GetSignalIndex(signalIdx, signalName.Buffer());
                TypeDescriptor td = GetSignalType(signalIdx);
                AnyType at = cdb.GetType("Value");
                void* signalAddress = NULL;
                AnyType newAt(td, 0u, signalAddress);
                newAt.SetNumberOfDimensions(at.GetNumberOfDimensions());
                for (uint32 j = 0u; j < 3u; j++) {
                    newAt.SetNumberOfElements(j, at.GetNumberOfElements(j));
                }
                StreamString timeStamp;
                cdb.Read("TimeStamp", timeStamp);
                //REPORT_ERROR(ErrorManagement::Information, "TS %s:%s", signalName.Buffer(), timeStamp.Buffer());

                if (mutex.FastLock()) {
                    signalIndices[i] = signalIdx;
                    GetSignalMemoryBuffer(signalIdx, 1u, signalAddress);
                    newAt.SetDataPointer(signalAddress);
                    cdb.Read("Value", newAt);
                    mutex.FastUnLock();
                }
                cdb.MoveToAncestor(1u);

            }
        }
        else {
            REPORT_ERROR(ErrorManagement::Information, "Error in Parse");
        }
    }

    return err;
}

const char8 *HttpDiodeReceiver::GetBrokerName(StructuredDataI &data,
                                              const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
        brokerName = "MemoryMapInputBroker";
    }
    return brokerName;
}

CLASS_REGISTER(HttpDiodeReceiver, "1.0")

}
