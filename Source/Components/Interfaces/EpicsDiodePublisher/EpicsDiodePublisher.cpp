/**
 * @file EpicsDiodePublisher.cpp
 * @brief Source file for class EpicsDiodePublisher
 * @date 15 mag 2019
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
 * the class EpicsDiodePublisher (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "EpicsDiodePublisher.h"
#include "AdvancedErrorManagement.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

void ReceiverSyncCycleLoop(EpicsDiodePublisher &arg) {
    while (arg.quit == 0) {
        uint32 numberThreadsFinished = 0u;
        if (arg.fmutex.FastLock()) {
            numberThreadsFinished = arg.nThreadsFinished;
            arg.fmutex.FastUnLock();
        }

        if (numberThreadsFinished == arg.numberOfPoolThreads) {
            //sync here
            arg.nThreadsFinished = 0u;
            arg.dataSource->Synchronise(arg.memory, arg.changeFlag);
            arg.eventSem.Post();

            //wait the cycle time
            uint32 elapsed = (uint32)(((float32)(HighResolutionTimer::Counter() - arg.lastTickCounter) * 1000u * HighResolutionTimer::Period()));

            if (elapsed < arg.msecPeriod) {
                Sleep::MSec(arg.msecPeriod - elapsed);
            }
            arg.lastTickCounter = HighResolutionTimer::Counter();
        }
    }

    while (arg.nThreadsFinished != arg.numberOfPoolThreads) {
        Sleep::Sec(1u);
    }
    arg.eventSem.Post();
    Atomic::Increment(&arg.quit);

}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

EpicsDiodePublisher::EpicsDiodePublisher() :
        EmbeddedServiceMethodBinderI(),
        MultiThreadService((EmbeddedServiceMethodBinderI&) (*this)) {
    dataSource = NULL;

    memory = NULL;
    memoryPrec = NULL;
    totalMemorySize = 0u;
    numberOfVariables = 0u;
    changeFlag = NULL;
    numberOfSignalToBeSent = 0u;
    mainCpuMask = 0u;
    msecPeriod = 0u;
    nThreadsFinished = 0u;
    lastTickCounter = 0ull;
    currentIndex = 0u;
    quit = 0;
    eventSem.Create();

}

EpicsDiodePublisher::~EpicsDiodePublisher() {

    if (memory != NULL) {
        delete[] memory;
    }
    if (memoryPrec != NULL) {
        delete[] memoryPrec;
    }
    if (changeFlag != NULL) {
        delete[] changeFlag;
    }
}

bool EpicsDiodePublisher::Initialise(StructuredDataI &data) {
    bool ret = MultiThreadService::Initialise(data);
    if (ret) {
        ret = data.Read("NumberOfSignalsPerThread", numberOfSignalToBeSent);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please specify NumberOfSignalPerThread");
        }
    }
    if (ret) {
        ret = data.Read("MainCpuMask", mainCpuMask);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define MainCpuMask");
        }
    }
    if (ret) {
        ret = data.Read("MsecPeriod", msecPeriod);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define MsecPeriod");
        }
    }
    return ret;
}

bool EpicsDiodePublisher::SetDataSource(DiodeReceiver &dataSourceIn) {
    dataSource = &dataSourceIn;
    bool ret = false;
    if (dataSource != NULL) {
        while (!dataSource->InitialisationDone()) {
            Sleep::Sec(1u);
        }
        totalMemorySize = dataSource->GetTotalMemorySize();
        memory = (uint8*) HeapManager::Malloc(totalMemorySize);
        numberOfVariables = dataSource->GetNumberOfVariables();
        changeFlag = (uint8*) HeapManager::Malloc(numberOfVariables);
        uint32 sentPerCycle = (numberOfPoolThreads * numberOfSignalToBeSent);
        ret = (numberOfVariables >= sentPerCycle);
        if (ret) {
            MemoryOperationsHelper::Set(memory, 0, totalMemorySize);
            MemoryOperationsHelper::Set(changeFlag, 0, numberOfVariables);
            nThreadsFinished = 0u;
        }
        else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "The number of variables (%d) must be > than the variables sent per cycle (%d)",
                         numberOfVariables, sentPerCycle);
        }
    }
    return ret;
}

ErrorManagement::ErrorType EpicsDiodePublisher::Execute(ExecutionInfo& info) {
    //caput data section
    ErrorManagement::ErrorType err;

    if (info.GetStage() == MARTe::ExecutionInfo::StartupStage) {

    }
    else if (info.GetStage() == MARTe::ExecutionInfo::MainStage) {
        if (quit == 0) {
            uint32 currentIdx = 0u;
            if (fmutex.FastLock()) {
                currentIdx = currentIndex;
                currentIndex += numberOfSignalToBeSent;
                currentIndex %= numberOfVariables;
                eventSem.Reset();
                nThreadsFinished++;
                fmutex.FastUnLock();
            }
            //lock on the index list
            eventSem.Wait(TTInfiniteWait);
            uint32 n = currentIdx;
            if (dataSource != NULL) {
                PvRecDescriptor *pvs = dataSource->GetPvDescriptors();
                if (pvs != NULL) {
                    for (uint32 i = 0u; i < numberOfSignalToBeSent; i++) {
                        n %= numberOfVariables;
                        if (changeFlag[n] == 1) {

                            if (ca_array_put((pvs[n].pvType), pvs[n].numberOfElements, pvs[n].pvChid, memory + pvs[n].offset) != ECA_NORMAL) {
                                printf("ca_put failed for PV: %s\n", pvs[n].pvName);
                            }
                            else{
                                printf("ca_put ok for PV: %s\n", pvs[n].pvName);
                            }
                            (void) ca_pend_io(0.1);

                        }
                        n++;
                    }
                }
            }
        }
    }
    else {

    }
    return err;
}

ErrorManagement::ErrorType EpicsDiodePublisher::Start() {
    lastTickCounter = HighResolutionTimer::Counter();
    ErrorManagement::ErrorType err = MultiThreadService::Start();
    Threads::BeginThread((ThreadFunctionType) ReceiverSyncCycleLoop, this, THREADS_DEFAULT_STACKSIZE, NULL, ExceptionHandler::NotHandled, mainCpuMask);
    return err;
}

ErrorManagement::ErrorType EpicsDiodePublisher::Stop() {
    Atomic::Increment(&quit);
    eventSem.Wait();
    return MultiThreadService::Stop();
}
CLASS_REGISTER(EpicsDiodePublisher, "1.0")

}
