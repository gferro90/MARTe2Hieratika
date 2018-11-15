/**
 * @file MARTe2HieratikaInterfaceTest.cpp
 * @brief Source file for class MARTe2HieratikaInterfaceTest
 * @date 19 ott 2018
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
 * the class MARTe2HieratikaInterfaceTest (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "MARTe2HieratikaInterfaceTest.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

static bool Login(MARTe2HieratikaInterface &test,
                  StreamString &token) {
    StreamString response;

    test.LoginFunction("codac-dev-1", "", response);
    printf("LoginFunction: %s\n", response.Buffer());

    const char8 *pattern = "\"token\": \"";
    StreamString tokenPtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
    printf("%s\n", tokenPtr.Buffer());

    token.SetSize(0ull);
    bool ret = tokenPtr.Size() > 0;
    if (ret) {
        char8 term;
        tokenPtr.Seek(0ull);
        tokenPtr.GetToken(token, "\"", term);
    }
    return ret;
}

static bool Logout(MARTe2HieratikaInterface &test,
                   StreamString &token) {

    StreamString response;
    test.LogoutFunction(token.Buffer(), response);
    return response.Size() == 0;

}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

MARTe2HieratikaInterfaceTest::MARTe2HieratikaInterfaceTest() {
    // Auto-generated constructor stub for MARTe2HieratikaInterfaceTest
    // TODO Verify if manual additions are needed
}

MARTe2HieratikaInterfaceTest::~MARTe2HieratikaInterfaceTest() {
    // Auto-generated destructor stub for MARTe2HieratikaInterfaceTest
    // TODO Verify if manual additions are needed
}

bool MARTe2HieratikaInterfaceTest::TestConstructor() {
    MARTe2HieratikaInterface test;
    StreamString serverAddr;

    test.GetServerAddress(serverAddr);
    bool ret = (serverAddr == "127.0.0.1");

    ret &= test.GetServerPort() == 8080;
    ret &= test.GetHttpExchangeTimeout() == 2000;

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestSetServerAddress() {
    MARTe2HieratikaInterface test;
    StreamString serverAddr;

    test.GetServerAddress(serverAddr);
    bool ret = (serverAddr = "127.0.0.1");

    test.SetServerAddress("192.168.1.2");
    test.GetServerAddress(serverAddr);

    ret &= (serverAddr == "192.168.1.2");

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestSetServerPort() {
    MARTe2HieratikaInterface test;

    bool ret = test.GetServerPort() == 8080;
    test.SetServerPort(4444);

    ret &= test.GetServerPort() == 4444;

    return ret;

}

bool MARTe2HieratikaInterfaceTest::TestSetHttpExchangeTimeout() {
    MARTe2HieratikaInterface test;
    bool ret = test.GetHttpExchangeTimeout() == 2000;
    test.SetHttpExchangeTimeout(4000);
    ret &= test.GetHttpExchangeTimeout() == 4000;
    return ret;

}

bool MARTe2HieratikaInterfaceTest::TestGetServerAddress() {
    return TestSetServerAddress();
}

bool MARTe2HieratikaInterfaceTest::TestGetServerPort() {
    return TestSetServerPort();
}

bool MARTe2HieratikaInterfaceTest::TestGetHttpExchangeTimeout() {
    return TestSetHttpExchangeTimeout();
}

bool MARTe2HieratikaInterfaceTest::TestLoginFunction() {
    StreamString response;
    MARTe2HieratikaInterface test;
    bool ret = test.LoginFunction("codac-dev-1", "", response);
    printf("LoginFunction: %s\n", response.Buffer());

    ret &= StringHelper::SearchString(response.Buffer(), "username") != NULL;
    ret &= StringHelper::SearchString(response.Buffer(), "token") != NULL;
    ret &= StringHelper::SearchString(response.Buffer(), "groups") != NULL;

    const char8 *pattern = "\"token\": \"";
    StreamString tokenPtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
    printf("%s\n", tokenPtr.Buffer());

    ret &= tokenPtr.Size() > 0;
    if (ret) {
        StreamString token;
        char8 term;
        tokenPtr.Seek(0ull);
        tokenPtr.GetToken(token, "\"", term);

        response.SetSize(0ull);
        ret = test.LogoutFunction(token.Buffer(), response);
        printf("LogoutFunction: %s\n", response.Buffer());

    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestLogoutFunction() {
    //need to do the login first
    StreamString response;
    MARTe2HieratikaInterface test;
    test.LoginFunction("codac-dev-1", "", response);
    printf("LoginFunction: %s\n", response.Buffer());

    const char8 *pattern = "\"token\": \"";
    StreamString tokenPtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
    printf("%s\n", tokenPtr.Buffer());

    bool ret = tokenPtr.Size() > 0;
    if (ret) {
        StreamString token;
        char8 term;
        tokenPtr.Seek(0ull);
        tokenPtr.GetToken(token, "\"", term);

        response.SetSize(0ull);
        ret = test.LogoutFunction(token.Buffer(), response);
        ret &= response.Size() == 0;

    }
    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetUsers() {

    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {
        ret = test.GetUsers(token.Buffer(), response);
        printf("GetUsers: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "codac-dev-1") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), token.Buffer()) != NULL;
        }

    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetTid() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {
        ret = test.GetTid(token.Buffer(), response);
        printf("GetUsers: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "tid") != NULL;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetTransformationInfo() {

    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {
        ret = test.GetTransformationInfo("DemoTypes", token.Buffer(), response);
        printf("GetTransformationInfo: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::Compare(response.Buffer(), "[]") == 0;
        }

    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetPages() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {
        ret = test.GetPages(token.Buffer(), response);
        printf("GetPages: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "DemoTypes") != NULL;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetPage() {

    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {
        ret = test.GetPage("DemoTypes", response);
        printf("GetPage: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "html") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "head") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "body") != NULL;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetVariablesInfo() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {
        ret = test.GetVariablesInfo("DemoTypes", "[\"DT@BT@DBLA\"]", token.Buffer(), response);
        printf("GetVariablesInfo: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "alias") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "numberOfElements") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "name") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "type") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "value") != NULL;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;

}

bool MARTe2HieratikaInterfaceTest::TestGetScheduleFolders() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;
    if (ret) {
        ret = test.GetScheduleFolders("DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("GetScheduleFolders: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            ret = StringHelper::Compare(response.Buffer(), "[]") == 0;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;

}

bool MARTe2HieratikaInterfaceTest::TestGetSchedules() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;
    if (ret) {
        ret = test.GetSchedules("DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("GetSchedules: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "description") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "name") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "owner") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "pageName") != NULL;
            ret &= StringHelper::SearchString(response.Buffer(), "uid") != NULL;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestGetSchedulesVariablesValue() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    StreamString scheduleUid;
    if (ret) {
        ret = test.GetSchedules("DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("GetSchedules: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            const char8 *pattern = "\"uid\": ";
            StreamString schedNamePtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
            char8 term;
            schedNamePtr.Seek(0ull);
            schedNamePtr.GetToken(scheduleUid, "\"", term);
        }
    }

    response.SetSize(0ull);
    if (ret) {

        ret = test.GetSchedulesVariablesValue(token.Buffer(), scheduleUid.Buffer(), response);
        printf("GetSchedulesVariablesValue: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "DT@BT@DBLA") != NULL;

        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestUpdateSchedule() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    StreamString tid;
    if (ret) {
        ret = test.GetTid(token.Buffer(), response);
        printf("GetTid: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            const char8 *pattern = "\"tid\": ";
            StreamString tidPtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
            char8 term;
            tidPtr.Seek(0ull);
            tidPtr.GetToken(tid, "\"", term);
        }
    }

    response.SetSize(0ull);

    StreamString scheduleUid;
    if (ret) {
        ret = test.GetSchedules("DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("GetSchedules: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            const char8 *pattern = "\"uid\": ";
            StreamString schedNamePtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
            char8 term;
            schedNamePtr.Seek(0ull);
            schedNamePtr.GetToken(scheduleUid, "\"", term);
        }
    }

    response.SetSize(0ull);
    if (ret) {

        ret = test.UpdateSchedule("codac-dev-1", "{\"DT@BT@DBLA\":  [1, 2, 3, 4, 5, 6]}", token.Buffer(), tid.Buffer(), scheduleUid.Buffer(), response);
        printf("GetSchedulesVariablesValue: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::Compare(response.Buffer(), "ok") == 0;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestCommit() {

    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    StreamString tid;
    if (ret) {
        ret = test.GetTid(token.Buffer(), response);
        printf("GetTid: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            const char8 *pattern = "\"tid\": ";
            StreamString tidPtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
            char8 term;
            tidPtr.Seek(0ull);
            tidPtr.GetToken(tid, "\"", term);
        }
    }

    response.SetSize(0ull);

    StreamString scheduleUid;
    if (ret) {
        ret = test.GetSchedules("DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("GetSchedules: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            const char8 *pattern = "\"uid\": ";
            StreamString schedNamePtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
            char8 term;
            schedNamePtr.Seek(0ull);
            schedNamePtr.GetToken(scheduleUid, "\"", term);
        }
    }

    response.SetSize(0ull);
    if (ret) {

        ret = test.Commit("codac-dev-1", "{\"DT@BT@DBLA\":  [2, 3, 4, 5, 6, 7]}", token.Buffer(), tid.Buffer(), scheduleUid.Buffer(), response);
        printf("Commit: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::Compare(response.Buffer(), "ok") == 0;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestNewSchedule() {

    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {

        ret = test.NewSchedule("new_schedule", "a new test schedule", "DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("NewSchedule: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::SearchString(response.Buffer(), "new_schedule") != NULL;
        }
    }
    response.SetSize(0ull);
    StreamString scheduleUid;
    if (ret) {
        ret = test.GetSchedules("DemoTypes", "codac-dev-1", token.Buffer(), response);
        printf("GetSchedules: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {

            const char8 *beginPoint= StringHelper::SearchString(response.Buffer(), "new_schedule");


            const char8 *pattern = "\"uid\": ";
            StreamString schedNamePtr = StringHelper::SearchString(beginPoint, pattern) + StringHelper::Length(pattern);
            char8 term;
            schedNamePtr.Seek(0ull);
            schedNamePtr.GetToken(scheduleUid, "\"", term);
        }
    }
    response.SetSize(0ull);
    if (ret) {
        scheduleUid.Seek(0ull);
        ret = test.DeleteSchedule(scheduleUid.Buffer(), token.Buffer(), response);
        printf("DeleteSchedule: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::Compare(response.Buffer(), "ok") == 0;
        }
    }

    if (ret) {
        ret = Logout(test, token);
    }

    return ret;
}

bool MARTe2HieratikaInterfaceTest::TestDeleteSchedule() {
    return TestNewSchedule();
}

bool MARTe2HieratikaInterfaceTest::TestUpdatePlant() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    StreamString tid;
    if (ret) {
        ret = test.GetTid(token.Buffer(), response);
        printf("GetTid: %s\n", response.Buffer());
        if (ret) {
            //do some checks
            const char8 *pattern = "\"tid\": ";
            StreamString tidPtr = StringHelper::SearchString(response.Buffer(), pattern) + StringHelper::Length(pattern);
            char8 term;
            tidPtr.Seek(0ull);
            tidPtr.GetToken(tid, "\"", term);
        }
    }


    response.SetSize(0ull);
    if (ret) {

        ret = test.UpdatePlant("DemoTypes", "{\"DT@BT@DBLA\":  [2, 3, 4, 5, 6, 7]}", token.Buffer(), tid.Buffer(), response);
        printf("UpdatePlant: %s\n", response.Buffer());
        //at least one schedule
        if (ret) {
            //do some checks
            ret = StringHelper::Compare(response.Buffer(), "ok") == 0;
        }
    }



    if (ret) {
        ret = Logout(test, token);
    }

    return ret;

}

bool MARTe2HieratikaInterfaceTest::TestLoadPlant() {
    MARTe2HieratikaInterface test;

    StreamString token;
    bool ret = Login(test, token);

    StreamString response;

    if (ret) {

           ret = test.LoadPlant("[\"DemoTypes\"]", token.Buffer(), response);
           printf("LoadPlant: %s\n", response.Buffer());
           //at least one schedule
           if (ret) {
               //do some checks
               ret = StringHelper::Compare(response.Buffer(), "ok") == 0;
           }
       }


    if (ret) {
        ret = Logout(test, token);
    }

    return ret;

}

