/**
 * @file DiodeReceiver.h
 * @brief Header file for class DiodeReceiver
 * @date 04 dic 2018
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

 * @details This header file contains the declaration of the class DiodeReceiver
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef DIODERECEIVER_H_
#define DIODERECEIVER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <epicsStdlib.h>
#include <cadef.h>
#include <epicsGetopt.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MultiClientService.h"
#include "File.h"
#include "TCPSocket.h"
#include "EmbeddedServiceMethodBinderT.h"
#include "EventSem.h"
#include "HttpProtocol.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

const uint32 PV_NAME_MAX_SIZE_REC = 64u;

/**
 * Contains all the PV data
 */
struct PvRecDescriptor {
    /**
     * The channel identifier
     */
    chid pvChid;
    /**
     * The PV type
     */
    chtype pvType;
    /**
     * The number of elements > 0
     */
    uint32 numberOfElements;

    /**
     * The PV name
     */
    char8 pvName[PV_NAME_MAX_SIZE_REC];

    /**
     * The AnyType associated to this PV
     */
    AnyType at;

    /**
     * The total size of the variable
     */
    uint32 totalSize;

    /**
     * The offset of the variable in the memory buffer
     */
    uint32 offset;

};

/**
 * @brief The diode receiver.
 *
 * @details This components is a server that spoons several threads, one for each incoming connection. Each thread receives HTTP messages
 * with data in this format:\n
 *   "EPICS_VARIABLE_NAME": [index|value|timestamp]\n
 * where index, value and timestamp are sent in binary format. The index is the index of the EPICS variable in the list of the variables on the sender.
 * The first time the variable is search by name (binary search) in the receiver list and the sender index is associated to the receiver index in order
 * to access directly to the variables.
 *
 * @details A set of different threads gets at each cycle the value of the changed variables and makes a ca_put.
 * @details Follows an example of the configuration:
 * <pre>
 * +Receiver = {
 *    Class = DiodeReceiver
 *    NumberOfInitThreads=12 //the number of threads that initialise the EPICS environment and do the ca_put
 *    MinNumberOfThreads=1 //the minimum number of connections (not to be changed)
 *    MaxNumberOfThreads=110 //the maximum number of connections
 *    NumberOfPoolThreads = 1 //the number of the threads waiting connections at the beginning (not to be changed)
 *    ServerPort = 4444 //the server port
 *    Timeout = 0xFFFFFFFF //the timeout (not to be changed)
 *    InputFilePath = "DiodePVs_One_Process_.xml" //the file containing the list of EPICS variables
 *    FirstVariableName = "RFQTDT:PS10K:VSet"  //the first variable name in the .xml file
 *    MsecPeriod = 500 //the ca_put cycle period (default 1000)
 *    MaxNumberOfVariables = 0xFFFFFFFF //the maximum number of variables to manage (default 0xFFFFFFFF)
 *    NumberOfCpus = 8 //the number of cpus (default 4)
 *    AcceptTimeout = 0xFFFFFFFF //the maximum time to wait for a connection (default TTInfiniteWait)
 * }
 * </pre>
 */
class DiodeReceiver: public MultiClientService {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Constructor
     */
    DiodeReceiver();

    /**
     * @brief Destructor
     */
    virtual ~DiodeReceiver();

    /**
     * @see MultiClientService::Initialise
     * @details Reads the configuration parameters and initialises the PV list reading from the
     * input xml file.
     */
    virtual bool Initialise(StructuredDataI &data);

    /**
     * @see MultiClientService::Start
     * @details Starts the threads to initialise the EPICS environment. Once the environment has been initialised,
     * the threads managing the incoming connections starts and receives the value and timestamps of the variables.
     * The initial threads polls every cycle to ca_put the changed variables.
     */
    virtual ErrorManagement::ErrorType Start();

    /**
     * @brief The server routine.
     * @details Spoons a new thread for each incoming connection.
     */
    ErrorManagement::ErrorType ServerCycle(MARTe::ExecutionInfo &information);

    /**
     * @brief The callback executed after the connection.
     * @details Receives the HTTP messages parses the content and for each PV variable received
     * updates its value and timestamp in the buffer. Sets also a flag if the received variable has changed such
     * that the DiodeReceiverCycleLoop thread can ca_put it in the next cycle.
     */
    ErrorManagement::ErrorType ClientService(TCPSocket * const commClient);

    /**
     * @brief Stops the threads
     */
    virtual ErrorManagement::ErrorType Stop();

    /**
     * @brief Called by ServerCycle to copy the PC buffer and the flags.
     */
    virtual bool Synchronise(uint8 *memoryOut,
                             uint8 *changedFlags);

    /**
     * @brief Retrieves the number of PV variables
     */
    uint32 GetNumberOfVariables();

    /**
     * @brief Retrieves the PV descriptors
     */
    PvRecDescriptor *GetPvDescriptors();

    /**
     * @brief Retrieves the total memory size
     */
    uint64 GetTotalMemorySize();

    /**
     * @brief Asserts that the initialisation of the EPICS environment has finished.
     */
    bool InitialisationDone();

    /**
     * @brief The initialisation thread callback
     * @details Initialises the EPICS environment and ca_put the PVs every cycle
     */
    friend void DiodeReceiverCycleLoop(DiodeReceiver &arg);

    /**
     * @see MultiClientService::AddThread
     * @details Assign the threads on different cpu to load the balance and execute it.
     */
    virtual ErrorManagement::ErrorType AddThread();

protected:

    /**
     * @brief Help function to read a new chunk from the client.
     */
    ErrorManagement::ErrorType ReadNewChunk(TCPSocket * const commClient,
                                            StreamString &payload,
                                            bool isChunked,
                                            uint32 &chunkSize,
                                            uint32 &contentLength);

    /**
     * @brief Help function to read the PV name and index from the client.
     */
    bool ReadVarNameAndIndex(StreamString &payload,
                             StreamString &varName,
                             uint32 &receivedIndex,
                             uint8 &receivedTypeId,
                             uint32 &receivedSize,
                             uint32 &receivedOffset,
                             uint32 &processedSize,
                             const char8 * &dataPtr);

    /**
     * @brief Help function to map the received index with the local one in the buffer.
     */
    bool GetLocalIndex(StreamString &payload,
                       StreamString &varName,
                       uint32 receivedIndex,
                       uint32 receivedSize,
                       uint32 receivedOffset,
                       uint32 &index,
                       uint32 &processedSize,
                       bool &controlOk);

    /**
     * @brief Help function to read the value of the received PV.
     */
    void ReadVarValueAndSkip(StreamString &payload,
                             const char8 *dataPtr,
                             uint32 index,
                             uint32 processedSize,
                             uint8 receivedTypeId,
                             uint32 varOffset,
                             bool controlOk);

    /**
     * @brief Help funcion to send the OK reply message to the received HTTP
     * message.
     */
    ErrorManagement::ErrorType SendOkReplyMessage(HttpProtocol &protocol,
                                                  TCPSocket * const commClient);

    /**
     * @brief Help function to send the ERROR reply message in case of errors to
     * close the connection.
     */
    void SendErrorReplyMessage(HttpProtocol &protocol,
                               TCPSocket * const commClient);

    /**
     * @brief Retrieves the local PV index associated to a given variable name.
     */
    bool GetLocalVariableIndex(const char8 *varName,
                               uint32 &index);

    /**
     * The socket listening for incoming connections
     */
    TCPSocket server;

    /**
     * The server embedded method
     */
    EmbeddedServiceMethodBinderT<DiodeReceiver> embeddedMethod;

    /**
     * The server port
     */
    uint32 serverPort;

    /**
     * The maximum time to wait for a new incoming connection
     */
    TimeoutType acceptTimeout;

    /**
     * A spinlock used to synchronise all the threads
     */
    FastPollingMutexSem syncSem;

    /**
     * The list of PV descriptors
     */
    PvRecDescriptor *pvs;

    /**
     * The number of PVs
     */
    uint32 numberOfVariables;

    /**
     * The number of cpus
     */
    uint32 numberOfCpus;

    /**
     * The memory buffer
     */
    uint8 *memory;

    /**
     * The second buffer used for the threads that does the ca_put
     */
    uint8 *memory2;

    /**
     * A buffer storing the previous value of the PVs
     */
    uint8 *memoryPrec;

    /**
     * A vector to map the received PV index to the local PV index
     */
    uint32 *pvMapping;

    /**
     * A spinlock used to check when the initialisation phase has terminated
     */
    volatile int32 threadSetContext;

    /**
     * The change flag vector
     */
    uint8 *changeFlag;

    /**
     * The second buffer changed flag vector
     */
    uint8 *changeFlag2;

    /**
     * Stores the high resolution counter to compute the cycle time
     */
    uint64 lastCounter;

    /**
     * A spinlock used to terminate the threads
     */
    volatile int32 quit;

    /**
     * The total memory size
     */
    uint32 totalMemorySize;

    /**
     * An event semaphore used to synchronise the threads
     */
    EventSem eventSem;

    /**
     * The number of threads used to initialise the EPICS
     * environment and to ca_put the variables
     */
    uint32 numberOfInitThreads;

    /**
     * A counter used to wait that all the threads received the HTTP
     * message
     */
    uint32 threadCnt;

    /**
     * The maximum number of PVs
     */
    uint32 maxNumberOfVariables;

    /**
     * Stores the high resolution counter to compute the cycle time
     * for each thread
     */
    uint64 *lastTickCounter;

    /**
     * The cycle time period
     */
    uint32 msecPeriod;

    /**
     * Used to set differents cpus to the threads in order
     * to load the balance.
     */
    uint32 currentCpuMask;

    /**
     * The read timeout
     */
    TimeoutType readTimeout;


    /**
     * The maximum array size
     */
    uint32 maxArraySize;
};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_INTERFACES_DIODERECEIVER_DIODERECEIVER_H_ */

