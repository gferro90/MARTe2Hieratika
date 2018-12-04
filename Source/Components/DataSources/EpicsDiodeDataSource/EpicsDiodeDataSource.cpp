/**
 * @file EpicsDataSource.cpp
 * @brief Source file for class EpicsDataSource
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
 * the class EpicsDataSource (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "EpicsDiodeDataSource.h"

#include "AdvancedErrorManagement.h"
#include "MemoryMapInputBroker.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {
/**
 * @brief Callback function for the ca_create_subscription. Single point of access which
 * delegates the events to the corresponding EPICSPV instance.
 */
/*lint -e{1746} function must match required prototype and thus cannot be changed to constant reference.*/
void EpicsDiodeDataSourceEventCallback(struct event_handler_args const args) {
    EpicsDiodeDataSource::PVArgs *pv = static_cast<EpicsDiodeDataSource::PVArgs *>(args.usr);
    pv->mutexPtr->FastLock();
    if (pv != NULL_PTR(EpicsDiodeDataSource::PVArgs *)) {
        (void) MemoryOperationsHelper::Copy(pv->memory[1], args.dbr, pv->memorySize);
    }
    //epicsTimeStamp
    pv->mutexPtr->FastUnLock();
}
}
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
EpicsDiodeDataSource::EpicsDiodeDataSource() :
        MemoryDataSourceI() {
    pvs = NULL_PTR(EpicsDiodeDataSource::PVArgs *);
    mutex.Create();
}

/*lint -e{1551} must stop the SingleThreadService in the destructor.*/
EpicsDiodeDataSource::~EpicsDiodeDataSource() {
    uint32 n;
    uint32 numberOfSignals = GetNumberOfSignals();
    if (pvs != NULL_PTR(EpicsDiodeDataSource::PVArgs *)) {
        for (n = 0u; (n < numberOfSignals); n++) {
            (void) ca_clear_subscription(pvs[n].pvEvid);
            (void) ca_clear_event(pvs[n].pvEvid);
            (void) ca_clear_channel(pvs[n].pvChid);
            if (pvs[n].memory[0] != NULL_PTR(void *)) {
                GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(pvs[n].memory[0]);
            }
            if (pvs[n].memory[1] != NULL_PTR(void *)) {
                GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(pvs[n].memory[1]);
            }
        }
        delete[] pvs;

    }
    ca_detach_context();
    ca_context_destroy();

}

bool EpicsDiodeDataSource::Initialise(StructuredDataI & data) {
    bool ok = MemoryDataSourceI::Initialise(data);
    if (ok) {
        ok = data.MoveRelative("Signals");
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Could not move to the Signals section");
        }
        if (ok) {
            ok = data.Copy(originalSignalInformation);
        }
        if (ok) {
            ok = originalSignalInformation.MoveToRoot();
        }
        //Do not allow to add signals in run-time
        if (ok) {
            ok = signalsDatabase.MoveRelative("Signals");
        }
        if (ok) {
            ok = signalsDatabase.Write("Locked", 1u);
        }
        if (ok) {
            ok = signalsDatabase.MoveToAncestor(1u);
        }
    }
    if (ok) {
        ok = data.MoveToAncestor(1u);
    }
    return ok;
}

bool EpicsDiodeDataSource::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    //Check the signal index of the timing signal.
    uint32 numberOfSignals = GetNumberOfSignals();
    if (ok) {
        ok = (numberOfSignals > 0u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "At least one signal shall be defined");
        }
    }
    if (ok) {
        //Do not allow samples
        uint32 functionNumberOfSignals = 0u;
        uint32 n;
        if (GetFunctionNumberOfSignals(InputSignals, 0u, functionNumberOfSignals)) {
            for (n = 0u; (n < functionNumberOfSignals) && (ok); n++) {
                uint32 nSamples;
                ok = GetFunctionSignalSamples(InputSignals, 0u, n, nSamples);
                if (ok) {
                    ok = (nSamples == 1u);
                }
                if (!ok) {
                    REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
                }
            }
        }
    }
    return ok;
}

bool EpicsDiodeDataSource::AllocateMemory() {
    bool ret=MemoryDataSourceI::AllocateMemory();
    if(ret){
        pvs = new EpicsDiodeDataSource::PVArgs[numberOfSignals];
        /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
        ret=ca_context_create(ca_enable_preemptive_callback) == ECA_NORMAL;
        if (ret) {
            REPORT_ERROR(ErrorManagement::FatalError, "ca_enable_preemptive_callback failed");
        }

        uint32 n;
        uint32 numberOfSignals = GetNumberOfSignals();
        if (pvs != NULL_PTR(EpicsDiodeDataSource::PVArgs *)) {
            for (n = 0u; (n < numberOfSignals); n++) {
            }
        }


        for (n = 0u; (n < numberOfSignals); n++) {
            pvs[n].memory[0] = NULL_PTR(void *);
            pvs[n].memory[1] = NULL_PTR(void *);
        }
        for (n = 0u; (n < numberOfSignals) && (ret); n++) {
            //Note that the RealTimeApplicationConfigurationBuilder is allowed to change the order of the signals w.r.t. to the originalSignalInformation
            StreamString orderedSignalName;
            ret = GetSignalName(n, orderedSignalName);
            if (ret) {
                //Have to mix and match between the original setting of the DataSource signal
                //and the ones which are later added by the RealTimeApplicationConfigurationBuilder
                ret = originalSignalInformation.MoveRelative(orderedSignalName.Buffer());
            }
            StreamString pvName;
            if (ret) {
                ret = originalSignalInformation.Read("PVName", pvName);
                if (!ret) {
                    uint32 nn = n;
                    REPORT_ERROR(ErrorManagement::ParametersError, "No PVName specified for signal at index %d", nn);
                }
            }
            TypeDescriptor td = GetSignalType(n);
            if (ret) {
                (void) StringHelper::CopyN(&pvs[n].pvName[0], pvName.Buffer(), PV_NAME_MAX_SIZE);
                if (td == SignedInteger8Bit) {
                    pvs[n].pvType = DBR_CHAR;
                }
                else if (td == UnsignedInteger8Bit) {
                    pvs[n].pvType = DBR_CHAR;
                }
                else if (td == SignedInteger16Bit) {
                    pvs[n].pvType = DBR_SHORT;
                }
                else if (td == UnsignedInteger16Bit) {
                    pvs[n].pvType = DBR_SHORT;
                }
                else if (td == SignedInteger32Bit) {
                    pvs[n].pvType = DBR_LONG;
                }
                else if (td == UnsignedInteger32Bit) {
                    pvs[n].pvType = DBR_LONG;
                }
                else if (td == Float32Bit) {
                    pvs[n].pvType = DBR_FLOAT;
                }
                else if (td == Float64Bit) {
                    pvs[n].pvType = DBR_DOUBLE;
                }
                else {
                    REPORT_ERROR(ErrorManagement::ParametersError, "Type %s is not supported", TypeDescriptor::GetTypeNameFromTypeDescriptor(td));
                    ret = false;
                }
            }
            uint32 byteSize=0u;
            if (ret) {
                ret = GetSignalByteSize(n,byteSize);
            }
            uint32 numberOfElements;
            if(ret){
                ret=GetSignalNumberOfElements(n, numberOfElements);
            }
            if (ret) {
                pvs[n].numberOfElements = numberOfElements;
                pvs[n].memorySize = byteSize;
                pvs[n].memory[0] = &memory[signalOffsets[n]];
                pvs[n].memory[1] = &memory[stateMemorySize+signalOffsets[n]];
                pvs[n].mutexPtr=&mutex;
                ret = originalSignalInformation.MoveToAncestor(1u);
            }
            if(ret){
                ret=(ca_create_channel(&pvs[n].pvName[0], NULL_PTR(caCh *), NULL_PTR(void *), 20u, &pvs[n].pvChid) == ECA_NORMAL);
                /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
                if (ret) {
                    REPORT_ERROR(ErrorManagement::FatalError, "ca_create_channel failed for PV with name %s", pvs[n].pvName);
                }
                if (ret) {
                    /*lint -e{9130} -e{835} -e{845} -e{747} Several false positives. lint is getting confused here for some reason.*/
                    if (ca_create_subscription(pvs[n].pvType, pvs[n].numberOfElements, pvs[n].pvChid, DBE_VALUE, &EpicsDiodeDataSourceEventCallback, &pvs[n],
                                               &pvs[n].pvEvid) != ECA_NORMAL) {
                        REPORT_ERROR(ErrorManagement::FatalError, "ca_create_subscription failed for PV %s", pvs[n].pvName);
                    }
                }
            }

        }
    }
    return ret;
}



/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: The brokerName only depends on the direction */
const char8* EpicsDiodeDataSource::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
        brokerName = "MemoryMapInputBroker";
    }
    return brokerName;
}


/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool EpicsDiodeDataSource::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}



uint32 EpicsDiodeDataSource::GetNumberOfStatefulMemoryBuffers() {
    return 2u;
}


bool EpicsDiodeDataSource::Synchronise() {
    return true;
}


uint32 EpicsDiodeDataSource::GetCurrentStateBuffer(){
    if(mutex.FastLock()){
        MemoryOperationsHelper::Copy(memory, memory+stateMemorySize, stateMemorySize);
        mutex.FastUnLock();
    }
    return 0u;
}




CLASS_REGISTER(EpicsDiodeDataSource, "1.0")

}
