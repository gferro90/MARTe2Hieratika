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

namespace MARTe {

/**
 * @brief Receives the values of N signals from a HTTP message and sets their values.
 *
 * @details An independent thread receives HTTP messages in chunked transfer encoding more
 * and with the following Json payload (see HttpDiodeDataSource)
 * {"SignalName": {\n
 *    "Type": float32\n
 *    "Value": 1.5\n
 *  },\n
 *  ...\n
 *  }\n
 * The payload is parsed and the signals that matches the signal names received are updated with
 * the receives respective values.
 *
 * @details The last signal is a uint32 vector that contains the received signal indexes in teh current cycle.
 */
class HttpDiodeReceiver: public MemoryDataSourceI, public EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Constructor
     */
    HttpDiodeReceiver();

    /**
     * @brief Destructor
     */
    virtual ~HttpDiodeReceiver();

    /**
     * @see MemoryDataSourceI::Initialise
     * @details The user must define the following parameters in configuration:\n
     *   - ServerPort = 1234 //the server listening port
     * The user can optionally define the following parameters:\n
     *   - CPUMask = 0x1 //the cpu mask where the independent thread is allowed to be executed. Default 0xFF
     *   - AcceptTimeout = 1000 //timeout in milliseconds for the function that waits for client connections.
     *     Default: TTInfiniteWait
     */
    virtual bool Initialise(StructuredDataI &data);

    /**
     * @see MemoryDataSourceI::Synchronise
     * @return true
     */
    virtual bool Synchronise();

    /**
     * @see MemoryDataSourceI::SetConfiguredDatabase
     * @details Waits for the client connection on the specified port.
     */
    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    /**
     * @brief Starts the independent thread execution.
     */
    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    /**
     * @see DataSourceI::GetCurrentStateBuffer
     * @brief Return the buffer index where the broker can copy from.
     */
    virtual uint32 GetCurrentStateBuffer();

    /**
     * @see DataSourceI::GetNumberOfStatefulMemoryBuffers
     * @brief Returns 2.
     * @details This data source employs two internal buffers. One is where the independent
     * thread writes the value of the signals once it receives the HTTP message (HTTP buffer), the other is
     * the currently used by the brokers to transfer data to GAMs (broker buffer). Each PrepareInputOffsets call,
     * the memory is copied from the HTTP buffer to the brokers buffer and the buffers are switched.
     */
    virtual uint32 GetNumberOfStatefulMemoryBuffers();

    /**
     * @see EmbeddedServiceMethodBinderI::Execute
     * @brief The independent thread routine.
     * @details Waits continously for HTTP messages from the client, parses the chunked Json payload and updates
     * the signal values with the values received.
     */
    virtual ErrorManagement::ErrorType Execute(ExecutionInfo & info);

    virtual const char8 *GetBrokerName(StructuredDataI &data,
                                       const SignalDirection direction);

protected:

    /**
     * The server listening port
     */
    uint32 serverPort;

    /**
     * The server socket
     */
    TCPSocket server;

    /**
     * The wait for connection timeout
     */
    TimeoutType acceptTimeout;

    /**
     * The client used to communicate with the client
     */
    TCPSocket client;

    /**
     * The independent thread executor
     */
    SingleThreadService executor;

    /**
     * A pointer to the last signal which is
     * a vector containing the indexes of the signals
     * that arrives cycle by cycle
     */
    uint32 *signalIndices;

    /**
     * A mutex used for the synchronisation with
     * the independent thread
     */
    FastPollingMutexSem mutex;

    /**
     * The number of signals to receive in each HTTP message.
     */
    uint32 nSignalsToReceive;

    uint8 isIndependentThread;

    uint8 firstTime;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* HTTPDIODERECEIVER_H_ */

