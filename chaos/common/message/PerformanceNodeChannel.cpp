/*
 *	PerformanceNodeChannel.cpp
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

#include <chaos/common/message/PerformanceNodeChannel.h>
using namespace chaos::common::message;
namespace chaos_data = chaos::common::data;
namespace chaos_direct_io = chaos::common::direct_io;

//! base constructor
PerformanceNodeChannel::PerformanceNodeChannel(NetworkBroker *msg_broker, CNetworkAddress *node_network_address, chaos_direct_io::DirectIOClient *_client_instance):
MessageChannel(msg_broker), client_instance(_client_instance) {
	setRemoteNodeAddress(node_network_address->ipPort);
}

PerformanceNodeChannel::~PerformanceNodeChannel() {
	releasePerformanceSession(local_performance_session);
}

//Get the performance session for a chaos node
int PerformanceNodeChannel::getPerformanceSession(chaos_direct_io::DirectIOPerformanceSession **performance_session_handler,
												  uint32_t ms_timeout) {
    int err = chaos::ErrorCode::EC_NO_ERROR;
	if(!performance_session_handler) return  -100;
	
	//get the local endpoint
	chaos_direct_io::DirectIOServerEndpoint *local_session_endpoint = broker->getDirectIOServerEndpoint();
	if(!local_session_endpoint) return -101;
	
	std::string remote_endpoint_url;
	chaos_data::CDataWrapper init_performance_session_param;
	
	//set the init parameter
	init_performance_session_param.addStringValue(PerformanceSystemRpcKey::KEY_REQUEST_SERVER_DESCRITPION,
												  local_session_endpoint->getUrl());
	
	//sent the request and waith the ansewer for startp local session
	auto_ptr<chaos_data::CDataWrapper> init_session_result(MessageChannel::sendRequest(PerformanceSystemRpcKey::SYSTEM_PERFORMANCE_DOMAIN,
																					   PerformanceSystemRpcKey::ACTION_PERFORMANCE_INIT_SESSION,
																					   &init_performance_session_param,
																					   ms_timeout));
	CHECK_TIMEOUT_AND_RESULT_CODE(init_session_result, err)
	if(err == ErrorCode::EC_NO_ERROR) {
        auto_ptr<chaos_data::CDataWrapper> info_pack(init_session_result->getCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE));
        if(info_pack.get() && info_pack->hasKey(PerformanceSystemRpcKey::KEY_REQUEST_SERVER_DESCRITPION)){
			
			remote_endpoint_url = info_pack->getStringValue(PerformanceSystemRpcKey::KEY_REQUEST_SERVER_DESCRITPION);
            chaos_direct_io::DirectIOClientConnection *local_session_client_connection = client_instance->getNewConnection(remote_endpoint_url);
			if(!local_session_client_connection) {
				//i need to release the enpoint
				broker->releaseDirectIOServerEndpoint(local_session_endpoint);
			}
			
			// i can create session
			local_performance_session = *performance_session_handler = new chaos_direct_io::DirectIOPerformanceSession(local_session_client_connection, local_session_endpoint);
			if(!*performance_session_handler) {
				client_instance->releaseConnection(local_session_client_connection);
				//i need to release the enpoint
				broker->releaseDirectIOServerEndpoint(local_session_endpoint);
				
				err = -103;
			} else {
				try {
					chaos::utility::InizializableService::initImplementation(*performance_session_handler, NULL, "DirectIOPerformanceSession", __PRETTY_FUNCTION__);
				} catch(chaos::CException ex) {
					chaos::utility::InizializableService::deinitImplementation(*performance_session_handler,  "DirectIOPerformanceSession", __PRETTY_FUNCTION__);
					err = -104;
				}
			}
        }
    } else {
		//i need to release the enpoint
		broker->releaseDirectIOServerEndpoint(local_session_endpoint);
		err = -102;
	}
    return err;
}
//! release the performance session
int PerformanceNodeChannel::releasePerformanceSession(chaos_direct_io::DirectIOPerformanceSession *performance_session,
													  uint32_t ms_timeout) {
	chaos_data::CDataWrapper init_performance_session_param;
	if(!performance_session) return -1;
	if(local_performance_session != performance_session) return -1;
	
	try{
		//set the init parameter
		init_performance_session_param.addStringValue(PerformanceSystemRpcKey::KEY_REQUEST_SERVER_DESCRITPION,
													  performance_session->server_endpoint->getUrl());

		//sent the request and waith the ansewer for startp local session
		auto_ptr<chaos_data::CDataWrapper> init_session_result(MessageChannel::sendRequest(PerformanceSystemRpcKey::SYSTEM_PERFORMANCE_DOMAIN,
																						   PerformanceSystemRpcKey::ACTION_PERFORMANCE_CLOSE_SESSION,
																						   &init_performance_session_param,
																						   ms_timeout));

		
		chaos::utility::InizializableService::deinitImplementation(performance_session,  "DirectIOPerformanceSession", __PRETTY_FUNCTION__);
		if(performance_session->client_connection) client_instance->releaseConnection(performance_session->client_connection);
		//i need to release the enpoint
		if(performance_session->server_endpoint) broker->releaseDirectIOServerEndpoint(performance_session->server_endpoint);
		
	} catch(chaos::CException ex) {
		return -100;
	}
	delete(local_performance_session);
	local_performance_session = NULL;
	return 0;
}