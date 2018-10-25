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

#include "MARTe2HieratikaMessageDispatcher.h"

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
            REPORT_ERROR(ErrorManagement::InitialisationError, "ServerPort not defined")
        }

        if (ret) {
            uint32 timeoutT;
            if (data.Read("ReceiveMessageTimeout", timeoutT)) {
                timeout = timeoutT;
            }
            else {
                timeout = 1000u;
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
        REPORT_ERROR(ErrorManagement::InitialisationError, "ServerIpAddress not defined")
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
        ErrorManagement::ErrorType err = filter->GetMessage(message, timeout);
        if (err.ErrorsCleared()) {
            //switch case
            StreamString functionName = (const char8 *) (message->GetFunction());

            //the payload must be a ConfigurationDatabase for parameters
            ReferenceT < ConfigurationDatabase > payload = message->Get(0u);
            bool ret = payload.IsValid();

            if (functionName == "LoginFunction") {
                StreamString userName;
                StreamString passw;
                ret = payload->Read("UserName", userName);
                if (ret) {
                    ret = payload->Read("Password", passw);
                }
                if (ret) {
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = LoginFunction(userName.Buffer(), passw.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }

            }
            else if (functionName == "GetUsers") {
                StreamString token;
                ret = payload->Read("Token", token);
                if (ret) {
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = GetUsers(token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
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
                    SetResponseStream(stream, message, payload);

                    ret = GetTransformationInfo(pageName.Buffer(), token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }

            }
            else if (functionName == "GetPages") {
                StreamString token;
                StreamString streamId;
                ret = payload->Read("Token", token);
                if (ret) {
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = GetPages(token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }
            }
            else if (functionName == "GetPage") {
                StreamString pageName;
                StreamString stream;
                ret = payload->Read("PageName", pageName);
                if (ret) {
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = GetPage(pageName.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
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
                    SetResponseStream(stream, message, payload);

                    ret = GetVariablesInfo(pageName.Buffer(), variables.Buffer(), token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
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
                    SetResponseStream(stream, message, payload);

                    ret = GetScheduleFolders(pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
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
                    SetResponseStream(stream, message, payload);

                    ret = GetSchedules(pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
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
                    SetResponseStream(stream, message, payload);

                    ret = GetSchedules(pageName.Buffer(), userName.Buffer(), token.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }
            }
            else if (functionName == "UpdateSchedule") {
                StreamString userName;
                StreamString variables;
                StreamString token;
                StreamString scheduleUID;
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
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = UpdateSchedule(userName.Buffer(), variables.Buffer(), token.Buffer(), scheduleUID.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }

            }
            else if (functionName == "Commit") {
                StreamString userName;
                StreamString variables;
                StreamString token;
                StreamString scheduleUID;
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
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = Commit(userName.Buffer(), variables.Buffer(), token.Buffer(), scheduleUID.Buffer(), *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }
            }
            else if (functionName == "NewSchedule") {
                StreamString scheduleName;
                StreamString description;
                StreamString pageName;
                StreamString userName;
                StreamString token;
                StreamString scheduleUID;
                ret = payload->Read("ScheduleName", scheduleName);
                if (ret) {
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
                    ret = payload->Read("ScheduleID", scheduleUID);
                }
                if (ret) {
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = NewSchedule(scheduleName.Buffer(), description.Buffer(), pageName.Buffer(), userName.Buffer(), token.Buffer(), scheduleUID.Buffer(),
                                      *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }
            }
            else if (functionName == "LoadPlant") {
                StreamString scheduleName;
                StreamString userName;
                StreamString description;
                StreamString pageNames;
                StreamString token;
                StreamString scheduleUID;
                ret = payload->Read("ScheduleName", scheduleName);
                if (ret) {
                    ret = payload->Read("UserName", userName);
                }
                if (ret) {
                    ret = payload->Read("Description", description);
                }
                if (ret) {
                    ret = payload->Read("PageNames", pageNames);
                }
                if (ret) {
                    ret = payload->Read("Token", token);
                }
                if (ret) {
                    ret = payload->Read("ScheduleID", scheduleUID);
                }
                if (ret) {
                    ReferenceT < BufferedStreamI > stream;
                    SetResponseStream(stream, message, payload);

                    ret = LoadPlant(scheduleName.Buffer(), userName.Buffer(), description.Buffer(), pageNames.Buffer(), token.Buffer(), scheduleUID.Buffer(),
                                    *response);
                    if (ret) {
                        ret = SendReply(stream, message, payload);
                    }
                }
            }
            else {
                //todo error
            }
        }
    }
    else {

    }
    return err;
}

bool MARTe2HieratikaMessageDispatcher::SendReply(ReferenceT<BufferedStreamI> &stream,
                                                 ReferenceT<Message> &message,
                                                 ReferenceT<ConfigurationDatabase> &payload) {

    bool ret = true;
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
                                                         ReferenceT<Message> &message,
                                                         ReferenceT<ConfigurationDatabase> &payload) {
    StreamString streamId;
    if (payload->Read("Stream", streamId)) {
        stream = ObjectRegistryDatabase::Instance()->Find(streamId.Buffer());
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

CLASS_REGISTER(MARTe2HieratikaMessageDispatcher, "1.0")

}
