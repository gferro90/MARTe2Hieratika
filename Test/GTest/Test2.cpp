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

#include "TCPSocket.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

int main(int argc,
         char **argv) {
    TCPSocket server;

    if (!server.Open()) {
        printf("Error opening socket\n");
        return -1;
    }

    if (!server.Listen(4444, 255)) {
        printf("Error in listen\n");
        return -1;
    }

    TCPSocket clientConn;

    if (server.WaitConnection(TTInfiniteWait, &clientConn)) {
        printf("New connection\n");
        char8 buffer[1024];
        while (1) {
            uint32 size = 1024;
            MemoryOperationsHelper::Set(buffer, '\0', size);
            clientConn.Read(buffer, size);
            if(size==0u){
                break;
            }
            printf("%s", buffer);
        }
    }

    return 1;
}

