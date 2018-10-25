/**
 * @file MARTe2HieratikaMessageDispatcherTest.cpp
 * @brief Source file for class MARTe2HieratikaMessageDispatcherTest
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
 * the class MARTe2HieratikaMessageDispatcherTest (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "MARTe2HieratikaMessageDispatcherTest.h"
#include "ConfigurationDatabase.h"
#include "DataSourceI.h"
#include "GAMSchedulerI.h"
#include "MemoryMapMultiBufferInputBroker.h"
#include "MemoryMapMultiBufferOutputBroker.h"
#include "MemoryMapSynchronisedMultiBufferInputBroker.h"
#include "MemoryMapSynchronisedMultiBufferOutputBroker.h"
#include "ObjectRegistryDatabase.h"
#include "RealTimeApplication.h"
#include "StandardParser.h"
#include "EventSem.h"
#include "QueuedReplyMessageCatcherFilter.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

class MARTe2HieratikaMessageDispatcherTestDS: public DataSourceI, public MessageI {
public:

    CLASS_REGISTER_DECLARATION()
    MARTe2HieratikaMessageDispatcherTestDS();

    virtual ~MARTe2HieratikaMessageDispatcherTestDS();

    virtual bool Synchronise();

    virtual bool AllocateMemory();

    virtual bool GetSignalMemoryBuffer(const uint32 signalIdx,
                                       const uint32 bufferIdx,
                                       void *&signalAddress);

    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    virtual const char8 *GetBrokerName(StructuredDataI &data,
                                       const SignalDirection direction);

private:
    uint32 value;
};

MARTe2HieratikaMessageDispatcherTestDS::MARTe2HieratikaMessageDispatcherTestDS() {
    value = 0u;
}

MARTe2HieratikaMessageDispatcherTestDS::~MARTe2HieratikaMessageDispatcherTestDS() {

}

bool MARTe2HieratikaMessageDispatcherTestDS::Synchronise() {
    ReferenceT < Message > message = Get(0);
    bool ret = message.IsValid();
    if (ret) {
        EventSem waitSem;
        waitSem.Create();
        waitSem.Reset();
        ReferenceContainer eventReplyContainer;
        eventReplyContainer.Insert(message);
        ReferenceT < QueuedReplyMessageCatcherFilter > filter;
        filter = ReferenceT < QueuedReplyMessageCatcherFilter > (new (NULL) QueuedReplyMessageCatcherFilter());
        filter->SetMessagesToCatch(eventReplyContainer);
        filter->SetEventSemaphore(waitSem);
        MessageI::InstallMessageFilter(filter, 0);
        //Send a message here
        SendMessage(message, this);
        waitSem.Wait();
        StreamString response;
        ReferenceT < Message > mess = (eventReplyContainer.Get(0));
        ReferenceT < ConfigurationDatabase > payload = mess->Get(0);
        payload->Read("Response", response);
        printf("GetPage reponse %s\n", response.Buffer());
        MessageI::RemoveMessageFilter (filter);

    }
    return ret;
}

bool MARTe2HieratikaMessageDispatcherTestDS::AllocateMemory() {
    return true;
}

bool MARTe2HieratikaMessageDispatcherTestDS::GetSignalMemoryBuffer(const uint32 signalIdx,
                                                                   const uint32 bufferIdx,
                                                                   void *&signalAddress) {
    return &value;
}

bool MARTe2HieratikaMessageDispatcherTestDS::PrepareNextState(const char8 * const currentStateName,
                                                              const char8 * const nextStateName) {
    return true;
}

const char8 *MARTe2HieratikaMessageDispatcherTestDS::GetBrokerName(StructuredDataI &data,
                                                                   const SignalDirection direction) {
    if (direction == InputSignals) {
        float32 frequency;
        if (data.Read("Frequency", frequency)) {
            if (frequency >= 0.) {
                return "MemoryMapSynchronisedInputBroker";
            }
        }
        return "MemoryMapInputBroker";
    }
    else {
        float32 frequency;
        if (data.Read("Frequency", frequency)) {
            if (frequency >= 0.) {
                return "MemoryMapSynchronisedOutputBroker";
            }
        }
        return "MemoryMapOutputBroker";
    }

}

CLASS_REGISTER(MARTe2HieratikaMessageDispatcherTestDS, "1.0")

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

MARTe2HieratikaMessageDispatcherTest::MARTe2HieratikaMessageDispatcherTest() {
    // Auto-generated constructor stub for MARTe2HieratikaMessageDispatcherTest
    // TODO Verify if manual additions are needed
}

MARTe2HieratikaMessageDispatcherTest::~MARTe2HieratikaMessageDispatcherTest() {
    // Auto-generated destructor stub for MARTe2HieratikaMessageDispatcherTest
    // TODO Verify if manual additions are needed
}

bool MARTe2HieratikaMessageDispatcherTest::TestConstructor() {
    MARTe2HieratikaMessageDispatcher test;
    return true;
}

bool MARTe2HieratikaMessageDispatcherTest::TestInitialise() {
    StreamString config = "ServerIpAddress = \"192.168.1.1\"\n"
            "ServerPort=4444\n"
            "ReceiveMessageTimeout=1000\n"
            "HttpMessageTimeout=1000\n"
            "CPUMask=1\n";

    ConfigurationDatabase cdb;
    config.Seek(0);
    StandardParser parser(config, cdb);
    bool ret = parser.Parse();

    if (ret) {
        cdb.MoveToRoot();
        MARTe2HieratikaMessageDispatcher test;
        ret = test.Initialise(cdb);
    }

    return ret;
}

bool MARTe2HieratikaMessageDispatcherTest::TestExecute() {
    static const char8 * const config = ""
            "$Application1 = {"
            "    Class = RealTimeApplication"
            "    +HieratikaInterface = {"
            "        Class = MARTe2HieratikaMessageDispatcher"
            "        ServerIpAddress = \"127.0.0.1\""
            "        ServerPort = 8080"
            "        ReceiveMessageTimeout = 1000"
            "        HttpMessageTimeout = 1000"
            "    }"
            "    +ResponseStream = {"
            "        Class= StreamStringObject"
            "    }"
            "    +Functions = {"
            "        Class = ReferenceContainer"
            "        +GAMA = {"
            "            Class = IOGAM"
            "            InputSignals = {"
            "               Time = {"
            "                   DataSource = Timer"
            "                   Type = uint32"
            "                   Frequency = 1"
            "               }"
            "               Signal1 = {"
            "                   DataSource = Drv1"
            "                   Type = uint32"
            "               }"
            "            }"
            "            OutputSignals = {"
            "               TriggerDDB = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "               Signal1DDB = {"
            "                   DataSource = DDB1"
            "                   Type = uint32"
            "               }"
            "            }"
            "        }"
            "    }"
            "    +Data = {"
            "        Class = ReferenceContainer"
            "        +DDB1 = {"
            "            Class = GAMDataSource"
            "        }"
            "        +Drv1 = {"
            "            Class = MARTe2HieratikaMessageDispatcherTestDS"
            "            +MessageToSend = {"
            "                Class = Message"
            "                Function = GetPage"
            "                Mode = ExpectsIndirectReply"
            "                Destination = Application1.HieratikaInterface"
            "                +Payload = {"
            "                    Class = ConfigurationDatabase"
            "                    PageName = FALCON"
            "                    Stream = Application1.ResponseStream"
            "                }"
            "            }"
            "        }"
            "        +Timer = {"
            "            Class = LinuxTimer"
            "            Signals = {"
            "                Counter = {"
            "                    Type = uint32"
            "                }"
            "                Time = {"
            "                    Type = uint32"
            "                }"
            "            }"
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
    ObjectRegistryDatabase *god = ObjectRegistryDatabase::Instance();

    ReferenceT<MARTe2HieratikaMessageDispatcherTestDS> ds;
    if (ret) {
        ds = god->Find("Application1.Data.Drv1");
        ret = ds.IsValid();
    }

    if (ret) {
        ret = ds->Synchronise();
    }

    if (ret) {
        ReferenceT < StreamString > stream = god->Find("Application1.ResponseStream");
        ret = stream.IsValid();
        if (ret) {
            printf("response here %s\n", stream->Buffer());
        }
    }

    return ret;
}

