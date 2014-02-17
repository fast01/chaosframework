
/*
 *	ZMQDirectIOClient.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
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
#ifndef __CHAOSFramework__ZMQDirectIOClient__
#define __CHAOSFramework__ZMQDirectIOClient__

#include <chaos/common/direct_io/DirectIOClient.h>
#include <chaos/common/direct_io/impl/ZMQBaseClass.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>



namespace chaos {
	namespace common {
		namespace direct_io {
			namespace channel {
				class DirectIOVirtualClientChannel;
			}
            namespace impl {
                REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY(ZMQDirectIOClient, DirectIOClient), private ZMQBaseClass {
                    REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(ZMQDirectIOClient)

					friend class DirectIOVirtualClientChannel;
					
                    int32_t priority_port;
					
					int32_t service_port;

                    boost::shared_mutex mutex_socket_manipolation;
					
					void *zmq_context;
					
                    void *socket_priority;
                    
                    void *socket_service;
					
					void *socket_monitor;
					
                    inline uint32_t writeToSocket(void *socket, DirectIODataPack *data_pack);
                    
                    //set the spread functionality on zmq socket
                    //void switchModeTo(DirectIOConnectionSpreadType::DirectIOConnectionSpreadType connection_mode);
                    
					bool thread_run;
                    boost::shared_ptr<boost::thread> monitor_thread;
					
					//! check the connection with the endpoint for the two socket
					void *monitorWorker();
                    
                    ZMQDirectIOClient(string alias);
                    
                    ~ZMQDirectIOClient();
				protected:
					
                    // send the data to the server layer on priority channel
                    uint32_t sendPriorityData(DirectIODataPack *data_pack);
                    
                    // send the data to the server layer on the service channel
                    uint32_t sendServiceData(DirectIODataPack *data_pack);
                public:
                    
                    //! Initialize instance
                    void init(void *init_data) throw(chaos::CException);

                    
                    //! Deinit the implementation
                    void deinit() throw(chaos::CException);
					
					int addServer(std::string server_desc);
					
					int removeServer(std::string server_desc);

                };
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__DirectIOZMQClient__) */