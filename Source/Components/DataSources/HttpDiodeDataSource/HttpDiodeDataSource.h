/**
 * @file HttpDiodeDataSource.h
 * @brief Header file for class HttpDiodeDataSource
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

 * @details This header file contains the declaration of the class HttpDiodeDataSource
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef HTTPDIODEDATASOURCE_H_
#define HTTPDIODEDATASOURCE_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/
#include <cadef.h>

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

#include "MemoryDataSourceI.h"
#include "HttpChunkedStream.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

/**
 * @brief Each cycle it sends the N selected signals to the host in a HTTP message.
 *
 * @details The last declared signal belonging to this data source must be a vector of uint32 indexes containing
 * the indexes of the signals that must be enveloped in a HTTP message (using Json syntax) and sent to the host at
 * the current cycle.
 *
 * @details This data source can be accessed only by one GAM that provides all the signals in the same order of the
 * data source signals configuration (otherwise the indexes will be wrong) and, for each cycle, the vector containing the
 * indexes of the signals to be sent (see PriorityGAM).
 *
 * @details The HTTP message that will be sent at each cycle performs the chunked transfer encoding sending as a payload the
 * following Json code:\n
 * {"SignalName": {\n
 *    "Type": float32\n
 *    "Value": 1.5\n
 *  },\n
 *  ...\n
 *  }
 *
 */
class HttpDiodeDataSource: public MemoryDataSourceI {
public:

    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Constructor
     */
    HttpDiodeDataSource();

    /**
     * @brief Destructor
     */
    virtual ~HttpDiodeDataSource();

    /**
     * @see MemoryDataSourceI::Initialise
     * @details The user must define the following parameters in the configuration:\n
     *   - ServerIpAddress = "127.0.0.1" //the server host ip address\n
     *   - ServerPort = 1234 //the server host listening port\n
     * The user could optionally define the following parameters in the configuration:\n
     *   - ConnectionTimeout = 1000 //timeout in milliseconds for the connection. Default: TTInfiniteWait
     */
    virtual bool Initialise(StructuredDataI &data);

    /**
     * @see MemoryDataSourceI::Synchronise
     * @return true
     */
    virtual bool Synchronise();

    /**
     * @see MemoryDataSourceI::SetConfiguredDatabase
     * @details Connects to the host server and checks that only one GAM is
     * interacting with this data source.
     */
    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    /**
     * @brief Stores the pointer to the last signal being the vector containing the
     * indexes of the signals to be sent.
     */
    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    /**
     * @see DataSourceI::PrepareOutputOffsets
     * @brief Sends the HTTP header.
     * @details Being called before the broker operations, it sends the HTTP message header specifying the
     * chunked transfer encoding mode.
     */
    virtual void PrepareOutputOffsets();

    /**
     * @see DataSourceI::PrepareInputOffsets
     * @brief Returns true
     */
    virtual bool GetOutputOffset(const uint32 signalIdx,
                                 const uint32 numberOfSamples,
                                 uint32 &offset);

    /**
     * @see DataSourceI::TerminateOutputCopy
     * @details Sends the HTTP message payload that consists in a block written in
     * Json syntax containing the type and value of the signals to be sent.
     */
    virtual bool TerminateOutputCopy(const uint32 signalIdx,
                                     const uint32 offset,
                                     const uint32 numberOfSamples);

protected:

    /**
     * Counts the signals currently sent to the host
     */
    uint32 signalsSentCounter;

    /**
     * The socket stream to communicate
     * with the host server
     */
    HttpChunkedStream client;

    /**
     * A pointer to the last signals, namely
     * the vector containing the indexes of the
     * signals to be sent
     */
    uint32 *signalIndexList;

    /**
     * The number of signals to be sent
     */
    uint32 signalToSent;

    /**
     * The server host ip address
     */
    StreamString serverIpAddress;

    /**
     * The server listening port
     */
    uint32 serverPort;

    /**
     * The connection timeout
     */
    TimeoutType connectionTimeout;

    uint8 firstTime;

    char8 **timeStamps;

    chanId *chids;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_DATASOURCES_HTTPDIODEDATASOURCE_HTTPDIODEDATASOURCE_H_ */

