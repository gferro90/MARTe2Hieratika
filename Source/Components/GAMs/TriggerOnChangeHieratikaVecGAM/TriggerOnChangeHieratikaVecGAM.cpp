/**
 * @file TriggerOnChangeHieratikaVecGAM.cpp
 * @brief Source file for class TriggerOnChangeHieratikaVecGAM
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

 * @details This source file contains the definition of all the methods for
 * the class TriggerOnChangeHieratikaVecGAM (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "TriggerOnChangeHieratikaVecGAM.h"
#include "AdvancedErrorManagement.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

TriggerOnChangeHieratikaVecGAM::TriggerOnChangeHieratikaVecGAM() :
        TriggerOnChangeHieratikaGAM() {
    splitted = NULL;

}

TriggerOnChangeHieratikaVecGAM::~TriggerOnChangeHieratikaVecGAM() {
    if (splitted != NULL) {
        delete[] splitted;
    }

    if (signalIndex != NULL) {
        delete[] signalIndex;
    }
}

bool TriggerOnChangeHieratikaVecGAM::Initialise(StructuredDataI & data) {
    bool ret = TriggerOnChangeHieratikaGAM::Initialise(data);
    if (ret) {
        ret = data.Read("VariableName", varName);
    }
    return ret;
}

bool TriggerOnChangeHieratikaVecGAM::Setup() {
    bool ret = TriggerOnChangeHieratikaGAM::Setup();
    if (ret) {
        ReferenceT < EventConditionTrigger > event = events->Get(0u);
        ret = event.IsValid();

        if (ret) {
            ReferenceT < Message > varMess = this->Find("GetVariableMessage");
            //get the tid
            ReferenceT < ConfigurationDatabase > varPayload = varMess->Find("Payload");
            varPayload->Write("Token", token.Buffer());
            StreamString varInCdb = "[\"";
            varInCdb += varName;
            varInCdb += "\"]";
            varPayload->Write("Variables", varInCdb.Buffer());
            replyStream->SetSize(0ull);
            eventSem->Reset();

            event->SendMessage(varMess, this);
            eventSem->Wait();

            StreamString varInfo = *(replyStream.operator->());
            const char8 *pattern = "\"value\": ";
            const char8 *valPtr = StringHelper::SearchString(varInfo.Buffer(), pattern);
            ret = (valPtr != NULL);
            if (ret) {
                StreamString toVal = (valPtr + StringHelper::Length(pattern));
                toVal.Seek(0ull);
                char8 term;
                toVal.GetToken(varValue, "]", term, "");
                varValue += "]";
                toVal.GetToken(varValue, "]", term, "");
                varValue += "]";
                toVal.GetToken(varValue, "]", term, "");
                varValue += "]";
                printf("varValue=%s %lld\n", varValue.Buffer(), varValue.Size());

                //save the index of our signals
                varValue.Seek(0ull);

                splitted = new StreamString[numberOfFields + 1u];
                signalIndex = new uint32[numberOfFields];

                varValue.GetToken(splitted[0], "[", term, "");
                splitted[0] += "[";
                varValue.GetToken(splitted[0], "[", term, "");
                splitted[0] += "[";
                varValue.GetToken(splitted[0], "[", term, "");
                splitted[0] += "[";

                StreamString values = varValue.Buffer() + varValue.Position();
                values.Seek(0ull);
                varValue.Seek(0ull);

                term = '\"';
                uint32 n = 0u;
                uint32 cnt = 0u;
                while (term != '\0') {
                    StreamString token;
                    varValue.GetToken(token, "\"", term, "");
                    token.SetSize(0ull);
                    varValue.GetToken(token, "\"", term, "");
                    StreamString value;
                    char8 term2;
                    values.GetToken(value, ",", term2, "");
                    bool found = false;
                    for (uint32 i = 0u; (i < numberOfFields) && (!found); i++) {
                        StreamString signalName;
                        GetSignalName(InputSignals, i, signalName);
                        found = (token == signalName);
                        if (found) {
                            signalIndex[cnt] = i;
                            printf("split[%d]=%s\n", cnt, splitted[cnt].Buffer());
                            cnt++;
                            if (cnt == numberOfFields) {
                                if (term2 == '\0') {
                                    splitted[cnt] = "]]";
                                }
                                else {
                                    splitted[cnt] = values.Buffer() + values.Position();
                                }
                                printf("split[%d]=%s\n", cnt, splitted[cnt].Buffer());
                                term = '\0';
                            }
                        }
                    }
                    if (!found) {
                        splitted[cnt] += value;
                        splitted[cnt] += ",";
                    }
                    n++;
                }

            }
        }
    }
    return ret;
}

bool TriggerOnChangeHieratikaVecGAM::Execute() {
    bool ret = true;
    if (MemoryOperationsHelper::Compare(&previousValue[0], &currentValue[0], totalSize) != 0) {

        //update the cdb
        splitted[0].Seek(0ull);
        StreamString variables = "{\"";
        variables += varName;
        variables += "\": ";
        variables += splitted[0];

        for (uint32 i = 0u; (i < numberOfFields) && (ret); i++) {
            variables += "\"";
            variables.Printf("%!", signalValues[signalIndex[i]]);
            variables += "\\r\"";
            splitted[i + 1u].Seek(0ull);
            if (splitted[i + 1u] != "]]") {
                variables += ",";
            }
            variables += splitted[i + 1u];
        }
        variables += "}";
        variables.Seek(0ull);
        printf("Variables=%s\n", variables.Buffer());

        //for each commit event message update the variables field
        uint32 numberOfMessagesToUpdate = messagesToUpdate.Size();
        for (uint32 i = 0u; (i < numberOfMessagesToUpdate) && (ret); i++) {
            ReferenceT < ConfigurationDatabase > payload = messagesToUpdate.Get(i);
            ret = payload.IsValid();
            if (ret) {
                ret = payload->Delete("Variables");
                if (ret) {
                    ret = payload->Write("Variables", variables);
                }
            }
        }
    }

    if (ret) {
        ret = TriggerOnChangeGAM::Execute();
    }
    return ret;

}
CLASS_REGISTER(TriggerOnChangeHieratikaVecGAM, "1.0")
}
