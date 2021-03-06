/*
 *	DirectIOVirtualChannel.h
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
#ifndef __CHAOSFramework__DirectIOVirtualClientChannel__
#define __CHAOSFramework__DirectIOVirtualClientChannel__

#include <stdint.h>
#include <chaos/common/direct_io/DirectIOForwarder.h>
#include <chaos/common/direct_io/channel/DirectIOVirtualChannel.h>

namespace chaos {
	namespace common {
		namespace direct_io {
			class DirectIOClient;
			class DirectIOForwarder;
            class DirectIOClientConnection;
            
			struct DirectIODataPack;
			
			//! forward declaration
			class DirectIOClientDeallocationHandler;
			
            namespace channel {
				
				class DirectIOVirtualClientChannel :
				protected DirectIOVirtualChannel,
				protected DirectIOClientDeallocationHandler {
					friend class chaos::common::direct_io::DirectIOClientConnection;
                    
					DirectIOForwarderHandler  forward_handler;

				protected:
					
					//! subclass can override this with custom persisint() after channel allocation) implementation
					DirectIOClientDeallocationHandler *header_deallocator;
					
					//!only virtual channel class can access this class to permit
					//! the regulatio of the call
					DirectIOForwarder *client_instance;
					
					//priority socket
					int64_t sendPriorityData(chaos::common::direct_io::DirectIODataPack *data_pack,
											 DirectIOSynchronousAnswer **synchronous_answer = NULL);
					int64_t sendPriorityData(chaos::common::direct_io::DirectIODataPack *data_pack,
											 DirectIOClientDeallocationHandler *data_deallocator,
											 DirectIOSynchronousAnswer **synchronous_answer = NULL);
					
					//service socket
					int64_t sendServiceData(chaos::common::direct_io::DirectIODataPack *data_pack,
											DirectIOSynchronousAnswer **synchronous_answer = NULL);
					int64_t sendServiceData(chaos::common::direct_io::DirectIODataPack *data_pack,
											DirectIOClientDeallocationHandler *data_deallocator,
											DirectIOSynchronousAnswer **synchronous_answer = NULL);
					
					// prepare header for defaut connection data
					inline DirectIODataPack *completeChannnelDataPack(DirectIODataPack *data_pack, bool synchronous_answer = false) {
						//complete the datapack
						data_pack->header.dispatcher_header.fields.synchronous_answer = synchronous_answer;
						data_pack->header.dispatcher_header.fields.channel_idx = channel_route_index;
						return data_pack;
					}
					
                    DirectIOVirtualClientChannel(std::string channel_name, uint8_t channel_route_index);
                    ~DirectIOVirtualClientChannel();
					
					//! default header deallocator implementation
					void freeSentData(void* sent_data_ptr, DisposeSentMemoryInfo *free_info_ptr);
				public:
					//void setEndpoint(uint16_t _endpoint);
                };
                
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__DirectIOVirtualChannel__) */
