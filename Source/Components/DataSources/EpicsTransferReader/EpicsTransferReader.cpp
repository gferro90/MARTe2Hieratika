/**
 * @file EpicsTransferReader.cpp
 * @brief Source file for class EpicsTransferReader
 * @date 21 nov 2018
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
 * the class EpicsTransferReader (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "EpicsTransferReader.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
struct callbackFunArg {
    pv* pv;
    uint8 *pvMem;
    uint32 memSize;
    uint8 *tsMem;
    FastPollingMutexSem *mutex;
};

/*
typedef struct {
    char* name;
    chid chid;
    long dbfType;
    long dbrType;
    unsigned long nElems;       // True length of data in value
    unsigned long reqElems;     // Requested length of data
    int status;
    void* value;
    epicsTimeStamp tsPreviousC;
    epicsTimeStamp tsPreviousS;
    char firstStampPrinted;
    char onceConnected;
} pv;
*/

static void EPICSCAInputEventCallback(struct event_handler_args args) {
    pv *ppv = ( pv * ) ca_puser ( args.chid );

    callbackFunArg *pvargs = (callbackFunArg *) (args.usr);
    pv->mutex->FastLock();
    (void) MemoryOperationsHelper::Copy(pvargs->pvMem, args.dbr, pvargs->memSize);
    (void) MemoryOperationsHelper::Copy(pvargs->tsMem, ppv->value->stamp, 8u);

    pv->mutex->FastUnLock();
}







static void connectionHandlerCallback ( struct connection_handler_args args )
{
    pv *ppv = ( pv * ) ca_puser ( args.chid );
    if ( args.op == CA_OP_CONN_UP ) {
        nConn++;
        if (!ppv->onceConnected) {
            ppv->onceConnected = 1;
                                /* Set up pv structure */
                                /* ------------------- */

                                /* Get natural type and array count */
            ppv->dbfType = ca_field_type(ppv->chid);
            ppv->dbrType = dbf_type_to_DBR_TIME(ppv->dbfType); /* Use native type */
            if (dbr_type_is_ENUM(ppv->dbrType))                /* Enums honour -n option */
            {
                if (enumAsNr) ppv->dbrType = DBR_TIME_INT;
                else          ppv->dbrType = DBR_TIME_STRING;
            }
            else if (floatAsString &&
                     (dbr_type_is_FLOAT(ppv->dbrType) || dbr_type_is_DOUBLE(ppv->dbrType)))
            {
                ppv->dbrType = DBR_TIME_STRING;
            }
                                /* Set request count */
            ppv->nElems   = ca_element_count(ppv->chid);
            ppv->reqElems = reqElems > ppv->nElems ? ppv->nElems : reqElems;

                                /* Issue CA request */
                                /* ---------------- */
            /* install monitor once with first connect */
            ppv->status = ca_create_subscription(ppv->dbrType,
                                                ppv->reqElems,
                                                ppv->chid,
                                                eventMask,
                                                EPICSCAInputEventCallback,
                                                (void*)ppv,
                                                NULL);
        }
    }
    else if ( args.op == CA_OP_CONN_DOWN ) {
        nConn--;
        ppv->status = ECA_DISCONN;
        print_time_val_sts(ppv, reqElems);
    }
}







}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

EpicsTransferReader::EpicsTransferReader() :
        TransferDataSource() {
    // Auto-generated constructor stub for EpicsTransferReader
    // TODO Verify if manual additions are needed

    cpuMask = 0u;
    stackSize = 0u;

}

EpicsTransferReader::~EpicsTransferReader() {
    // Auto-generated destructor stub for EpicsTransferReader
    // TODO Verify if manual additions are needed
}

bool EpicsTransferReader::Synchronise() {
    return true;
}

uint32 EpicsTransferReader::GetNumberOfStatefulMemoryBuffers() {
    return 2u;
}

bool EpicsTransferReader::Initialise(StructuredDataI & data) {

    bool ret = DataSourceI::Initialise(data);
    if (ret) {
        if (!data.Read("CPUs", cpuMask)) {
            REPORT_ERROR(ErrorManagement::Information, "No CPUs defined. Using default = %d", cpuMask);
        }
        if (!data.Read("StackSize", stackSize)) {
            REPORT_ERROR(ErrorManagement::Information, "No StackSize defined. Using default = %d", stackSize);
        }
        executor.SetStackSize(stackSize);
        executor.SetCPUMask(cpuMask);
    }
    return ret;
}


bool EpicsTransferReader::SetConfiguredDatabase(StructuredDataI & data){
    bool ret=DataSourceI::SetConfiguredDatabase(data);
    if(ret){
        //the last signal is the big uint64 ts memory
        uint32 numberOfVariables=(numberOfSignals-1u);
        chids=new chanId[numberOfVariables];
    }
    return ret;
}


ErrorManagement::ErrorType EpicsTransferReader::Execute(ExecutionInfo& info) {
    ErrorManagement::ErrorType err = ErrorManagement::NoError;
    if (info.GetStage() == ExecutionInfo::StartupStage) {
        /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
        if (ca_context_create(ca_enable_preemptive_callback) != ECA_NORMAL) {
            err = ErrorManagement::FatalError;
            REPORT_ERROR(err, "ca_enable_preemptive_callback failed");
        }

        uint32 n;
        uint32 nOfSignals = GetNumberOfSignals();
        memoryConfig.MoveRelative("MemoryConfiguration");
        uint32 nOfSignals = memoryConfig.GetNumberOfChildren();
        for (n = 0u; (n < nOfSignals); n++) {
            StreamString signalName = memoryConfig.GetChildname(n);
            /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
            if (ca_create_channel(signalName.Buffer(), NULL_PTR(caCh *), NULL_PTR(void *), 20u, &chids[n]) != ECA_NORMAL) {
                err = ErrorManagement::FatalError;
                REPORT_ERROR(err, "ca_create_channel failed for PV with name %s", signalName.Buffer());
            }
            if (err.ErrorsCleared()) {
                chtype pvType = DBR_DOUBLE;

                memoryConfig.MoveToChild(n);
                StreamString type;
                memoryConfig.Read("Type", type);

                if (type == "int16") {
                    pvType.pvType = DBR_SHORT;
                }
                else if (type == "uint16") {
                    pvType = DBR_SHORT;
                }
                else if (type == "int32") {
                    pvType = DBR_LONG;
                }
                else if (type == "uint32") {
                    pvType = DBR_LONG;
                }
                else if (type == "float32") {
                    pvType = DBR_FLOAT;
                }
                else if (type == "float64") {
                    pvType = DBR_DOUBLE;
                }
                else {
                    REPORT_ERROR(ErrorManagement::ParametersError, "Type %s is not supported", type.Buffer());
                    err = true;
                }
                uint32 pvNumberOfElements = 1u;
                if (!memoryConfig.Read("NumberOfElements", pvNumberOfElements)) {
                    pvNumberOfElements = 1u;
                }

                evid pvEvid;
                if (err.ErrorsCleared()) {
                    //need the memory of the pv
                    callbackFunArg args;
                    args.pvMem = memory[1u] + offsets[n];
                    if (n < (nOfSignals - 1u)) {
                        args.memSize = (offsets[n + 1u] - offsets[n]);
                    }
                    else {
                        args.memSize = (totalMemorySize - offsets[n])
                    }
                    args.mutex = &mutex;

                    if (ca_create_subscription(pvType, pvNumberOfElements, chids[n], DBE_VALUE, &EPICSCAInputEventCallback, &args, &pvEvid) != ECA_NORMAL) {
                        err = ErrorManagement::FatalError;
                        REPORT_ERROR(err, "ca_create_subscription failed for PV %s", signalName.Buffer());
                    }
                }

            }
            if (err.ErrorsCleared()) {
                memoryConfig.MoveToAncestor(1u);
            }
        }
        if (err.ErrorsCleared()) {
            memoryConfig.MoveToAncestor(1u);
        }
    }
    else if (info.GetStage() != ExecutionInfo::BadTerminationStage) {
        Sleep::Sec(1.0);
    }
    else {
        uint32 n;
        uint32 nOfSignals = GetNumberOfSignals();
        if (pvs != NULL_PTR(PVWrapper *)) {
            for (n = 0u; (n < nOfSignals); n++) {
                (void) ca_clear_subscription(pvs[n].pvEvid);
                (void) ca_clear_event(pvs[n].pvEvid);
                (void) ca_clear_channel(pvs[n].pvChid);
            }
        }
        ca_detach_context();
        ca_context_destroy();
    }

    return err;
}

const char8 *EpicsTransferReader::GetBrokerName(StructuredDataI &data,
                                                const SignalDirection direction) {
    return "MemoryMapInputBroker";
}

uint32 EpicsTransferReader::GetCurrentStateBuffer() {
    //copy the memory from one to the other
    if (mutex.FastLock()) {
        MemoryOperationsHelper::Copy(memory[0u], memory[1u], totalMemorySize);
        mutex.FastUnLock()
    }
    return 0u;
}

}
