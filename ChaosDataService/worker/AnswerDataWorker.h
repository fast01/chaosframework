/*
 *	AnswerDataWorker.h
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

#ifndef __CHAOSFramework__AnswerDataWorker__
#define __CHAOSFramework__AnswerDataWorker__

#include "DataWorker.h"
#include "../cache_system/cache_system.h"

#include <map>
#include <string>
#include <stdint.h>

#include <chaos/common/utility/TemplatedKeyObjectContainer.h>
#include <chaos/common/utility/InizializableService.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/direct_io/DirectIOClient.h>
#include <chaos/common/direct_io/DirectIOClientConnection.h>
#include <chaos/common/direct_io/DirectIODataPack.h>
#include <chaos/common/direct_io/channel/DirectIODeviceClientChannel.h>
#include <chaos/common/network/NetworkBroker.h>

#include <boost/shared_ptr.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>

using namespace chaos::utility;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;


namespace chaos_direct_io = chaos::common::direct_io;
namespace chaos_direct_io_ch = chaos::common::direct_io::channel;

namespace chaos{
    namespace data_service {
		namespace worker {
			
			//!worker information for the device live storage
			struct AnswerDataWorkerJob : public WorkerJob {
				opcode_headers::DirectIODeviceChannelHeaderGetOpcode *request_header;
			};
			
			struct ClientConnectionInfo {
				chaos_direct_io::DirectIOClientConnection		*connection;
				chaos_direct_io_ch::DirectIODeviceClientChannel *channel;
			};
			
			//! worker for live device sharing
			class AnswerDataWorker :
			public DataWorker,
			protected chaos::utility::TemplatedKeyObjectContainer<uint32_t, ClientConnectionInfo*>,
			protected chaos_direct_io::DirectIOClientConnectionEventHandler {
				std::string cache_impl_name;
				chaos_direct_io::DirectIOClient *direct_io_client;
				boost::shared_mutex mutex_add_new_client;
				
				cache_system::CacheDriver *cache_driver_instance;
			protected:
				void executeJob(WorkerJobPtr job_info);
				
				//! handler method for receive the direct io client connection event
				void handleEvent(uint32_t connection_identifier, DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event);
				
				void freeObject(uint32_t key, ClientConnectionInfo *elementPtr);
				void disposeClientInfo(ClientConnectionInfo *client_info);
				DirectIODeviceClientChannel *getClientChannel(opcode_headers::DirectIODeviceChannelHeaderGetOpcode *client_header);
			public:
				AnswerDataWorker(chaos_direct_io::DirectIOClient *_client_instance, cache_system::CacheDriver *_cache_driver_instance);
				~AnswerDataWorker();
				void init(void *init_data) throw (chaos::CException);
				void deinit() throw (chaos::CException);
			};
			
		}
	}
}

#endif /* defined(__CHAOSFramework__AnswerDataWorker__) */