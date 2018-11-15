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
 * Decides the N signals to be sent every cycle on the base of a priority
 * mechanism.
 * Each cycle the N previous sent signal will be reset and to all the signals
 * a fix cycle amount is added to the priority. If the signal changed its value
 * another amount is added increasing its priority.
 * The N signals with the highest priorities will be sent.
 */
class PriorityGAM: public GAM {
public:
    CLASS_REGISTER_DECLARATION()
    PriorityGAM();

    virtual ~PriorityGAM();

    virtual bool Initialise(StructuredDataI & data);

    virtual bool Setup();

    virtual bool Execute();

protected:

    uint32 cycleTax;

    uint32 changeTax;

    uint8 *prevSignalMem;

    uint32 *byteSize;

    uint32 *offset;

    uint32 *sortedIndices;

    uint32 numberOfSignalToBeSent;

    uint32 *priorityList;

    uint32 *priorityListBuffer;

    uint32 totalSize;

    uint8 firstTime;
};

}
/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* PRIORITYGAM_H_ */

