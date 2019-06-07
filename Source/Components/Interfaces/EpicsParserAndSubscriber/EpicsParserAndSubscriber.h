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
#include "MultiThreadService.h"
#include "EventSem.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

const uint32 PV_NAME_MAX_SIZE = 64u;

/**
 * Contains all the PV data
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

    /**
     * The index in the array
     */
    uint32 index;

    /**
     * Pointer to the memory that asserts if this
     * pv has changed
     */
    uint8 *changedFlag;

    /**
     * The pv timestamp
     */
    epicsTimeStamp *timeStamp;

    /**
     * A pointer to the shared mutex
     */
    FastPollingMutexSem *syncMutex;

    /**
     * The offset in the allocated memory
     */
    uint64 offset;

    /**
     * The type descriptor
     */
    TypeDescriptor td;
};

/**
 * @brief Stores the value and the time-stamp of the EPICS variables, whose names are contained
 * in a XML file to be given in input.
 *
 * @details Parses an XML file containing the name of the EPICS PVs. Then calls cainfo
 * to get the type and the number of elements of each variables and allocates the memory
 * to store the PV values and time-stamp. After that a subscription for each PV will be created
 * and when a PV value changes, the callback will store its value and timestamp in the memory.
 *
 * @details The Synchronise function can be used to get the current memory containing the PVs values and
 * time-stamps and the changed flags memory containing a flag for each variable asserting if the variable has
 * changed from the last call.
 */
class EpicsParserAndSubscriber: public MultiThreadService, public EmbeddedServiceMethodBinderI {
public:

    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Constructor
     */
    EpicsParserAndSubscriber();

    /**
     * @brief Destructor
     */
    virtual ~EpicsParserAndSubscriber();

    /**
     * @see Object::Initialise
     * @details The user must specify the following parameters:\n
     *   XmlFilePath: the path of the input file containing the names of the PV variables\n
     *   FirstVariableName: the name of the first PV in the XML file in order to start reading
     */
    virtual bool Initialise(StructuredDataI &data);

    /**
     * @brief Parses the XML file to get the PV names
     * @details Reads the PV names from the XML file specified in the configuration,
     * creates the array of PvDescriptor and starts filling the structure for each variable.
     * Then, starts the internal thread to register the subscribe callback for each PV (see Execute)
     */
    virtual bool ParseAndSubscribe();

    /**
     * @brief Allows another component to get the current PV values and time-stamps.
     * @param[out] memoryOut where the memory containing PV values and time-stamps must be copied on.
     * @param[out] changedFlags where the memory containing the changed flags must be copied on.
     * @post The internal changed flags will be reset to zero after the copy.
     */
    virtual bool Synchronise(uint8 *memoryOut,
                             uint8 *changedFlags);

    /**
     * @brief Returns the number of PVs
     * @return the number of PVs
     */
    uint32 GetNumberOfVariables();

    /**
     * @brief Returns the array of the PVs descriptors.
     * @return the array of the PVs descriptors.
     */
    PvDescriptor *GetPvDescriptors();

    /**
     * @brief Returns the allocated memory size
     * @return the allocated memory size
     */
    uint64 GetTotalMemorySize();

    /**
     * @brief The thread routine that registers the subscription callback for each PV
     * to get its value and timestamp
     */
    ErrorManagement::ErrorType Execute(ExecutionInfo& info);

    /**
     * @brief Returns true if the thread has terminated the initialisation.
     * @return true if the thread has terminated the initialisation.
     */
    bool InitialisationDone();

    /**
     * @brief Stops the execution
     */
    virtual ErrorManagement::ErrorType Stop();

private:

    /**
     * @brief Help function to get the EPICS variable information such as type and memory size
     * to create the memory buffer where to write the values on subscription.
     * @param[in] beg the start index.
     * @param[in] end the end index.
     * @param[in] threadId the thread identifier.
     */
    bool FillMemorySizes(uint32 beg,
                         uint32 end,
                         uint32 threadId);

    /**
     * @brief Help function to create the EPICS variable subscriptions
     * @param[in] beg the start index.
     * @param[in] end the end index.
     */
    void CreateSubscriptions(uint32 beg,
                             uint32 end);

    /**
     * @brief Help function to detach and destroy the EPICS context when finished or in case
     * of errors.
     * @param[in] threadId the thread identifier.
     */
    void CleanContext(uint32 threadId);

    /**
     * The first variable name
     */
    StreamString firstVariableName;

    /**
     * The input XML file
     * containing the name of all
     * the PVs
     */
    StreamString xmlFilePath;

    /**
     * An array of descriptors, one
     * for each pv
     */
    PvDescriptor *pvDescriptor;

    /**
     * The number of PVs
     */
    uint32 numberOfVariables;

    /**
     * The allocated memory for the PVs
     */
    uint8 **memory;

    /**
     * The memory allocated for the
     * changed flags to check if a PV
     * has changed or not
     */
    uint8 *changedFlagMem;

    /**
     * The mutex used to synchronise
     * the subscriptions with the main thread
     */
    FastPollingMutexSem fmutex;

    /**
     * Asserts that the initialisation
     * terminates
     */
    uint32 initialisationDone;

    /**
     * A thread counter
     */
    uint32 threatCnt;

    /**
     * The number of variables managed by each thread
     */
    uint32 nVarsPerChunk;

    /**
     * A vector containing the memory sizes for each thread
     */
    uint64 *memorySize;

    /**
     * An event semaphore to trigger the stop of
     * the execution
     */
    EventSem eventSem;

    /**
     * The maximum number of variables
     */
    uint32 maxNumberOfVariables;


};
}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* EPICSPARSERANDSUBSCIBER_H_ */

