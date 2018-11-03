/**
 * @file Test.cpp
 * @brief Source file for class Test
 * @date 15 ott 2018
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
 * the class Test (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "Source/Components/Interfaces/MARTe2HieratikaInterface/MARTe2HieratikaInterface.h"
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
#include "HttpClient.h"
#include "HttpDefinition.h"
#include "JsonParser.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

using namespace MARTe;

int main(int argc,
         char **argv) {

//	HttpClient test;

    MARTe2HieratikaInterface m2HierInterface;
#if 0
    test.SetServerAddress("127.0.0.1");
    test.SetServerPort(8080);
    test.SetServerUri("/login");
#endif
    StreamString readOut;

    //Step 1: login
    if (!m2HierInterface.LoginFunction("codac-dev-1", "", readOut)) {
        return -1;
    }

    printf("login response=%s\n", readOut.Buffer());

    StreamString reply = "\"reply\": ";
    reply += readOut;
    reply.Seek(0);
    ConfigurationDatabase data;
    JsonParser jsonparser(reply, data);

    jsonparser.Parse();

    StreamString token;
    data.MoveAbsolute("reply");
    data.Read("token", token);
    printf("token=%s\n", token.Buffer());

    //step 2: get users (really needed?)
    //test.SetServerUri("/getusers");
    readOut.SetSize(0);

    if (!m2HierInterface.GetUsers(token.Buffer(), readOut)) {
        return -1;
    }

    printf("getusers response=%s\n", readOut.Buffer());

    //step 3 stream??

    //step 4 gettransformationinfo
    //test.SetServerUri("/gettransformationsinfo");

    readOut.SetSize(0);

    if (!m2HierInterface.GetTransformationInfo("FALCON", token.Buffer(), readOut)) {
        return -1;
    }
    printf("GetTransformationInfo response=%s\n", readOut.Buffer());

    //step 4b getpages

    readOut.SetSize(0);
    if (!m2HierInterface.GetPages(token.Buffer(), readOut)) {
        return -1;
    }
    printf("GetPages response=%s\n", readOut.Buffer());

    //step 5 get the page
    //test.SetServerUri("/pages/FALCON.html?1539601761805");

    readOut.SetSize(0);
    if (!m2HierInterface.GetPage("FALCON", readOut)) {
        return -1;
    }
    printf("GetPage response=%s\n", readOut.Buffer());

    //step 6 get variable info
    //StreamString varList;
    StreamString variables;

#if 0
    StreamString variables = "[";

    const char8* buff = readOut.Buffer();
    bool first = true;
    uint32 i = 0u;
    while (buff != NULL && i < 14000) {
        buff = StringHelper::SearchString(buff, "id=");
        if (buff != NULL) {
            buff += 3;

            //char8 sep[2] = { '\0', '\0' };
            if (buff[0] == '\'' || buff[0] == '"') {
                //sep[0] = buff[0];
                buff++;

                char8 term;
                StreamString idx = buff;
                idx.Seek(0);
                StreamString variable;
                idx.GetToken(variable, "\"'", term);
                printf("var=%s\n", variable.Buffer());
                if (StringHelper::SearchChar(variable.Buffer(), '@') != NULL) {
                    if (!first) {
                        variables += ",";
                    }
                    variables += "\"";
                    variables += variable;
                    variables += "\"";
                    first = false;
                    i++;
                }
            }
        }
    }
    variables += "]";
#endif

    readOut.Seek(0);
    if (!m2HierInterface.ExtractAllVariablesFromPage(readOut, variables)) {
        return -1;
    }
    //test.SetServerUri("/getvariablesinfo");

    printf("%s\n", variables.Buffer());
    readOut.SetSize(0);
    if (!m2HierInterface.GetVariablesInfo("FALCON", variables.Buffer(), token.Buffer(), readOut)) {
        return -1;
    }

    printf("GetVariablesInfo response=%s\n", readOut.Buffer());

    //step 7 get schedulefolders
    //test.SetServerUri("/getschedulefolders");

    readOut.SetSize(0);
    if (!m2HierInterface.GetScheduleFolders("FALCON", "codac-dev-1", token.Buffer(), readOut)) {
        return -1;
    }

    printf("GetScheduleFolders response=%s\n", readOut.Buffer());

    //step 8 get schedules
    //test.SetServerUri("/getschedules");

    readOut.SetSize(0);
    if (!m2HierInterface.GetSchedules("FALCON", "codac-dev-1", token.Buffer(), readOut)) {
        return -1;
    }

    printf("GetSchedules response=%s\n", readOut.Buffer());

    StreamString scheduleUID;

    const char8*beg = StringHelper::SearchString(readOut.Buffer(), "pageName");
    beg++;
    beg = StringHelper::SearchString(beg, "pageName");

    if (beg != NULL) {
        const char8* toSearch = "\"uid\": ";
        const char8*ptr = StringHelper::SearchString(beg, toSearch);
        if (ptr != NULL) {
            StreamString scheduleUIDtot = ptr + StringHelper::Length(toSearch) + 1;
            char8 term;
            scheduleUIDtot.Seek(0);
            scheduleUIDtot.GetToken(scheduleUID, "\" \\", term);
        }
        printf("%s\n", scheduleUID.Buffer());
    }
    //step 10 get schedule variables
    //test.SetServerUri("/getschedulevariablesvalues");

    readOut.SetSize(0);
    if (!m2HierInterface.GetSchedulesVariablesValue(token.Buffer(), scheduleUID.Buffer(), readOut)) {
        return -1;
    }

    printf("GetSchedulesVariableValues response=%s\n", readOut.Buffer());

    //step 9 post an update
    //test.SetServerUri("/updateschedule");

    readOut.SetSize(0);
    const char8 *vars =
            "{\"FALCON@FALCON\":[[1,1,3,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0],[0,0,0],[0,0,0]]}";
    if (!m2HierInterface.UpdateSchedule("codac-dev-1", vars, token.Buffer(), scheduleUID.Buffer(), readOut)) {
        return -1;
    }

    printf("UpdateSchedule response=%s\n", readOut.Buffer());

    //step 10 commit
    //test.SetServerUri("/commitschedule");

    readOut.SetSize(0ULL);
    if (!m2HierInterface.Commit("codac-dev-1", vars, token.Buffer(), scheduleUID.Buffer(), readOut)) {
        return -1;
    }

    printf("CommitSchedule response=%s\n", readOut.Buffer());

    //step 11 create a new schedule
    //test.SetServerUri("/createschedule");

    readOut.SetSize(0);
    if (!m2HierInterface.NewSchedule("pippopluto", "pippopluto commit", "FALCON", "codac-dev-1", token.Buffer(), readOut)) {
        return -1;
    }

    printf("CreateSchedule response=%s\n", readOut.Buffer());
    //step 12 Load into plant
    //test.SetServerUri("/loadintoplant");

    readOut.SetSize(0);

    if (!m2HierInterface.LoadPlant("pippopluto", "pippopluto commit", "[\"FALCON\"]", token.Buffer(), readOut)) {
        return -1;
    }

    printf("LoadPlant response=%s\n", readOut.Buffer());
    printf("EVERYTHING OK!!\n");
    return 1;
}

