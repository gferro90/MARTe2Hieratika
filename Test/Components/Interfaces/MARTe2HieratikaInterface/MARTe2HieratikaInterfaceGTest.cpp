/**
 * @file MARTe2HieratikaInterfaceGTest.cpp
 * @brief Source file for class MARTe2HieratikaInterfaceGTest
 * @date 05 nov 2018
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
 * the class MARTe2HieratikaInterfaceGTest (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include <limits.h>

#include "../MARTe2HieratikaInterface/MARTe2HieratikaInterfaceTest.h"
#include "gtest/gtest.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
TEST(MARTe2HieratikaInterfaceGTest,TestConstructor) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestConstructor());
}

TEST(MARTe2HieratikaInterfaceGTest,TestSetServerAddress) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestSetServerAddress());
}

TEST(MARTe2HieratikaInterfaceGTest,TestSetServerPort) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestSetServerPort());
}

TEST(MARTe2HieratikaInterfaceGTest,TestSetHttpExchangeTimeout) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestSetHttpExchangeTimeout());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetServerAddress) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetServerAddress());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetServerPort) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetServerPort());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetHttpExchangeTimeout) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetHttpExchangeTimeout());
}

TEST(MARTe2HieratikaInterfaceGTest,TestLoginFunction) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestLoginFunction());
}

TEST(MARTe2HieratikaInterfaceGTest,TestLogoutFunction) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestLogoutFunction());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetUsers) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetUsers());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetTid) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetTid());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetTransformationInfo) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetTransformationInfo());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetPages) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetPages());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetPage) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetPage());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetVariablesInfo) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetVariablesInfo());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetScheduleFolders) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetScheduleFolders());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetSchedules) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetSchedules());
}

TEST(MARTe2HieratikaInterfaceGTest,TestGetSchedulesVariablesValue) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestGetSchedulesVariablesValue());
}

TEST(MARTe2HieratikaInterfaceGTest,TestUpdateSchedule) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestUpdateSchedule());
}

TEST(MARTe2HieratikaInterfaceGTest,TestCommit) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestCommit());
}

TEST(MARTe2HieratikaInterfaceGTest,TestNewSchedule) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestNewSchedule());
}

TEST(MARTe2HieratikaInterfaceGTest,TestDeleteSchedule) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestDeleteSchedule());
}

TEST(MARTe2HieratikaInterfaceGTest,TestUpdatePlant) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestUpdatePlant());
}

TEST(MARTe2HieratikaInterfaceGTest,TestLoadPlant) {
    MARTe2HieratikaInterfaceTest test;
    ASSERT_TRUE(test.TestLoadPlant());
}
