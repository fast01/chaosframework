/*
 *	QueryDataConsumer.cpp
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

#ifndef __CHAOSFramework__QueryDataConsumer__
#define __CHAOSFramework__QueryDataConsumer__

#include "dataservice_global.h"
#include "vfs/VFSManager.h"
#include "worker/DataWorker.h"
#include "cache_system/cache_system.h"
#include <chaos/common/utility/ObjectSlot.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/direct_io/DirectIOServerEndpoint.h>
#include <chaos/common/direct_io/channel/DirectIODeviceServerChannel.h>
#include <chaos/common/utility/TimingUtil.h>
#include <chaos/common/network/NetworkBroker.h>

#include <boost/atomic.hpp>

using namespace chaos::utility;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::direct_io::channel::opcode_headers;

namespace chaos{
    namespace data_service {
        
        class ChaosDataService;
        
        class QueryDataConsumer : public DirectIODeviceServerChannel::DirectIODeviceServerChannelHandler, public utility::StartableService {
            friend class ChaosDataService;
			std::string cache_impl_name;
			
			
			ChaosDataServiceSetting					*settings;
			chaos::NetworkBroker					*network_broker;
			
            DirectIOServerEndpoint					*server_endpoint;
			DirectIODeviceServerChannel				*device_channel;
			
			vfs::VFSManager *vfs_manager_instance;
			boost::atomic<uint16_t> device_data_worker_index;
			chaos::data_service::worker::DataWorker	**device_data_worker;
			chaos::common::utility::ObjectSlot<chaos::data_service::worker::DataWorker*> answer_worker_list;
			
            void consumeCDataWrapper(uint8_t channel_opcode, chaos::common::data::CDataWrapper *data_wrapper);
            void consumePutEvent(DirectIODeviceChannelHeaderPutOpcode *header, void *channel_data, uint32_t channel_data_len);
            void consumeGetEvent(DirectIODeviceChannelHeaderGetOpcode *header, void *channel_data, uint32_t channel_data_len);

        public:
			QueryDataConsumer(vfs::VFSManager *_vfs_manager_instance);
            ~QueryDataConsumer();
            void init(void *init_data) throw (chaos::CException);
            void start() throw (chaos::CException);
            void stop() throw (chaos::CException);
            void deinit() throw (chaos::CException);

        };
    }
}

#endif /* defined(__CHAOSFramework__QueryDataConsumer__) */