/*
 *	DirectIOClientConnection.h
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

#include <chaos/common/data/cache/FastHash.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/direct_io/DirectIOClientConnection.h>

using namespace chaos::common::direct_io;

#define DIOVCC_LOG_HEAD "[DirectIOVirtualClientChannel: "<< server_description <<"] - "

#define DIOVCCLAPP_ LAPP_ << DIOVCC_LOG_HEAD
#define DIOVCCLDBG_ LDBG_ << DIOVCC_LOG_HEAD
#define DIOVCCLERR_ LERR_ << DIOVCC_LOG_HEAD

// current client ip in string form
std::string DirectIOClientConnection::my_str_ip;

// current client ip in 64 bit form
uint64_t DirectIOClientConnection::my_i64_ip = 0;


DirectIOClientConnection::DirectIOClientConnection(std::string _server_description):server_description(_server_description), event_handler(NULL) {
	//set the default connection hash
	connection_hash = chaos::common::data::cache::FastHash::hash(server_description.c_str(), server_description.size(), 0);
}

DirectIOClientConnection::~DirectIOClientConnection() {
	
}

const char * DirectIOClientConnection::getServerDescription() {
	return server_description.c_str();
}

std::string DirectIOClientConnection::getStrIp() {
    return my_str_ip;
}
uint64_t DirectIOClientConnection::getI64Ip() {
    return my_i64_ip;
}

void DirectIOClientConnection::setConnectionHash(uint32_t _connection_hash) {
	connection_hash = _connection_hash;
}

uint32_t DirectIOClientConnection::getConenctionHash() {
	return connection_hash;
}

void DirectIOClientConnection::setEventHandler(DirectIOClientConnectionEventHandler *_event_handler) {
	event_handler = _event_handler;
}

DirectIOClientConnectionStateType::DirectIOClientConnectionStateType DirectIOClientConnection::getState() {
	return current_state;
}

void DirectIOClientConnection::lowLevelManageEvent(DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event_state) {
	current_state = event_state;
	DEBUG_CODE( switch(current_state)  {
		case DirectIOClientConnectionStateType::DirectIOClientConnectionEventConnected:
			DIOVCCLDBG_ << "Set to state ->  DirectIOClientConnectionEventConnected";
			break;
		case DirectIOClientConnectionStateType::DirectIOClientConnectionEventDisconnected:
			DIOVCCLDBG_ << "Set to state ->  DirectIOClientConnectionEventDisconnected";
			break;
	});
	//forward event to the handler
	if(event_handler) event_handler->handleEvent(connection_hash, current_state);
}

// New channel allocation by name
channel::DirectIOVirtualClientChannel *DirectIOClientConnection::getNewChannelInstance(std::string channel_name) {
	channel::DirectIOVirtualClientChannel *channel = chaos::ObjectFactoryRegister<channel::DirectIOVirtualClientChannel>::getInstance()->getNewInstanceByName(channel_name.c_str());
    //sub class method for register the instance
    if(channel) {
		channel->client_instance = this;
		TemplatedKeyObjectContainer::registerElement(channel->channel_route_index, channel);
    }
	return channel;
}

// New channel allocation by name
void DirectIOClientConnection::releaseChannelInstance(channel::DirectIOVirtualClientChannel *channel_instance) {
	if(channel_instance) {
        TemplatedKeyObjectContainer::deregisterElementKey(channel_instance->channel_route_index);
		channel_instance->client_instance = NULL;
        delete(channel_instance);
    }
}

void DirectIOClientConnection::freeObject(unsigned int hash, DirectIOClientConnection *connection) {
	//releaseConnection(connection);
}