/**
 * @file TriggerOnChangeHieratikaVecGAM.h
 * @brief Header file for class TriggerOnChangeHieratikaVecGAM
 * @date 01 nov 2018
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

 * @details This header file contains the declaration of the class TriggerOnChangeHieratikaVecGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef TRIGGERONCHANGEHIERATIKAVECGAM_H_
#define TRIGGERONCHANGEHIERATIKAVECGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "TriggerOnChangeHieratikaGAM.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class TriggerOnChangeHieratikaVecGAM: public TriggerOnChangeHieratikaGAM {
public:
    CLASS_REGISTER_DECLARATION()

    TriggerOnChangeHieratikaVecGAM();

    virtual ~TriggerOnChangeHieratikaVecGAM();

    virtual bool Initialise(StructuredDataI & data);

    virtual bool Setup();

    virtual bool Execute();

protected:

    StreamString varName;

    StreamString varValue;

    StreamString *splitted;
};

}
/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_GAMS_TRIGGERONCHANGEHIERATIKAVECGAM_TRIGGERONCHANGEHIERATIKAVECGAM_H_ */

