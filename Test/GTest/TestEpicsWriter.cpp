/**
 * @file TestEpicsWriter.cpp
 * @brief Source file for class TestEpicsWriter
 * @date 14 mag 2019
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
 * the class TestEpicsWriter (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <epicsStdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cadef.h>
#include <epicsGetopt.h>

#include "tool_lib.h"
#include "dbTest.h"
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "File.h"
#include "StreamString.h"
#include "StreamStructuredData.h"
#include "StandardPrinter.h"
#include "StreamMemoryReference.h"
#include "StandardParser.h"
#include "ConfigurationDatabase.h"
#include "Threads.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;
const uint32 PV_NAME_MAX_SIZE = 64u;
#define MAX_ARR_LEN 100

uint32 numberOfVariables = 0u;

/**
 * Contains all the PV data
 */
struct PvDescriptor {
    chid pvChid;
    evid pvEvid;
    chtype pvType;
    void *memory;
    uint32 numberOfElements;
    uint32 memorySize;
    char8 pvName[PV_NAME_MAX_SIZE];
    uint32 index;
    uint64 offset;
    TypeDescriptor td;
};

static PvDescriptor *pvDescriptor;
volatile int32 quit = 0;

static void StopApp(int sig) {
    //Second time this is called? Kill the application.
    Atomic::Increment(&quit);
}

static int cainfo(chid &pvChid,
                  char8 *pvName,
                  chtype &type,
                  uint32 &numberOfElements,
                  uint32 &memorySize,
                  TypeDescriptor &td) {
    int32 dbfType;
    uint32 nElems;
    enum channel_state state;
    const char8 *stateStrings[] = { "never connected", "previously connected", "connected", "closed" };

    state = ca_state(pvChid);
    nElems = ca_element_count(pvChid);
    dbfType = ca_field_type(pvChid);

    if (state != 2) {
        printf("The variable %s is not connected, state=%s\n", pvName, stateStrings[state]);
    }

    numberOfElements = nElems;
    type = dbfType;
    const char8* epicsTypeName = dbf_type_to_text(dbfType);

    printf("%s %s %d\n", pvName, epicsTypeName, numberOfElements);

    if (StringHelper::Compare(epicsTypeName, "DBF_DOUBLE") == 0u) {
        memorySize = 8u;
        td = Float64Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
        memorySize = 4u;
        td = Float32Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
        memorySize = 4u;
        td = SignedInteger32Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
        memorySize = 4u;
        td = UnsignedInteger32Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
        memorySize = 2u;
        td = SignedInteger16Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
        memorySize = 2u;
        td = UnsignedInteger16Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
        memorySize = 1u;
        td = SignedInteger8Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
        memorySize = 1u;
        td = UnsignedInteger8Bit;
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_STRING") == 0) {
        memorySize = MAX_STRING_SIZE;
        td.numberOfBits = MAX_STRING_SIZE * 8u;
        td.isStructuredData = false;
        td.type = CArray;
        td.isConstant = false;
    }
    else {
        memorySize = MAX_STRING_SIZE;
        td.numberOfBits = MAX_STRING_SIZE * 8u;
        td.isStructuredData = false;
        td.type = CArray;
        td.isConstant = false;
        type = DBF_STRING;
        /*type=DBF_DOUBLE;
         memorySize = 8u;
         td = Float64Bit;*/
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

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

void WriterCycleLoop(uint32 &quit) {
    while (quit == 0u) {
        for (uint32 n = 0u; n < numberOfVariables; n++) {
            uint32 temp;
            AnyType source(pvDescriptor[n].td, 0, pvDescriptor[n].memory);
            if (TypeConvert(temp, source)) {
                temp++;
                TypeConvert(source, temp);
                if(n==0u){
                    printf("temp=%d\n", temp);
                }
                ca_array_put(pvDescriptor[n].pvType, pvDescriptor[n].numberOfElements, pvDescriptor[n].pvChid, pvDescriptor[n].memory);
                (void) ca_pend_io(0.1);
            }
        }
        Sleep::Sec(1);
    }
    Atomic::Increment(&quit);
}

int main(int argc,
         const char **argv) {

    signal(SIGTERM, StopApp);
    signal(SIGINT, StopApp);

    //open the XML file
    File xmlFile;
    if (!xmlFile.Open(argv[1], File::ACCESS_MODE_R)) {
        printf("Failed opening file %s\n", argv[1]);
        return -1;
    }

    StreamString variable;
    bool start = false;
    xmlFile.Seek(0ull);

    while (GetVariable(xmlFile, variable)) {

        if (variable == argv[2]) {
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

    start = false;
    xmlFile.Seek(0ull);

    uint32 counter = 0u;
    while (GetVariable(xmlFile, variable)) {
        //printf("variable=%s\n", variable.Buffer());

        if (!start) {
            if (variable == argv[2]) {
                start = true;
            }
        }
        if (start) {

            pvDescriptor[counter].pvType = DBF_DOUBLE;
            pvDescriptor[counter].index = counter;
            StringHelper::Copy(pvDescriptor[counter].pvName, variable.Buffer());
            counter++;
        }

        variable.SetSize(0ull);
    }

    xmlFile.Close();

    //initialise the context
    int32 result = ca_context_create(ca_disable_preemptive_callback);
    if (result != ECA_NORMAL) {
        fprintf(stderr, "CA error %s occurred while trying "
                "to start channel access.\n",
                ca_message(result));
    }

    uint32 totalMemorySize;
    for (uint32 i = 0u; i < numberOfVariables; i++) {

        result = ca_create_channel(&pvDescriptor[i].pvName[0], NULL, NULL, 20u, &pvDescriptor[i].pvChid);
        printf("creating channel %s\n", pvDescriptor[i].pvName);
        ca_pend_io(0.1);

        cainfo(pvDescriptor[i].pvChid, pvDescriptor[i].pvName, pvDescriptor[i].pvType, pvDescriptor[i].numberOfElements, pvDescriptor[i].memorySize,
               pvDescriptor[i].td);
        if (pvDescriptor[i].numberOfElements > MAX_ARR_LEN) {
            pvDescriptor[i].numberOfElements = 0u;
        }
        else {
            totalMemorySize += (pvDescriptor[i].numberOfElements * pvDescriptor[i].memorySize);
        }

    }
    uint8 *memory = (uint8*) HeapManager::Malloc(totalMemorySize);
    MemoryOperationsHelper::Set(memory, 0, totalMemorySize);

    uint32 memoryOffset = 0u;
    for (uint32 counter = 0u; counter < numberOfVariables; counter++) {
        pvDescriptor[counter].memory = memory + memoryOffset;
        pvDescriptor[counter].offset = memoryOffset;
        //printf("creating subscription=%s\n", pvDescriptor[counter].pvName);
        if (pvDescriptor[counter].numberOfElements > 0) {
            memoryOffset += (pvDescriptor[counter].numberOfElements * pvDescriptor[counter].memorySize);
        }
    }

    //begin the thread

    Threads::BeginThread((ThreadFunctionType) WriterCycleLoop, &quit, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled);

    while (quit < 2) {
        Sleep::Sec(1);
    }

    /* Shut down Channel Access */
    ca_context_destroy();

    return 0;
}

