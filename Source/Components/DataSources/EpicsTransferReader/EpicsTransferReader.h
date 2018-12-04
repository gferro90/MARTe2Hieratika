/**
 * @file EpicsTransferReader.h
 * @brief Header file for class EpicsTransferReader
 * @date 21 nov 2018
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

 * @details This header file contains the declaration of the class EpicsTransferReader
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef EPICSTRANSFERREADER_H_
#define EPICSTRANSFERREADER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "TransferDataSource.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

class EpicsTransferReader: public TransferDataSource, public EmbeddedServiceMethodBinderI {
public:

    EpicsTransferReader();

    virtual ~EpicsTransferReader();

    virtual bool Synchronise();

    virtual uint32 GetNumberOfStatefulMemoryBuffers();

    virtual const char8 *GetBrokerName(StructuredDataI &data, const SignalDirection direction);

    virtual uint32 GetCurrentStateBuffer();


    virtual bool SetConfiguredDatabase(StructuredDataI & data);

protected:

    uint32 cpuMask;

    uint32 stackSize;

    SingleThreadService executor;

    FastPollingMutexSem mutex;

    chanId* chids;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* EPICSTRANSFERREADER_H_ */

