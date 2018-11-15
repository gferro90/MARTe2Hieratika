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
    bufferIdx = 0u;
    signalIndices = NULL;
    nSignalsToReceive = 0u;
}

HttpDiodeReceiver::~HttpDiodeReceiver() {
    if (!executor.Stop()) {
        if (!executor.Stop()) {
            REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
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
            acceptTimeout = TTInfiniteWait;
            uint32 acceptTimeoutMSec;
            if (data.Read("AcceptTimeout", acceptTimeoutMSec)) {
                acceptTimeout = acceptTimeoutMSec;
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
            ret = server.WaitConnection(acceptTimeout, &client);
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
    if (executor.GetStatus() == EmbeddedThreadI::OffState) {
        ok = executor.Start();
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
    uint8 otherBuffer = (bufferIdx + 1u);
    otherBuffer &= 0x1;
    return otherBuffer;
}

void HttpDiodeReceiver::PrepareInputOffsets() {
    //
    if (mutex.FastLock()) {
        REPORT_ERROR(ErrorManagement::Information, "HERE PrepareInputOffsets");

        uint8 otherBuffer = (bufferIdx + 1u);
        otherBuffer &= 0x1;
        void *signalAddr1 = NULL;
        GetSignalMemoryBuffer(0u, otherBuffer, signalAddr1);
        void *signalAddr2 = NULL;
        GetSignalMemoryBuffer(0u, bufferIdx, signalAddr2);
        MemoryOperationsHelper::Copy(signalAddr1, signalAddr2, stateMemorySize);
        bufferIdx++;
        bufferIdx &= 0x1;
        mutex.FastUnLock();
    }

}

bool HttpDiodeReceiver::GetInputOffset(const uint32 signalIdx,
                                       const uint32 numberOfSamples,
                                       uint32 &offset) {
    return true;
}

bool HttpDiodeReceiver::TerminateOutputCopy(const uint32 signalIdx,
                                            const uint32 offset,
                                            const uint32 numberOfSamples) {
    return true;
}

ErrorManagement::ErrorType HttpDiodeReceiver::Execute(ExecutionInfo & info) {
    ErrorManagement::ErrorType err;

    if (info.GetStage() == ExecutionInfo::BadTerminationStage) {
        Sleep::Sec(1.0);
    }
    else {
        //get the full message
        HttpProtocol protocol(client);

        err = !(protocol.ReadHeader());
        StreamString line;

        char8 buff[1024];

        StreamString payload = "\"Payload\": ";
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

                        payload += buff;
                        sizeRead += sizeToRead;

                    }
                    if (chunkSize != 0) {
                        uint32 size = 2;
                        client.Read(buff, size);
                    }

                }
            }
            while (chunkSize > 0u);
            client.GetLine(line, false);

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
            if (mutex.FastLock()) {
                for (uint32 i = 0u; i < nSignalsToReceive; i++) {
                    StreamString signalName = cdb.GetChildName(i);
                    cdb.MoveToChild(i);
                    uint32 signalIdx;
                    GetSignalIndex(signalIdx, signalName.Buffer());
                    signalIndices[i] = signalIdx;
                    void* signalAddress = NULL;
                    GetSignalMemoryBuffer(signalIdx, bufferIdx, signalAddress);
                    TypeDescriptor td = GetSignalType(signalIdx);
                    AnyType at = cdb.GetType("Value");
                    AnyType newAt(td, 0u, signalAddress);
                    newAt.SetNumberOfDimensions(at.GetNumberOfDimensions());
                    for (uint32 j = 0u; j < 3u; j++) {
                        newAt.SetNumberOfElements(j, at.GetNumberOfElements(j));
                    }
                    StreamString val;
                    cdb.Read("Value", newAt);
                    cdb.Read("Value", val);
                    REPORT_ERROR(ErrorManagement::Information, "HERE val=%s", val.Buffer());
                    cdb.MoveToAncestor(1u);

                }
                mutex.FastUnLock();
            }
        }
    }

    return err;
}

CLASS_REGISTER(HttpDiodeReceiver, "1.0")

}
