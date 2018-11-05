/**
 * @file FileMonitorDataSource.cpp
 * @brief Source file for class FileMonitorDataSource
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
 * the class FileMonitorDataSource (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "AdvancedErrorManagement.h"
#include "FileMonitorDataSource.h"
#include "File.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

FileMonitorDataSource::FileMonitorDataSource() :
        MemoryDataSourceI(),
        fileMonitor() {
    lineNumber = NULL;
    signalIndex = NULL;
    signalNameLen = NULL;

}

FileMonitorDataSource::~FileMonitorDataSource() {
    if (lineNumber != NULL) {
        delete[] lineNumber;
    }
    if (signalIndex != NULL) {
        delete[] signalIndex;
    }
    if (signalNameLen != NULL) {
        delete[] signalNameLen;
    }
}

bool FileMonitorDataSource::Initialise(StructuredDataI & data) {
    bool ret = MemoryDataSourceI::Initialise(data);
    if (ret) {

        bool ret = data.Read("FilePath", filePath);
        if (ret) {
            fileMonitor.SetByName(filePath.Buffer());
            lastTimeStamp = fileMonitor.GetLastWriteTime();
        }
        else {
            REPORT_ERROR(ErrorManagement::FatalError, "Please define FilePath parameter");
        }
    }
    return ret;
}

bool FileMonitorDataSource::SetConfiguredDatabase(StructuredDataI & data) {
    bool ret = MemoryDataSourceI::SetConfiguredDatabase(data);
    if (ret) {
        File file;
        ret = file.Open(filePath.Buffer(), BasicFile::ACCESS_MODE_R);
        if (ret) {
            uint32 numberOfSignals = GetNumberOfSignals();

            lineNumber = new uint32[numberOfSignals];
            signalIndex = new uint32[numberOfSignals];
            signalNameLen = new uint32[numberOfSignals];
            uint32 cnt = 0u;
            uint32 lineCnt = 0u;
            //get line
            StreamString line;
            file.Seek(0);
            while (file.GetLine(line)) {
                //no comments in the line
                if (StringHelper::SearchChar(line.Buffer(), '#') == NULL) {
                    char8 term;
                    line.Seek(0ull);
                    StreamString varName;
                    line.GetToken(varName, "=", term);

                    bool found = false;
                    for (uint32 i = 0u; (i < numberOfSignals) && (!found); i++) {
                        StreamString signalName;
                        GetSignalName(i, signalName);

                        found = (signalName == varName);
                        if (found) {
                            if (cnt < numberOfSignals) {
                                signalNameLen[cnt] = (signalName.Size() + 1u);
                                lineNumber[cnt] = lineCnt;
                                signalIndex[cnt] = i;
                            }
                            uint32 x=signalIndex[cnt];
                            REPORT_ERROR(ErrorManagement::Information,"Found %s %d",signalName.Buffer(), x);
                            cnt++;
                        }
                    }
                }
                line.SetSize(0ull);
                lineCnt++;
            }
            ret = (cnt == numberOfSignals);
            if (!ret) {
                REPORT_ERROR(ErrorManagement::FatalError, "There are variable repetitions in the file!");
            }
        }
        else{
            REPORT_ERROR(ErrorManagement::FatalError, "Failed to open the file %s",filePath.Buffer());
        }

        if (ret) {
            file.Close();
        }
    }
    return ret;
}

bool FileMonitorDataSource::Synchronise() {
    return true;
}

void FileMonitorDataSource::PrepareInputOffsets() {
    //monitor the last change date
    REPORT_ERROR(ErrorManagement::FatalError, "Called PrepareInputOffsets");

    TimeStamp timeNow = fileMonitor.GetLastWriteTime();
    if (MemoryOperationsHelper::Compare(&timeNow, &lastTimeStamp, sizeof(TimeStamp)) != 0) {
        File file;
        if (file.Open(filePath.Buffer(), BasicFile::ACCESS_MODE_R)) {
            uint32 cnt = 0u;
            uint32 n = 0u;
            StreamString line;
            file.Seek(0);
            while (file.GetLine(line)) {
                if (cnt == lineNumber[n]) {
                    void* signalMem = NULL;
                    if (GetSignalMemoryBuffer(signalIndex[n], 0u, signalMem)) {
                        //need the signal type
                        TypeDescriptor td = GetSignalType(signalIndex[n]);
                        AnyType at(td, 0u, signalMem);
                        StreamString signalValue = &(line.Buffer()[signalNameLen[n]]);
                        REPORT_ERROR(ErrorManagement::FatalError, "SignalValue=%s %d", signalValue.Buffer(), signalIndex[n]);
                        TypeConvert(at, signalValue);
                    }
                    n++;
                }
                line.SetSize(0ull);
                cnt++;
            }
            MemoryOperationsHelper::Copy(&lastTimeStamp, &timeNow, sizeof(TimeStamp));
            file.Close();
        }
    }
}

bool FileMonitorDataSource::PrepareNextState(const char8 * const currentStateName,
                                             const char8 * const nextStateName) {
    return true;
}

bool FileMonitorDataSource::GetInputOffset(const uint32 signalIdx, const uint32 numberOfSamples, uint32 &offset) {
    return true;
}


CLASS_REGISTER(FileMonitorDataSource, "1.0")

}
