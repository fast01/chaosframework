/*
 *	DirectIOSystemAPIClientChannel.cpp
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
#include <chaos/common/direct_io/channel/DirectIOSystemAPIClientChannel.h>

namespace chaos_data = chaos::common::data;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::direct_io::channel::opcode_headers;

//define the static deallocator class
DirectIOSystemAPIClientChannel::DirectIOSystemAPIClientChannelDeallocator
DirectIOSystemAPIClientChannel::STATIC_DirectIOSystemAPIClientChannelDeallocator;

//base constructor
DirectIOSystemAPIClientChannel::DirectIOSystemAPIClientChannel(std::string alias):
DirectIOVirtualClientChannel(alias, DIOSystemAPI_Channel_Index) {
	//associate the default static allocator
	header_deallocator = &STATIC_DirectIOSystemAPIClientChannelDeallocator;
}

//base destructor
DirectIOSystemAPIClientChannel::~DirectIOSystemAPIClientChannel() {
	
}

// start a new snapshoot creation
int64_t DirectIOSystemAPIClientChannel::makeNewDatasetSnapshot(const std::string& snapshot_name,
						   const std::vector<std::string>& producer_keys,
						   DirectIOSystemAPINewSnapshootResult **api_result_handle) {
	int64_t err = 0;
	DirectIOSynchronousAnswer *answer = NULL;
	if(snapshot_name.size() > 255) {
		//bad snapshoot name size
		return -1000;
	}
	//allocate the datapack
	DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
	
	//allocate the header
	DirectIOSystemAPIChannelOpcodeNewSnapshootHeaderPtr new_snapshot_opcode_header = (DirectIOSystemAPIChannelOpcodeNewSnapshootHeaderPtr)calloc(sizeof(DirectIOSystemAPIChannelOpcodeNewSnapshootHeader), 1);
	
	//set opcode
	data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::SystemAPIChannelOpcodeNewNewSnapshootDataset);
	
	//copy the snapshot name
	std::memcpy(new_snapshot_opcode_header->field.snap_name, snapshot_name.c_str(), snapshot_name.size());

	//set header
	DIRECT_IO_SET_CHANNEL_HEADER(data_pack, new_snapshot_opcode_header, sizeof(DirectIOSystemAPIChannelOpcodeNewSnapshootHeader))
	if(producer_keys.size()) {
		//we have also a set of producer key so senti it in the data part of message
		std::string producer_key_concatenation;
		for(std::vector<std::string>::const_iterator it = producer_keys.begin();
			it != producer_keys.end();) {
			//add key
			producer_key_concatenation.append(*it);
			
			if((++it) != producer_keys.end()) {
				producer_key_concatenation.append(",");
			}
		}
		//set the header field for the producer concatenation string
		new_snapshot_opcode_header->field.producer_key_set_len = TO_LITTE_ENDNS_NUM(uint32_t, (uint32_t)producer_key_concatenation.size());

		//copy the memory for forwarding buffer
		void * producer_key_concatenation_memory = calloc(producer_key_concatenation.size(), 1);
		std::memcpy(producer_key_concatenation_memory, producer_key_concatenation.c_str(), producer_key_concatenation.size());
		//set as data
		DIRECT_IO_SET_CHANNEL_DATA(data_pack, producer_key_concatenation_memory, (uint32_t)producer_key_concatenation.size());
	}
	//send data with synchronous answer flag
	if((err = (int)sendPriorityData(data_pack, &answer))) {
		//error getting last value
		if(answer && answer->answer_data) free(answer->answer_data);
	} else {
		//we got answer
		if(answer && answer->answer_size == sizeof(DirectIOSystemAPINewSnapshootResult)) {
			*api_result_handle  = static_cast<DirectIOSystemAPINewSnapshootResult*>(answer->answer_data);
			(*api_result_handle)->error = FROM_LITTLE_ENDNS_NUM(int32_t, (*api_result_handle)->error);
		} else {
			*api_result_handle = NULL;
		}
	}
	if(answer) free(answer);
	return err;
}

//! default data deallocator implementation
void DirectIOSystemAPIClientChannel::DirectIOSystemAPIClientChannelDeallocator::freeSentData(void* sent_data_ptr,
																							 DisposeSentMemoryInfo *free_info_ptr) {
	switch(free_info_ptr->sent_part) {
		case DisposeSentMemoryInfo::SentPartHeader:{
			switch(static_cast<opcode::SystemAPIChannelOpcode>(free_info_ptr->sent_opcode)) {
				case opcode::SystemAPIChannelOpcodeNewNewSnapshootDataset:
					free(sent_data_ptr);
					break;
				default:
					break;
			}
			break;
		}
			
		case DisposeSentMemoryInfo::SentPartData: {
			switch(static_cast<opcode::SystemAPIChannelOpcode>(free_info_ptr->sent_opcode)) {
				case opcode::SystemAPIChannelOpcodeNewNewSnapshootDataset:
					free(sent_data_ptr);
					break;
				default:
					break;
			}
			break;
		}
	}
}