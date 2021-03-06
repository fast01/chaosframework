/*
 *	IODirectIODriver.cpp
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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/io/IODirectIODriver.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/common//global.h>
#include <chaos/common/utility/InizializableService.h>

#include <chaos/common/direct_io/impl/ZMQDirectIOClientConnection.h>


#define LMEMDRIVER_ LAPP_ << "[Memcached IO Driver] - "


#define IODirectIODriver_LOG_HEAD "[IODirectIODriver] - "

#define IODirectIODriver_LAPP_ LAPP_ << IODirectIODriver_LOG_HEAD
#define IODirectIODriver_DLDBG_ LDBG_ << IODirectIODriver_LOG_HEAD
#define IODirectIODriver_LERR_ LERR_ << IODirectIODriver_LOG_HEAD

using namespace chaos;
using namespace chaos::common::io;
using namespace chaos::utility;

using namespace std;
using namespace boost;
using namespace boost::algorithm;

namespace chaos_data = chaos::common::data;
namespace chaos_dio = chaos::common::direct_io;
namespace chaos_dio_channel = chaos::common::direct_io::channel;

//using namespace memcache;

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
IODirectIODriver::IODirectIODriver(std::string alias):
NamedService(alias),
current_endpoint_p_port(0),
current_endpoint_s_port(0),
current_endpoint_index(0),
connectionFeeder(alias, this),
uuid(UUIDUtil::generateUUIDLite()){
	//clear
	std::memset(&init_parameter, 0, sizeof(IODirectIODriverInitParam));
	
	device_server_channel = NULL;
	
	read_write_index = 0;
	data_cache.data_ptr = NULL;
	data_cache.data_len = 0;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
IODirectIODriver::~IODirectIODriver() {
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::setDirectIOParam(IODirectIODriverInitParam& _init_parameter) {
	//store the configuration
	init_parameter = _init_parameter;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::init(void *_init_parameter) throw(CException) {
	IODataDriver::init(_init_parameter);
	
	IODirectIODriver_LAPP_ << "Check init parameter";
	
	if(!init_parameter.network_broker) throw CException(-1, "No network broker configured", __PRETTY_FUNCTION__);
	
	init_parameter.client_instance = init_parameter.network_broker->getDirectIOClientInstance();
	if(!init_parameter.client_instance) throw CException(-1, "No client configured", __PRETTY_FUNCTION__);
	
	init_parameter.endpoint_instance = init_parameter.network_broker->getDirectIOServerEndpoint();
	if(!init_parameter.endpoint_instance) throw CException(-1, "No endpoint configured", __PRETTY_FUNCTION__);
	
	//initialize client
	InizializableService::initImplementation(init_parameter.client_instance, _init_parameter, init_parameter.client_instance->getName(), __PRETTY_FUNCTION__);
	
	//get the client and server channel
	IODirectIODriver_LAPP_ << "Allcoate the default device server channel";
	device_server_channel = (chaos_dio_channel::DirectIODeviceServerChannel *)init_parameter.endpoint_instance->getNewChannelInstance("DirectIODeviceServerChannel");
	device_server_channel->setHandler(this);
	
	//store endpoint idnex for fast access
	current_endpoint_p_port = init_parameter.endpoint_instance->getPublicServerInterface()->getPriorityPort();
	current_endpoint_s_port = init_parameter.endpoint_instance->getPublicServerInterface()->getServicePort();
	current_endpoint_index = init_parameter.endpoint_instance->getRouteIndex();
	IODirectIODriver_LAPP_ << "Our receiving priority port is " << current_endpoint_p_port << " and enpoint is " <<current_endpoint_index;
	
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::deinit() throw(CException) {
	IODirectIODriver_LAPP_ << "Remove active query";
	//acquire write lock on map
	boost::unique_lock<boost::shared_mutex> wmap_loc(map_query_future_mutex);

	//scan all remained query
	for(std::map<string, QueryFuture*>::iterator it = map_query_future.begin();
		it != map_query_future.end();
		it++) {
		releaseQuery(it->second);
	}
	map_query_future.clear();
	
	if(data_cache.data_ptr) {
		IODirectIODriver_LAPP_ << "delete last data received";
		free(data_cache.data_ptr);
	}
	
	//remove all url and service
	IODirectIODriver_LAPP_ << "Remove all urls";
	connectionFeeder.clear();
	
	//initialize client
	InizializableService::deinitImplementation(init_parameter.client_instance, init_parameter.client_instance->getName(), __PRETTY_FUNCTION__);
	delete(init_parameter.client_instance);
	
	//deinitialize server channel
	if(device_server_channel) {
		init_parameter.endpoint_instance->releaseChannelInstance(device_server_channel);
	}
	
	if(init_parameter.endpoint_instance) {
		init_parameter.network_broker->releaseDirectIOServerEndpoint(init_parameter.endpoint_instance);
	}
	IODataDriver::deinit();
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::storeRawData(const std::string& key,
									chaos::common::data::SerializationBuffer *serialization,
									int store_hint)  throw(CException) {
	CHAOS_ASSERT(serialization)
	boost::shared_lock<boost::shared_mutex>(mutext_feeder);
	//if(next_client->connection->getState() == chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventConnected)
	IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
	serialization->disposeOnDelete = !next_client;
	if(next_client) {
		//free the packet
		serialization->disposeOnDelete = false;
		next_client->device_client_channel->storeAndCacheDataOutputChannel(key,
																		   (void*)serialization->getBufferPtr(),
																		   (uint32_t)serialization->getBufferLen(),
																		   (direct_io::channel::DirectIODeviceClientChannelPutMode)store_hint);
	} else {
		DEBUG_CODE(IODirectIODriver_DLDBG_ << "No available socket->loose packet");
	}
	delete(serialization);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
char* IODirectIODriver::retriveRawData(const std::string& key, size_t *dim)  throw(CException) {
	char* result = NULL;
	boost::shared_lock<boost::shared_mutex>(mutext_feeder);
	IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
	if(!next_client) return NULL;
	
	uint32_t size =0;
	next_client->device_client_channel->requestLastOutputData(key, (void**)&result, size);
	*dim = (size_t)size;
	return result;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
chaos::common::data::CDataWrapper* IODirectIODriver::updateConfiguration(chaos::common::data::CDataWrapper* newConfigration) {
	//lock the feeder access
	boost::unique_lock<boost::shared_mutex>(mutext_feeder);
	//checkif someone has passed us the device indetification
	if(newConfigration->hasKey(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS)){
		IODirectIODriver_LAPP_ << "Get the DataManager LiveData address value";
		auto_ptr<chaos::common::data::CMultiTypeDataArrayWrapper> liveMemAddrConfig(newConfigration->getVectorValue(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS));
		size_t numerbOfserverAddressConfigured = liveMemAddrConfig->size();
		for ( int idx = 0; idx < numerbOfserverAddressConfigured; idx++ ){
			string serverDesc = liveMemAddrConfig->getStringElementAtIndex(idx);
			if(!common::direct_io::DirectIOClient::checkURL(serverDesc)) {
				IODirectIODriver_DLDBG_ << "Data proxy server description " << serverDesc << " non well formed";
				continue;
			}
			//add new url to connection feeder
			connectionFeeder.addURL(chaos::common::network::URL(serverDesc));
		}
	}
	return NULL;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::disposeService(void *service_ptr) {
	if(!service_ptr) return;
	IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(service_ptr);
	next_client->connection->releaseChannelInstance(next_client->device_client_channel);
	init_parameter.client_instance->releaseConnection(next_client->connection);
	delete(next_client);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void* IODirectIODriver::serviceForURL(const common::network::URL& url, uint32_t service_index) {
	IODirectIODriver_DLDBG_ << "Add connection for " << url.getURL();
	IODirectIODriverClientChannels * clients_channel = NULL;
	chaos_direct_io::DirectIOClientConnection *tmp_connection = init_parameter.client_instance->getNewConnection(url.getURL());
	if(tmp_connection) {
		clients_channel = new IODirectIODriverClientChannels();
		clients_channel->connection = tmp_connection;
		clients_channel->device_client_channel = (chaos_dio_channel::DirectIODeviceClientChannel *)tmp_connection->getNewChannelInstance("DirectIODeviceClientChannel");
		if(!clients_channel->device_client_channel) {
			IODirectIODriver_DLDBG_ << "Error creating client device channel for " << url.getURL();
			init_parameter.client_instance->releaseConnection(tmp_connection);
			delete(clients_channel);
			return NULL;
		}
		
		//set the answer information
		clients_channel->device_client_channel->setAnswerServerInfo(current_endpoint_p_port, current_endpoint_s_port, current_endpoint_index);
		
		//set this driver instance as event handler for connection
		clients_channel->connection->setEventHandler(this);
		clients_channel->connection->setCustomStringIdentification(boost::lexical_cast<std::string>(service_index));
	} else {
		IODirectIODriver_DLDBG_ << "Error creating client connection for " << url.getURL();
	}
	return clients_channel;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::handleEvent(chaos_direct_io::DirectIOClientConnection *client_connection,
										   chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event) {
	//if the channel has bee disconnected turn the relative index offline, if onli reput it online
	boost::unique_lock<boost::shared_mutex>(mutext_feeder);
	uint32_t service_index = boost::lexical_cast<uint32_t>(client_connection->getCustomStringIdentification());
	DEBUG_CODE(IODirectIODriver_DLDBG_ << "Manage event for service with index " << service_index << " and url" << client_connection->getURL();)
	switch(event) {
		case chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventConnected:
			connectionFeeder.setURLOnline(service_index);
			break;
			
		case chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventDisconnected:
			connectionFeeder.setURLOffline(service_index);
			break;
	}
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
QueryFuture *IODirectIODriver::performQuery(const std::string& key,
											uint64_t start_ts,
											uint64_t end_ts) {
	IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
	if(!next_client) return NULL;
	
	QueryFuture *q = NULL;
	std::string query_id;
	if(!next_client->device_client_channel->queryDataCloud(key, start_ts, end_ts, query_id)) {
		//acquire write lock
		boost::unique_lock<boost::shared_mutex> wmap_loc(map_query_future_mutex);
		q = _getNewQueryFutureForQueryID(query_id);
		if(q) {
			//add query to map
			map_query_future.insert(make_pair<string, QueryFuture*>(query_id, q));
		} else {
			releaseQuery(q);
		}
	}
	return q;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODirectIODriver::releaseQuery(QueryFuture *query_future) {
	//acquire write lock
	boost::unique_lock<boost::shared_mutex> wmap_loc(map_query_future_mutex);
	if(map_query_future.count(query_future->getQueryID())) {
		map_query_future.erase(query_future->getQueryID());
	}
	//delete query
	IODataDriver::releaseQuery(query_future);
}


/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
int IODirectIODriver::consumeDataCloudQueryStartResult(chaos_dio_channel::opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloudStartResult *header) {
	
	//get the query id
	std::string query_id(header->field.query_id, 8);

	//acquire read lock
	boost::shared_lock<boost::shared_mutex> rmap_loc(map_query_future_mutex);

	//get qeury iterator on map
	std::map<string, QueryFuture*>::iterator it = map_query_future.find(query_id);
	if(it != map_query_future.end()) {
		//initialize the query for for the result receivement
		_startQueryFutureResult(*it->second, header->field.total_element_found);
	}
	return 0;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
int IODirectIODriver::consumeDataCloudQueryResult(chaos_dio_channel::opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloudResult *header,
												  void *data_found,
												  uint32_t data_lenght,
												  chaos_dio::DirectIOSynchronousAnswerPtr synchronous_answer){
	std::string query_id(header->field.query_id, 8);

	//acquire read lock
	boost::shared_lock<boost::shared_mutex> rmap_loc(map_query_future_mutex);
	
	//get qeury iterator on map
	std::map<string, QueryFuture*>::iterator it = map_query_future.find(query_id);
	if(it != map_query_future.end()) {
		try{
			chaos_data::CDataWrapper *data_pack = new chaos_data::CDataWrapper((char *)data_found);
			//we have map so we will add the new packet
			_pushResultToQueryFuture(*it->second, data_pack, header->field.element_index);
		}catch(...) {
			IODirectIODriver_LERR_ << "error parsing reuslt data pack";
		}
	}
	
	//deelte received data
	free(data_found);
	free(header);
	return 0;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
int IODirectIODriver::consumeDataCloudQueryEndResult(chaos_dio_channel::opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloudEndResult *header,
													 void *error_message_string_data,
													 uint32_t error_message_string_data_length) {
	//get the query id
	std::string query_id(header->field.query_id, 8);
	
	//acquire read lock
	boost::shared_lock<boost::shared_mutex> rmap_loc(map_query_future_mutex);
	
	//get qeury iterator on map
	std::map<string, QueryFuture*>::iterator it = map_query_future.find(query_id);
	if(it != map_query_future.end()) {
		if(error_message_string_data_length) {
			std::string error_message((char*)error_message_string_data, error_message_string_data_length);
			//initialize the query for for the result receivement
			_endQueryFutureResult(*it->second, header->field.error, error_message);
		} else {
			//initialize the query for for the result receivement
			_endQueryFutureResult(*it->second, header->field.error);
		}
		
	}

}