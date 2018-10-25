/**
 * @file MARTe2HieratikaMessageDispatcher.h
 * @brief Header file for class MARTe2HieratikaMessageDispatcher
 * @date 24 ott 2018
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

 * @details This header file contains the declaration of the class MARTe2HieratikaMessageDispatcher
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef MARTE2HIERATIKAMESSAGEDISPATCHER_H_
#define MARTE2HIERATIKAMESSAGEDISPATCHER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MessageI.h"
#include "MARTe2HieratikaInterface.h"
#include "EmbeddedServiceMethodBinderI.h"
#include "QueueingMessageFilter.h"
#include "SingleThreadService.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

class MARTe2HieratikaMessageDispatcher: public Object, public MessageI, public MARTe2HieratikaInterface, public EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()

    MARTe2HieratikaMessageDispatcher();
    virtual ~MARTe2HieratikaMessageDispatcher();

    virtual bool Initialise(StructuredDataI &data);

    virtual ErrorManagement::ErrorType Execute(ExecutionInfo & info);


    void GetResponse(BufferedStreamI &output);

protected:

    bool SendReply(ReferenceT < BufferedStreamI > &stream, ReferenceT<Message> &message, ReferenceT<ConfigurationDatabase> &payload);
    void SetResponseStream(ReferenceT < BufferedStreamI > &stream, ReferenceT<Message> &message, ReferenceT<ConfigurationDatabase> &payload);

    ReferenceT<QueueingMessageFilter> filter;

    TimeoutType timeout;

    BufferedStreamI *response;

    StreamString internalResponse;
    /**
     * The internal thread executor.
     */
    SingleThreadService executor;

    /**
     * The affinity of the SingleThreadService.
     */
    ProcessorType cpuMask;

};

}
/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* MARTE2HIERATIKAMESSAGEDISPATCHER_H_ */

