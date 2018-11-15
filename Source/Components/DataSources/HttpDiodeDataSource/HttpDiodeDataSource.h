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

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

#include "MemoryDataSourceI.h"
#include "HttpChunkedStream.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class HttpDiodeDataSource: public MemoryDataSourceI {
public:

    CLASS_REGISTER_DECLARATION()

    HttpDiodeDataSource();
    virtual ~HttpDiodeDataSource();

    virtual bool Initialise(StructuredDataI &data);

    virtual bool Synchronise();

    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);



    virtual void PrepareOutputOffsets();

    virtual bool GetOutputOffset(const uint32 signalIdx,
                                              const uint32 numberOfSamples,
                                              uint32 &offset);

    virtual bool TerminateOutputCopy(const uint32 signalIdx,
                                                  const uint32 offset,
                                                  const uint32 numberOfSamples);

protected:
    uint32 signalsSentCounter;

    HttpChunkedStream client;

    uint32 *signalIndexList;

    uint32 signalToSent;

    StreamString serverIpAddress;

    uint32 serverPort;

    TimeoutType connectionTimeout;

    uint8 firstTime;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_DATASOURCES_HTTPDIODEDATASOURCE_HTTPDIODEDATASOURCE_H_ */

