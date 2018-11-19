/**
 * @file EpicsSignalGenTool.cpp
 * @brief Source file for class EpicsSignalGenTool
 * @date 12 nov 2018
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
 * the class EpicsSignalGenTool (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <epicsStdlib.h>

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
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

using namespace MARTe;
static int tsInitC = 0;
static epicsTimeStamp tsStart, tsFirst, tsPreviousC, tsPreviousS;
capri caPriority = DEFAULT_CA_PRIORITY;
int create_pvs(pv* pvs,
               int nPvs,
               caCh *pCB) {
    int n;
    int result;
    int returncode = 0;

    if (!tsInitC) /* Initialize start timestamp */
    {
        epicsTimeGetCurrent(&tsStart);
        tsInitC = 1;
    }
    /* Issue channel connections */
    for (n = 0; n < nPvs; n++) {
        result = ca_create_channel(pvs[n].name, pCB, &pvs[n], caPriority, &pvs[n].chid);
        if (result != ECA_NORMAL) {
            fprintf(stderr, "CA error %s occurred while trying "
                    "to create channel '%s'.\n",
                    ca_message(result), pvs[n].name);
            pvs[n].status = result;
            returncode = 1;
        }
    }

    return returncode;
}

int connect_pvs(pv* pvs,
                int nPvs) {
    int returncode = create_pvs(pvs, nPvs, 0);
    if (returncode == 0) {
        /* Wait for channels to connect */
        int result = ca_pend_io(0.1);
        if (result == ECA_TIMEOUT) {
            if (nPvs > 1) {
                fprintf(stderr, "Channel connect timed out: some PV(s) not found.\n");
            }
            else {
                fprintf(stderr, "Channel connect timed out: '%s' not found.\n", pvs[0].name);
            }
            returncode = 1;
        }
    }
    return returncode;
}





static int cainfo(pv &pvs,
                  StreamString &type,
                  uint32 &numberOfElements) {
    long dbfType;
    long dbrType;
    unsigned long nElems;
    enum channel_state state;
    char *stateStrings[] = { "never connected", "previously connected", "connected", "closed" };
    char *boolStrings[] = { "no ", "" };

    /* Print the status data */
    /* --------------------- */

    state = ca_state(pvs.chid);
    nElems = ca_element_count(pvs.chid);
    dbfType = ca_field_type(pvs.chid);
    dbrType = dbf_type_to_DBR(dbfType);

    printf("%s\n"
           "    State:            %s\n"
           "    Host:             %s\n"
           "    Access:           %sread, %swrite\n"
           "    Native data type: %s\n"
           "    Request type:     %s\n"
           "    Element count:    %lu\n",
           pvs.name, stateStrings[state], ca_host_name(pvs.chid), boolStrings[ca_read_access(pvs.chid)], boolStrings[ca_write_access(pvs.chid)],
           dbf_type_to_text(dbfType), dbr_type_to_text(dbrType), nElems);

    type = dbf_type_to_text(dbfType) + 4u;

    numberOfElements = nElems;

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
    /*
     while ((StringHelper::Compare(variable.Buffer(), "") == 0) && (ret)) {
     char8 term;
     ret = xmlFile.GetToken(token, "<", term, "");
     //printf("token1=%s\n", token.Buffer());

     token.SetSize(0ull);
     ret &= xmlFile.GetToken(token, ">", term, "");
     //printf("token2=%s\n", token.Buffer());

     token.SetSize(0ull);
     ret &= xmlFile.GetToken(variable, "<", term, "");
     ret &= xmlFile.GetToken(token, ">", term, "");
     //printf("token3=%s\n", token.Buffer());

     token.Seek(0ull);
     }
     */
    return ret;
}

int main(int argc,
         const char **argv) {

    /*
     StreamString configFile=""
     "XmlInputFilePath = \"/home/pc/MARTe2Project/GIT/MARTe2Hieratika/Startup/myVars.xml\"\n"
     "OutputFilePath = \"generated.cfg\"\n"
     "RTThreadsCpuMasks = {0x1 0x2}";
     */

    //open the XML file
    File xmlFile;
    if (!xmlFile.Open(argv[1], File::ACCESS_MODE_R)) {
        printf("Failed opening file %s\n", argv[1]);
        return -1;
    }

    File outputFile;
    if (!outputFile.Open(argv[2], BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT | BasicFile::FLAG_TRUNC)) {
        printf("Failed opening db file %s\n", argv[2]);
        return -1;
    }
    int32 result = ca_context_create(ca_disable_preemptive_callback);
    if (result != ECA_NORMAL) {
        fprintf(stderr, "CA error %s occurred while trying "
                "to start channel access.\n",
                ca_message(result));
        return 1;
    }
    StreamString variable;
    bool start = false;
    xmlFile.Seek(0ull);
    while (GetVariable(xmlFile, variable)) {

        printf("variable=%s\n", variable.Buffer());
        if (variable == "Comment") {
            start = true;
            variable.SetSize(0ull);
            continue;
        }
        if (start) {

            StreamString type = "DOUBLE";
            uint32 numberOfElements = 1u;
            pv pvs;
            pvs.name = variable.Buffer();
            result = connect_pvs(&pvs, 1);
            if (!result) {
                result = cainfo(pvs, type, numberOfElements);
            }

            //dbpr(variable.Buffer(),-20);

            if (numberOfElements < 2u) {
                outputFile.Printf("%s", "record( ai,");
                outputFile.Printf("%s)\n", variable.Buffer());
                outputFile.Printf("%s\n", "{");
                outputFile.Printf("%s\n", "field(DTYP, \"Soft Channel\")");
                outputFile.Printf("%s\n\n", "}");
            }
            else {
                outputFile.Printf("%s", "record( waveform,");
                outputFile.Printf("%s)\n", variable.Buffer());
                outputFile.Printf("%s\n", "{");
                outputFile.Printf("%s\n", "field(DTYP, \"Soft Channel\")");
                outputFile.Printf("field(NELM, %d)\n", numberOfElements);
                outputFile.Printf("field(FTVL, %s)\n", type.Buffer());
                outputFile.Printf("%s\n\n", "}");
            }

        }
        variable.SetSize(0ull);
    }
    outputFile.Flush();
    outputFile.Close();
    xmlFile.Close();
    /* Shut down Channel Access */
    ca_context_destroy();

    return 0;
}
