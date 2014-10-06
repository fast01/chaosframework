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

#include <map>
#include <deque>
#include <memory>
#include <stdint.h>


#include <boost/thread.hpp>
#include <boost/heap/priority_queue.hpp>
#include <boost/atomic.hpp>

#include <chaos/common/data/DatasetDB.h>
#include <chaos/common/data/cache/AttributesSetting.h>
#include <chaos/common/batch_command/BatchCommand.h>
#include <chaos/common/batch_command/BatchCommandExecutor.h>
#include <chaos/common/utility/ObjectInstancer.h>

#include <chaos/cu_toolkit/DataManager/KeyDataStorage.h>
#include <chaos/cu_toolkit/driver_manager/DriverErogatorInterface.h>


namespace chaos {
    namespace cu {
        
        //forward declaration
        namespace dm {
            namespace driver {
                class DriverAccessor;
            }
        }
        
        namespace control_manager {
			//forward declaration
			class SCAbstractControlUnit;

            namespace slow_command {
				
				namespace boost_cont = boost::container;
				namespace chaos_data = chaos::common::data;
				namespace chaos_cache = chaos::common::data::cache;
				namespace chaos_batch = chaos::common::batch_command;

				
                //forward declaration
                class SlowCommand;
                
                //! Macro for helping the allocation of the isntancer of the class implementing the slow command
#define SLOWCOMMAND_INSTANCER(SlowCommandClass) new chaos::common::utility::NestedObjectInstancer<chaos::cu::control_manager::slow_command::SlowCommand, chaos_batch::BatchCommand>(new chaos::common::utility::TypedObjectInstancer<SlowCommandClass, chaos::cu::control_manager::slow_command::SlowCommand>())
				
                //! Slow command execution sand box
                /*!
                    This class is the environment where the exeecution of the slow command handlers take place.
                 */
                class SlowCommandExecutor : private chaos_batch::BatchCommandExecutor {

                    friend class chaos::cu::control_manager::SCAbstractControlUnit;

                    //!Live push driver
                    data_manager::KeyDataStorage  *keyDataStoragePtr;
                    
                    //! the reference to the master device database
                    chaos_data::DatasetDB *deviceSchemaDbPtr;
                    
					//! The driver erogator
					chaos::cu::driver_manager::DriverErogatorInterface *driverAccessorsErogator;
                protected:
                    
                    //! Private constructor
                    SlowCommandExecutor(std::string _executorID,
										chaos_data::DatasetDB *_deviceSchemaDbPtr);
                    
                    //! Private deconstructor
                    ~SlowCommandExecutor();
                    
                    //Check if the waithing command can be installed
                    chaos_batch::BatchCommand *instanceCommandInfo(std::string& commandAlias);

                public:
                    
                    // Initialize instance
                    virtual void init(void*) throw(chaos::CException);

                    //! Install a command associated with a type
                    /*!
                     Install the isntancer for a determinated SlowCommand, for an easly way to do this can be used
                     the macro SLOWCOMMAND_INSTANCER(SlowCommandClass) where "SlowCommandClass" is the calss that
                     extend SlowCommand to implement a new command. The access to the internal map is not sincornized
                     and need to be made befor the executor will be started
                     \param alias the name associated to the command
                     \param instancer the instance of the instancer that will produce the "instance" of the command
                     */
                    void installCommand(std::string alias, chaos::common::utility::NestedObjectInstancer<SlowCommand, chaos_batch::BatchCommand> *instancer);
                };
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__SlowCommandExecutor__) */
