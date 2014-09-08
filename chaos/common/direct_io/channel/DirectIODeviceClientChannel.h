/*
 *	DirectIODeviceClientChannel.h
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
#ifndef __CHAOSFramework__DirectIODeviceClientChannel__
#define __CHAOSFramework__DirectIODeviceClientChannel__

#include <string>
#include <stdint.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/direct_io/channel/DirectIODeviceChannelGlobal.h>
#include <chaos/common/direct_io/channel/DirectIOVirtualClientChannel.h>

namespace chaos_data = chaos::common::data;
namespace chaos {
	namespace common {
		namespace direct_io {
			namespace channel {
				
				typedef struct AnswerServerInfo {
					uint16_t p_server_port;
					uint16_t s_server_port;
                    uint16_t endpoint;
					uint64_t ip;
					uint32_t hash;
				}AnswerServerInfo;
							
				typedef enum DirectIODeviceClientChannelPutMode {
					DirectIODeviceClientChannelPutModeStoricizeOnly		= 0,
					DirectIODeviceClientChannelPutModeLiveOnly			= 1,
					DirectIODeviceClientChannelPutModeStoricizeAnLive	= 2
				}DirectIODeviceClientChannelPutMode;
				
				//! Class for the managment of pushing data for the device dataset
				/*!
				 This class manage the forwarding of data that represent the device dataset channels (i/O)
				 */
				REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY(DirectIODeviceClientChannel, DirectIOVirtualClientChannel) {
					REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(DirectIODeviceClientChannel)
					
					//fixed header
					//the real size of the put header computed according to the dimension of the key
					uint32_t put_header_computed_size;
					opcode_headers::DirectIODeviceChannelHeaderPutOpcode *put_opcode_header;
					
					//the precomputed header for get last shared output channel
					opcode_headers::DirectIODeviceChannelHeaderGetOpcode get_opcode_header;

					std::string device_id;
					DirectIODeviceClientChannelPutMode put_mode;
					AnswerServerInfo answer_server_info;
					
					void prepare_put_opcode();
					void prepare_get_opcode();
				protected:
					DirectIODeviceClientChannel(std::string alias);
					void freeSentData(void *data,  DisposeSentMemoryInfo& dispose_memory_info);
				public:
					~DirectIODeviceClientChannel();
					
					//! Set the device id for this channel
					void setDeviceID(std::string _CU_id, DirectIODeviceClientChannelPutMode _put_mode = DirectIODeviceClientChannelPutModeLiveOnly);
					
					//! set the information on witch port forward the answer(the ip is the ip of the machine)
					void setAnswerServerInfo(uint16_t p_server_port, uint16_t s_server_port, uint16_t answer_enpoint);
					
					//! Send device serialization with priority
                    int64_t storeAndCacheDataOutputChannel(void *buffer, uint32_t buffer_len);
					
					//! Send a request for the last output data
                    int64_t requestLastOutputData(void **result, uint32_t &size);
					
					//! Perform a temporal query on a key
					/*!
					 Perform a query on a data cloud key(aka device id)
					 \param key to search
					 \param start_ts start of timestamp to search
					 \param end_ts end of the timestamp where limit the search
					 */
                    int64_t queryDataCloud(std::string keys, uint64_t start_ts, uint64_t end_ts);
				};

				
			}
		}
	}
}

#endif /* defined(__CHAOSFramework__DirectIODeviceClientChannel__) */
