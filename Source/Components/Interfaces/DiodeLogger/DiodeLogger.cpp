/**
 * @file DiodeLogger.cpp
 * @brief Source file for class DiodeLogger
 * @date 02 ago 2020
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
 * the class DiodeLogger (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "DiodeLogger.h"
#include "AdvancedErrorManagement.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

DiodeLogger::DiodeLogger() {
    // Auto-generated constructor stub for DiodeLogger
    // TODO Verify if manual additions are needed

    numberOfSignals = 0u;
    windowSize = 0xFFFFFFFFu;
    printCycles = 1u;
    windows = NULL;
    counter = 0u;
    index = 0u;
}

DiodeLogger::~DiodeLogger() {
    // Auto-generated destructor stub for DiodeLogger
    // TODO Verify if manual additions are needed

    if (windows != NULL) {
        for (uint32 i = 0u; i < numberOfSignals; i++) {
            delete windows[i];
        }
        delete[] windows;
    }

    debugFile.Close();
}

bool DiodeLogger::Initialise(StructuredDataI &data) {

    bool ret = Object::Initialise(data);
    if (ret) {
        StreamString loggerFilePath;
        ret = data.Read("LoggerFile", loggerFilePath);
        if (ret) {
            if (!debugFile.Open(loggerFilePath.Buffer(), File::ACCESS_MODE_W | File::FLAG_CREAT | File::FLAG_TRUNC)) {
                REPORT_ERROR(ErrorManagement::FatalError, "Failed opening file");
            }
        }
        else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define LoggerFile");
        }
    }
    if (ret) {
        ret = data.Read("NumberOfSignals", numberOfSignals);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Please define NumberOfSignals");
        }
    }
    if (ret) {
        if (!data.Read("WindowSize", windowSize)) {
            windowSize = 0xFFFFFFFFu;
        }
        if (!data.Read("PrintCycles", printCycles)) {
            printCycles = 1u;
        }
    }

    if (ret) {
        windows = new int64*[numberOfSignals];

        for (uint32 i = 0u; i < numberOfSignals; i++) {
            windows[i] = new int64[windowSize];
            for (uint32 j = 0u; j < windowSize; j++) {
                windows[i][j] = 0;
            }
        }
    }

    return ret;
}

void DiodeLogger::AddSample(int64 *samples) {

    for (uint32 i = 0u; i < numberOfSignals; i++) {
        windows[i][index] = samples[i];
    }
    index++;
    index %= numberOfSignals;

    counter++;
    if ((counter % printCycles) == 0u) {

        for (uint32 i = 0u; i < numberOfSignals; i++) {
            int64 max = -0x7FFFFFFFFFFFFFFE;
            int64 min = 0x7FFFFFFFFFFFFFFF;
            int64 sum = 0;
            for (uint32 j = 0u; j < windowSize; j++) {
                if (windows[i][j] < min) {
                    min = windows[i][j];
                }
                if (windows[i][j] > max) {
                    max = windows[i][j];
                }
                sum += windows[i][j];
            }
            float64 mean = ((float64) sum) / windowSize;

            debugFile.Printf("Signal[%d]: max=%d, min=%d, mean=%f\n", i, max, min, mean);
        }
        counter = 0u;
    }
}

uint32 DiodeLogger::GetNumberOfSignals(){
    return numberOfSignals;
}

CLASS_REGISTER(DiodeLogger, "1.0")
