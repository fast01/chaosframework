/*
 *	DirectIOSystemAPIClientChannel.h
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

#ifndef __CHAOSFramework__DirectIOSystemAPIClientChannel__
#define __CHAOSFramework__DirectIOSystemAPIClientChannel__

#include <string>
#include <stdint.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/direct_io/channel/DirectIODeviceChannelGlobal.h>
#include <chaos/common/direct_io/channel/DirectIOVirtualClientChannel.h>

namespace chaos_data = chaos::common::data;

namespace chaos {
	namespace common {
		namespace direct_io {
			namespace channel {

				using namespace chaos::common::direct_io::channel::opcode_headers;
				//! System API client channel
				REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY(DirectIOSystemAPIClientChannel, DirectIOVirtualClientChannel) {
					REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(DirectIOSystemAPIClientChannel)
					friend class DirectIODispatcher;
					
					DirectIOSystemAPIClientChannel(std::string alias);
					~DirectIOSystemAPIClientChannel();
					
					class DirectIOSystemAPIClientChannelDeallocator:
					public DirectIOClientDeallocationHandler {
					protected:
						void freeSentData(void* sent_data_ptr, DisposeSentMemoryInfo *free_info_ptr);
					};
					//static deallocator forthis channel
					static DirectIOSystemAPIClientChannelDeallocator STATIC_DirectIOSystemAPIClientChannelDeallocator;
				public:
					//! perform the creation of new named Snapshot
					/*!
					 The creation of the new snapshot is started in the chaos data service after received 
					 the request. The API return in async way if the creation is started
					 \param snapshot_name the name of the snapshot
					 \param producer_keys the list of the producer key to include in the snapshot, if it is empty
					 all producer key are included.
					 \api_result is the result of the api execution
					 \return the error of the directio client in the forwarding message operation
					 */
					int64_t makeNewDatasetSnapshot(const std::string& snapshot_name,
												   const std::vector<std::string>& producer_keys,
												   DirectIOSystemAPISnapshotResult **api_result_handle);
					
					//! delete the snapshot identified by name
					/*!
					 Delete the entry of the snapshot and all dataset associated to it
					 \param snapshot_name the name of the snapshot to delete
					 */
					int64_t deleteDatasetSnapshot(const std::string& snapshot_name,
												  DirectIOSystemAPISnapshotResult **api_result_handle);
					
					//! get the snapshot for one or more producer key
					/*!
					 Retrieve the dataset form the snapshot for one or more producer key.
					 Is possible to decide the type of the snapshot to get
					 \param snapshot_name the name of the snapshot to delete
					 \param channel_type the type of the channel [0-output, 1-input, 2-custom, 3-system]]
					 \param producer_key the key of the producer for wich return the datasets
					 */
					int64_t getDatasetSnapshotForProducerKey(const std::string& snapshot_name,
															 const std::string& producer_key,
															 uint32_t channel_type,
															 DirectIOSystemAPIGetDatasetSnapshotResult **api_result_handle);
				};
			}
		}
	}
}
#endif /* defined(__CHAOSFramework__DirectIOSystemAPIClientChannel__) */
