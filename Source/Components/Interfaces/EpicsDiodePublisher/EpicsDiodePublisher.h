/**
 * @file EpicsDiodePublisher.h
 * @brief Header file for class EpicsDiodePublisher
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

 * @details This header file contains the declaration of the class EpicsDiodePublisher
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef EPICSDIODEPUBLISHER_H_
#define EPICSDIODEPUBLISHER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DiodeReceiver.h"
#include "MultiThreadService.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class EpicsDiodePublisher: public MultiThreadService, public EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()

    EpicsDiodePublisher();
    virtual ~EpicsDiodePublisher();

    virtual bool Initialise(StructuredDataI &data);

    bool SetDataSource(DiodeReceiver &dataSourceIn);

    virtual ErrorManagement::ErrorType Execute(ExecutionInfo& info);

    friend void ReceiverSyncCycleLoop(EpicsDiodePublisher &arg);

    ErrorManagement::ErrorType Start();

    ErrorManagement::ErrorType Stop();

protected:

    DiodeReceiver *dataSource;

    uint8 *memory;
    uint8 *memoryPrec;
    uint32 totalMemorySize;
    uint32 numberOfVariables;
    uint8 *changeFlag;

    uint32 numberOfSignalToBeSent;
    uint32 mainCpuMask;
    uint32 msecPeriod;

    uint32 nThreadsFinished;
    uint64 lastTickCounter;

    FastPollingMutexSem fmutex;
    uint32 currentIndex;
    volatile int32 quit;

    EventSem eventSem;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_INTERFACES_EPICSDIODEPUBLISHER_EPICSDIODEPUBLISHER_H_ */

