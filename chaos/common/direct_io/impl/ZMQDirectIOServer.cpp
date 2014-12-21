/*
 *	ZMQDirectIOServer.cpp
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

#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/direct_io/DirectIODataPack.h>
#include <chaos/common/direct_io/impl/ZMQDirectIOServer.h>
#include <boost/format.hpp>


#define ZMQDIO_SRV_LOG_HEAD "["<<getName()<<"] - "

#define ZMQDIO_SRV_LAPP_ LAPP_ << ZMQDIO_SRV_LOG_HEAD
#define ZMQDIO_SRV_LDBG_ LDBG_ << ZMQDIO_SRV_LOG_HEAD
#define ZMQDIO_SRV_LERR_ LERR_ << ZMQDIO_SRV_LOG_HEAD


#define DIRECTIO_FREE_ANSWER_DATA(x)\
if(x && x->answer_data) free(x->answer_data);\
if(x) free(x);\
x = NULL;


namespace chaos_data = chaos::common::data;

using namespace chaos::common::direct_io::impl;

void free_answer(void *data, void *hint) {
	chaos::common::direct_io::DirectIOSynchronousAnswerPtr answer_data = static_cast<chaos::common::direct_io::DirectIOSynchronousAnswerPtr>(hint);
	DIRECTIO_FREE_ANSWER_DATA(answer_data)
}

ZMQDirectIOServer::ZMQDirectIOServer(std::string alias):
DirectIOServer(alias),
zmq_context(NULL),
priority_socket(NULL),
service_socket(NULL),
run_server(false) {};

ZMQDirectIOServer::~ZMQDirectIOServer(){};

//! Initialize instance
void ZMQDirectIOServer::init(void *init_data) throw(chaos::CException) {
    DirectIOServer::init(init_data);
    chaos_data::CDataWrapper *init_cw = static_cast<chaos_data::CDataWrapper*>(init_data);
    if(!init_cw) throw chaos::CException(0, "No configration has been provided", __FUNCTION__);
	
	//get the port from configuration
	priority_port = init_cw->getInt32Value(common::direct_io::DirectIOConfigurationKey::DIRECT_IO_PRIORITY_PORT);
	if(priority_port <= 0) throw chaos::CException(0, "Bad priority port configured", __FUNCTION__);
	
	service_port = init_cw->getInt32Value(common::direct_io::DirectIOConfigurationKey::DIRECT_IO_SERVICE_PORT);
	if(service_port <= 0) throw chaos::CException(0, "Bad service port configured", __FUNCTION__);
	
	//create the endpoint strings
    priority_socket_bind_str = boost::str( boost::format("tcp://*:%1%") % priority_port);
    ZMQDIO_SRV_LAPP_ << "priority socket bind url: " << priority_socket_bind_str;
    
    service_socket_bind_str = boost::str( boost::format("tcp://*:%1%") % service_port);
    ZMQDIO_SRV_LAPP_ << "service socket bind url: " << service_socket_bind_str;
}

//! Start the implementation
void ZMQDirectIOServer::start() throw(chaos::CException) {
    DirectIOServer::start();
    run_server = true;
	
    //create the ZMQContext
    zmq_context = zmq_ctx_new();
    if(zmq_context == NULL) throw chaos::CException(0, "Error creating zmq context", __FUNCTION__);
    
    //et the thread number
    zmq_ctx_set(zmq_context, ZMQ_IO_THREADS, 2);
    ZMQDIO_SRV_LAPP_ << "ZMQ context created";
	
    //queue thread
    ZMQDIO_SRV_LAPP_ << "Allocating and binding priority socket to "<< priority_socket_bind_str;
    ZMQDIO_SRV_LAPP_ << "Allocating threads for manage the requests";
    server_threads_group.add_thread(new boost::thread(boost::bind(&ZMQDirectIOServer::worker, this, false)));
    server_threads_group.add_thread(new boost::thread(boost::bind(&ZMQDirectIOServer::worker, this, true)));
    ZMQDIO_SRV_LAPP_ << "Threads allocated and started";
}

//! Stop the implementation
void ZMQDirectIOServer::stop() throw(chaos::CException) {
    run_server = false;
	
    if(zmq_context) {
        ZMQDIO_SRV_LAPP_ << "Deallocating zmq context";
        zmq_ctx_shutdown(zmq_context);
        zmq_ctx_term(zmq_context);
        zmq_context = NULL;
        ZMQDIO_SRV_LAPP_ << "ZMQ Context deallocated";
    }
	
    //wiath all thread
	ZMQDIO_SRV_LAPP_ << "Join on all thread";
    server_threads_group.join_all();
    ZMQDIO_SRV_LAPP_ << "All thread stopped";
    DirectIOServer::stop();
}

//! Deinit the implementation
void ZMQDirectIOServer::deinit() throw(chaos::CException) {
    //serverThreadGroup.stopGroup(true);
	
    DirectIOServer::deinit();
}

#define PS_STR(x) (x?"service":"priority")
void ZMQDirectIOServer::worker(bool priority_service) {
	char						header_buffer[DIRECT_IO_HEADER_SIZE];
	int							linger				= 500;
	int							water_mark			= 500;
	int							timeout				= 200;
	void						*socket				= NULL;
    int							err					= 0;
	bool						send_synchronous_answer = false;
    DirectIODataPack			*data_pack			= NULL;
	DirectIOSynchronousAnswerPtr synchronous_answer = NULL;
	
	ZMQDIO_SRV_LAPP_ << "Startup thread for " << PS_STR(priority_service);
	
    //allcoate the delegate for this thread
    DirectIOHandlerPtr delegate = priority_service?&DirectIOHandler::serviceDataReceived:&DirectIOHandler::priorityDataReceived;
    ZMQDIO_SRV_LAPP_ << "Allocating and binding service socket to " << service_socket_bind_str;
    socket = zmq_socket (zmq_context, ZMQ_ROUTER);
    err = zmq_setsockopt (socket, ZMQ_LINGER, &linger, sizeof(int));
    if(err) {
        std::string msg = boost::str( boost::format("Error Setting linger to %1% socket") % PS_STR(priority_service));
        ZMQDIO_SRV_LAPP_ << msg;
        return;
    }
    err = zmq_setsockopt (socket, ZMQ_RCVHWM, &water_mark, sizeof(int));
    if(err) {
        std::string msg = boost::str( boost::format("Error Setting ZMQ_RCVHWM to %1% socket") % PS_STR(priority_service));
        ZMQDIO_SRV_LAPP_ << msg;
        return;
    }
    
    err = zmq_setsockopt (socket, ZMQ_SNDHWM, &water_mark, sizeof(int));
    if(err) {
        std::string msg = boost::str( boost::format("Error Setting ZMQ_SNDHWM to %1% socket") % PS_STR(priority_service));
        ZMQDIO_SRV_LAPP_ << msg;
        return;
    }
    
    err = zmq_setsockopt (socket, ZMQ_SNDTIMEO, &timeout, sizeof(int));
    if(err) {
        std::string msg = boost::str( boost::format("Error Setting ZMQ_SNDTIMEO to %1% socket") % PS_STR(priority_service));
        ZMQDIO_SRV_LAPP_ << msg;
        return;
    }
    
	if(priority_service) {
		err = zmq_bind(socket, priority_socket_bind_str.c_str());
		if(err) {
			std::string msg = boost::str( boost::format("Error binding service socket to  %1% ") % service_socket_bind_str);
			ZMQDIO_SRV_LAPP_ << msg;
			return;
		}
	} else {
		err = zmq_bind(socket, service_socket_bind_str.c_str());
		if(err) {
			std::string msg = boost::str( boost::format("Error binding priority socket to  %1% ") % priority_socket_bind_str);
			ZMQDIO_SRV_LAPP_ << msg;
			return;
		}
	}
	ZMQDIO_SRV_LAPP_ << "Entering in the thread loop for " << PS_STR(priority_service) << " socket";
    while (run_server) {
        try {
			//received the zmq identity
			std::string identity;
			std::string empty_delimiter;
			err = stringReceive(socket, identity);
			if(err == -1) {
                continue;
            }

			//receive the zmq evenlod delimiter
			err = stringReceive(socket, empty_delimiter);
			if(err == -1 ) {
                continue;
            }
			
			
            //read header
            err = zmq_recv(socket, header_buffer, DIRECT_IO_HEADER_SIZE, 0);
            if(err == -1 ||
			   err != DIRECT_IO_HEADER_SIZE) {
                continue;
            }

			//create new datapack
			data_pack = new DirectIODataPack();
			
			//clear all memory
			std::memset(data_pack, 0, sizeof(DirectIODataPack));
			
			//set dispatch header data
			data_pack->header.dispatcher_header.raw_data = DIRECT_IO_GET_DISPATCHER_DATA(header_buffer);
			
			//get the synchronous answer flag;
			send_synchronous_answer = data_pack->header.dispatcher_header.fields.synchronous_answer;
			
			//check what i need to reice
			switch(data_pack->header.dispatcher_header.fields.channel_part) {
				case DIRECT_IO_CHANNEL_PART_EMPTY:
					break;
				case DIRECT_IO_CHANNEL_PART_HEADER_ONLY:
					
					//init header data buffer
					data_pack->header.channel_header_size = DIRECT_IO_GET_CHANNEL_HEADER_SIZE(header_buffer);
					data_pack->channel_header_data = malloc(data_pack->header.channel_header_size);
					data_pack->channel_data = NULL;
                    
					//init message with buffer
					err = zmq_recv(socket, data_pack->channel_header_data, data_pack->header.channel_header_size, 0);
					//err = zmq_msg_recv(&m_header_data, socket, 0);
					if(err == -1) {
						free(data_pack->channel_header_data);
						delete data_pack;
						continue;
					}
					break;
				case DIRECT_IO_CHANNEL_PART_DATA_ONLY:
					
					//init data buffer
					data_pack->header.channel_data_size = DIRECT_IO_GET_CHANNEL_DATA_SIZE(header_buffer);
					data_pack->channel_data = malloc(data_pack->header.channel_data_size);
					data_pack->channel_header_data = NULL;
                    
					//init message with buffer
					err = zmq_recv(socket, data_pack->channel_data, data_pack->header.channel_data_size, 0);
					if(err == -1) {
						free(data_pack->channel_data);
						delete data_pack;
						continue;
					}
					break;
				case DIRECT_IO_CHANNEL_PART_HEADER_DATA:
					//init header data and channel data buffers
					data_pack->header.channel_header_size = DIRECT_IO_GET_CHANNEL_HEADER_SIZE(header_buffer);
					data_pack->channel_header_data = malloc(data_pack->header.channel_header_size);
					
					data_pack->header.channel_data_size = DIRECT_IO_GET_CHANNEL_DATA_SIZE(header_buffer);
					data_pack->channel_data = malloc(data_pack->header.channel_data_size);
					
					//reiceve all data
					err = zmq_recv(socket, data_pack->channel_header_data, data_pack->header.channel_header_size, 0);
					//err = zmq_msg_recv(&m_header_data, socket, 0);
					if(err == -1) {
						free(data_pack->channel_header_data);
						free(data_pack->channel_data);
						delete data_pack;
						continue;
					}
					err = zmq_recv(socket, data_pack->channel_data, data_pack->header.channel_data_size, 0);
					//err = zmq_msg_recv(&m_channel_data, socket, 0);
					if(err == -1) {
						free(data_pack->channel_header_data);
						free(data_pack->channel_data);
						delete data_pack;
						continue;
					}
					break;
			}
			
			//check if we need to send async answer
			if(data_pack->header.dispatcher_header.fields.synchronous_answer) {
				//the client waith an answer
				synchronous_answer = (DirectIOSynchronousAnswerPtr) calloc(sizeof(synchronous_answer), 1);
			}
			
			//dispatch to endpoint
			err = DirectIOHandlerPtrCaller(handler_impl, delegate)(data_pack, synchronous_answer);
			//check if we need to send async answer
			if(send_synchronous_answer) {
				//sending identity
				stringSendMore(socket, identity.c_str());

				//sending envelop delimiter
				stringSendMore(socket, EmptyMessage);
			
				//send the data
				zmq_msg_t answer_data;
				//construct answer zmq message
				err = zmq_msg_init_data (&answer_data,
										synchronous_answer->answer_data,
										synchronous_answer->answer_size,
										free_answer,
										synchronous_answer);
				
				if(err == -1) {
					ZMQDIO_SRV_LAPP_ << "Error creating message for asnwer";
					DIRECTIO_FREE_ANSWER_DATA(synchronous_answer)
				} else {
					err = zmq_sendmsg(socket, &answer_data, 0);
					if(err == -1) {
						ZMQDIO_SRV_LAPP_ << "Error sending answer";
						DIRECTIO_FREE_ANSWER_DATA(synchronous_answer)
					}
				}
				//close the message
				zmq_msg_close(&answer_data);
			}
			
			
        } catch (CException& ex) {
            DECODE_CHAOS_EXCEPTION(ex)
        }
    }
	ZMQDIO_SRV_LAPP_ << "Leaving the thread loop for " << (priority_service?"service":"priority") << " socket";
	ZMQDIO_SRV_LAPP_ << "Stopping priority socket";
    if(socket) {
		zmq_close(socket);
		socket = NULL;
	}
}
