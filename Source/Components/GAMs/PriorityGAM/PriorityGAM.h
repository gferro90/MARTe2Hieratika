/**
 * @file PriorityGAM.h
 * @brief Header file for class PriorityGAM
 * @date 06 nov 2018
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

 * @details This header file contains the declaration of the class PriorityGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef PRIORITYGAM_H_
#define PRIORITYGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "GAM.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{
/**
 * @brief Selects the N signals to be sent every cycle on the base of a priority
 * mechanism.
 *
 * @details This GAM acts similarly to the IOGAM, copying the values of the input signals to the output signals.
 * The difference is that the last signal is a vector containing the selected signal indexes that have a greater priority
 * because they have changed in the last cycles. Note that this information is meaningful only if all the output signals
 * of this GAM are mapped in the same order to one single data source. Thus, follow the restrictions for the use of this GAM:\n
 *   - InputSize == (OutputSize - LastSignalSize)\n
 *   - All output signals mapped on the same DataSource
 * @Warning If in the data source that collects the output signals the signals are declared in a different order by configuration,
 * the selected indexes vector will be wrong because it refers to the order of the signals in the GAM. This error is not caught.
 *
 * @details The use case for this GAM is that the output signals must be sent (from the data source connected in output)
 * in chunks of size N (specified by the length of the vector of the last output signal), following the policy that if the signal has
 * changed, so it has to be sent first.
 * On the first cycles the GAM selects the signals only by a FIFO mechanism, until all the signals are covered, to send the initial value
 * of all the signals. On the following cycles, by default always a FIFO mechanism is employed, but the signals that changes are pushed to
 * the first positions of the queue.
 */
class PriorityGAM: public GAM {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Constructor
     */
    PriorityGAM();

    /**
     * @brief Destructor
     */
    virtual ~PriorityGAM();

    /**
     * @see GAM::Setup
     * @details Initialises the indexes queue of the signals to be sent and checks that
     *   - InputSize == (OutputSize - LastSignalSize)
     *   - All output signals mapped on the same DataSource
     *   - LastSignalNumberOfElements <= NumberOfInputSignals
     */
    virtual bool Setup();

    /**
     * @see GAM::Execute
     * @details Copies the input signals memory to the outputs signals memory and updates the last signal with the vector of the
     * N indexes of the signals that must be sent. This indexes are selected on the base of a FIFO mechanism, but if the signal
     * has changed, it will be pushed to the first positions of the queue.
     */
    virtual bool Execute();

protected:

    /**
     * Stores the previous values of the input signals.
     */
    uint8 *prevSignalMem;

    /**
     * A pointer to the last signal that contains the indexes
     * of the N signals to be sent
     */
    uint32 *sortedIndices;

    /**
     * Stores the number of signals to be sent.
     */
    uint32 numberOfSignalToBeSent;

    /**
     * The total size of the input signals memory
     */
    uint32 totalSize;

    /**
     * A counter used to send all the initial values
     * of the signals before starting the priority
     * policy.
     */
    uint8 chunckCounter;

    /**
     * The sorted queue that stores the indexes to be sent
     * at each cycle
     */
    uint32 *indexList;

    /**
     * The current index in the sorted queue
     */
    uint32 currentIdx;

    /**
     * The minimum position in the queue where to put the changed
     * signals. It can be different than zero if some changed signals
     * are not sent in the previous cycles.
     */
    uint32 currentChangePos;


    uint32 numberOfChunks;
};

}
/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* PRIORITYGAM_H_ */

