/**
 * @file PriorityGAMTest.cpp
 * @brief Source file for class PriorityGAMTest
 * @date 30 nov 2018
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
 * the class PriorityGAMTest (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "PriorityGAMTest.h"
#include "StandardParser.h"
#include "ConfigurationDatabase.h"
#include "ObjectRegistryDatabase.h"
#include "RealTimeApplication.h"
#include "MemoryDataSourceI.h"
#include "MemoryMapSynchronisedInputBroker.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
class PriorityGAMTestGAM: public PriorityGAM {
public:
    CLASS_REGISTER_DECLARATION()
    PriorityGAMTestGAM();
    virtual ~PriorityGAMTestGAM();
    uint8 *GetPrevSignalMem();
    uint32 *GetSortedIndices();
    uint32 GetNumberOfSignalToBeSent();
    uint32 GetTotalSize();
    uint8 GetFirstTime();
    uint32 *GetIndexList();
    uint32 GetGurrentIdx();
    uint32 GetCurrentChangePos();
    void *GetOutputSignalMemoryX(const uint32 signalIdx) const;
};

PriorityGAMTestGAM::PriorityGAMTestGAM() {

}

PriorityGAMTestGAM::~PriorityGAMTestGAM() {

}

void *PriorityGAMTestGAM::GetOutputSignalMemoryX(const uint32 signalIdx) const{
    return GAM::GetOutputSignalMemory(signalIdx);
}


uint8 *PriorityGAMTestGAM::GetPrevSignalMem() {
    return prevSignalMem;
}

uint32 *PriorityGAMTestGAM::GetSortedIndices() {
    return sortedIndices;
}

uint32 PriorityGAMTestGAM::GetNumberOfSignalToBeSent() {
    return numberOfSignalToBeSent;
}

uint32 PriorityGAMTestGAM::GetTotalSize() {
    return totalSize;
}

uint8 PriorityGAMTestGAM::GetFirstTime() {
    return chunckCounter;
}

uint32 *PriorityGAMTestGAM::GetIndexList() {
    return indexList;
}

uint32 PriorityGAMTestGAM::GetGurrentIdx() {
    return currentIdx;
}

uint32 PriorityGAMTestGAM::GetCurrentChangePos() {
    return currentChangePos;
}
CLASS_REGISTER(PriorityGAMTestGAM, "1.0")

class PriorityGAMTestDS: public MemoryDataSourceI {
public:
    CLASS_REGISTER_DECLARATION()

    PriorityGAMTestDS();
    virtual ~PriorityGAMTestDS();

    virtual bool Synchronise();

    virtual const char8 *GetBrokerName(StructuredDataI &data,
                                       const SignalDirection direction);

    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    virtual bool SetChangeRate(uint32 signalIdx,
                               uint32 changeRateIn);

    virtual bool SetConfiguredDatabase(StructuredDataI & data);

private:
    uint32 *changeRate;
    uint32 cycleCounter;
};

PriorityGAMTestDS::PriorityGAMTestDS() {
    changeRate = NULL;
    cycleCounter = 0u;
}

PriorityGAMTestDS::~PriorityGAMTestDS() {

}

bool PriorityGAMTestDS::Synchronise() {
    cycleCounter++;
    for (uint32 i = 0u; i < numberOfSignals; i++) {
        if ((cycleCounter % changeRate[i]) == 0u) {
            uint32 *ptr;
            GetSignalMemoryBuffer(i, 0u, (void*&) ptr);
            (*ptr)++;
        }
    }
    return true;
}

const char8 *PriorityGAMTestDS::GetBrokerName(StructuredDataI &data,
                                              const SignalDirection direction) {
    return "MemoryMapSynchronisedInputBroker";
}

bool PriorityGAMTestDS::PrepareNextState(const char8 * const currentStateName,
                                         const char8 * const nextStateName) {
    return true;
}

bool PriorityGAMTestDS::SetChangeRate(uint32 signalIdx,
                                      uint32 changeRateIn) {
    if (signalIdx < numberOfSignals) {
        changeRate[signalIdx] = changeRateIn;
    }
}

bool PriorityGAMTestDS::SetConfiguredDatabase(StructuredDataI & data) {
    bool ret = MemoryDataSourceI::SetConfiguredDatabase(data);
    if (ret) {
        changeRate = new uint32[numberOfSignals];
        for (uint32 i = 0u; i < numberOfSignals; i++) {
            changeRate[i] = 10u;
        }
    }
    return ret;
}

CLASS_REGISTER(PriorityGAMTestDS, "1.0")

/**
 * Helper function to setup a MARTe execution environment
 */
static bool InitialiseMemoryMapInputBrokerEnviroment(const char8 * const config) {

    HeapManager::AddHeap(GlobalObjectsDatabase::Instance()->GetStandardHeap());
    ConfigurationDatabase cdb;
    StreamString configStream = config;
    configStream.Seek(0);
    StandardParser parser(configStream, cdb);

    bool ok = parser.Parse();

    ObjectRegistryDatabase *god = ObjectRegistryDatabase::Instance();

    if (ok) {
        god->Purge();
        ok = god->Initialise(cdb);
    }
    ReferenceT < RealTimeApplication > application;
    if (ok) {
        application = god->Find("Application1");
        ok = application.IsValid();
    }
    if (ok) {
        ok = application->ConfigureApplication();
    }
    return ok;
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

PriorityGAMTest::PriorityGAMTest() {
    // Auto-generated constructor stub for PriorityGAMTest
    // TODO Verify if manual additions are needed
}

PriorityGAMTest::~PriorityGAMTest() {
    // Auto-generated destructor stub for PriorityGAMTest
    // TODO Verify if manual additions are needed
}

bool PriorityGAMTest::TestConstructor() {
    PriorityGAMTestGAM test;
    bool ret = test.GetPrevSignalMem() == NULL;
    ret &= test.GetSortedIndices() == NULL;
    ret &= test.GetNumberOfSignalToBeSent() == 0u;
    ret &= test.GetTotalSize() == 0u;
    ret &= test.GetFirstTime() == 0u;
    ret &= ret &= test.GetIndexList() == NULL;
    ret &= test.GetGurrentIdx() == 0u;
    ret &= test.GetCurrentChangePos() == 0u;

    return true;
}

bool PriorityGAMTest::TestSetup() {

    char8 *config = "$Application1 = {"
            "    Class = RealTimeApplication"
            "    +Functions = {"
            "        Class = ReferenceContainer"
            "        +GAMA = {"
            "            Class = PriorityGAM"
            "            InputSignals = {"
            "               Signal1 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "                   Frequency = 0"
            "               }"
            "               Signal2 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "            }"
            "            OutputSignals = {"
            "               Signal1 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal2 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Indexes = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "                   NumberOfDimensions = 1"
            "                   NumberOfElements = 5"
            "               }"
            "            }"
            "        }"
            "    }"
            "    +Data = {"
            "        Class = ReferenceContainer"
            "        +Drv1 = {"
            "            Class = PriorityGAMTestDS"
            "        }"
            "        +DDB1 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +Timings = {"
            "            Class = TimingDataSource"
            "        }"
            "    }"
            "    +States = {"
            "        Class = ReferenceContainer"
            "        +State1 = {"
            "            Class = RealTimeState"
            "            +Threads = {"
            "                Class = ReferenceContainer"
            "                +Thread1 = {"
            "                    Class = RealTimeThread"
            "                    Functions = {GAMA}"
            "                }"
            "            }"
            "        }"
            "    }"
            "    +Scheduler = {"
            "        Class = GAMScheduler"
            "        TimingDataSource = Timings"
            "    }"
            "}";

    bool ret = InitialiseMemoryMapInputBrokerEnviroment(config);
    ObjectRegistryDatabase::Instance()->Purge();
    return ret;
}

bool PriorityGAMTest::TestSetup_FalseNoSameDS() {
    char8 *config = "$Application1 = {"
            "    Class = RealTimeApplication"
            "    +Functions = {"
            "        Class = ReferenceContainer"
            "        +GAMA = {"
            "            Class = PriorityGAM"
            "            InputSignals = {"
            "               Signal1 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "                   Frequency = 0"
            "               }"
            "               Signal2 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "            }"
            "            OutputSignals = {"
            "               Signal1 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal2 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = DDB2"
            "                   Type = uint32"
            "               }"
            "               Indexes = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "                   NumberOfDimensions = 1"
            "                   NumberOfElements = 5"
            "               }"
            "            }"
            "        }"
            "    }"
            "    +Data = {"
            "        Class = ReferenceContainer"
            "        +Drv1 = {"
            "            Class = PriorityGAMTestDS"
            "        }"
            "        +DDB1 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +DDB2 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +Timings = {"
            "            Class = TimingDataSource"
            "        }"
            "    }"
            "    +States = {"
            "        Class = ReferenceContainer"
            "        +State1 = {"
            "            Class = RealTimeState"
            "            +Threads = {"
            "                Class = ReferenceContainer"
            "                +Thread1 = {"
            "                    Class = RealTimeThread"
            "                    Functions = {GAMA}"
            "                }"
            "            }"
            "        }"
            "    }"
            "    +Scheduler = {"
            "        Class = GAMScheduler"
            "        TimingDataSource = Timings"
            "    }"
            "}";

    bool ret = !InitialiseMemoryMapInputBrokerEnviroment(config);
    ObjectRegistryDatabase::Instance()->Purge();
    return ret;
}

bool PriorityGAMTest::TestSetup_FalseNoSameNumberOfSignals() {
    char8 *config = "$Application1 = {"
            "    Class = RealTimeApplication"
            "    +Functions = {"
            "        Class = ReferenceContainer"
            "        +GAMA = {"
            "            Class = PriorityGAM"
            "            InputSignals = {"
            "               Signal1 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "                   Frequency = 0"
            "               }"
            "               Signal2 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "            }"
            "            OutputSignals = {"
            "               Signal1 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal2 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Indexes = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "                   NumberOfDimensions = 1"
            "                   NumberOfElements = 5"
            "               }"
            "            }"
            "        }"
            "    }"
            "    +Data = {"
            "        Class = ReferenceContainer"
            "        +Drv1 = {"
            "            Class = PriorityGAMTestDS"
            "        }"
            "        +DDB1 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +DDB2 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +Timings = {"
            "            Class = TimingDataSource"
            "        }"
            "    }"
            "    +States = {"
            "        Class = ReferenceContainer"
            "        +State1 = {"
            "            Class = RealTimeState"
            "            +Threads = {"
            "                Class = ReferenceContainer"
            "                +Thread1 = {"
            "                    Class = RealTimeThread"
            "                    Functions = {GAMA}"
            "                }"
            "            }"
            "        }"
            "    }"
            "    +Scheduler = {"
            "        Class = GAMScheduler"
            "        TimingDataSource = Timings"
            "    }"
            "}";

    bool ret = !InitialiseMemoryMapInputBrokerEnviroment(config);
    ObjectRegistryDatabase::Instance()->Purge();
    return ret;
}

bool PriorityGAMTest::TestSetup_FalseNoSameIOSize() {
    char8 *config = "$Application1 = {"
            "    Class = RealTimeApplication"
            "    +Functions = {"
            "        Class = ReferenceContainer"
            "        +GAMA = {"
            "            Class = PriorityGAM"
            "            InputSignals = {"
            "               Signal1 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "                   Frequency = 0"
            "               }"
            "               Signal2 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "            }"
            "            OutputSignals = {"
            "               Signal1 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal2 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = DDB1"
            "                   Type = uint64"
            "               }"
            "               Indexes = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "                   NumberOfDimensions = 1"
            "                   NumberOfElements = 5"
            "               }"
            "            }"
            "        }"
            "    }"
            "    +Data = {"
            "        Class = ReferenceContainer"
            "        +Drv1 = {"
            "            Class = PriorityGAMTestDS"
            "        }"
            "        +DDB1 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +DDB2 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +Timings = {"
            "            Class = TimingDataSource"
            "        }"
            "    }"
            "    +States = {"
            "        Class = ReferenceContainer"
            "        +State1 = {"
            "            Class = RealTimeState"
            "            +Threads = {"
            "                Class = ReferenceContainer"
            "                +Thread1 = {"
            "                    Class = RealTimeThread"
            "                    Functions = {GAMA}"
            "                }"
            "            }"
            "        }"
            "    }"
            "    +Scheduler = {"
            "        Class = GAMScheduler"
            "        TimingDataSource = Timings"
            "    }"
            "}";

    bool ret = !InitialiseMemoryMapInputBrokerEnviroment(config);
    ObjectRegistryDatabase::Instance()->Purge();
    return ret;
}

bool PriorityGAMTest::TestExecute() {

    char8 *config = "$Application1 = {"
            "    Class = RealTimeApplication"
            "    +Functions = {"
            "        Class = ReferenceContainer"
            "        +GAMA = {"
            "            Class = PriorityGAMTestGAM"
            "            InputSignals = {"
            "               Signal1 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "                   Frequency = 0"
            "               }"
            "               Signal2 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "            }"
            "            OutputSignals = {"
            "               Signal1 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal2 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal3 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal4 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal5 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal6 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal7 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal8 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal9 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal10 = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Indexes = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "                   NumberOfDimensions = 1"
            "                   NumberOfElements = 5"
            "               }"
            "            }"
            "        }"
            "    }"
            "    +Data = {"
            "        Class = ReferenceContainer"
            "        +Drv1 = {"
            "            Class = PriorityGAMTestDS"
            "        }"
            "        +DDB1 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +Timings = {"
            "            Class = TimingDataSource"
            "        }"
            "    }"
            "    +States = {"
            "        Class = ReferenceContainer"
            "        +State1 = {"
            "            Class = RealTimeState"
            "            +Threads = {"
            "                Class = ReferenceContainer"
            "                +Thread1 = {"
            "                    Class = RealTimeThread"
            "                    Functions = {GAMA}"
            "                }"
            "            }"
            "        }"
            "    }"
            "    +Scheduler = {"
            "        Class = GAMScheduler"
            "        TimingDataSource = Timings"
            "    }"
            "}";
    bool ret = InitialiseMemoryMapInputBrokerEnviroment(config);
    ObjectRegistryDatabase* god = ObjectRegistryDatabase::Instance();

    ReferenceT<PriorityGAMTestGAM> prioGAM;
    if (ret) {
        prioGAM = god->Find("Application1.Functions.GAMA");
        ret = prioGAM.IsValid();
    }

    ReferenceT < PriorityGAMTestDS > testDs;
    if(ret){
        testDs = god->Find("Application1.Data.Drv1");
        ret = testDs.IsValid();
    }

    //get the brokers
    ReferenceT<MemoryMapSynchronisedInputBroker> inputBroker;
    if(ret){
        ReferenceContainer inputBrokers;
        ret=prioGAM->GetInputBrokers(inputBrokers);
        if(ret){
            inputBroker=inputBrokers.Get(0);
            ret=inputBroker.IsValid();
        }
    }

    uint32 numberOfOutputSignals;
    uint32 numberOfIndexes;
    uint32* indexes=NULL;
    if(ret){
        numberOfOutputSignals=prioGAM->GetNumberOfOutputSignals();
        indexes=(uint32*)prioGAM->GetOutputSignalMemoryX(numberOfOutputSignals-1u);
        ret=prioGAM->GetSignalNumberOfElements(OutputSignals, numberOfOutputSignals-1u, numberOfIndexes);
    }

    if(ret){
        testDs->SetChangeRate(0u, 1u);
        for(uint32 i=0u; i<20u; i++){
            inputBroker->Execute();
            prioGAM->Execute();
            for(uint32 j=0u; j<numberOfIndexes; j++){
                printf("%d ", indexes[j]);
            }
            printf("\n");
        }
    }

    god->Purge();
    return ret;
}

