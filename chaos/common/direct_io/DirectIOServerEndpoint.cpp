/*
 *	DirectIOServerEndpoint.cpp
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
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/direct_io/DirectIOServerEndpoint.h>

using namespace chaos::common::direct_io;

#define DirectIOServerEndpoint_LOG_HEAD "[DirectIOServerEndpoint_LOG_HEAD - "<<endpoint_route_index<<"] - "

#define DIOSE_LAPP_ LAPP_ << DirectIOServerEndpoint_LOG_HEAD
#define DIOSE_LDBG_ LDBG_ << DirectIOServerEndpoint_LOG_HEAD
#define DIOSE_LERR_ LERR_ << DirectIOServerEndpoint_LOG_HEAD

DirectIOServerEndpoint::DirectIOServerEndpoint() {
	channel_slot = (channel::DirectIOVirtualServerChannel**)malloc(sizeof(channel::DirectIOVirtualServerChannel**)*MAX_ENDPOINT_CHANNEL);
}

DirectIOServerEndpoint::~DirectIOServerEndpoint() {
	if(channel_slot) free(channel_slot);
}

uint16_t DirectIOServerEndpoint::getRouteIndex() {
	return endpoint_route_index;
}

//! Add a new channel instantiator
channel::DirectIOVirtualServerChannel *DirectIOServerEndpoint::registerChannelInstance(channel::DirectIOVirtualServerChannel *channel_instance) {
	if(!channel_instance) return NULL;
	// gest exsclusive access
	boost::upgrade_lock<boost::shared_mutex> lock(mutex_channel_slot);
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
	
	DIOSE_LDBG_ << "Register channel " << channel_instance->getName() << " with route index " << (int)channel_instance->getChannelRouterIndex();
	if(channel_instance->getChannelRouterIndex() > (MAX_ENDPOINT_CHANNEL-1)) return NULL;
	channel_slot[channel_instance->getChannelRouterIndex()] = channel_instance;
	return channel_instance;
}

//! Dispose the channel
void DirectIOServerEndpoint::deregisterChannelInstance(channel::DirectIOVirtualServerChannel *channel_instance) {
	if(!channel_instance) return;
	// get exsclusive access
	boost::upgrade_lock<boost::shared_mutex> lock(mutex_channel_slot);
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
	
	// now we have exclusive access
	if(channel_instance->getChannelRouterIndex() > (MAX_ENDPOINT_CHANNEL-1)) return;
	channel_slot[channel_instance->getChannelRouterIndex()] = NULL;
}


// New channel allocation by name
channel::DirectIOVirtualServerChannel *DirectIOServerEndpoint::getNewChannelInstance(std::string channel_name) throw (CException) {
	channel::DirectIOVirtualServerChannel *channel = chaos::ObjectFactoryRegister<channel::DirectIOVirtualServerChannel>::getInstance()->getNewInstanceByName(channel_name.c_str());
	registerChannelInstance(channel);
	return channel;
}

// New channel allocation by name
void DirectIOServerEndpoint::releaseChannelInstance(channel::DirectIOVirtualServerChannel *channel_instance) throw (CException) {
	deregisterChannelInstance(channel_instance);
	if(channel_instance) delete(channel_instance);
}

// Event for a new data received
void DirectIOServerEndpoint::priorityDataReceived(DirectIODataPack *data_pack) {
	boost::shared_lock<boost::shared_mutex> Lock(mutex_channel_slot);
	channel_slot[data_pack->header.fields.channel_idx]->server_channel_delegate->consumeDataPack(data_pack);
}

// Event for a new data received
void DirectIOServerEndpoint::serviceDataReceived(DirectIODataPack *data_pack) {
	boost::shared_lock<boost::shared_mutex> Lock(mutex_channel_slot);
	channel_slot[data_pack->header.fields.channel_idx]->server_channel_delegate->consumeDataPack(data_pack);
}