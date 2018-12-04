/**
 * @file Main.cpp
 * @brief Source file for class Main
 * @date 01/03/2017
 * @author Andre' Neto
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
 * the class Main (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "ClassRegistryDatabase.h"
#include "ClassRegistryItem.h"
#include "ConfigurationDatabase.h"
#include "ErrorManagement.h"
#include "File.h"
#include "GlobalObjectsDatabase.h"
#include "Object.h"
#include "ObjectRegistryDatabase.h"
#include "ProcessorType.h"
#include "RealTimeApplication.h"
#include "Reference.h"
#include "ReferenceT.h"
#include "StreamString.h"
#include "StandardParser.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

void MainErrorProcessFunction(const MARTe::ErrorManagement::ErrorInformation &errorInfo, const char * const errorDescription) {
    MARTe::StreamString errorCodeStr;
    MARTe::ErrorManagement::ErrorCodeToStream(errorInfo.header.errorType, errorCodeStr);
    printf("[%s - %s:%d]: %s\n", errorCodeStr.Buffer(), errorInfo.fileName, errorInfo.header.lineNumber, errorDescription);
}

static MARTe::ReferenceT<MARTe::RealTimeApplication> rtApp;

static bool keepRunning = true;
static bool killApp = false;
static void StopApp(int sig) {
    //Second time this is called? Kill the application.
    if (!killApp) {
        killApp = true;
    }
    else {
        printf("Application killed.\n");  
        _exit(0);
    }
    printf("Stopping application.\n");  
    if (rtApp.IsValid()) {
        rtApp->StopCurrentStateExecution();
    }
    MARTe::ObjectRegistryDatabase::Instance()->Purge();
    printf("Application successfully stopped.\n");
    keepRunning = false;
    _exit(0);
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    using namespace MARTe;
    ProcessorType::SetDefaultCPUs(0x1);
    SetErrorProcessFunction(&MainErrorProcessFunction);
    if (argc != 5) {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are -f FILENAME -s FIRST_STATE | -m MSG_DESTINATION:MSG_FUNCTION");
        return -1;
    }
    StreamString argv1 = argv[1];
    StreamString argv3 = argv[3];
    StreamString filename;
    StreamString firstState;
    StreamString messageArgs;

    if (argv1 == "-f") {
        filename = argv[2];
    }
    else if (argv3 == "-f") {
        filename = argv[4];
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are -f FILENAME -s FIRST_STATE | -m MSG_DESTINATION:MSG_FUNCTION");
        return -1;
    }

    if (argv1 == "-s") {
        firstState = argv[2];
    }
    else if (argv3 == "-s") {
        firstState = argv[4];
    }
    else if (argv1 == "-m") {
        messageArgs = argv[2];
    }
    else if (argv3 == "-m") {
        messageArgs = argv[4];
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Arguments are -f FILENAME -s FIRST_STATE | -m MSG_DESTINATION:MSG_FUNCTION");
        return -1;
    }
    mlockall(MCL_CURRENT | MCL_FUTURE);

    BasicFile f;
    bool ok = f.Open(filename.Buffer(), BasicFile::ACCESS_MODE_R);
    if (ok) {
        f.Seek(0);
    }
    else {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Failed to open input file");
    }
    ConfigurationDatabase cdb;
    StreamString err;
    if (ok) {
        StandardParser parser(f, cdb, &err);
        ok = parser.Parse();
    }
    if (!ok) {
        StreamString errPrint;
        errPrint.Printf("Failed to parse %s", err.Buffer());
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, errPrint.Buffer());
    }

    ObjectRegistryDatabase *objDb = NULL;
    if (ok) {
        objDb = ObjectRegistryDatabase::Instance();
        objDb->Initialise(cdb);
    }
    if (!ok) {
        REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Failed to load godb");
    }

    if (ok) {
        uint32 nOfObjs = objDb->Size();
        uint32 n;
        bool found = false;
        for (n = 0u; (n < nOfObjs) && (!found); n++) {
            rtApp = objDb->Get(n);
            found = rtApp.IsValid();
            if (found) {
                ok = rtApp->ConfigureApplication();
                if (!ok) {
                    REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Failed to load Configure RealTimeApplication");
                    return -1;
                }
                if (ok) {
                    if (firstState.Size() > 0) {
                        ok = rtApp->PrepareNextState(firstState.Buffer());

                        if (ok) {
                            rtApp->StartNextStateExecution();
                        }
                    }
                    else {
                        ReferenceT<Message> message(new Message());
                        ConfigurationDatabase msgConfig;
                        StreamString destination;
                        StreamString function;
                        char8 term;
                        messageArgs.Seek(0LLU);
                        ok = messageArgs.GetToken(destination, ":", term);
                        if (ok) {
                            ok = messageArgs.GetToken(function, ":", term);
                        }
                        if (!ok) {
                            REPORT_ERROR_STATIC(ErrorManagement::ParametersError, "Message format is MSG_DESTINATION:MSG_FUNCTION");
                        }
                        msgConfig.Write("Destination", destination.Buffer());
                        msgConfig.Write("Function", function.Buffer());
                        if (ok) {
                            ok = message->Initialise(msgConfig);
                        }
                        if (ok) {
                            MessageI::SendMessage(message);
                        }
                    }
                }
            }
        }
        if (!found) {
            ok = false;
        }
    }
    f.Close();
    if (ok) {
        signal(SIGTERM, StopApp);
        signal(SIGINT, StopApp);
        while (keepRunning) {
            Sleep::Sec(1.0);
        }
    }

    munlockall();
    return 0;
}

