/**
 * @file PrioritySender.h
 * @brief Header file for class PrioritySender
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

 * @details This header file contains the declaration of the class PrioritySender
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef PRIORITYSENDER_H_
#define PRIORITYSENDER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "FastPollingMutexSem.h"
#include "MultiThreadService.h"
#include "EpicsParserAndSubscriber.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class PrioritySender: public MultiThreadService {
public:
    CLASS_REGISTER_DECLARATION()

    PrioritySender();

    virtual ~PrioritySender();

    virtual bool Initialise(StructuredDataI &data);

    bool SetDataSource(EpicsParserAndSubscriber &dataSourceIn);

    virtual ErrorManagement::ErrorType ThreadCycle(ExecutionInfo & info);

    void Quit();

    friend void CycleLoop(PrioritySender &arg);
    virtual ErrorManagement::ErrorType Start();

private:


    EmbeddedServiceMethodBinderT<PrioritySender> embeddedMethod;

    uint32 *indexList;
    EpicsParserAndSubscriber *dataSource;
    uint8 *memory;

    uint8*changeFlag;

    uint64 totalMemorySize;
    uint32 numberOfVariables;

    FastPollingMutexSem syncSem;

    uint32 currentIdx;
    uint32 currentChangePos;

    uint32 numberOfSignalToBeSent;

    EventSem eventSem;
    uint32 nThreadsFinished;
    uint32 chunckCounter;

    uint32 numberOfChunks;

    StreamString serverIpAddress;
    uint32 serverPort;
    TimeoutType connectionTimeout;

    uint32 mainCpuMask;

    uint8 quit;

    uint64 lastTickCounter;
    uint32 msecPeriod;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* PRIORITYSENDER_H_ */

