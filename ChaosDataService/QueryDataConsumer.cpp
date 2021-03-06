/*
 *	QueryDataConsumer.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2014 INFN, National Institute of Nuclear Physics
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

#include "QueryDataConsumer.h"
#include "worker/DeviceSharedDataWorker.h"
#include "worker/SnapshotCreationWorker.h"

#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/utility/endianess.h>
#include <chaos/common/utility/InetUtility.h>

#include <boost/thread.hpp>

using namespace chaos::data_service;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;

#define QueryDataConsumer_LOG_HEAD "[QueryDataConsumer] - "

#define QDCAPP_ LAPP_ << QueryDataConsumer_LOG_HEAD
#define QDCDBG_ LDBG_ << QueryDataConsumer_LOG_HEAD << __FUNCTION__ << " - "
#define QDCERR_ LERR_ << QueryDataConsumer_LOG_HEAD << __FUNCTION__ << "(" << __LINE__ << ") - "

QueryDataConsumer::QueryDataConsumer(vfs::VFSManager *_vfs_manager_instance,
									 db_system::DBDriver *_db_driver):
vfs_manager_instance(_vfs_manager_instance),
db_driver(_db_driver),
query_engine(NULL){}

QueryDataConsumer::~QueryDataConsumer() {}

void QueryDataConsumer::init(void *init_data) throw (chaos::CException) {
	if(!settings)  throw chaos::CException(-1, "No setting provided", __FUNCTION__);
	if(!settings->cache_driver_impl.size())  throw chaos::CException(-2, "No cache implemetation provided", __FUNCTION__);
	if(!settings->startup_chache_servers.size())  throw chaos::CException(-3, "No cache servers provided", __FUNCTION__);
	
	//get new chaos direct io endpoint
	server_endpoint = network_broker->getDirectIOServerEndpoint();
	if(!server_endpoint) throw chaos::CException(-4, "Invalid server endpoint", __FUNCTION__);
	QDCAPP_ << "QueryDataConsumer initialized with endpoint "<< server_endpoint->getRouteIndex();
	
	QDCAPP_ << "Allocating DirectIODeviceServerChannel";
	device_channel = (DirectIODeviceServerChannel*)server_endpoint->getNewChannelInstance("DirectIODeviceServerChannel");
	if(!device_channel) throw chaos::CException(-5, "Error allocating device server channel", __FUNCTION__);
	device_channel->setHandler(this);
	
	QDCAPP_ << "Allocating DirectIOSystemAPIServerChannel";
	system_api_channel = (DirectIOSystemAPIServerChannel*)server_endpoint->getNewChannelInstance("DirectIOSystemAPIServerChannel");
	if(!system_api_channel) throw chaos::CException(-5, "Error allocating system api server channel", __FUNCTION__);
	system_api_channel->setHandler(this);
	
	//
	cache_impl_name = settings->cache_driver_impl;
	cache_impl_name.append("CacheDriver");
	QDCAPP_ << "The cache implementation to allocate is " << cache_impl_name;
	
	db_impl_name = settings->db_driver_impl;
	db_impl_name.append("DBDriver");
	QDCAPP_ << "The db implementation to allocate is " << db_impl_name;
	//device data worker instance
	device_data_worker = (chaos::data_service::worker::DataWorker**) malloc(sizeof(chaos::data_service::worker::DataWorker**) * settings->caching_worker_num);
	if(!device_data_worker) throw chaos::CException(-5, "Error allocating device workers", __FUNCTION__);
	
	//get the cached driver
	cache_driver = chaos::ObjectFactoryRegister<cache_system::CacheDriver>::getInstance()->getNewInstanceByName(cache_impl_name);
	
	//allocate query manager
	if(!settings->cache_only) {
		query_engine = new query_engine::QueryEngine(network_broker->getDirectIOClientInstance(),
													 settings->query_manager_thread_poll_size,
													 vfs_manager_instance);
		if(!query_engine) throw chaos::CException(-5, "Error allocating Query Engine", __FUNCTION__);
		chaos::utility::StartableService::initImplementation(query_engine, init_data, "QueryEngine", __PRETTY_FUNCTION__);
	}
	//Sahred data worker
	chaos::data_service::worker::DeviceSharedDataWorker *tmp = NULL;
	for(int idx = 0; idx < settings->caching_worker_num; idx++) {
		device_data_worker[idx] = (tmp = new chaos::data_service::worker::DeviceSharedDataWorker(cache_impl_name, vfs_manager_instance));
		tmp->init(&settings->caching_worker_setting);
		QDCAPP_ << "Configure server on device worker " << idx;
		for(CacheServerListIterator iter = settings->startup_chache_servers.begin();
			iter != settings->startup_chache_servers.end();
			iter++) {
			tmp->addServer(*iter);
		}
		tmp->updateServerConfiguration();
		tmp->start();
	}
	
	QDCAPP_ << "Allocating Snapshot worker";
	if(!settings->cache_only) {
		snapshot_data_worker = new chaos::data_service::worker::SnapshotCreationWorker(cache_impl_name,
																					   db_driver,
																					   network_broker);
		if(!snapshot_data_worker) throw chaos::CException(-5, "Error allocating snapshot worker", __FUNCTION__);
		chaos::utility::StartableService::initImplementation(snapshot_data_worker, init_data, "SnapshotCreationWorker", __PRETTY_FUNCTION__);
		for(CacheServerListIterator iter = settings->startup_chache_servers.begin();
			iter != settings->startup_chache_servers.end();
			iter++) {
			((chaos::data_service::worker::SnapshotCreationWorker*)snapshot_data_worker)->addServer(*iter);
		}
		((chaos::data_service::worker::SnapshotCreationWorker*)snapshot_data_worker)->updateServerConfiguration();
	}
	
	//add server to cache driver
	for(CacheServerListIterator iter = settings->startup_chache_servers.begin();
		iter != settings->startup_chache_servers.end();
		iter++) {
		cache_driver->addServer(*iter);
	}
	cache_driver->updateConfig();
	
	//start virtual file mantainers timer
	if(!settings->cache_only) {
		QDCAPP_ << "Start virtual file mantainers timer with a timeout of " << settings->vfile_mantainer_delay*1000 << "seconds";
		chaos::common::async_central::AsyncCentralManager::getInstance()->addTimer(this, 0, settings->vfile_mantainer_delay*1000);
	}
}

void QueryDataConsumer::start() throw (chaos::CException) {
	//start query engine
	if(query_engine) chaos::utility::StartableService::startImplementation(query_engine, "QueryEngine", __PRETTY_FUNCTION__);
	
	//! start the snapshot creation worker
	if(!settings->cache_only &&
	   snapshot_data_worker) {
		chaos::utility::StartableService::startImplementation(snapshot_data_worker, "SnapshotCreationWorker", __PRETTY_FUNCTION__);
	}
}

void QueryDataConsumer::stop() throw (chaos::CException) {
	//! stop the snapshot creation worker
	if(!settings->cache_only &&
	   snapshot_data_worker) {
		chaos::utility::StartableService::stopImplementation(snapshot_data_worker, "SnapshotCreationWorker", __PRETTY_FUNCTION__);
	}

	//stop query engine
	if(query_engine) chaos::utility::StartableService::stopImplementation(query_engine, "QueryEngine", __PRETTY_FUNCTION__);
}

void QueryDataConsumer::deinit() throw (chaos::CException) {
	if(!settings->cache_only) {
		QDCAPP_ << "Remove virtual file mantainers timer";
		chaos::common::async_central::AsyncCentralManager::getInstance()->removeTimer(this);
	}
	
	QDCAPP_ << "Release direct io device channel into the endpoint";
	server_endpoint->releaseChannelInstance(device_channel);
	
	if(query_engine) {
		chaos::utility::StartableService::deinitImplementation(query_engine, "QueryEngine", __PRETTY_FUNCTION__);
		DELETE_OBJ_POINTER(query_engine)
	}
	
	QDCAPP_ << "Deallocating device push data worker list";
	for(int idx = 0; idx < settings->caching_worker_num; idx++) {
		QDCAPP_ << "Release device worker "<< idx;
		device_data_worker[idx]->stop();
		device_data_worker[idx]->deinit();
		DELETE_OBJ_POINTER(device_data_worker[idx])
	}
	free(device_data_worker);
	
	//! deinit the snapshot creation worker
	if(!settings->cache_only &&
	   snapshot_data_worker) {
		try{
			chaos::utility::StartableService::deinitImplementation(snapshot_data_worker, "SnapshotCreationWorker", __PRETTY_FUNCTION__);
		} catch(...) {
		}
		QDCAPP_ << "Deallocating Snapshot data worker";
		delete(snapshot_data_worker);
		snapshot_data_worker = NULL;
	}
	
	//delete the cache driver
	if(cache_driver) {
		delete(cache_driver);
		cache_driver = NULL;
	}
}


//async central timer hook
void QueryDataConsumer::timeout() {
	for (int idx = 0; idx < settings->caching_worker_num; idx++) {
		device_data_worker[idx]->mantain();
	}
}

#pragma mark DirectIODeviceServerChannelHandler
int QueryDataConsumer::consumePutEvent(DirectIODeviceChannelHeaderPutOpcode *header,
									   void *channel_data,
									   uint32_t channel_data_len,
									   DirectIOSynchronousAnswerPtr synchronous_answer) {
	uint32_t index_to_use = device_data_worker_index++ % settings->caching_worker_num;
	chaos::data_service::worker::DeviceSharedWorkerJob *job = new chaos::data_service::worker::DeviceSharedWorkerJob();
	job->request_header = header;
	job->data_pack = channel_data;
	job->data_pack_len = channel_data_len;
	if(device_data_worker[index_to_use]->submitJobInfo(job)) {
		DEBUG_CODE(QDCDBG_ << "error pushing data into worker queue");
		delete job;
	}
	return 0;
}

int QueryDataConsumer::consumeDataCloudQuery(DirectIODeviceChannelHeaderOpcodeQueryDataCloud *header,
											 const std::string& search_key,
											 uint64_t search_start_ts,
											 uint64_t search_end_ts,
											 DirectIOSynchronousAnswerPtr synchronous_answer) {
	//compose the DirectIO endpoint where forward the answer
	std::string answer_server_description = boost::str(boost::format("%1%:%2%:%3%|%4%") %
													   UI64_TO_STRIP(header->field.address) %
													   header->field.p_port %
													   header->field.s_port %
													   header->field.endpoint);
	//compose the id of the query
	std::string query_id(header->field.query_id, 8);
	//execute the query
	query_engine->executeQuery(new query_engine::DataCloudQuery(query_id,
																db_system::DataPackIndexQuery(search_key,
																							  search_start_ts,
																							  search_end_ts,
																							  chaos::DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT),
																answer_server_description));
	//delete header and
	if(header) free(header);
	return 0;
}

int QueryDataConsumer::consumeGetEvent(DirectIODeviceChannelHeaderGetOpcode *header,
									   void *channel_data,
									   uint32_t channel_data_len,
									   DirectIOSynchronousAnswerPtr synchronous_answer) {
	int err = cache_driver->getData(channel_data,
								 channel_data_len,
								 &synchronous_answer->answer_data,
								 synchronous_answer->answer_size);
	if(channel_data) free(channel_data);
	if(header) free(header);
	return err;
}

#pragma mark DirectIOSystemAPIServerChannelHandler
// Manage the creation of a snapshot
int QueryDataConsumer::consumeNewSnapshotEvent(opcode_headers::DirectIOSystemAPIChannelOpcodeNDGSnapshotHeader *header,
											   void *concatenated_unique_id_memory,
											   uint32_t concatenated_unique_id_memory_size,
											   DirectIOSystemAPISnapshotResult *api_result) {
	int err = 0;
	//check if we can work
	if(settings->cache_only) {
		//data service is in cache only mode throw the error
		api_result->error = -1;
		std::strncpy(api_result->error_message, "Chaos Data Service is in cache only", 255);
		//delete header
		if(header) free(header);
		return 0;
	}
	
	//prepare and submit the job into worker
	chaos::data_service::worker::SnapshotCreationJob *job = new chaos::data_service::worker::SnapshotCreationJob();
	//copy snapshot name
	job->snapshot_name = header->field.snap_name;
	//! copy the vector in job configuration
	if(concatenated_unique_id_memory_size && concatenated_unique_id_memory) {
		job->concatenated_unique_id_memory = (char*)concatenated_unique_id_memory;
		job->concatenated_unique_id_memory_size = concatenated_unique_id_memory_size;
	}
	if((err = snapshot_data_worker->submitJobInfo(job))) {
		api_result->error = err;
		switch (err) {
			case 1: {
				//there is already a snapshot with same name managed tha other job
				std::strcpy(api_result->error_message, "There is already a snapshot with same name managed tha other job");
				break;
			}
			default:
				//other errors
				std::strcpy(api_result->error_message, "Error creating new snapshot");
				break;
		}
		DEBUG_CODE(QDCDBG_ << api_result->error_message << "[" << job->snapshot_name << "]");
		
		if(concatenated_unique_id_memory) free(concatenated_unique_id_memory);
		delete job;
	} else {
		api_result->error = 0;
		std::strcpy(api_result->error_message, "Creation submitted");
	}
	if(header) free(header);
	return 0;
}

// Manage the delete operation on an existing snapshot
int QueryDataConsumer::consumeDeleteSnapshotEvent(opcode_headers::DirectIOSystemAPIChannelOpcodeNDGSnapshotHeader *header,
												  DirectIOSystemAPISnapshotResult *api_result) {
	int err = 0;
	if(settings->cache_only) {
		//data service is in cache only mode throw the error
		api_result->error = -1;
		std::strncpy(api_result->error_message, "Chaos Data Service is in cache only", 255);
		//delete header
		if(header) free(header);
		return 0;
	}
	if((err = db_driver->snapshotDeleteWithName(header->field.snap_name))) {
		api_result->error = err;
		std::strcpy(api_result->error_message, "Error deleteing the snapshot");
		QDCERR_ << api_result->error_message << "->" << header->field.snap_name;
	} else {
		api_result->error = 0;
		std::strcpy(api_result->error_message, "Snapshot deleted");
	}
	return err;
}

// Return the dataset for a producerkey ona specific snapshot
int QueryDataConsumer::consumeGetDatasetSnapshotEvent(opcode_headers::DirectIOSystemAPIChannelOpcodeNDGSnapshotHeader *header,
													  const std::string& producer_id,
													  void **channel_found_data,
													  uint32_t& channel_found_data_length,
													  DirectIOSystemAPISnapshotResult *api_result) {
	int err = 0;
	std::string channel_type;
	if(settings->cache_only) {
		//data service is in cache only mode throw the error
		api_result->error = -1;
		std::strncpy(api_result->error_message, "Chaos Data Service is in cache only", 255);
		//delete header
		if(header) free(header);
		return -1;
	}
	
	//trduce int to postfix channel type
	switch(header->field.channel_type) {
		case 0:
			channel_type = DataPackPrefixID::OUTPUT_DATASE_PREFIX;
			break;
		case 1:
			channel_type = DataPackPrefixID::INPUT_DATASE_PREFIX;
			break;
		case 2:
			channel_type = DataPackPrefixID::CUSTOM_DATASE_PREFIX;
			break;
		case 3:
			channel_type = DataPackPrefixID::SYSTEM_DATASE_PREFIX;
			break;
	}
	
	if((err = db_driver->snapshotGetDatasetForProducerKey(header->field.snap_name,
														  producer_id,
														  channel_type,
														  channel_found_data,
														  channel_found_data_length))) {
		api_result->error = err;
		std::strcpy(api_result->error_message, "Error retriving the snapshoted dataaset for producer key");
		QDCERR_ << api_result->error_message << "[" << header->field.snap_name << "/" << producer_id<<"]";
	} else {
		if(*channel_found_data) {
			api_result->error = 0;
			std::strcpy(api_result->error_message, "Snapshot found");
		} else {
			api_result->error = -2;
			std::strcpy(api_result->error_message, "Channel data not found in snapshot");
			
		}
	}
	return err;
}
