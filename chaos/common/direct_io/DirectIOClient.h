/*
 *	DirectIOClient.h
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
#ifndef __CHAOSFramework__DirectIOClient__
#define __CHAOSFramework__DirectIOClient__

#include <map>
#include <string>
#include <inttypes.h>

#include <chaos/common/utility/ObjectInstancer.h>
#include <chaos/common/utility/NamedService.h>
#include <chaos/common/utility/InizializableService.h>
#include <chaos/common/utility/TemplatedKeyObjectContainer.h>


#include <chaos/common/direct_io/DirectIOTypes.h>
#include <chaos/common/direct_io/DirectIOClientConnection.h>
#include <chaos/common/direct_io/DirectIOURLManagment.h>

#include <boost/thread.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <chaos/common/data/cache/FastHash.h>



namespace chaos {
	
	namespace common {
		
		//forward declaration
		namespace network {
			class NetworkBroker;
		}
		namespace direct_io {
			//forward declaration
            namespace channel {
                class DirectIOVirtualClientChannel;
            }

			
            typedef std::map< unsigned int, channel::DirectIOVirtualClientChannel* > ChannelMap;
            typedef std::map< unsigned int, channel::DirectIOVirtualClientChannel* >::iterator ChannelMapIterator;
            typedef chaos::utility::TemplatedKeyObjectContainer<std::string, DirectIOClientConnection*> DCKeyObjectContainer;
			//! Direct IO client base class
			/*!
			 This class represent the base interface for the operation on direct io output channel,
			 The client layer will connect to the server one to send data. Client can connect to
			 may server and the same data will be forwarded to all server
			 dio_client---> data message -->dio_server
			 */
			class DirectIOClient :	public NamedService,
			public chaos::utility::InizializableService,
			public DirectIOURLManagment,
			protected DCKeyObjectContainer {
				friend class chaos::common::network::NetworkBroker;
				
				std::string			impl_alias;
                boost::atomic_uint	channel_counter;
			protected:
				void forwardEventToClientConnection(DirectIOClientConnection *client, DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event_type);
			public:
                DirectIOClient(string alias);
				virtual ~DirectIOClient();
				
				//! create a new connection for a server
				/*!
				 Allocate a new connection from server description with endpoitn ex: ip:p-port:s_port|endpoint
				 */
				virtual DirectIOClientConnection *getNewConnection(std::string server_description_with_endpoint);
                
                virtual DirectIOClientConnection *getNewConnection(std::string server_description, uint16_t endpoint) = 0;
				
				//! Release the connection
				virtual void releaseConnection(DirectIOClientConnection *connection_to_release) = 0;
			};
			
		}
	}
}

#endif /* defined(__CHAOSFramework__DirectIOClient__) */
