/**
 * @file EpicsParserAndSubscriber.h
 * @brief Header file for class EpicsParserAndSubscriber
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

 * @details This header file contains the declaration of the class EpicsParserAndSubscriber
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef EPICSPARSERANDSUBSCIBER_H_
#define EPICSPARSERANDSUBSCIBER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <epicsStdlib.h>

#include <cadef.h>
#include <epicsGetopt.h>

#include "tool_lib.h"

#include <string.h>



/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "Object.h"
#include "StreamString.h"
#include "EmbeddedServiceMethodBinderI.h"
#include "SingleThreadService.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

const uint32 PV_NAME_MAX_SIZE = 64u;

/**
 * Wraps a PV
 */
struct PvDescriptor {
    /**
     * The channel identifier
     */
    chid pvChid;
    /**
     * The event identifier
     */
    evid pvEvid;
    /**
     * The PV type
     */
    chtype pvType;
    /**
     * The memory of the signal associated to this channel
     */
    void *memory;
    /**
     * The number of elements > 0
     */
    uint32 numberOfElements;
    /**
     * The memory size
     */
    uint32 memorySize;
    /**
     * The PV name
     */
    char8 pvName[PV_NAME_MAX_SIZE];


    uint32 index;

    uint8 *changedFlag;

    epicsTimeStamp *timeStamp;

    FastPollingMutexSem *syncMutex;

    uint64 offset;

};


class EpicsParserAndSubscriber: public Object, public EmbeddedServiceMethodBinderI {
public:

    CLASS_REGISTER_DECLARATION()
    EpicsParserAndSubscriber();

    virtual bool Initialise(StructuredDataI &data);


    virtual ~EpicsParserAndSubscriber();

    //initialisation:
    //parses the excel variables and creates subscriptions
    //for value and timestamp
    virtual bool ParseAndSubscribe();

    //to be called by the prioriy executor
    virtual bool Synchronise(uint8 *memoryOut, uint8 *changedFlags);


    uint32 GetNumberOfVariables();

    PvDescriptor *GetPvDescriptors();

    uint64 GetTotalMemorySize();

    ErrorManagement::ErrorType Execute(ExecutionInfo& info);

    bool InitialisationDone();

private:
    StreamString firstVariableName;
    StreamString xmlFilePath;
    PvDescriptor *pvDescriptor;
    uint32 numberOfVariables;
    uint64 totalMemorySize;
    uint8 *memory;
    uint8 *changedFlagMem;
    FastPollingMutexSem fmutex;
    SingleThreadService executor;
    uint8 initialisationDone;
};
}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* EPICSPARSERANDSUBSCIBER_H_ */

