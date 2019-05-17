/**
 * @file FileMonitorDataSource.h
 * @brief Header file for class FileMonitorDataSource
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

 * @details This header file contains the declaration of the class FileMonitorDataSource
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef FILEMONITORDATASOURCE_H_
#define FILEMONITORDATASOURCE_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MemoryDataSourceI.h"
#include "TimeStamp.h"
#include "Directory.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

class FileMonitorDataSource: public MemoryDataSourceI {
public:
    CLASS_REGISTER_DECLARATION()


    FileMonitorDataSource();

    virtual ~FileMonitorDataSource();

    virtual bool Initialise(StructuredDataI & data);

    virtual bool Synchronise();

    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    virtual void PrepareInputOffsets();

    virtual bool GetInputOffset(const uint32 signalIdx, const uint32 numberOfSamples, uint32 &offset);

    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    virtual const char8 *GetBrokerName(StructuredDataI &data, const SignalDirection direction);

protected:

    StreamString filePath;

    uint32 *lineNumber;
    uint32 *signalIndex;
    uint32 *signalNameLen;

    Directory fileMonitor;
    TimeStamp lastTimeStamp;
};

}
/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* FILEMONITORDATASOURCE_H_ */

