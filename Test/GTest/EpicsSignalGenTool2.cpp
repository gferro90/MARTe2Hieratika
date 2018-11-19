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

    const char8* epicsTypeName = dbf_type_to_text(dbfType);
    type = "float64";
    if (StringHelper::Compare(epicsTypeName, "DBF_DOUBLE") == 0u) {
        type = "float64";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_FLOAT") == 0u) {
        type = "float32";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_LONG") == 0u) {
        type = "int32";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_ULONG") == 0u) {
        type = "uint32";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_SHORT") == 0u) {
        type = "int16";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_USHORT") == 0u) {
        type = "uint16";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_CHAR") == 0u) {
        type = "int8";
    }
    else if (StringHelper::Compare(epicsTypeName, "DBF_UCHAR") == 0u) {
        type = "uint8";
    }
    else {
        type = "float64";
    }

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

    StreamString xmlFilePath;
    if (!localCdb.Read("XmlInputFilePath", xmlFilePath)) {
        printf("XmlInputFilePath not specified in configuration\n");
        return -1;
    }
    StreamString outputFilePath;
    if (!localCdb.Read("OutputFilePath", outputFilePath)) {
        printf("OutputFilePath not specified in configuration\n");
        return -1;
    }
    uint32 toReceivePerThread;
    if (!localCdb.Read("HttpToReceivePerThread", toReceivePerThread)) {
        printf("HttpToSendPerThread not specified in configuration\n");
        return -1;
    }

    float32 cycleFrequency;
    if (!localCdb.Read("CycleFrequency", cycleFrequency)) {
        printf("CycleFrequency not specified in configuration\n");
        return -1;
    }

    AnyType cpuMasks = localCdb.GetType("RTThreadsCpuMasks");
    if (cpuMasks.IsVoid()) {
        printf("RTThreadsCpuMasks not specified in configuration\n");
        return -1;
    }

    uint32 numberOfThreads = cpuMasks.GetNumberOfElements(0u);
    Vector < uint32 > threadsCpu(numberOfThreads);
    if (!localCdb.Read("RTThreadsCpuMasks", threadsCpu)) {
        printf("Failed reading RTThreadsCpuMasks\n");
        return -1;
    }

    uint32 serverInitialPort;
    if (!localCdb.Read("ServerInitialPort", serverInitialPort)) {
        printf("ServerInitialPort not specified in configuration\n");
        return -1;
    }

    uint32 httpInputCpus;
    if (!localCdb.Read("HttpInputCpuMask", httpInputCpus)) {
        httpInputCpus = 0x4;
    }

    uint32 epicsOutputCpus;
    if (!localCdb.Read("EpicsOutputCpusMask", epicsOutputCpus)) {
        epicsOutputCpus = 0x8;
    }

    uint32 epicsOutputStackSize;
    if (!localCdb.Read("EpicsOutputStackSize", epicsOutputStackSize)) {
        epicsOutputStackSize = 10000000;
    }

    uint32 epicsOutputNBuffers;
    if (!localCdb.Read("EpicsOutputNumberOfBuffers", epicsOutputNBuffers)) {
        epicsOutputNBuffers = 100;
    }

    StreamStructuredData < StandardPrinter > cdb;
    File newCfg;
    newCfg.Open(outputFilePath.Buffer(), BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT | BasicFile::FLAG_TRUNC);
    newCfg.SetCalibWriteParam(0u);
    //newCfg.SetBufferSize(1024,1024*1024);
    //newCfg.SetSize(1024 * 1024);
    newCfg.Seek(0ull);
    cdb.SetStream(newCfg);

    //open the XML file
    File xmlFile;
    if (!xmlFile.Open(xmlFilePath.Buffer(), File::ACCESS_MODE_R)) {
        printf("Failed opening file %s\n", argv[1]);
        return -1;
    }

    int32 result = ca_context_create(ca_disable_preemptive_callback);
    if (result != ECA_NORMAL) {
        fprintf(stderr, "CA error %s occurred while trying "
                "to start channel access.\n",
                ca_message(result));
        return 1;
    }

    uint32 numberOfVariables = 0u;
    {
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
                numberOfVariables++;
            }
            variable.SetSize(0ull);
        }
    }

    uint32 nVarsPerThread = numberOfVariables / numberOfThreads;
    printf("numberOfvariables=%d, nVarsPerThread=%d\n", numberOfVariables, nVarsPerThread);

    //create the configurationdatabase
    //application
    //ConfigurationDatabase cdb;
    cdb.CreateAbsolute("$Application");
    cdb.Write("Class", "RealTimeApplication");
    cdb.CreateAbsolute("$Application.+Functions");
    cdb.Write("Class", "ReferenceContainer");
    for (uint32 i = 0u; i < numberOfThreads; i++) {
        //create the N sync gams
        StreamString syncGamName = "+SyncGAM";
        syncGamName.Printf("%d", i);
        cdb.CreateRelative(syncGamName.Buffer());
        cdb.Write("Class", "IOGAM");
        cdb.CreateRelative("InputSignals");
        cdb.CreateRelative("Time");
        cdb.Write("Type", "uint32");
        StreamString linuxTimerName = "LinuxTimer";
        linuxTimerName.Printf("%d", i);
        cdb.Write("DataSource", linuxTimerName.Buffer());
        cdb.Write("Frequency", cycleFrequency);
        cdb.MoveToAncestor(2u);
        cdb.CreateRelative("OutputSignals");
        StreamString outputSignal = "TimeOnDDB";
        outputSignal.Printf("%d", i);
        cdb.CreateRelative(outputSignal.Buffer());
        cdb.Write("Type", "uint32");
        cdb.Write("DataSource", "DDB1");
        cdb.MoveToAncestor(3u);

        //create the N prio gams
        StreamString prioGamName = "+CopyGAM";
        prioGamName.Printf("%d", i);
        cdb.CreateRelative(prioGamName.Buffer());
        cdb.Write("Class", "IOGAM");
        cdb.CreateRelative("InputSignals");

        StreamString variable;
        bool start = false;
        xmlFile.Seek(0ull);
        uint32 counter = 0u;
        StreamString HttpDs = "HttpDiodeInput";
        HttpDs.Printf("%d", i);
        while (GetVariable(xmlFile, variable)) {

            printf("variable=%s\n", variable.Buffer());
            if (variable == "Comment") {
                start = true;
                variable.SetSize(0ull);
                continue;
            }
            if (start) {
                if (counter > (nVarsPerThread * i)) {
                    if (counter >= (nVarsPerThread * (i + 1u))) {
                        break;
                    }

                    StreamString varOut = variable;
                    varOut += "_Http";

                    StreamString type = "float64";
                    uint32 numberOfElements = 1u;
                    pv pvs;
                    pvs.name = variable.Buffer();
                    result = connect_pvs(&pvs, 1);
                    if (!result) {
                        result = cainfo(pvs, type, numberOfElements);
                    }

                    newCfg.Printf("\n%s = {\n", varOut.Buffer());
                    newCfg.Printf("Type = %s\n", type.Buffer());
                    newCfg.Printf("DataSource = %s\n", HttpDs.Buffer());
                    if (numberOfElements > 1u) {
                        newCfg.Printf("%s\n", "NumberOfDimensions = 1");
                        newCfg.Printf("NumberOfElements = %d\n", numberOfElements);
                    }
                    newCfg.Printf("%s\n", "}");
                }
                counter++;

            }
            variable.SetSize(0ull);
        }

        StreamString indexesSignal = "ReceivedIndexes";
        indexesSignal.Printf("%d", i);
        newCfg.Printf("\n%s = {\n", indexesSignal.Buffer());
        newCfg.Printf("%s\n", "NumberOfDimensions = 1");
        newCfg.Printf("NumberOfElements = %d\n", toReceivePerThread);
        newCfg.Printf("%s\n", "Type = uint32");
        newCfg.Printf("DataSource = %s\n", HttpDs.Buffer());
        newCfg.Printf("%s\n", "}");

        printf("done IS\n");
        cdb.MoveToAncestor(1u);
        cdb.CreateRelative("OutputSignals");

        xmlFile.Seek(0ull);
        start = false;
        counter = 0u;
        StreamString diodeDs = "EpicsDiodeOutput";
        diodeDs.Printf("%d", i);
        while (GetVariable(xmlFile, variable)) {
            printf("variable=%s\n", variable.Buffer());
            if (variable == "Comment") {
                start = true;
                variable.SetSize(0ull);
                continue;
            }
            if (start) {
                if (counter > (nVarsPerThread * i)) {
                    if (counter >= (nVarsPerThread * (i + 1u))) {
                        break;
                    }

                    StreamString type = "float64";
                    uint32 numberOfElements = 1u;
                    pv pvs;
                    pvs.name = variable.Buffer();
                    result = connect_pvs(&pvs, 1);
                    if (!result) {
                        result = cainfo(pvs, type, numberOfElements);
                    }

                    newCfg.Printf("\n%s = {\n", variable.Buffer());
                    newCfg.Printf("Type = %s\n", type.Buffer());
                    newCfg.Printf("DataSource = %d\n", diodeDs.Buffer());
                    if (numberOfElements > 1u) {
                        newCfg.Printf("%s\n", "NumberOfDimensions = 1");
                        newCfg.Printf("NumberOfElements = %d\n", numberOfElements);
                    }
                    newCfg.Printf("%s\n", "}");

                }
                counter++;
            }
            variable.SetSize(0ull);
        }

        indexesSignal = "ReceivedIndexesDDB";
        indexesSignal.Printf("%d", i);
        newCfg.Printf("\n%s = {\n", indexesSignal.Buffer());
        newCfg.Printf("%s\n", "NumberOfDimensions = 1");
        newCfg.Printf("NumberOfElements = %d\n", toReceivePerThread);
        newCfg.Printf("%s\n", "Type = uint32");
        newCfg.Printf("%s\n", "DataSource = DDB1");
        newCfg.Printf("%s\n", "}");

        printf("done OS\n");
        cdb.MoveToAncestor(2u);

    }
    cdb.MoveToAncestor(1u);

    cdb.CreateAbsolute("$Application.+Data");
    cdb.Write("Class", "ReferenceContainer");

    cdb.CreateAbsolute("$Application.+Data.+DDB1");
    cdb.Write("Class", "GAMDataSource");
    cdb.MoveToAncestor(1u);

    cdb.CreateAbsolute("$Application.+Data.+Timings");
    cdb.Write("Class", "TimingDataSource");
    cdb.MoveToAncestor(1u);

    //for each thread create a EpicsDiodeDataSource and a HttpDiodeDataSource
    uint32 port = serverInitialPort;
    for (uint32 i = 0u; i < numberOfThreads; i++) {
        StreamString epicsDsName = "+EpicsDiodeOutput";
        epicsDsName.Printf("%d", i);
        cdb.CreateRelative(epicsDsName.Buffer());
        cdb.Write("Class", "EPICSCA::EPICSCAOutput");
        cdb.Write("CPUs", epicsOutputCpus);
        cdb.Write("StackSize", epicsOutputStackSize);
        cdb.Write("NumberOfBuffers",epicsOutputNBuffers);

        cdb.CreateRelative("Signals");

        StreamString variable;
        bool start = false;
        xmlFile.Seek(0ull);
        uint32 counter = 0u;
        while (GetVariable(xmlFile, variable)) {

            printf("variable=%s\n", variable.Buffer());
            if (variable == "Comment") {
                start = true;
                variable.SetSize(0ull);
                continue;
            }
            if (start) {
                if (counter > (nVarsPerThread * i)) {
                    if (counter >= (nVarsPerThread * (i + 1u))) {
                        break;
                    }
                    newCfg.Printf("\n%s = {\n", variable.Buffer());
                    newCfg.Printf("PVName = %s\n", variable.Buffer());
                    newCfg.Printf("%s\n", "}");
                }
                counter++;
            }
            variable.SetSize(0ull);
        }

        cdb.MoveToAncestor(2u);
        StreamString httpDsName = "+HttpDiodeInput";
        httpDsName.Printf("%d", i);
        cdb.CreateRelative(httpDsName.Buffer());
        cdb.Write("Class", "HttpDiodeReceiver");
        cdb.Write("ServerPort", port);
        cdb.Write("CPUMask", httpInputCpus);
        port++;

        cdb.CreateRelative("Signals");

        xmlFile.Seek(0ull);
        start = false;
        counter = 0u;
        while (GetVariable(xmlFile, variable)) {
            printf("variable=%s\n", variable.Buffer());
            if (variable == "Comment") {
                start = true;
                variable.SetSize(0ull);
                continue;
            }
            if (start) {
                if (counter > (nVarsPerThread * i)) {
                    if (counter >= (nVarsPerThread * (i + 1u))) {
                        break;
                    }
                    StreamString varOut = variable;
                    varOut += "_Http";
                    newCfg.Printf("\n%s = {\n", varOut.Buffer());
                    newCfg.Printf("%s\n", "Samples = 1");
                    newCfg.Printf("%s\n", "}");
                }
                counter++;
            }
            variable.SetSize(0ull);
        }
        //add the last signal
        StreamString indexesSignal = "ReceivedIndexes";
        indexesSignal.Printf("%d", i);
        newCfg.Printf("\n%s = {\n", indexesSignal.Buffer());
        newCfg.Printf("%s\n", "NumberOfDimensions = 1");
        newCfg.Printf("NumberOfElements = %d\n", toReceivePerThread);
        newCfg.Printf("%s\n", "Type = uint32");
        newCfg.Printf("%s\n", "}");

        cdb.MoveToAncestor(2u);
        StreamString linuxTimerDsName = "+LinuxTimer";
        linuxTimerDsName.Printf("%d", i);
        cdb.CreateRelative(linuxTimerDsName.Buffer());
        cdb.Write("Class", "LinuxTimer");
        cdb.Write("SleepNature", "Default");
        cdb.Write("ExecutionMode", "RealTimeThread");
        cdb.CreateRelative("Signals");
        cdb.CreateRelative("Counter");
        cdb.Write("Type", "uint32");
        cdb.MoveToAncestor(1u);
        cdb.CreateRelative("Time");
        cdb.Write("Type", "uint32");
        cdb.MoveToAncestor(3u);
    }
    cdb.CreateAbsolute("$Application.+States");
    cdb.Write("Class", "ReferenceContainer");
    cdb.CreateAbsolute("$Application.+States.+State1");
    cdb.Write("Class", "RealTimeState");
    cdb.CreateAbsolute("$Application.+States.+State1.+Threads");
    cdb.Write("Class", "ReferenceContainer");

    for (uint32 i = 0u; i < numberOfThreads; i++) {
        StreamString threadName = "+Thread";
        threadName.Printf("%d", i);
        cdb.CreateRelative(threadName.Buffer());
        cdb.Write("Class", "RealTimeThread");
        cdb.Write("CPUs", threadsCpu[i]);
        //TODO assign the cpu mask
        Vector < StreamString > functions(2u);
        functions[0] = "SyncGAM";
        functions[0].Printf("%d", i);
        functions[0].Seek(0ull);
        functions[1] = "CopyGAM";
        functions[1].Printf("%d", i);
        functions[1].Seek(0ull);
        cdb.Write("Functions", functions);
        cdb.MoveToAncestor(1u);
    }
    cdb.MoveToAncestor(3u);

    cdb.CreateAbsolute("$Application.+Scheduler");
    cdb.Write("Class", "GAMScheduler");
    cdb.Write("TimingDataSource", "Timings");
    cdb.MoveToRoot();
    newCfg.Close();
    xmlFile.Close();
    /* Shut down Channel Access */
    ca_context_destroy();

    return 0;
}
