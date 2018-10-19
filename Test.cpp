/**
 * @file Test.cpp
 * @brief Source file for class Test
 * @date 15 ott 2018
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

 * @details This source file contains the definition of all the methods for
 * the class Test (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
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
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

using namespace MARTe;

void LoginFunction(HttpClient &test, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "username=codac-dev-1&password=";
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void GetUsers(HttpClient &test, StreamString &token, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void GetTransformationInfo(HttpClient &test, StreamString &token,
		StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&pageName=FALCON";
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void GetPage(HttpClient &test, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());
	param =
			"text/html,application/html+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8";
	protocol->Write("Accept", param.Buffer());

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCGet, NULL, 2000u);
}

void GetVariablesInfo(HttpClient &test, StreamString &token,
		StreamString &response, StreamString &variables) {
	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	protocol->Delete("Accept");
	StreamString body = "token=";
	body += token;
	body += "&pageName=FALCON";
	body += "&variables=";
	StreamString varEncoded;
	HttpDefinition::HttpEncode(varEncoded, variables.Buffer());
	varEncoded.Seek(0);
	body += varEncoded;
	body.Seek(0);
	printf("%s\n", body.Buffer());

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);
}

void GetScheduleFolders(HttpClient &test, StreamString &token,
		StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&pageName=FALCON";
	body += "&username=codac-dev-1";
	body += "&parentFolders=%5B%5D";
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void GetSchedules(HttpClient &test, StreamString &token,
		StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&pageName=FALCON";
	body += "&username=codac-dev-1";
	body += "&parentFolders=%5B%5D";
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void GetSchedulesVariablesValue(HttpClient &test, StreamString &token,
		StreamString scheduleUID, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&scheduleUID=";
	StreamString varEncoded;
	HttpDefinition::HttpEncode(varEncoded, scheduleUID.Buffer());
	printf("%s\n", varEncoded.Buffer());
	body += varEncoded.Buffer();
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void UpdateSchedule(HttpClient &test, StreamString &token,
		StreamString scheduleUID, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&tid=11290_139686796103072";
	body += "&username=codac-dev-1";
	body += "&scheduleUID=";
	StreamString varEncoded1;
	HttpDefinition::HttpEncode(varEncoded1, scheduleUID.Buffer());
	printf("%s\n", varEncoded1.Buffer());
	body += varEncoded1;
	body += "&variables=";
	const char8 *varToEncode =
			"{\"FALCON@FALCON\":[[1,1,3,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0],[0,0,0],[0,0,0]]}";
	StreamString varEncoded;
	HttpDefinition::HttpEncode(varEncoded, varToEncode);

	//body += varEncoded.Buffer();
	body += varEncoded;

	body.Seek(0);
	printf("%s |%lld|\n", body.Buffer(), body.Size());
	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void Commit(HttpClient &test, StreamString &token, StreamString scheduleUID,
		StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&tid=11290_139686796103072";
	body += "&username=codac-dev-1";
	body += "&scheduleUID=";
	StreamString varEncoded1;
	HttpDefinition::HttpEncode(varEncoded1, scheduleUID.Buffer());
	printf("%s\n", varEncoded1.Buffer());
	body += varEncoded1;
	body += "&variables=";
	const char8 *varToEncode =
			"{\"FALCON@FALCON\":[[1,1,3,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0],[0,0,0],[0,0,0]]}";
	StreamString varEncoded;
	HttpDefinition::HttpEncode(varEncoded, varToEncode);

	//body += varEncoded.Buffer();
	body += varEncoded;

	body.Seek(0);
	printf("%s |%lld|\n", body.Buffer(), body.Size());
	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}

void NewSchedule(HttpClient &test, StreamString &token,
		StreamString scheduleUID, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&name=pippopluto";
	body += "&description=pippopluto schedule";
	body += "&parentFolders=%5B%5D";
	body += "&pageName=FALCON";
	body += "&username=codac-dev-1";
	body += "&inheritFromSchedule=false";

	body.Seek(0);
	printf("%s |%lld|\n", body.Buffer(), body.Size());
	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}


void LoadPlant(HttpClient &test, StreamString &token,
		StreamString scheduleUID, StreamString &response) {

	HttpProtocol *protocol = test.GetHttpProtocol();
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	StreamString param = "localhost:8080";
	protocol->Write("Host", param.Buffer());
	param = "keep-alive";
	protocol->Write("Connection", param.Buffer());
	param = "application/x-www-form-urlencoded; charset=UTF-8";
	protocol->Write("Content-Type", param.Buffer());
	param = "gzip, deflate, br";
	protocol->Write("Accept-Encoding", param.Buffer());
	param = "no-cache";
	protocol->Write("Cache-Control", param.Buffer());

	StreamString body = "token=";
	body += token;
	body += "&name=pippopluto";
	body += "&description=pippopluto schedule";
	body += "&parentFolders=%5B%5D";
	body += "&pageNames=";
	StreamString varEncoded;
	HttpDefinition::HttpEncode(varEncoded, "[\"FALCON\"]");
	body += varEncoded;


	body.Seek(0);
	printf("%s |%lld|\n", body.Buffer(), body.Size());
	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);

}


int main(int argc, char **argv) {

	HttpClient test;

	test.SetServerAddress("127.0.0.1");
	test.SetServerPort(8080);
	test.SetServerUri("/login");
	StreamString readOut;

	//Step 1: login
	LoginFunction(test, readOut);
	printf("login response=%s\n", readOut.Buffer());

	StreamString reply = "\"reply\": ";
	reply += readOut;
	reply.Seek(0);
	ConfigurationDatabase data;
	JsonParser jsonparser(reply, data);

	jsonparser.Parse();

	StreamString token;
	data.MoveAbsolute("reply");
	data.Read("token", token);
	printf("token=%s\n", token.Buffer());

	//step 2: get users (really needed?)
	test.SetServerUri("/getusers");
	readOut.SetSize(0);

	GetUsers(test, token, readOut);

	printf("getusers response=%s\n", readOut.Buffer());

	//step 3 stream??

	//step 4 gettransformationinfo
	test.SetServerUri("/gettransformationsinfo");

	readOut.SetSize(0);
	GetTransformationInfo(test, token, readOut);
	printf("GetTransformationInfo response=%s\n", readOut.Buffer());

	//step 5 get the page
	test.SetServerUri("/pages/FALCON.html?1539601761805");

	readOut.SetSize(0);
	GetPage(test, readOut);
	printf("GetPage response=%s\n", readOut.Buffer());

	//step 6 get variable info
	StreamString varList;
	StreamString variables = "[";

	const char8* buff = readOut.Buffer();
	bool first = true;
	uint32 i = 0u;
	while (buff != NULL && i < 14000) {
		buff = StringHelper::SearchString(buff, "id=");
		if (buff != NULL) {
			buff += 3;

			//char8 sep[2] = { '\0', '\0' };
			if (buff[0] == '\'' || buff[0] == '"') {
				//sep[0] = buff[0];
				buff++;

				char8 term;
				StreamString idx = buff;
				idx.Seek(0);
				StreamString variable;
				idx.GetToken(variable, "\"'", term);
				printf("var=%s\n", variable.Buffer());
				if (StringHelper::SearchChar(variable.Buffer(), '@') != NULL) {
					if (!first) {
						variables += ",";
					}
					variables += "\"";
					variables += variable;
					variables += "\"";
					first = false;
					i++;
				}
			}
		}
	}
	variables += "]";
	printf("var=%s\n", variables.Buffer());

	test.SetServerUri("/getvariablesinfo");

	readOut.SetSize(0);
	GetVariablesInfo(test, token, readOut, variables);

	printf("GetVariablesInfo response=%s\n", readOut.Buffer());

	//step 7 get schedulefolders
	test.SetServerUri("/getschedulefolders");

	readOut.SetSize(0);
	GetScheduleFolders(test, token, readOut);

	printf("GetScheduleFolders response=%s\n", readOut.Buffer());

	//step 8 get schedules
	test.SetServerUri("/getschedules");

	readOut.SetSize(0);
	GetSchedules(test, token, readOut);

	printf("GetSchedules response=%s\n", readOut.Buffer());
	StreamString scheduleUID;

	const char8*beg = StringHelper::SearchString(readOut.Buffer(), "pageName");
	beg++;
	beg = StringHelper::SearchString(beg, "pageName");

	if (beg != NULL) {
		const char8* toSearch = "\"uid\": ";
		const char8*ptr = StringHelper::SearchString(beg, toSearch);
		if (ptr != NULL) {
			StreamString scheduleUIDtot = ptr + StringHelper::Length(toSearch)
					+ 1;
			char8 term;
			scheduleUIDtot.Seek(0);
			scheduleUIDtot.GetToken(scheduleUID, "\" \\", term);
		}
		printf("%s\n", scheduleUID.Buffer());
	}
	//step 10 get schedule variables
	test.SetServerUri("/getschedulevariablesvalues");

	readOut.SetSize(0);
	GetSchedulesVariablesValue(test, token, scheduleUID, readOut);

	printf("GetSchedulesVariableValues response=%s\n", readOut.Buffer());

	//step 9 post an update
	test.SetServerUri("/updateschedule");

	readOut.SetSize(0);
	UpdateSchedule(test, token, scheduleUID, readOut);

	printf("UpdateSchedule response=%s\n", readOut.Buffer());

	//step 10 commit
	test.SetServerUri("/commitschedule");

	readOut.SetSize(0);
	Commit(test, token, scheduleUID, readOut);

	printf("CommitSchedule response=%s\n", readOut.Buffer());

	//step 11 create a new schedule
	test.SetServerUri("/createschedule");

	readOut.SetSize(0);
	NewSchedule(test, token, scheduleUID, readOut);

	printf("CreateSchedule response=%s\n", readOut.Buffer());

	//step 12 Load into plant
	test.SetServerUri("/loadintoplant");

	readOut.SetSize(0);
	LoadPlant(test, token, scheduleUID, readOut);

	printf("LoadPlant response=%s\n", readOut.Buffer());

	return 1;
}

