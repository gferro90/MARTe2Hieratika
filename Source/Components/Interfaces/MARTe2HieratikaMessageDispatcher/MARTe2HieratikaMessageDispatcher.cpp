/**
 * @file MARTe2HieratikaMessageDispatcher.cpp
 * @brief Source file for class MARTe2HieratikaMessageDispatcher
 * @date 24 ott 2018
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
 * the class MARTe2HieratikaMessageDispatcher (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "AdvancedErrorManagement.h"
#include "MARTe2HieratikaMessageDispatcher.h"
#include "TimeStamp.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

MARTe2HieratikaMessageDispatcher::MARTe2HieratikaMessageDispatcher() :
        Object(),
        MessageI(),
        MARTe2HieratikaInterface(),
        EmbeddedServiceMethodBinderI(),
        executor(*this) {
    filter = ReferenceT < QueueingMessageFilter > (new (NULL) QueueingMessageFilter());
    (void) MessageI::InstallMessageFilter(filter, 0);
    response = &internalResponse;
}

MARTe2HieratikaMessageDispatcher::~MARTe2HieratikaMessageDispatcher() {
    (void) MessageI::RemoveMessageFilter(filter);
}

bool MARTe2HieratikaMessageDispatcher::Initialise(StructuredDataI &data) {
    StreamString serverIpAddress;

    bool ret = data.Read("ServerIpAddress", serverIpAddress);
    if (ret) {
        SetServerAddress(serverIpAddress.Buffer());
        uint32 serverPort = 0u;
        ret = data.Read("ServerPort", serverPort);
        if (ret) {
            SetServerPort(serverPort);
        }
        else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "ServerPort not defined");
        }

        if (ret) {
            uint32 timeoutT;
            if (data.Read("ReceiveMessageTimeout", timeoutT)) {
                messageTimeout = timeoutT;
            }
            else {
                messageTimeout = 1000u;
            }
        }

        if (ret) {
            uint32 timeoutT;
            if (data.Read("HttpMessageTimeout", timeoutT)) {
                SetHttpExchangeTimeout(timeoutT);
            }
            else {
                SetHttpExchangeTimeout (TTInfiniteWait);
            }
        }
    }
    else {
        REPORT_ERROR(ErrorManagement::InitialisationError, "ServerIpAddress not defined");
    }

    if (ret) {
        uint32 cpuMaskIn;
        if (!data.Read("CPUMask", cpuMaskIn)) {
            cpuMaskIn = 0xFFu;
            REPORT_ERROR(ErrorManagement::Warning, "CPUMask not specified using: %d", cpuMaskIn);
        }
        cpuMask = cpuMaskIn;
    }
    if (ret) {
        executor.SetCPUMask(cpuMask);
        ErrorManagement::ErrorType err = executor.Start();
        ret = err.ErrorsCleared();

    }

    return ret;
}

ErrorManagement::ErrorType MARTe2HieratikaMessageDispatcher::Execute(ExecutionInfo & info) {

    ErrorManagement::ErrorType err = ErrorManagement::NoError;
    if (info.GetStage() == ExecutionInfo::StartupStage) {

    }
    else if (info.GetStage() != ExecutionInfo::BadTerminationStage) {

        //pull from the queue
        ReferenceT < Message > message;
        ErrorManagement::ErrorType err = filter->GetMessage(message, messageTimeout);
        if (err.ErrorsCleared()) {
            //switch case
            //the payload must be a ConfigurationDatabase for parameters
            ReferenceT < ConfigurationDatabase > payload = message->Find("Payload");
            bool ret = payload.IsValid();
            if (ret) {
                StreamString functionName;
                ret = payload->Read("HieratikaCommand", functionName);
                if (ret) {

                    if (functionName == "Login") {
                        REPORT_ERROR(ErrorManagement::Information, "Hereee Login received");

                        StreamString userName;
                        StreamString passw;
                        ret = payload->Read("UserName", userName);
                        if (ret) {
                            ret = payload->Read("Password", passw);
                        }
                        if (ret) {

                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = LoginFunction(userName.Buffer(), passw.Buffer(), *response);

                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }

                    }
                    else if (functionName == "Logout") {
                        StreamString token;
                        StreamString passw;
                        ret = payload->Read("Token", token);
                        if (ret) {

                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = LogoutFunction(token.Buffer(), *response);

                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }

                    }
                    else if (functionName == "GetUsers") {
                        StreamString token;
                        ret = payload->Read("Token", token);
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetUsers(token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "GetTid") {
                        StreamString token;
                        ret = payload->Read("Token", token);
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetTid(token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "GetTransformationInfo") {
                        StreamString pageName;
                        StreamString token;
                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetTransformationInfo(pageName.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }

                    }
                    else if (functionName == "GetPages") {
                        StreamString token;
                        StreamString streamId;
                        ret = payload->Read("Token", token);
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetPages(token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "GetPage") {
                        StreamString pageName;
                        StreamString stream;
                        REPORT_ERROR(ErrorManagement::Information, "Hereee GetPage received");

                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetPage(pageName.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }

                    }
                    else if (functionName == "GetVariablesInfo") {
                        StreamString pageName;
                        StreamString variables;
                        StreamString token;
                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ret = payload->Read("Variables", variables);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetVariablesInfo(pageName.Buffer(), variables.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "GetScheduleFolders") {
                        StreamString pageName;
                        StreamString userName;
                        StreamString token;
                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ret = payload->Read("UserName", userName);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetScheduleFolders(pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "GetSchedules") {
                        StreamString pageName;
                        StreamString userName;
                        StreamString token;
                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ret = payload->Read("UserName", userName);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetSchedules(pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "GetSchedulesVariablesValue") {
                        StreamString pageName;
                        StreamString userName;
                        StreamString token;
                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ret = payload->Read("UserName", userName);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = GetSchedules(pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "UpdateSchedule") {
                        StreamString userName;
                        StreamString variables;
                        StreamString token;
                        StreamString scheduleUID;
                        StreamString tid;
                        ret = payload->Read("UserName", userName);
                        if (ret) {
                            ret = payload->Read("Variables", variables);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ret = payload->Read("ScheduleID", scheduleUID);
                        }
                        if (ret) {
                            ret = payload->Read("Tid", tid);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = UpdateSchedule(userName.Buffer(), variables.Buffer(), token.Buffer(), scheduleUID.Buffer(), tid.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }

                    }
                    else if (functionName == "Commit") {
                        StreamString userName;
                        StreamString variables;
                        StreamString token;
                        StreamString scheduleUID;
                        StreamString tid;
                        ret = payload->Read("UserName", userName);
                        if (ret) {
                            ret = payload->Read("Variables", variables);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ret = payload->Read("ScheduleID", scheduleUID);
                        }
                        if (ret) {
                            ret = payload->Read("Tid", tid);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = Commit(userName.Buffer(), variables.Buffer(), token.Buffer(), scheduleUID.Buffer(), tid.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "NewSchedule") {
                        StreamString scheduleName;
                        StreamString description;
                        StreamString pageName;
                        StreamString userName;
                        StreamString token;
                        ret = payload->Read("ScheduleName", scheduleName);
                        if (ret) {
                            //if empty schedule name then timestamp
                            if (scheduleName == "") {
                                TimeStamp date;
                                HighResolutionTimer::GetTimeStamp(date);
                                uint32 usec = date.GetMicroseconds();
                                uint32 sec = date.GetSeconds();
                                uint32 min = date.GetMinutes();
                                uint32 hour = date.GetHour();
                                uint32 day = date.GetDay();
                                uint32 month = date.GetMonth();
                                uint32 year = date.GetYear();
                                scheduleName.Printf("%d-%d-%d_%d:%d_%d:%d", year, month, day, hour, min, sec, usec);
                            }
                            ret = payload->Read("Description", description);
                        }
                        if (ret) {
                            ret = payload->Read("PageName", pageName);
                        }
                        if (ret) {
                            ret = payload->Read("UserName", userName);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = NewSchedule(scheduleName.Buffer(), description.Buffer(), pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "DeleteSchedule") {
                        StreamString scheduleUID;
                        StreamString token;
                        ret = payload->Read("ScheduleID", scheduleUID);
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = DeleteSchedule(scheduleUID.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }

                    }

                    else if (functionName == "UpdatePlant") {
                        StreamString pageName;
                        StreamString variables;
                        StreamString token;
                        StreamString tid;
                        ret = payload->Read("PageName", pageName);
                        if (ret) {
                            ret = payload->Read("Variables", variables);
                        }
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }
                        if (ret) {
                            ret = payload->Read("Tid", tid);
                        }
                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = UpdatePlant(pageName.Buffer(), variables.Buffer(), token.Buffer(), tid.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else if (functionName == "LoadPlant") {
                        StreamString pageNames;
                        StreamString token;
                        ret = payload->Read("PageNames", pageNames);
                        if (ret) {
                            ret = payload->Read("Token", token);
                        }

                        if (ret) {
                            ReferenceT < BufferedStreamI > stream;
                            ReferenceT < EventSem > semaphore;

                            SetResponseStream(stream, semaphore, message, payload);

                            ret = LoadPlant(pageNames.Buffer(), token.Buffer(), *response);
                            if (ret) {
                                ret = SendReply(stream, semaphore, message, payload);
                            }
                        }
                    }
                    else {
                        //error
                        REPORT_ERROR(ErrorManagement::FatalError, "Unrecognised hieratika command %s", functionName.Buffer());
                    }
                }
            }
            else {
                REPORT_ERROR(ErrorManagement::FatalError, "Please Define HieratikaCommand parameter inside Payload");
            }
        }
    }
    else {

    }
    return err;
}

bool MARTe2HieratikaMessageDispatcher::SendReply(ReferenceT<BufferedStreamI> &stream,
                                                 ReferenceT<EventSem> &semaphore,
                                                 ReferenceT<Message> &message,
                                                 ReferenceT<ConfigurationDatabase> &payload) {

    bool ret = true;

    if (semaphore.IsValid()) {
        semaphore->Post();
    }

    if (message->ExpectsIndirectReply()) {
        if (stream.IsValid()) {
            uint32 size = internalResponse.Size();
            ret = stream->Write(internalResponse.Buffer(), size);
        }
        if (ret) {
            ret = internalResponse.Seek(0ULL);
        }
        if (ret) {
            message->SetAsReply(true);
            ret = payload->Write("Response", internalResponse);
        }
        if (ret) {
            ret = SendMessage(message, this);
        }
    }
    return ret;
}

void MARTe2HieratikaMessageDispatcher::SetResponseStream(ReferenceT<BufferedStreamI> &stream,
                                                         ReferenceT<EventSem> &semaphore,
                                                         ReferenceT<Message> &message,
                                                         ReferenceT<ConfigurationDatabase> &payload) {
    StreamString streamId;
    if (payload->Read("Stream", streamId)) {
        stream = ObjectRegistryDatabase::Instance()->Find(streamId.Buffer());
    }
    StreamString semaphoreId;
    if (payload->Read("Semaphore", semaphoreId)) {
        semaphore = ObjectRegistryDatabase::Instance()->Find(semaphoreId.Buffer());
    }
    if (message->ExpectsIndirectReply()) {
        internalResponse.SetSize(0ULL);
        response = &internalResponse;
    }
    else {
        if (stream.IsValid()) {
            response = stream.operator->();
        }
    }

}

void MARTe2HieratikaMessageDispatcher::Purge(ReferenceContainer &purgeList) {
    if (!executor.Stop()) {
        if (!executor.Stop()) {
            REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
        }
    }
    Object::Purge(purgeList);
}

CLASS_REGISTER(MARTe2HieratikaMessageDispatcher, "1.0")

}
