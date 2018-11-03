/**
 * @file TriggerOnChangeHieratikaGAM.cpp
 * @brief Source file for class TriggerOnChangeHieratikaGAM
 * @date 26 ott 2018
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
 * the class TriggerOnChangeHieratikaGAM (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "TriggerOnChangeHieratikaGAM.h"
#include "JsonParser.h"
#include "ConfigurationDatabase.h"
#include "EventConditionTrigger.h"
#include "Message.h"
#include "StreamString.h"
#include "EventSem.h"
#include "HttpChunkedStream.h"
#include "StreamStructuredData.h"
#include "JsonPrinter.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

TriggerOnChangeHieratikaGAM::TriggerOnChangeHieratikaGAM() :
        TriggerOnChangeGAM() {
    // Auto-generated constructor stub for TriggerOnChangeHieratikaGAM
    // TODO Verify if manual additions are needed
    signalValues = NULL;
    totalSize = 0u;
}

TriggerOnChangeHieratikaGAM::~TriggerOnChangeHieratikaGAM() {
    // Auto-generated destructor stub for TriggerOnChangeHieratikaGAM
    // TODO Verify if manual additions are needed
    if (signalValues != NULL) {
        delete[] signalValues;
    }
}

bool TriggerOnChangeHieratikaGAM::Setup() {
    bool ret = TriggerOnChangeGAM::Setup();
    if (ret) {
        ret = (events->Size() > 0u);
        if (ret) {
            ReferenceT < EventConditionTrigger > event = events->Get(0u);
            ret = event.IsValid();
            if (ret) {
                ReferenceT < Message > initialMessage = this->Find("InitialMessage");
                ret = initialMessage.IsValid();
                if (ret) {
                    eventSem = this->Find("StreamSemaphore");
                    ret = eventSem.IsValid();
                    if (ret) {
                        eventSem->Create();
                        eventSem->Reset();
                        //ErrorManagement::ErrorType err =
                        event->SendMessage(initialMessage, this);
                        eventSem->Wait();
                    }
                    //ret = err.ErrorsCleared();
                }
                if (ret) {
                    replyStream = this->Find("ReplyStream");
                    REPORT_ERROR(ErrorManagement::Information, "Reply is %s", replyStream->Buffer());

                    ret = replyStream.IsValid();
                    if (ret) {
                        replyStream->Seek(0u);
                        StreamString reply = "\"reply\": ";
                        reply += *(replyStream.operator->());
                        reply.Seek(0);
                        ConfigurationDatabase data;
                        JsonParser jsonparser(reply, data);

                        jsonparser.Parse();

                        data.MoveAbsolute("reply");
                        data.Read("token", token);
                    }
                }

                //get the tid
                if (ret) {
                    ReferenceT < ConfigurationDatabase > tidPayload(GlobalObjectsDatabase::Instance()->GetStandardHeap());
                    tidPayload->Write("Token", token.Buffer());
                    StreamString command = "GetTid";
                    tidPayload->Write("HieratikaCommand", command.Buffer());
                    ReferenceT < Message > tidMessage(GlobalObjectsDatabase::Instance()->GetStandardHeap());
                    ConfigurationDatabase messInit;
                    messInit.Write("Function", "dummy");
                    const char8 *destination = initialMessage->GetDestination();
                    REPORT_ERROR(ErrorManagement::Information, "%s", destination);

                    messInit.Write("Destination", destination);
                    ReferenceT < ConfigurationDatabase > initialMessagePayload = this->Find("InitialMessage.Payload");
                    ret = initialMessagePayload.IsValid();
                    if (ret) {
                        StreamString stream;
                        if (initialMessagePayload->Read("Stream", stream)) {
                            ret = tidPayload->Write("Stream", stream.Buffer());
                        }

                        if (ret) {
                            StreamString semaphore;
                            if (initialMessagePayload->Read("Semaphore", semaphore)) {
                                ret = tidPayload->Write("Semaphore", semaphore.Buffer());
                            }
                        }

                        if (ret) {
                            tidMessage->Initialise(messInit);
                            tidPayload->SetName("Payload");
                            tidMessage->Insert(tidPayload);
                            replyStream->SetSize(0ull);
                            eventSem->Reset();

                            event->SendMessage(tidMessage, this);
                            eventSem->Wait();
                        }
                    }

                }
                if (ret) {
                    REPORT_ERROR(ErrorManagement::Information, "HereH");

                    const char8 *match = "\"tid\": \"";
                    const char8 *tidPtr = StringHelper::SearchString(replyStream->Buffer(), match);
                    StreamString tidAll;

                    if (tidPtr != NULL) {
                        tidPtr += StringHelper::Length("\"tid\": \"");
                        tidAll = tidPtr;
                    }

                    char8 tt;
                    tidAll.Seek(0ull);
                    tidAll.GetToken(tid, "\"", tt);

                    REPORT_ERROR(ErrorManagement::Information, "tid=%s", tid.Buffer());
                }

                if (ret) {
                    //put the token within each hieratika message
                    uint32 numberOfEvents = events->Size();
                    for (uint32 i = 0u; (i < numberOfEvents) && (ret); i++) {
                        ReferenceT < EventConditionTrigger > event = events->Get(i);
                        ret = event.IsValid();
                        uint32 numberOfMessagesPerEvent = event->Size();
                        for (uint32 j = 0u; (j < numberOfMessagesPerEvent) && (ret); j++) {
                            ReferenceT < Message > mess = event->Get(j);
                            ret = mess.IsValid();
                            if (ret) {
                                ReferenceT < ConfigurationDatabase > payload = mess->Find("Payload");
                                if (payload.IsValid()) {
                                    AnyType at = payload->GetType("Token");
                                    if (at.IsVoid()) {
                                        ret = payload->Write("Token", token);
                                    }
                                    at = payload->GetType("Tid");
                                    if (at.IsVoid()) {
                                        ret = payload->Write("Tid", tid);
                                    }
                                }
                            }
                        }
                    }
                    if (ret) {
                        REPORT_ERROR(ErrorManagement::Information, "Token is %s", token.Buffer());
                    }
                }
            }
        }
    }
    if (ret) {
        uint32 numberOfSignals = GetNumberOfInputSignals();
        ret = (numberOfFields == numberOfSignals);
    }
    if (ret) {

        signalValues = new AnyType[numberOfFields];
        for (uint32 i = 0u; i < numberOfFields; i++) {
            signalValues[i] = AnyType(packetConfig[i].type, 0u, GetInputSignalMemory(i));
            uint32 byteSize = 0u;
            GetSignalByteSize(InputSignals, i, byteSize);
            totalSize += byteSize;
        }

        for (uint32 i = 0u; i < numberOfEvents; i++) {
            ReferenceT < EventConditionTrigger > event = events->Get(i);
            if (event.IsValid()) {
                uint32 eventMessages = event->Size();
                for (uint32 j = 0u; j < eventMessages; j++) {
                    ReferenceT < Message > mess = event->Get(j);
                    if (mess.IsValid()) {
                        ReferenceT < ConfigurationDatabase > payload = mess->Find("Payload");
                        if (payload.IsValid()) {
                            AnyType at = payload->GetType("Variables");
                            if (!at.IsVoid()) {
                                messagesToUpdate.Insert(payload);
                            }
                        }
                    }
                }
            }
        }

    }

    return ret;
}

bool TriggerOnChangeHieratikaGAM::Execute() {
    bool ret = true;
    if (MemoryOperationsHelper::Compare(&previousValue[0], &currentValue[0], totalSize) != 0) {
        //update the cdb
        StreamString variables;
        StreamStructuredData < JsonPrinter > sdata;
        sdata.SetStream(variables);

        //variables.Printf("%s", "{\"DT@BT@TIME\":10, \"DT@BT@SIGU16\":100, \n\"DT@BT@SIGU32\":4000");

        variables.Printf("%s", "{");
        for (uint32 i = 0u; (i < numberOfFields) && (ret); i++) {
            ret = sdata.Write(packetConfig[i].name.Buffer(), signalValues[i]);
        }
        variables.Printf("%s", "}");
        variables.Seek(0ull);
        REPORT_ERROR(ErrorManagement::Information, "Variables=%s", variables.Buffer());

        //for each commit event message update the variables field
        uint32 numberOfMessagesToUpdate = messagesToUpdate.Size();
        for (uint32 i = 0u; (i < numberOfMessagesToUpdate) && (ret); i++) {
            ReferenceT < ConfigurationDatabase > payload = messagesToUpdate.Get(i);
            ret = payload.IsValid();
            if (ret) {
                ret = payload->Delete("Variables");
                if (ret) {
                    ret = payload->Write("Variables", variables);
                }
            }
        }
    }

    if (ret) {
        ret = TriggerOnChangeGAM::Execute();
    }
    return ret;
}

CLASS_REGISTER(TriggerOnChangeHieratikaGAM, "1.0")

}

