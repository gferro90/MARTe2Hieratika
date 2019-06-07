/**
 * @file PrioritySender.h
 * @brief Header file for class PrioritySender
 * @date 01 dic 2018
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

 * @details This header file contains the declaration of the class PrioritySender
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef PRIORITYSENDER_H_
#define PRIORITYSENDER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "FastPollingMutexSem.h"
#include "MultiThreadService.h"
#include "EpicsParserAndSubscriber.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
#include "HttpChunkedStream.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe{

/**
 * @brief Sorts and sends the value and time-stamp of a given queue of PV variables.
 *
 * @details The main thread cycles at a frequency defined by the user and gets the fresh memory
 * containing the PV values and time-stamps from a EpicsParserAndSubscriber object. It gets also
 * a vector of flags that assert if the related PV has changed or not from the last call. The changed
 * variables are pushed in the first position of the queue in order to be sent first.
 *
 * @details A pool of threads wait that the main thread sorts the queue and then they send the
 * variables in the first positions. During the cycle of the main thread, N*numberOfPoolThreads PVs
 * should be sent to different ports of the same server using HTTP protocol, where N (the number of
 * PVs to be sent by each thread per cycle) is defined by the user. If the threads are not capable to
 * send all the variables during the cycle time, the main thread will jump a cycle.
 *
 * @details Follows an example of the configuration:
 * <pre>
 * +Sender = {
 *     Class = PrioritySender
 *     NumberOfPoolThreads = 20 //how many open connections
 *     NumberOfSignalPerThread = 200 //how many signals per thread per cycle
 *     ServerIp = "127.0.0.1" //the remote IP address
 *     ServerInitialPort = 4444 //the remote port
 *     NumberOfCpus = 8 //the number of the CPUs (default: 4)
 *     Timeout = 0xFFFFFFFF //the timeout
 *     MsecPeriod=500 //the cycle time period
 * }
 * </pre>
 */
class PrioritySender: public MultiThreadService {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Constructor
     */
    PrioritySender();

    /**
     * @brief Destructor
     */
    virtual ~PrioritySender();

    /**
     * @see MultiThreadService::Initialise
     * @details The user must specify the following parameters:\n
     *   NumberOfSignalPerThread: the number of variables that each thread must send per cycle\n
     *   ServerIp: the server ip address\n
     *   ServerInitialPort: the server starting port. If 10 threads and the port is 50,
     *     they will use the port 50-59\n
     *   MainCpuMask: the cpu mask for the main thread\n
     *   MsecPeriod: the period of the main thread in milliseconds\n
     * The user can specify the following parameter:\n
     *   ConnectionTimeout: the timeout for the connect function.
     */
    virtual bool Initialise(StructuredDataI &data);

    /**
     * @brief Connects to the EpicsParserAndSubscriber that provides the PVs.
     * @param[in] dataSourceIn the PV variables provider
     */
    bool SetDataSource(EpicsParserAndSubscriber &dataSourceIn);

    /**
     * @brief The routine to be executed by the threads in the pool.
     * @details The pool threads wait that the main threads posts the semaphore
     * after the queue sorting, then each one of them manages to send its number
     * of PVs starting from the top of the queue.
     */
    virtual ErrorManagement::ErrorType ThreadCycle(ExecutionInfo & info);


    /**
     * @brief The main thread cycle loop.
     * @details Calls EpicsParserAndSubscriber::Synchronise to get the PV values and time-stamps
     * and the changed flags. Then it sorts the queue and post the semaphore such that the other
     * threads in the pool can send the first variables.
     */
    friend void PrioritySenderCycleLoop(PrioritySender &arg);

    /**
     * @see MultiThreadService::Start
     * @brief Starts the main thread and the threads in the pool.
     */
    virtual ErrorManagement::ErrorType Start();


    /**
     * @see MultiThreadService::Start
     * @brief Stops all the threads.
     */
    virtual ErrorManagement::ErrorType Stop();


private:

    /**
     * @brief Help function to send the variables to the receiver
     */
    ErrorManagement::ErrorType SendVariables(HttpChunkedStream &client);


    /**
     * @brief Help function to send the close connection HTTP message in case
     * of error
     */
    ErrorManagement::ErrorType SendCloseConnectionMessage(HttpChunkedStream &client);

    /**
     * Wraps the callback of the threads in the pool
     */
    EmbeddedServiceMethodBinderT<PrioritySender> embeddedMethod;

    /**
     * Holds the sorted indexes queue
     */
    uint32 *indexList;

    /**
     * Link to the PVs provider
     */
    EpicsParserAndSubscriber *dataSource;

    /**
     * The allocated memory to store the PVs
     * values and time-stamps
     */
    uint8 *memory;

    /**
     * The memory containing the changed flags
     * for each PV
     */
    uint8 *changeFlag;

    /**
     * The memory size
     */
    uint64 totalMemorySize;

    /**
     * The number of PVs
     */
    uint32 numberOfVariables;

    /**
     * The semaphore used to synchronise
     * the threads in the pool
     */
    FastPollingMutexSem syncSem;

    /**
     * The initial position in the
     * PVs queue
     */
    uint32 currentIdx;

    /**
     * Holds the number of changed PVs that
     * have not be sent in the previous cycle
     */
    uint32 currentChangePos;

    /**
     * The number of variables to be sent
     * by each thread in the pool
     */
    uint32 numberOfSignalToBeSent;

    /**
     * The event semaphore used by the
     * main thread to trigger the
     * execution of the pool threads
     */
    EventSem eventSem;

    /**
     * Used from the main thread to check if all the
     * threads terminate to send their variables
     */
    uint32 nThreadsFinished;

    /**
     * Used to block the sorting at the beginning
     * to send all the initial values of the PVs
     * and when too many PVs have changed
     */
    uint32 chunckCounter;

    /**
     * How many cycles to send all the variables
     */
    uint32 numberOfChunks;

    /**
     * The server ip address
     */
    StreamString serverIpAddress;

    /**
     * The server initial port
     */
    uint32 serverInitialPort;

    /**
     * The server initial port
     */
    uint32 serverPort;

    /**
     * The connection timeout
     */
    TimeoutType connectionTimeout;

    /**
     * The cpu mask of the main thread
     */
    uint32 numberOfCpus;

    /**
     * Used to trigger the stop of the threads
     */
    volatile int32 quit;

    /**
     * Stores the last tick counter
     * to synchronise the the main thread loop
     */
    uint64 lastTickCounter;

    /**
     * The period of the main thread in milliseconds
     */
    uint32 msecPeriod;


};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* PRIORITYSENDER_H_ */

