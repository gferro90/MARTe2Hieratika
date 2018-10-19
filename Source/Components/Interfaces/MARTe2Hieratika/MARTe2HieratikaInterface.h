/**
 * @file MARTe2HieratikaInterface.h
 * @brief Header file for class MARTe2HieratikaInterface
 * @date 19 ott 2018
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

 * @details This header file contains the declaration of the class MARTe2HieratikaInterface
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef MARTE2HIERATIKAINTERFACE_H_
#define MARTE2HIERATIKAINTERFACE_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "ClassRegistryDatabase.h"
#include "ClassRegistryItem.h"
#include "ConfigurationDatabase.h"
#include "ErrorManagement.h"
#include "File.h"
#include "GlobalObjectsDatabase.h"
#include "Object.h"
#include "ObjectRegistryDatabase.h"
#include "ProcessorType.h"
#include "RealTimeApplication.h"
#include "Reference.h"
#include "ReferenceT.h"
#include "StreamString.h"
#include "StandardParser.h"
#include "HttpClient.h"
#include "HttpDefinition.h"
#include "JsonParser.h"
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

class MARTe2HieratikaInterface {
public:
	MARTe2HieratikaInterface();
	virtual ~MARTe2HieratikaInterface();

	bool LoginFunction(HttpClient &test, StreamString &response);

	virtual bool GetUsers(HttpClient &test, StreamString &token,
			StreamString &response);

	virtual bool GetTransformationInfo(HttpClient &test, StreamString &token,
			StreamString &response);

	virtual bool GetPage(HttpClient &test, StreamString &response);

	virtual bool GetVariablesInfo(HttpClient &test, StreamString &token,
			StreamString &response, StreamString &variables);
	virtual bool GetScheduleFolders(HttpClient &test, StreamString &token,
			StreamString &response);

	virtual bool GetSchedules(HttpClient &test, StreamString &token,
			StreamString &response);

	virtual bool GetSchedulesVariablesValue(HttpClient &test, StreamString &token,
			StreamString scheduleUID, StreamString &response);

	virtual bool UpdateSchedule(HttpClient &test, StreamString &token,
			StreamString scheduleUID, StreamString &response);

	virtual bool Commit(HttpClient &test, StreamString &token, StreamString scheduleUID,
			StreamString &response);

	virtual bool NewSchedule(HttpClient &test, StreamString &token,
			StreamString scheduleUID, StreamString &response);

	virtual bool LoadPlant(HttpClient &test, StreamString &token,
			StreamString scheduleUID, StreamString &response);

};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* MARTE2HIERATIKAINTERFACE_H_ */

