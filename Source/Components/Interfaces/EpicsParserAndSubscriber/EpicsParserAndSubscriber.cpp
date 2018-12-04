/**
 * @file EpicsParserAndSubscriber.cpp
 * @brief Source file for class EpicsParserAndSubscriber
 * @date 01 dic 2018
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
 * the class EpicsParserAndSubscriber (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "EpicsParserAndSubscriber.h"
#include "File.h"
#include "AdvancedErrorManagement.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

static void GetValueCallback(evargs args) {

    PvDescriptor *pv = static_cast<PvDescriptor *>(args.usr);
    if ((pv->syncMutex)->FastLock()) {
        (void) MemoryOperationsHelper::Copy(pv->memory, args.dbr, pv->memorySize * pv->numberOfElements);
        pv->changedFlag[pv->index] = 1;
        (pv->syncMutex)->FastUnLock();
    }
}

static void GetTimeoutCallback(evargs args) {
    PvDescriptor *mypv = static_cast<PvDescriptor *>(args.usr);
    if ((mypv->syncMutex)->FastLock()) {
        //pv *ppv = (pv*) ca_puser (mypv->pvChid);

        epicsTimeStamp *ptsNewS = &((struct dbr_time_string *) (args.dbr))->stamp;
        //(void) MemoryOperationsHelper::Copy(mypv->memory, ppv->value, mypv->memorySize * mypv->numberOfElements);
        (void) MemoryOperationsHelper::Copy(mypv->timeStamp, ptsNewS, sizeof(epicsTimeStamp));
        mypv->changedFlag[mypv->index] = 1;
        (mypv->syncMutex)->FastUnLock();
    }
    //epicsTimeToStrftime(myargs, 128, "%Y-%m-%d %H:%M:%S.%06f", ptsNewS);
}

static bool GetVariable(File &xmlFile,
                        StreamString &variable) {
    StreamString token;
    char8 term;
    bool ret = xmlFile.GetToken(token, "<", term, " \n");

    token.SetSize(0ull);
    while (variable == "" && (ret)) {
        token.SetSize(0ull);
        ret = xmlFile.GetToken(token, ">", term, " \n");
        //printf("token2=%s\n", token.Buffer());
        token.SetSize(0ull);
        ret &= xmlFile.GetToken(variable, "<", term, " \n");
    }

    ret &= xmlFile.GetToken(token, ">", term, "");

    return ret;
}

static int cainfo(chid &pvChid,
                  chtype &type,
                  uint32 &numberOfElements,
                  uint32 &memorySize) {
    long dbfType;
    long dbrType;
    unsigned long nElems;
    enum channel_state state;
    char *stateStrings[] = { "never connected", "previously connected", "connected", "closed" };
    char *boolStrings[] = { "no ", "" };

    /* Print the status data */
    /* --------------------- */

    state = ca_state(pvChid);
    nElems = ca_element_count(pvChid);
    dbfType = ca_field_type(pvChid);

    dbrType = dbf_type_to_DBR(dbfType);
    /*
     printf("%s\n"
     "    State:            %s\n"
     "    Host:             %s\n"
     "    Access:           %sread, %swrite\n"
     "    Native data type: %s\n"
     "    Request type:     %s\n"
     "    Element count:    %lu\n",
     pvs.name, stateStrings[state], ca_host_name(pvChid), boolStrings[ca_read_access(pvChid)], boolStrings[ca_write_access(pvChid)],
     dbf_type_to_text(dbfType), dbr_type_to_text(dbrType), nElems);
     */
    numberOfElements = nElems;

    const char8* epicsTypeName = dbf_type_to_text(dbfType);
    type = DBF_DOUBLE;
    if (StringHelper::Compare(epicsTypeName, "D//BF_DOUBLE") == 0u) {
        type = DBF_DOUBLE;
        memorySize = 8u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
        type = DBF_FLOAT;
        memorySize = 4u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
        type = DBF_LONG;
        memorySize = 4u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
        type = DBF_LONG;
        memorySize = 4u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
        type = DBF_SHORT;
        memorySize = 2u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
        type = DBF_SHORT;
        memorySize = 2u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
        type = DBF_CHAR;
        memorySize = 1u;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
        type = DBF_CHAR;
        memorySize = 1u;
    }
    else {
        type = DBF_DOUBLE;
        memorySize = 8u;
    }

    printf("NumberOfElements=%d, MemorySize=%d\n", numberOfElements, memorySize);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

EpicsParserAndSubscriber::EpicsParserAndSubscriber() :
        Object(),
        EmbeddedServiceMethodBinderI(),
        executor(*this) {
    pvDescriptor = NULL;
    numberOfVariables = 0u;
    totalMemorySize = 0u;
    memory = NULL;
    changedFlagMem = NULL;
    fmutex.Create();
    initialisationDone = 0u;
}

EpicsParserAndSubscriber::~EpicsParserAndSubscriber() {

    if (!executor.Stop()) {
        if (!executor.Stop()) {
            REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
        }
    }
    if (pvDescriptor != NULL) {
        delete[] pvDescriptor;
    }

    if (changedFlagMem != NULL) {
        HeapManager::Free((void*&) changedFlagMem);
    }

    if (memory != NULL) {
        HeapManager::Free((void*&) memory);
    }

}

bool EpicsParserAndSubscriber::Initialise(StructuredDataI &data) {
    bool ret = Object::Initialise(data);
    if (ret) {
        ret = data.Read("XmlFilePath", xmlFilePath);
        if (ret) {
            ret = data.Read("FirstVariableName", firstVariableName);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::FatalError, "Please specify FirstVariableName");
            }
        }
        else {
            REPORT_ERROR(ErrorManagement::FatalError, "Please specify XmlFilePath");
        }

    }
    return ret;
}

bool EpicsParserAndSubscriber::ParseAndSubscribe() {
    //open the xml file
    File xmlFile;
    if (!xmlFile.Open(xmlFilePath.Buffer(), File::ACCESS_MODE_R)) {
        printf("Failed opening file %s\n", xmlFilePath.Buffer());
        return false;
    }

    xmlFile.Seek(0ull);

    StreamString variable;
    bool start = false;
    //get the number of variables
    numberOfVariables = 0u;
    xmlFile.Seek(0ull);
    while (GetVariable(xmlFile, variable)) {

        if (variable == firstVariableName) {
            start = true;
        }
        if (start) {
            printf("variable=%s\n", variable.Buffer());
            numberOfVariables++;
        }
        variable.SetSize(0ull);
    }

    //create the pv descriptors
    pvDescriptor = new PvDescriptor[numberOfVariables];
    variable.SetSize(0ull);
    changedFlagMem = (uint8*) HeapManager::Malloc(numberOfVariables);

    xmlFile.Seek(0ull);
    uint32 counter = 0u;
    variable.SetSize(0ull);
    start = false;
    while (GetVariable(xmlFile, variable)) {

        if (!start) {
            if (variable == firstVariableName) {
                start = true;
            }
        }
        if (start) {

            pvDescriptor[counter].pvType = DBF_DOUBLE;
            pvDescriptor[counter].index = counter;
            pvDescriptor[counter].changedFlag = changedFlagMem;
            pvDescriptor[counter].syncMutex = &fmutex;
            StringHelper::Copy(pvDescriptor[counter].pvName, variable.Buffer());
            counter++;
        }

        variable.SetSize(0ull);
    }

    xmlFile.Close();

    executor.Start();

    return true;
}

ErrorManagement::ErrorType EpicsParserAndSubscriber::Execute(ExecutionInfo& info) {
    ErrorManagement::ErrorType err = ErrorManagement::NoError;
    if (info.GetStage() == ExecutionInfo::StartupStage) {

        //initialise the context
        int32 result = ca_context_create(ca_enable_preemptive_callback);
        if (result != ECA_NORMAL) {
            fprintf(stderr, "CA error %s occurred while trying "
                    "to start channel access.\n",
                    ca_message(result));
        }

        for (uint32 i = 0u; i < numberOfVariables; i++) {

            ca_create_channel(&pvDescriptor[i].pvName[0], NULL, NULL, 20u, &pvDescriptor[i].pvChid);
            ca_pend_io(0.1);

            cainfo(pvDescriptor[i].pvChid, pvDescriptor[i].pvType, pvDescriptor[i].numberOfElements, pvDescriptor[i].memorySize);
            totalMemorySize += (pvDescriptor[i].numberOfElements * pvDescriptor[i].memorySize) + sizeof(epicsTimeStamp);

        }
        memory = (uint8*) HeapManager::Malloc(totalMemorySize);

        uint32 memoryOffset = 0u;
        for (uint32 counter = 0u; counter < numberOfVariables; counter++) {
            pvDescriptor[counter].memory = memory + memoryOffset;
            pvDescriptor[counter].offset = memoryOffset;
            pvDescriptor[counter].timeStamp = (epicsTimeStamp*) (memory + memoryOffset
                    + (pvDescriptor[counter].numberOfElements * pvDescriptor[counter].memorySize));
            memoryOffset += (pvDescriptor[counter].numberOfElements * pvDescriptor[counter].memorySize) + sizeof(epicsTimeStamp);
            printf("create subscription=%s\n", pvDescriptor[counter].pvName);

            ca_create_subscription(pvDescriptor[counter].pvType, pvDescriptor[counter].numberOfElements, pvDescriptor[counter].pvChid, DBE_VALUE,
                                   &GetValueCallback, &pvDescriptor[counter], &pvDescriptor[counter].pvEvid);

            ca_create_subscription(DBR_TIME_STRING, pvDescriptor[counter].numberOfElements, pvDescriptor[counter].pvChid, DBE_VALUE, &GetTimeoutCallback,
                                   &pvDescriptor[counter], &pvDescriptor[counter].pvEvid);

        }
        initialisationDone = 1u;

    }
    else if (info.GetStage() != ExecutionInfo::BadTerminationStage) {
        Sleep::Sec(1.0);
    }
    else {
        uint32 n;
        if (pvDescriptor != NULL) {
            for (n = 0u; (n < numberOfVariables); n++) {
                (void) ca_clear_subscription(pvDescriptor[n].pvEvid);
                (void) ca_clear_event(pvDescriptor[n].pvEvid);
                (void) ca_clear_channel(pvDescriptor[n].pvChid);
            }
        }
        ca_detach_context();
        ca_context_destroy();
    }

    return err;
}

bool EpicsParserAndSubscriber::Synchronise(uint8 *memoryOut,
                                           uint8 *changedFlags) {
    //copy and reset the flags
    if (fmutex.FastLock()) {
        MemoryOperationsHelper::Copy(memoryOut, memory, totalMemorySize);
        MemoryOperationsHelper::Copy(changedFlags, changedFlagMem, numberOfVariables);
        MemoryOperationsHelper::Set(changedFlagMem, '\0', numberOfVariables);
        fmutex.FastUnLock();
    }
    return true;
}

uint32 EpicsParserAndSubscriber::GetNumberOfVariables() {
    return numberOfVariables;
}

PvDescriptor *EpicsParserAndSubscriber::GetPvDescriptors() {
    return pvDescriptor;
}

uint64 EpicsParserAndSubscriber::GetTotalMemorySize() {
    return totalMemorySize;
}

bool EpicsParserAndSubscriber::InitialisationDone() {
    return initialisationDone>0u;
}

CLASS_REGISTER(EpicsParserAndSubscriber, "1.0")

}

