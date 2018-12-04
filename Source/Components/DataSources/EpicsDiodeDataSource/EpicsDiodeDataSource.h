/**
 * @file EpicsDataSource.h
 * @brief Header file for class EpicsDataSource
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

 * @details This header file contains the declaration of the class EpicsDataSource
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef EPICSDATASOURCE_H_
#define EPICSDATASOURCE_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/
#include <cadef.h>

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MemoryDataSourceI.h"
#include "EmbeddedServiceMethodBinderI.h"
#include "EventSem.h"
#include "SingleThreadService.h"
#include "FastPollingMutexSem.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe{
const uint32 PV_NAME_MAX_SIZE = 64u;

/**
 * @brief A DataSource which allows to retrieved data from any number of PVs using the EPICS channel access client protocol.
 * Data is asynchronously retrieved using ca_create_subscriptions in the context of a different thread (w.r.t. to the real-time thread).
 *
 * The configuration syntax is (names are only given as an example):
 *
 * <pre>
 * +EpicsDiodeDataSource_1 = {
 *     Class = EPICSCA::EpicsDiodeDataSource
 *     StackSize = 1048576 //Optional the EmbeddedThread stack size. Default value is THREADS_DEFAULT_STACKSIZE * 4u
 *     CPUs = 0xff //Optional the affinity of the EmbeddedThread (where the EPICS context is attached).
 *     Signals = {
 *          PV1 = { //At least one shall be defined
 *             PVName = My::PV1 //Compulsory. Name of the PV.
 *             Type = uint32 //Compulsory. Supported types are uint16, int16, int32, uint32, uint64, int64, float32 and float64
 *             NumberOfElements = 1 //Arrays also supported
 *          }
 *          ...
 *     }
 * }
 *
 * </pre>
 */

class EpicsDiodeDataSource: public MemoryDataSourceI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor. NOOP.
     */
    EpicsDiodeDataSource ();

    /**
     * @brief Destructor.
     * @details TODO.
     */
    virtual ~EpicsDiodeDataSource();

    /**
     * @brief See DataSourceI::AllocateMemory. NOOP.
     * @return true.
     */
    virtual bool AllocateMemory();


    /**
     * @brief See DataSourceI::GetNumberOfMemoryBuffers.
     * @details Only InputSignals are supported.
     * @return MemoryMapInputBroker.
     */
    virtual const char8 *GetBrokerName(StructuredDataI &data,
                                       const SignalDirection direction);

    /**
     * @brief See DataSourceI::PrepareNextState. NOOP.
     * @return true.
     */
    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    /**
     * @brief Loads and verifies the configuration parameters detailed in the class description.
     * @return true if all the mandatory parameters are correctly specified and if the specified optional parameters have valid values.
     */
    virtual bool Initialise(StructuredDataI & data);

    /**
     * @brief Final verification of all the parameters. Setup of the memory required to hold all the signals.
     * @details This method verifies that all the parameters requested by the GAMs interacting with this DataSource
     *  are valid and consistent with the parameters set during the initialisation phase.
     * In particular the following conditions shall be met:
     * - All the signals have the PVName defined
     * - All the signals have one of the following types: uint32, int32, float32 or float64.
     * @return true if all the parameters are valid and the conditions above are met.
     */
    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    /**
     * @brief See DataSourceI::Synchronise.
     * @return false.
     */
    virtual bool Synchronise();

    /**
     * @brief Registered as the ca_create_subscription callback function.
     * It calls updates the memory of the corresponding PV variable.
     */
    friend void EpicsDiodeDataSourceEventCallback(struct event_handler_args args);

    virtual uint32 GetCurrentStateBuffer();


    virtual uint32 GetNumberOfStatefulMemoryBuffers();

private:

    /**
     * Wraps a PV
     */
    struct PVArgs {
        /**
         * The channel identifier
         */
        chid pvChid;

        /**
         * The memory of the signal associated to this channel
         */
        void *memory[2];

        /**
         * The PV type
         */
        chtype pvType;

        evid pvEvid;


        uint32 numberOfElements;
        /**
         * The memory size
         */
        uint32 memorySize;

        /**
         * The PV name
         */
        char8 pvName[PV_NAME_MAX_SIZE];

        FastPollingMutexSem *mutexPtr;
    };

    /**
     * List of PVs.
     */
    PVArgs *pvs;

    /**
     * Stores the configuration information received at Initialise.
     */
    ConfigurationDatabase originalSignalInformation;

    FastPollingMutexSem mutex;
};
}
/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* EPICSDATASOURCE_H_ */

