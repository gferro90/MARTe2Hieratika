/**
 * @file DiodeLogger.h
 * @brief Header file for class DiodeLogger
 * @date 02 ago 2020
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

 * @details This header file contains the declaration of the class DiodeLogger
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef SOURCE_COMPONENTS_INTERFACES_DIODELOGGER_DIODELOGGER_H_
#define 		SOURCE_COMPONENTS_INTERFACES_DIODELOGGER_DIODELOGGER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "StructuredDataI.h"
#include "Object.h"
#include "File.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

using namespace MARTe;

class DiodeLogger: public Object {
public:
    CLASS_REGISTER_DECLARATION()

    DiodeLogger();

    virtual bool Initialise(StructuredDataI &data);

    virtual ~DiodeLogger();

    void AddSample(int64 *samples);

    uint32 GetNumberOfSignals();

private:

    File debugFile;

    uint32 numberOfSignals;

    uint32 windowSize;

    uint32 printCycles;

    int64 **windows;

    uint32 index;

    uint32 counter;
};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_INTERFACES_DIODELOGGER_DIODELOGGER_H_ */

