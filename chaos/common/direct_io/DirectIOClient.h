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

#include <memory.h>
#include <inttypes.h>

#include <chaos/common/utility/StartableService.h>
#include <chaos/common/direct_io/DirectIOTypes.h>

namespace chaos {
	namespace common {
		namespace direct_io {
			
			//! Direct IO client base class
			/*!
				This class represent the base interface for the operation on direct io output channel,
				The client layer will connect to the server one to send data. Client can connect to 
				may server and the same data will be forwarded to all server
				dio_client---> data message -->dio_server
			 */
			class DirectIOClient : chaos::utility::StartableService {
				
				auto_ptr<DirectIOConnection> connection_info;
				
			public:
				
				DirectIOClient();
				
				virtual ~DirectIOClient();
				
				// Initialize instance
				void init(void *init_data) throw(chaos::CException);
				
				// Start the implementation
				void start() throw(chaos::CException);
				
				// Stop the implementation
				void stop() throw(chaos::CException);
				
				// Deinit the implementation
				void deinit() throw(chaos::CException);
				
				//! Send some data to the server
				int sendData(void *data, uint32_t len);
			};
			
		}
	}
}

#endif /* defined(__CHAOSFramework__DirectIOClient__) */