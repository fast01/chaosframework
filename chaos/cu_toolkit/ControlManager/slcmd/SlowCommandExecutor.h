/*
 *	SlowCommandExecutor.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2013 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */


#ifndef __CHAOSFramework__SlowCommandExecutor__
#define __CHAOSFramework__SlowCommandExecutor__

#include <memory>
#include <stdint.h>

#include <boost/thread.hpp>
#include <boost/heap/priority_queue.hpp>
#include <boost/atomic.hpp>

#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/ObjectInstancer.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/pqueue/CObjectProcessingPriorityQueue.h>

#include <chaos/cu_toolkit/ControlManager/slcmd/SlowCommandSandbox.h>

#define COMMAND_QUEUE_DEFAULT_LENGTH 1024

namespace chaos{
    namespace cu {
        
        //forward declaration
        class AbstractControlUnit;
        
        //forward declaration
        namespace dm {
            namespace driver {
                class DriverAccessor;
            }
        }
        
        namespace control_manager {
            namespace slow_command {
                //forward declaration
                class SlowCommand;
                class SlowCommandSandbox;
                
                //! Macro for helping the allocation of the isntancer of the class implementing the slow command
#define SLOWCOMMAND_INSTANCER(SlowCommandClass) new chaos::common::utility::TypedObjectInstancer<SlowCommandClass, SlowCommand>());
                
                //! Slow command execution sand box
                /*!
                    This class is the environment where the exeecution of the slow command handlers take place.
                 */
                class SlowCommandExecutor : public utility::StartableService {
                    friend class AbstractControlUnit;
                    
                    std::string executorID;
                    
                    
                    //! sandbox for the execution of the handler
                    SlowCommandSandbox  commandSandbox;
                    
                    //!the contro unit thread associated with this executor
                    CThread         *cuThreadForExecutor;
                    
                    shared_ptr<boost::thread>   incomingCheckThreadPtr;
                    
                    //!Mutex for priority queue managment
                    boost::mutex    mutextQueueManagment;
                    
                    //! until it is true the wueue is
                    bool performQueueCheck;
                    
                    //! Read thread waithing variable for the new insert element in queue
                    boost::condition_variable   readThreadWhait;
                    
                    //! Thread for whait until the queue is empty
                    boost::condition_variable   emptyQueueConditionLock;
                    
                    //! queue for the arrived command
                    boost::heap::priority_queue< PRIORITY_ELEMENT(CDataWrapper)* > commandSubmittedQueue;

                    
                    //! this map correlate the alias to the object instancer
                    std::map<string, chaos::common::utility::ObjectInstancer<SlowCommand>* > mapCommandInstancer;
                    
                    //-------------------- handler poiter --------------------
                    //! Pointer to the set phase handler's of the current command
                    SlowCommand::SetHandlerPtr setHandler;
                    
                    //! Pointer to the acquire pahse handler's of the current command
                    SlowCommand::AcquireHandlerPtr acquireHandler;
                    
                    //! Pointer to the correlation and commit pahse handler's of the current command
                    SlowCommand::CCHandlerPtr correlationAndCommitHandler;
                    //-------------------- handler poiter --------------------
                  
                    //! Private constructor
                    SlowCommandExecutor(CThread *_cuThreadForExecutor, std::string _executorID);
                    
                    //! Private deconstructor
                    ~SlowCommandExecutor();
                    
                    //! Check the incoming command rule
                    /*!
                      In case there is a command in the top of the commandSubmittedQueue this method
                      check if there are the condition to execute it
                     */
                    void performIncomingCommandCheck();
                    
                    //! Check if the waithing command can be installed
                    SlowCommand *instanceCommandInfo(CDataWrapper *submissionInfo);
                    
                    
                public:
                    
                    // Initialize instance
                    virtual void init(void*) throw(chaos::CException);
                    
                    // Start the implementation
                    virtual void start() throw(chaos::CException);
                    
                    // Start the implementation
                    virtual void stop() throw(chaos::CException);
                    
                    // Deinit the implementation
                    virtual void deinit() throw(chaos::CException);

                    //! Perform a command registration
                    /*!
                     An instance of the command si registered within the executor.
                     */
                    void setDefaultCommand(string& alias);
                    
                    //! Install a command associated with a type
                    /*!
                     Install the isntancer for a determinated SlowCommand, for an easly way to do this can be used
                     the macro SLOWCOMMAND_INSTANCER(SlowCommandClass) where "SlowCommandClass" is the calss that 
                     extend SlowCommand to implement a new command
                     */
                    void installCommand(string& alias, chaos::common::utility::ObjectInstancer<SlowCommand>* instancer);
                    
                    //! Submite the new sloc command information
                    /*!
                     The information for the command are contained into the DataWrapper data serialization,
                     they are put into the commandSubmittedQueue for to wait to be executed.
                     */
                    bool submitCommand(CDataWrapper *commandDescription, uint8_t priority = 50);
                };
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__SlowCommandExecutor__) */
