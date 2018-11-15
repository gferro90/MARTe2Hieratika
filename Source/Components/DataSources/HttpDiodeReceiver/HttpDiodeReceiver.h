/**
 * @file HttpDiodeReceiver.h
 * @brief Header file for class HttpDiodeReceiver
 * @date 14 nov 2018
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

 * @details This header file contains the declaration of the class HttpDiodeReceiver
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef HTTPDIODERECEIVER_H_
#define HTTPDIODERECEIVER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MemoryDataSourceI.h"
#include "TCPSocket.h"
#include "EmbeddedThreadI.h"
#include "SingleThreadService.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class HttpDiodeReceiver: public MemoryDataSourceI, public EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()

    HttpDiodeReceiver();

    virtual ~HttpDiodeReceiver();

    virtual bool Initialise(StructuredDataI &data);

    virtual bool Synchronise();

    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    virtual uint32 GetCurrentStateBuffer();

    virtual uint32 GetNumberOfStatefulMemoryBuffers();

    virtual void PrepareInputOffsets();

    virtual bool GetInputOffset(const uint32 signalIdx,
                                const uint32 numberOfSamples,
                                              uint32 &offset);

    virtual bool TerminateOutputCopy(const uint32 signalIdx,
                                                  const uint32 offset,
                                                  const uint32 numberOfSamples);

    virtual ErrorManagement::ErrorType Execute(ExecutionInfo & info);



protected:
    uint32 serverPort;

    TCPSocket server;

    TimeoutType acceptTimeout;

    TCPSocket client;

    SingleThreadService executor;

    uint8 bufferIdx;

    uint32 *signalIndices;

    FastPollingMutexSem mutex;

    uint32 nSignalsToReceive;

};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* HTTPDIODERECEIVER_H_ */

