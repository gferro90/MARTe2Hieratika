/**
 * @file DiodeStandaloneRec.cpp
 * @brief Source file for class DiodeStandaloneRec
 * @date 03 dic 2018
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
 * the class DiodeStandaloneRec (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

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
#include "EpicsParserAndSubscriber.h"
#include "DiodeReceiver.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;
void MainErrorProcessFunction(const MARTe::ErrorManagement::ErrorInformation &errorInfo, const char * const errorDescription) {
    MARTe::StreamString errorCodeStr;
    MARTe::ErrorManagement::ErrorCodeToStream(errorInfo.header.errorType, errorCodeStr);
    printf("[%s - %s:%d]: %s\n", errorCodeStr.Buffer(), errorInfo.fileName, errorInfo.header.lineNumber, errorDescription);
}
static bool keepRunning = true;
static bool killApp = false;
ReferenceT < DiodeReceiver > receiver;

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
    if (receiver.IsValid()) {
        receiver->Stop();
    }
    MARTe::ObjectRegistryDatabase::Instance()->Purge();
    printf("Application successfully stopped.\n");
    keepRunning = false;
    _exit(0);
}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

int main(int argc,
         const char **argv) {
    ProcessorType::SetDefaultCPUs(0x1);
    SetErrorProcessFunction(&MainErrorProcessFunction);
    BasicFile configFile;
    if (!configFile.Open(argv[1], File::ACCESS_MODE_R)) {
        printf("Failed opening configuration file %s\n", argv[1]);
        return -1;
    }

    ConfigurationDatabase localCdb;
    configFile.Seek(0ull);
    StandardParser parser(configFile, localCdb);
    parser.Parse();
    localCdb.MoveToRoot();

    ObjectRegistryDatabase *god = ObjectRegistryDatabase::Instance();
    god->Initialise(localCdb);

    receiver = god->Find("Receiver");

    if (receiver.IsValid()) {
        receiver->Start();
        signal(SIGTERM, StopApp);
        signal(SIGINT, StopApp);
        while(1){
            Sleep::Sec(10u);
        }
    }


    return 0;
}

