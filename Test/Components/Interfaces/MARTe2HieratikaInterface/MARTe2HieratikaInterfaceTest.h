/**
 * @file MARTe2HieratikaInterfaceTest.h
 * @brief Header file for class MARTe2HieratikaInterfaceTest
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

 * @details This header file contains the declaration of the class MARTe2HieratikaInterfaceTest
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef MARTE2HIERATIKAINTERFACETEST_H_
#define MARTE2HIERATIKAINTERFACETEST_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MARTe2HieratikaInterface.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

using namespace MARTe;

class MARTe2HieratikaInterfaceTest {
public:
    MARTe2HieratikaInterfaceTest();
    ~MARTe2HieratikaInterfaceTest();

    bool TestConstructor();

    bool TestSetServerAddress();

    bool TestSetServerPort();

    bool TestSetHttpExchangeTimeout();

    bool TestGetServerAddress();

    bool TestGetServerPort();

    bool TestGetHttpExchangeTimeout();

    bool TestLoginFunction();

    bool TestLogoutFunction();

    bool TestGetUsers();

    bool TestGetTid();

    bool TestGetTransformationInfo();

    bool TestGetPages();

    bool TestGetPage();

    bool TestGetVariablesInfo();

    bool TestGetScheduleFolders();

    bool TestGetSchedules();

    bool TestGetSchedulesVariablesValue();

    bool TestUpdateSchedule();

    bool TestCommit();

    bool TestNewSchedule();

    bool TestDeleteSchedule();

    bool TestUpdatePlant();

    bool TestLoadPlant();

};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* MARTE2HIERATIKAINTERFACETEST_H_ */

