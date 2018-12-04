/**
 * @file DiodeReceiver.h
 * @brief Header file for class DiodeReceiver
 * @date 04 dic 2018
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

 * @details This header file contains the declaration of the class DiodeReceiver
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef DIODERECEIVER_H_
#define DIODERECEIVER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MultiThreadService.h"
#include "File.h"
#include "TCPSocket.h"
#include "EmbeddedServiceMethodBinderT.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class DiodeReceiver: public MultiThreadService {
public:
    CLASS_REGISTER_DECLARATION()
    DiodeReceiver();
    virtual ~DiodeReceiver();

    virtual bool Initialise(StructuredDataI &data);


    virtual ErrorManagement::ErrorType ThreadCycle(ExecutionInfo & info);

protected:
    EmbeddedServiceMethodBinderT<DiodeReceiver> embeddedMethod;

    StreamString serverIp;
    uint32 serverPort;

    TimeoutType acceptTimeout;

    File *outputFile;
    TCPSocket **openedFile;

    FastPollingMutexSem syncSem;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_INTERFACES_DIODERECEIVER_DIODERECEIVER_H_ */

