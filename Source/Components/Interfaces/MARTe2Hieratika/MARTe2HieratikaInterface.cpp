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
//#define NULL_PTR(x) NULL
MARTe2HieratikaInterface::MARTe2HieratikaInterface() {
	// Auto-generated constructor stub for MARTe2HieratikaInterface
	// TODO Verify if manual additions are needed

	client.SetServerAddress("127.0.0.1");
	client.SetServerPort(8080);
	protocol = client.GetHttpProtocol();
	timeout = 2000u;

}

MARTe2HieratikaInterface::~MARTe2HieratikaInterface() {
	// Auto-generated destructor stub for MARTe2HieratikaInterface
	// TODO Verify if manual additions are needed
}

void MARTe2HieratikaInterface::SetServerAddress(const char8 *ipAddress) {
	client.SetServerAddress(ipAddress);
}

void MARTe2HieratikaInterface::SetServerPort(uint32 port) {
	client.SetServerPort(port);
}

void MARTe2HieratikaInterface::SettHttpExchangeTimeout(TimeoutType &timeoutIn) {
	timeout = timeoutIn;
}

bool MARTe2HieratikaInterface::SetHeader() {

	StreamString param;
	StreamString ipAddress;
	client.GetServerAddress(ipAddress);
	bool ret = param.Printf("%s:%d", ipAddress, client.GetServerPort());
	if (ret) {
		ret = protocol->Write("Host", param.Buffer());
	}
	if (ret) {
		param = "keep-alive";
		ret = protocol->Write("Connection", param.Buffer());
	}
	if (ret) {
		param = "application/x-www-form-urlencoded; charset=UTF-8";
		ret = protocol->Write("Content-Type", param.Buffer());
	}
	if (ret) {
		param = "gzip, deflate, br";
		ret = protocol->Write("Accept-Encoding", param.Buffer());
	}
	if (ret) {
		param = "no-cache";
		ret = protocol->Write("Cache-Control", param.Buffer());
	}
	return ret;
}

bool MARTe2HieratikaInterface::LoginFunction(const char8 *userName,
		const char8 * passw, BufferedStreamI &response) {
	client.SetServerUri("/login");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {
		ret = SetHeader();
		StreamString body = "username=";
		if (ret) {
			body += userName;
			body += "&password=";
			body += passw;
			ret = body.Seek(0ULL);
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::GetUsers(const char8 *token,
		BufferedStreamI &response) {
	client.SetServerUri("/getusers");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}
	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";
		if (ret) {
			body += token;
			body.Seek(0ULL);
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;

}

bool MARTe2HieratikaInterface::GetTransformationInfo(const char8 *pageName,
		const char8 *token, BufferedStreamI &response) {

	client.SetServerUri("/gettransformationsinfo");
	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&pageName=";
			body += pageName;
			ret = body.Seek(0ULL);
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;

}


bool MARTe2HieratikaInterface::GetPages(const char8 *token, BufferedStreamI &response){
	client.SetServerUri("/getpages");
	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";
		if (ret) {
			body += token;
			ret = body.Seek(0ULL);
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}


bool MARTe2HieratikaInterface::GetPage(const char8 *pageName,
		BufferedStreamI &response) {
	StreamString uri = "/pages/";
	uri += pageName;
	uri += ".html";
	client.SetServerUri(uri.Buffer());
	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}
	if (ret) {
		StreamString param;
		StreamString ipAddress;
		client.GetServerAddress(ipAddress);
		ret = param.Printf("%s:%d", ipAddress, client.GetServerPort());
		if (ret) {
			ret = protocol->Write("Host", param.Buffer());
		}
		if (ret) {
			param = "keep-alive";
			ret = protocol->Write("Connection", param.Buffer());
		}
		if (ret) {
			param = "gzip, deflate, br";
			ret = protocol->Write("Accept-Encoding", param.Buffer());
		}
		if (ret) {
			param = "no-cache";
			ret = protocol->Write("Cache-Control", param.Buffer());
		}
		if (ret) {
			param =
					"text/html,application/html+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8";
			ret = protocol->Write("Accept", param.Buffer());
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCGet, NULL,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::ExtractAllVariablesFromPage(
		BufferedStreamI &page, BufferedStreamI &variables) {

	//step 6 get variable info
	uint32 size = 1u;
	bool ret = variables.Write("[", size);
	//StreamString variables = "[";

	//const char8* buff = page.Buffer();
	if (ret) {
		const uint32 buffSize = 1024u;
		const uint32 halfSize = (buffSize >> 1u);
		char8 buffer[1024];
		uint32 sizeToRead = halfSize;
		bool first = true;
		ret = MemoryOperationsHelper::Set(buffer, '\0', buffSize);
		if (ret) {
			uint32 offset = 0u;
			//double buffer style to avoid to cut
			while (sizeToRead == 512u) {
				if (offset > halfSize) {
					ret = MemoryOperationsHelper::Move(&buffer[0],
							&buffer[halfSize], halfSize);
					if (ret) {
						ret = MemoryOperationsHelper::Set(&buffer[halfSize],
								'\0', halfSize);
					}
					if (ret) {
						ret = page.Read(&buffer[halfSize], sizeToRead);
					}
					if (ret) {
						offset -= halfSize;
					}
				} else {
					ret = MemoryOperationsHelper::Set(&buffer[0], '\0',
							halfSize);
					if (ret) {
						ret = page.Read(&buffer[0], sizeToRead);
					}
				}

				if (ret) {
					const char8 *buff = (const char8 *) (&buffer[0]);
					while ((buff != NULL) && (offset < 512u)) {
						buff = StringHelper::SearchString(buff, "id=");
						if (buff != NULL) {
							offset = (buff - buffer);

							if (offset < 512u) {
								buff = &buff[3];
								//char8 sep[2] = { '\0', '\0' };
								if (buff[0] == '\'' || buff[0] == '"') {
									//sep[0] = buff[0];
									buff = &buff[1];
									char8 term;
									StreamString idx = buff;
									ret = idx.Seek(0ULL);
									StreamString variable;
									if (ret) {
										ret = idx.GetToken(variable, "\"'",
												term);
									}
									if (ret) {
										if (StringHelper::SearchChar(
												variable.Buffer(), '@')
												!= NULL) {
											if (!first) {
												variables.Write(",", size);
											}
											ret = variables.Write("\"", size);
											if (ret) {
												uint32 varSize =
														StringHelper::Length(
																variable.Buffer());
												ret = variables.Write(
														variable.Buffer(),
														varSize);
											}
											if (ret) {
												ret = variables.Write("\"",
														size);
											}
											first = false;
										}
									}
								}
							}
						}
					}
				}
			}
			if (ret) {
				uint32 finalSize = 2u;
				variables.Write("]", finalSize);
			}
		}
	}
	//printf("var=%s\n", variables.Buffer());
	return ret;

}

bool MARTe2HieratikaInterface::GetVariablesInfo(const char8 *pageName,
		const char8 *variables, const char8 *token, BufferedStreamI &response) {

	client.SetServerUri("/getvariablesinfo");
	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}
	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";
		StreamString varEncoded;
		if (ret) {
			body += token;
			body += "&pageName=FALCON";
			body += "&variables=";
			ret = HttpDefinition::HttpEncode(varEncoded, variables);
		}
		if (ret) {
			ret = varEncoded.Seek(0ULL);
		}
		if (ret) {
			body += varEncoded;
			ret = body.Seek(0ULL);
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::GetScheduleFolders(const char8 *pageName,
		const char8* userName, const char8 *token, BufferedStreamI &response) {
	client.SetServerUri("/getschedulefolders");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}
	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&pageName=";
			body += pageName;
			body += "&username=";
			body += userName;
			body += "&parentFolders=%5B%5D";
			ret = body.Seek(0ULL);
		}

		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::GetSchedules(const char8 *pageName,
		const char8 *userName, const char8 *token, BufferedStreamI &response) {
	client.SetServerUri("/getschedules");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}
	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&pageName=";
			body += pageName;
			body += "&username=";
			body += userName;
			body += "&parentFolders=%5B%5D";
			ret = body.Seek(0ULL);
		}

		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::GetSchedulesVariablesValue(const char8 *token,
		const char8 *scheduleUID, BufferedStreamI &response) {
	client.SetServerUri("/getschedulevariablesvalues");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {
		ret = SetHeader();
		StreamString body = "token=";

		if (ret) {
			body += token;
			body += "&scheduleUID=";
			StreamString varEncoded;
			HttpDefinition::HttpEncode(varEncoded, scheduleUID);
			body += varEncoded.Buffer();
			ret = body.Seek(0ULL);
		}
		if (ret) {

			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}

	}
	return ret;
}

bool MARTe2HieratikaInterface::UpdateSchedule(const char8 *userName,
		const char8* variables, const char8 *token, const char8 * scheduleUID,
		BufferedStreamI &response) {
	client.SetServerUri("/updateschedule");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {
		ret = SetHeader();

		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&tid=11290_139686796103072";
			body += "&username=";
			body += userName;
			body += "&scheduleUID=";
			StreamString varEncoded1;
			HttpDefinition::HttpEncode(varEncoded1, scheduleUID);
			printf("%s\n", varEncoded1.Buffer());
			body += varEncoded1;
			body += "&variables=";

			StreamString varEncoded;
			ret = HttpDefinition::HttpEncode(varEncoded, variables);
			if (ret) {
				//body += varEncoded.Buffer();
				body += varEncoded;
				ret = body.Seek(0);
			}
		}

		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::Commit(const char8 *userName,
		const char8 *variables, const char8 *token, const char8 *scheduleUID,
		BufferedStreamI &response) {

	client.SetServerUri("/commitschedule");
	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {
		ret = SetHeader();

		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&tid=11290_139686796103072";
			body += "&username=";
			body += userName;
			body += "&scheduleUID=";
			StreamString varEncoded1;
			HttpDefinition::HttpEncode(varEncoded1, scheduleUID);
			printf("%s\n", varEncoded1.Buffer());
			body += varEncoded1;
			body += "&variables=";
			StreamString varEncoded;
			ret = HttpDefinition::HttpEncode(varEncoded, variables);

			if (ret) {
				//body += varEncoded.Buffer();
				body += varEncoded;
				ret = body.Seek(0ULL);
			}
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::NewSchedule(const char8 *scheduleName,
		const char8 * description, const char8 *pageName, const char8 *userName,
		const char8 *token, const char8 *scheduleUID,
		BufferedStreamI &response) {

	client.SetServerUri("/createschedule");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}
	if (ret) {
		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&name=";
			body += scheduleName;
			body += "&description=";
			body += description;
			body += "&parentFolders=%5B%5D";
			body += "&pageName=";
			body += pageName;
			body += "&username=";
			body += userName;
			body += "&inheritFromSchedule=false";
			ret = body.Seek(0ULL);
		}
		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

bool MARTe2HieratikaInterface::LoadPlant(const char8 *scheduleName,
		const char8 *userName, const char8 *description, const char8 *pageNames,
		const char8 *token, const char8 *scheduleUID,
		BufferedStreamI &response) {

	client.SetServerUri("/loadintoplant");

	bool ret = true;
	if (!protocol->MoveAbsolute("OutputOptions")) {
		ret = protocol->CreateAbsolute("OutputOptions");
	}

	if (ret) {

		StreamString body = "token=";
		if (ret) {
			body += token;
			body += "&name=";
			body += scheduleName;
			body += "&description=";
			body += description;
			body += "&parentFolders=%5B%5D";
			body += "&pageNames=";
			StreamString varEncoded;
			ret = HttpDefinition::HttpEncode(varEncoded, pageNames);
			if (ret) {
				body += varEncoded;
				ret = body.Seek(0ULL);
			}
		}

		if (ret) {
			ret = protocol->MoveToRoot();
		}
		if (ret) {
			ret = client.HttpExchange(response, HttpDefinition::HSHCPost, &body,
					timeout);
		}
	}
	return ret;
}

}
