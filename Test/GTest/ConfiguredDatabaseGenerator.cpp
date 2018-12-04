/**
 * @file ConfiguredDatabaseGenerator.cpp
 * @brief Source file for class ConfiguredDatabaseGenerator
 * @date 29 nov 2018
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
 * the class ConfiguredDatabaseGenerator (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "ConfigurationDatabase.h"
#include "StandardParser.h"
#include "RealTimeApplicationConfigurationBuilder.h"
#include "File.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;

int main(int argc,
         const char **argv) {

    BasicFile configFile;
    if (!configFile.Open(argv[1], File::ACCESS_MODE_R)) {
        printf("Failed opening configuration file %s\n", argv[1]);
        return -1;
    }

    configFile.Seek(0ull);
    ConfigurationDatabase globalDatabase;
    StandardParser parser(configFile, globalDatabase);
    if (!parser.Parse()) {
        printf("Failed Parse\n");
        return -1;
    }

    globalDatabase.MoveAbsolute("$Application");

    RealTimeApplicationConfigurationBuilder builder(globalDatabase, "DDB1");
    builder.ConfigureBeforeInitialisation();
    ConfigurationDatabase functionsDatabase;
    ConfigurationDatabase dataDatabase;
    builder.Copy(functionsDatabase, dataDatabase);

    File functionFile;
    functionFile.Open(argv[2], BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT | BasicFile::FLAG_TRUNC);
    functionFile.SetCalibWriteParam(0u);
    functionFile.Seek(0ull);
    File dataFile;
    dataFile.Open(argv[3], BasicFile::ACCESS_MODE_W | BasicFile::FLAG_CREAT | BasicFile::FLAG_TRUNC);
    dataFile.SetCalibWriteParam(0u);
    dataFile.Seek(0ull);
    functionsDatabase.MoveToRoot();
    dataDatabase.MoveToRoot();

    functionFile.Printf("%!\n", *((StructuredDataI*)&functionsDatabase));
    functionFile.Flush();
    dataFile.Printf("%!", *((StructuredDataI*)&dataDatabase));
    dataFile.Flush();
    return 0;
}

