/**
 * @file MARTe2HieratikaInterface.cpp
 * @brief Source file for class MARTe2HieratikaInterface
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

 * @details This source file contains the definition of all the methods for
 * the class MARTe2HieratikaInterface (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "MARTe2HieratikaInterface.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

MARTe2HieratikaInterface::MARTe2HieratikaInterface() {
	// Auto-generated constructor stub for MARTe2HieratikaInterface
	// TODO Verify if manual additions are needed
}

MARTe2HieratikaInterface::~MARTe2HieratikaInterface() {
	// Auto-generated destructor stub for MARTe2HieratikaInterface
	// TODO Verify if manual additions are needed
}

bool MARTe2HieratikaInterface::LoginFunction(HttpClient &test,
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

	StreamString body = "username=codac-dev-1&password=";
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);
	return true;

}

bool MARTe2HieratikaInterface::GetUsers(HttpClient &test, StreamString &token,
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
	body.Seek(0);

	protocol->MoveToRoot();
	test.HttpExchange(response, HttpDefinition::HSHCPost, &body, 2000u);
	return true;

}

bool MARTe2HieratikaInterface::GetTransformationInfo(HttpClient &test,
		StreamString &token, StreamString &response) {

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
	return true;

}

bool MARTe2HieratikaInterface::GetPage(HttpClient &test,
		StreamString &response) {

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
	return true;
}

bool MARTe2HieratikaInterface::GetVariablesInfo(HttpClient &test,
		StreamString &token, StreamString &response, StreamString &variables) {
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
	return true;
}

bool MARTe2HieratikaInterface::GetScheduleFolders(HttpClient &test,
		StreamString &token, StreamString &response) {

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
	return true;
}

bool MARTe2HieratikaInterface::GetSchedules(HttpClient &test,
		StreamString &token, StreamString &response) {

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
	return true;
}

bool MARTe2HieratikaInterface::GetSchedulesVariablesValue(HttpClient &test,
		StreamString &token, StreamString scheduleUID, StreamString &response) {

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
	return true;
}

bool MARTe2HieratikaInterface::UpdateSchedule(HttpClient &test,
		StreamString &token, StreamString scheduleUID, StreamString &response) {

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
	return true;
}

bool MARTe2HieratikaInterface::Commit(HttpClient &test, StreamString &token,
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
	return true;
}

bool MARTe2HieratikaInterface::NewSchedule(HttpClient &test,
		StreamString &token, StreamString scheduleUID, StreamString &response) {

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
	return true;
}

bool MARTe2HieratikaInterface::LoadPlant(HttpClient &test, StreamString &token,
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
	return true;
}

}
