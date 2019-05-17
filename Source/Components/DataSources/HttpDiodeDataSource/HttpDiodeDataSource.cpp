/**
 * @file HttpDiodeDataSource.cpp
 * @brief Source file for class HttpDiodeDataSource
 * @date 06 nov 2018
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
 * the class HttpDiodeDataSource (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <epicsStdlib.h>
#include <string.h>

#include <cadef.h>
#include <epicsGetopt.h>

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "HttpDiodeDataSource.h"
#include "HttpProtocol.h"
#include "AdvancedErrorManagement.h"
#include "StreamStructuredData.h"
#include "JsonPrinter.h"
#include "HttpDefinition.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

static void GetTimeoutCallback(evargs args) {
    char8 *myargs = (char8 *) (args.usr);
    epicsTimeStamp *ptsNewS = &((struct dbr_time_string *) (args.dbr))->stamp;
    epicsTimeToStrftime(myargs, 128, "%Y-%m-%d %H:%M:%S.%06f", ptsNewS);
}
}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

HttpDiodeDataSource::HttpDiodeDataSource() :
        MemoryDataSourceI() {
    // Auto-generated constructor stub for HttpDiodeDataSource
    // TODO Verify if manual additions are needed
    signalsSentCounter = 0u;
    signalIndexList = NULL;
    signalToSent = 0u;
    client.SetChunkMode(false);
    client.SetCalibWriteParam(0u);
    client.SetBufferSize(32u, 32u);
    serverPort = 0u;
    firstTime = 0u;
    timeStamps = NULL;
    chids = NULL;
}

HttpDiodeDataSource::~HttpDiodeDataSource() {
    // Auto-generated destructor stub for HttpDiodeDataSource
    // TODO Verify if manual additions are needed
    (void) client.Close();
    if (timeStamps != NULL) {
        uint32 nVars = (numberOfSignals - 1u);
        for (uint32 i = 0u; i < nVars; i++) {
            delete[] timeStamps[i];
        }
        delete[] timeStamps;
    }

    if (chids != NULL) {
        delete[] chids;
    }
    ca_context_destroy();

}

bool HttpDiodeDataSource::Initialise(StructuredDataI &data) {
    bool ret = MemoryDataSourceI::Initialise(data);
    if (ret) {
        ret = data.Read("ServerIpAddress", serverIpAddress);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please specify the ServerIpAddress parameter");
        }
        else {
            ret = data.Read("ServerPort", serverPort);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Please specify the ServerPort parameter");
            }
        }

        if (ret) {
            uint32 connectionTimeoutMSec = 0u;
            connectionTimeout = TTInfiniteWait;
            if (data.Read("ConnectionTimeout", connectionTimeoutMSec)) {
                connectionTimeout = connectionTimeoutMSec;
            }
        }
    }
    return ret;
}

bool HttpDiodeDataSource::SetConfiguredDatabase(StructuredDataI & data) {
    bool ret = MemoryDataSourceI::SetConfiguredDatabase(data);
    if (ret) {
        uint32 numberOfVariables = (numberOfSignals - 1u);
        GetSignalNumberOfElements(numberOfVariables, signalToSent);
        timeStamps = new char8*[numberOfVariables];
        chids = new chanId[numberOfVariables];
        ca_context_create (ca_enable_preemptive_callback);

        for (uint32 i = 0u; i < numberOfVariables; i++) {
            timeStamps[i] = NULL;
            timeStamps[i] = new char8[128];
            MemoryOperationsHelper::Set(timeStamps[i], '\0', 128);
            StreamString signalName;
            GetSignalName(i, signalName);
            uint32 nameSize = signalName.Size();
            signalName.SetSize(nameSize - 5u);

            ca_create_channel(signalName.Buffer(), NULL, NULL, 20u, &chids[i]);

            uint32 numberOfElements;
            GetSignalNumberOfElements(i, numberOfElements);

            ca_create_subscription(DBR_TIME_STRING, numberOfElements, chids[i], DBE_VALUE, GetTimeoutCallback, (void*) timeStamps[i], NULL);
        }

    }
    if (ret) {
        //connect the client
        (void) client.Close();
        ret = client.Open();
        if (ret) {
            ret = client.SetBlocking(true);
            client.SetCalibWriteParam(0u);
        }
        if (ret) {
            ret = client.Connect(serverIpAddress.Buffer(), serverPort, connectionTimeout);
        }
    }
    if (ret) {
        ret = (GetNumberOfFunctions() == 1u);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Only one GAM can interact with this DataSourceI");
        }
    }
    return ret;
}

bool HttpDiodeDataSource::Synchronise() {
    return true;
}

bool HttpDiodeDataSource::PrepareNextState(const char8 * const currentStateName,
                                           const char8 * const nextStateName) {
    GetSignalMemoryBuffer(numberOfSignals - 1u, 0u, (void*&) signalIndexList);
    return true;
}

void HttpDiodeDataSource::PrepareOutputOffsets() {
    //send the header here
    if (firstTime > 0u) {
        firstTime = 2u;
        client.SetChunkMode(false);
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
        hprotocol.WriteHeader(false, HttpDefinition::HSHCPut, &hstream, "/archiver");
        client.Flush();
        client.SetChunkMode(true);
    }
    else {
        firstTime++;
    }

}

bool HttpDiodeDataSource::GetOutputOffset(const uint32 signalIdx,
                                          const uint32 numberOfSamples,
                                          uint32 &offset) {
    return true;
}

bool HttpDiodeDataSource::TerminateOutputCopy(const uint32 signalIdx,
                                              const uint32 offset,
                                              const uint32 numberOfSamples) {

    if (firstTime > 1u) {

        StreamStructuredData < JsonPrinter > sdata;
        sdata.SetStream(client);

        for (uint32 i = 0u; i < signalToSent; i++) {

            if (signalIdx == signalIndexList[i]) {

                if (signalsSentCounter == 0u) {
                    client.Printf("%s", "{");
                }
                StreamString signalName;
                GetSignalName(signalIdx, signalName);

                void*signalPtr = NULL;
                //send for each signal a json chunked payload
                GetSignalMemoryBuffer(signalIdx, 0u, signalPtr);
                TypeDescriptor td = GetSignalType(signalIdx);
                AnyType signalAt(td, 0u, signalPtr);
                uint32 numberOfElements;
                GetSignalNumberOfElements(signalIdx, numberOfElements);
                if (numberOfElements > 1u) {
                    signalAt.SetNumberOfDimensions(1u);
                    signalAt.SetNumberOfElements(0u, numberOfElements);
                }

                //StreamString x;
                //x.Printf("%J!", signalAt);
                //REPORT_ERROR(ErrorManagement::Information, "Print %s", x.Buffer());
                sdata.CreateRelative(signalName.Buffer());
                sdata.Write("Value", signalAt);
                REPORT_ERROR(ErrorManagement::Information, "Here 0");

                //ca_pend_event(0.01);
                REPORT_ERROR(ErrorManagement::Information, "Here 1");
                printf("%d %d\n", signalIdx, numberOfSignals);

                printf("%s\n", timeStamps[signalIdx]);
                REPORT_ERROR(ErrorManagement::Information, "Here 2");

                StreamString ts = "\"";
                ts+=timeStamps[signalIdx];
                ts += "\"";
                if (ts.Size() == 0ull) {
                    ts = "\"Undefined\"";
                }
                sdata.Write("TimeStamp", ts.Buffer());
                sdata.MoveToRoot();

                signalsSentCounter++;
                if (signalsSentCounter < signalToSent) {
                    client.Printf("%s", ",");
                }

                break;
            }
        }

        if (signalsSentCounter == signalToSent) {
            REPORT_ERROR(ErrorManagement::Information, "Flushing");

            client.Printf("%s", "}");
            client.Flush();
            client.FinalChunk();
            signalsSentCounter = 0u;
        }
    }

    return true;
}

const char8 *HttpDiodeDataSource::GetBrokerName(StructuredDataI &data, const SignalDirection direction){
    const char8* ret=NULL;
    if(direction==OutputSignals){
        ret="MemoryMapMultiBufferOutputBroker";
    }
    return ret;
}



CLASS_REGISTER(HttpDiodeDataSource, "1.0")
}
