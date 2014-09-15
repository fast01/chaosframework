//
//  MongoDBDriver.cpp
//  CHAOSFramework
//
//  Created by Claudio Bisegni on 28/03/14.
//  Copyright (c) 2014 INFN. All rights reserved.
//

#include "MongoDBDriver.h"
#include "MongoDBIndexCursor.h"

#include "../vfs/vfs.h"

#include <boost/format.hpp>

#include <mongo/client/dbclient.h>

#include <chaos/common/utility/TimingUtil.h>

using namespace chaos::data_service::vfs;
using namespace chaos::data_service::db_system;

namespace chaos_data = chaos::common::data;

#define MongoDBDriver_LOG_HEAD "[MongoDBDriver] - "
#define MDBID_LAPP_ LAPP_ << MongoDBDriver_LOG_HEAD
#define MDBID_LDBG_ LDBG_ << MongoDBDriver_LOG_HEAD << __FUNCTION__ << " - "
#define MDBID_LERR_ LERR_ << MongoDBDriver_LOG_HEAD << __FUNCTION__ << " - "


//!
MongoDBDriver::MongoDBDriver(std::string alias):DBDriver(alias) {}

//!
MongoDBDriver::~MongoDBDriver() {}

//! init
void MongoDBDriver::init(void *init_data) throw (chaos::CException) {
	DBDriver::init(init_data);
	int err = 0;
	std::string errmsg;
	std::string servers;
	chaos_data::CDataWrapper driver_custom_init;
	//allcoate ha pool class
	ha_connection_pool = new MongoDBHAConnectionManager(setting->servers, setting->key_value_custom_param);
	if(setting->key_value_custom_param.count("db")) {
		db_name = setting->key_value_custom_param["db"];
	} else {
		db_name = MONGO_DB_VFS_DB_NAME;
	}
	//setup index
	//db.vdomain.ensureIndex( { "vfs_domain":1, "vfs_unique_domain_code":1 } , { unique: true,  dropDups: true  } )
	//db.vfat.ensureIndex( { "vfs_path": 1, "vfs_domain":1 } , { unique: true,  dropDups: true  } )
	//db.runCommand( { shardCollection : "chaos_vfs.domains" , key : { domain_name: 1, domain_url:1 } , unique : true,  dropDups: true  } )
	
	//domain index
	mongo::BSONObj index_on_domain = BSON(MONGO_DB_FIELD_DOMAIN_NAME<<1<<MONGO_DB_FIELD_DOMAIN_UNIQUE_CODE<<1);
	err = ha_connection_pool->ensureIndex(db_name, MONGO_DB_VFS_DOMAINS_COLLECTION, index_on_domain, true, "", true);
	if(err) throw chaos::CException(-1, "Error creating domain collection index", __PRETTY_FUNCTION__);
	
	//domain url index
	index_on_domain = BSON(MONGO_DB_FIELD_DOMAIN_NAME<<1<<MONGO_DB_FIELD_DOMAIN_URL<<1);
	err = ha_connection_pool->ensureIndex(db_name, MONGO_DB_VFS_DOMAINS_URL_COLLECTION, index_on_domain, true, "", true);
	if(err) throw chaos::CException(-1, "Error creating domain urls collection index", __PRETTY_FUNCTION__);
	
	index_on_domain = BSON(MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH<<1<<MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN<<1<<MONGO_DB_FIELD_DATA_BLOCK_CREATION_TS<<1);
	err = ha_connection_pool->ensureIndex(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION, index_on_domain, true, "", true);
	if(err) throw chaos::CException(-1, "Error creating data block index", __PRETTY_FUNCTION__);
	
	index_on_domain = BSON(MONGO_DB_IDX_DATA_PACK_DID<<1<<MONGO_DB_IDX_DATA_PACK_ACQ_TS<<1);
	err = ha_connection_pool->ensureIndex(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION, index_on_domain, true, "", true);
	if(err) throw chaos::CException(-1, "Error creating data pack index collection index", __PRETTY_FUNCTION__);
}

//!deinit
void MongoDBDriver::deinit() throw (chaos::CException) {
	DBDriver::deinit();
	if(ha_connection_pool) delete(ha_connection_pool);
}

//! Register a new domain
int MongoDBDriver::vfsAddDomain(vfs::VFSDomain domain) {
	int err = 0;
	mongo::BSONObjBuilder domain_registration;
	mongo::BSONObjBuilder domain_url_registration;
	try {
		//try to add the domain
		domain_registration.append(MONGO_DB_FIELD_DOMAIN_NAME, domain.name);
		domain_registration.append(MONGO_DB_FIELD_DOMAIN_UNIQUE_CODE, domain.unique_code);
		mongo::BSONObj domain_insertation_obj = domain_registration.obj();
		err = ha_connection_pool->insert(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_DOMAINS_COLLECTION), domain_insertation_obj);
		if(err) {
			MDBID_LERR_ << "Error " << err << " creting domain entry";
			MDBID_LAPP_ << "Checking already stored domain info";
			mongo::BSONObj result;
			// no i can read the memoryzed unique code for my domain and see if it match
			err = ha_connection_pool->findOne(result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_DOMAINS_COLLECTION), domain_insertation_obj);
			if(err) {
				MDBID_LERR_ << "Error " << err << " retriving domain info";
			} else if(result.isEmpty()) {
				MDBID_LERR_ << "No domain found on index db --> somenthing is going bad on db";
				err = -1;
				return err;
			} else {
				MDBID_LAPP_ << "Checking domain unique code";
				MDBID_LAPP_ << "Local unique domain code->" << domain.unique_code;
				MDBID_LAPP_ << "Index db unique domaincode->" << result.getStringField(MONGO_DB_FIELD_DOMAIN_UNIQUE_CODE);
				if(domain.unique_code.compare(result.getStringField(MONGO_DB_FIELD_DOMAIN_UNIQUE_CODE))) {
					MDBID_LERR_ << "Unique code checking NOT PASSED";
					err = -2;
					return err;
				}
			}
		}
		//compose the insert
		domain_url_registration.append(MONGO_DB_FIELD_DOMAIN_NAME, domain.name);
		domain_url_registration.append(MONGO_DB_FIELD_DOMAIN_URL, domain.local_url);
		domain_url_registration.append(MONGO_DB_FIELD_DOMAIN_HB, mongo::Date_t(chaos::TimingUtil::getTimeStamp()));
		err = ha_connection_pool->insert(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_DOMAINS_URL_COLLECTION), domain_url_registration.obj());
		if(err) {
			if(err != 11000) {
				MDBID_LERR_ << "Error " << err << " creting domain entry";
			} else {
				err = vfsDomainHeartBeat(domain);
			}
		}
		
	} catch (const mongo::DBException &e) {
		MDBID_LERR_ << e.what();
		err = -5;
	}
	return err;
}

//! Give an heart beat for a domain
int MongoDBDriver::vfsDomainHeartBeat(vfs::VFSDomain domain) {
	int err = 0;
	mongo::BSONObjBuilder b_query;
	mongo::BSONObjBuilder b_update_filed;
	mongo::BSONObjBuilder b_update;
	try {
		//compose file search criteria
		b_query.append(MONGO_DB_FIELD_DOMAIN_NAME, domain.name);
		b_query.append(MONGO_DB_FIELD_DOMAIN_URL, domain.local_url);
		b_update_filed.append(MONGO_DB_FIELD_DOMAIN_HB, mongo::Date_t(chaos::TimingUtil::getTimeStamp()));
		b_update.append("$set", b_update_filed.obj());
		err = ha_connection_pool->update(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_DOMAINS_URL_COLLECTION), b_query.obj(), b_update.obj());
		if(err) {
			MDBID_LERR_ << "Error " << err << " updating the heartbeat for domain";
		}
		
	} catch (const mongo::DBException &e) {
		MDBID_LERR_ << e.what();
		err = -5;
	}
	return err;
}

//! Register a new data block wrote on stage area
int MongoDBDriver::vfsAddNewDataBlock(chaos_vfs::VFSFile *vfs_file,
										   chaos_vfs::DataBlock *data_block,
										   vfs::data_block_state::DataBlockState new_block_state) {
	int err = 0;
	bool f_exists = false;
	
	if(vfsFileExist(vfs_file, f_exists)) {
		MDBID_LERR_ << "Error checking file description of vfat";
		return -1;
	}
	
	if(!f_exists) {
		if(vfsCreateFileEntry(vfs_file)) {
			MDBID_LERR_ << "Error creating file description of vfat";
			return -2;
		}
	}
	
	//allocate data block on vfat
	mongo::BSONObjBuilder bson_search;
	mongo::BSONObjBuilder bson_block;
	
	mongo::BSONObjBuilder search_id;
	mongo::BSONObjBuilder search_id_return_field;
	
	mongo::BSONObj search_result;
	try {
		bson_search.append(MONGO_DB_FIELD_FILE_VFS_PATH, vfs_file->getVFSFileInfo()->vfs_fpath);
		bson_search.append(MONGO_DB_FIELD_FILE_VFS_DOMAIN, vfs_file->getVFSFileInfo()->vfs_domain);
		ha_connection_pool->findOne(search_result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), bson_search.obj());
		if(search_result.isEmpty()) {
			//cant be here..anyway give error
			MDBID_LERR_ << "Error getting file information";
			return -4;
		}
		
		//insert new
		bson_block.append(MONGO_DB_FIELD_FILE_PRIMARY_KEY, search_result["_id"].OID());
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_STATE, new_block_state);
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_CREATION_TS, mongo::Date_t(data_block->creation_time));
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_VALID_UNTIL_TS, mongo::Date_t(data_block->invalidation_timestamp));
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_MAX_BLOCK_SIZE, (long long)data_block->max_reacheable_size);
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH, data_block->vfs_path);
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN, (data_block->vfs_domain = vfs_file->getVFSFileInfo()->vfs_domain));
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_CURRENT_WORK_POSITION, (long long)0);
		bson_block.append(MONGO_DB_FIELD_DATA_BLOCK_HB, mongo::Date_t(chaos::TimingUtil::getTimeStamp()));
		
		ha_connection_pool->insert(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION),  bson_block.obj());
		
		//collecte the unique id of the datablock
		search_id << MONGO_DB_FIELD_FILE_VFS_PATH << data_block->vfs_path;
		search_id << MONGO_DB_FIELD_FILE_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		
		//set the result needed field
		search_id_return_field << "_id" << 1;
		mongo::BSONObj o = search_id_return_field.obj();
		
		ha_connection_pool->findOne(search_result,
									MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION),
									search_id.obj(),
									&o);
		
		if(search_result.isEmpty()) {
			//cant be here..anyway give error
			MDBID_LERR_ << "Error getting file information";
			return -5;
		}
		
		//retrive the id
		data_block->index_driver_uinique_id = search_result["_id"].OID().toString();
	} catch (const mongo::DBException &e) {
		MDBID_LERR_ << e.what();
		err = -5;
	}
	return err;
}

//! Delete a virtual file datablock
int MongoDBDriver::vfsDeleteDataBlock(chaos_vfs::VFSFile *vfs_file,
										   chaos_vfs::DataBlock *data_block) {
	int err = 0;
	mongo::BSONObjBuilder file_search;
	mongo::BSONObj file_search_result;
	mongo::BSONObjBuilder data_block_search;
	try{
		//add default index information
		file_search << MONGO_DB_FIELD_FILE_VFS_PATH << vfs_file->getVFSFileInfo()->vfs_fpath;
		file_search << MONGO_DB_FIELD_FILE_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		
		ha_connection_pool->findOne(file_search_result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), file_search.obj());
		if(file_search_result.isEmpty()) {
			//cant be here..anyway give error
			MDBID_LERR_ << "Error getting file information";
			return -4;
		}
		
		//insert new
		data_block_search << MONGO_DB_FIELD_FILE_PRIMARY_KEY << file_search_result["_id"].OID();
		data_block_search << MONGO_DB_FIELD_FILE_VFS_PATH << data_block->vfs_path;
		data_block_search << MONGO_DB_FIELD_FILE_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		
		mongo::BSONObj q = data_block_search.obj();
		DEBUG_CODE(MDBID_LDBG_ << "vfsDeleteDataBlock delete ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "query: " << q.jsonString());
		DEBUG_CODE(MDBID_LDBG_ << "vfsDeleteDataBlock delete ---------------------------------------------";)
		
		err = ha_connection_pool->remove(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION), q);
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! Set the state for a stage datablock
int MongoDBDriver::vfsSetStateOnDataBlock(chaos_vfs::VFSFile *vfs_file,
											   chaos_vfs::DataBlock *data_block,
											   int state) {
	int err = 0;
	mongo::BSONObjBuilder bson_search;
	mongo::BSONObjBuilder bson_block_query;
	mongo::BSONObjBuilder bson_block_update;
	mongo::BSONObjBuilder bson_block_update_filed;
	mongo::BSONObj search_result;
	try{
		bson_search.append(MONGO_DB_FIELD_FILE_VFS_PATH, vfs_file->getVFSFileInfo()->vfs_fpath);
		bson_search.append(MONGO_DB_FIELD_FILE_VFS_DOMAIN, vfs_file->getVFSFileInfo()->vfs_domain);
		err = ha_connection_pool->findOne(search_result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), bson_search.obj());
		
		if(err) {
			MDBID_LERR_ << "Error " << err << " searching vfs on vfat";
			return err;
		}
		if(search_result.isEmpty()) {
			//cant be here..anyway give error
			MDBID_LERR_ << "[CAN'T BE HERE]Error getting file information";
			
		}
		
		//compose query
		bson_block_query.append(MONGO_DB_FIELD_FILE_PRIMARY_KEY, search_result["_id"].OID());
		bson_block_query.append(MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH, data_block->vfs_path);
		bson_block_query.append(MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN, vfs_file->getVFSFileInfo()->vfs_domain);
		
		//compose udpate
		bson_block_update_filed.append(MONGO_DB_FIELD_DATA_BLOCK_STATE, state);
		bson_block_update.append("$set", bson_block_update_filed.obj());
		
		err = ha_connection_pool->update(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION), bson_block_query.obj(), bson_block_update.obj());
		if(err) {
			MDBID_LERR_ << "Error " << err << " updating state on datablock";
		}
	}catch(const mongo::DBException &e) {
		MDBID_LERR_ << e.what();
		err = -2;
	}
	return err;
}

//! Set the state for a stage datablock
int MongoDBDriver::vfsSetStateOnDataBlock(chaos_vfs::VFSFile *vfs_file,
											   chaos_vfs::DataBlock *data_block,
											   int cur_state,
											   int new_state,
											   bool& success) {
	int err = 0;
	mongo::BSONObjBuilder command;
	mongo::BSONObjBuilder query_master;
	mongo::BSONObj result;
	success = false;
	try{
		//domain search
		query_master << MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		//path search
		query_master << MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH << data_block->vfs_path;
		//query on state
		query_master << MONGO_DB_FIELD_DATA_BLOCK_STATE << cur_state;
		
		//start the find and modify command
		command << "findAndModify" << MONGO_DB_VFS_VBLOCK_COLLECTION;
		//compose file search criteria
		command << "query" << query_master.obj();
		//set the atomic update on the tate
		command << "update" << BSON("$set" << BSON( MONGO_DB_FIELD_DATA_BLOCK_STATE << new_state) );
		
		mongo::BSONObj q = command.obj();
		DEBUG_CODE(MDBID_LDBG_ << "vfsSetStateOnDataBlock query ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "Query: "  << q.jsonString();)
		DEBUG_CODE(MDBID_LDBG_ << "vfsSetStateOnDataBlock query ---------------------------------------------";)

		
		err = ha_connection_pool->runCommand(result, db_name, q);
		if(err) {
			MDBID_LERR_ << "Error " << err << " vfsSetStateOnDataBlock";
		} else if(result.isEmpty()){
			MDBID_LERR_ << "No datablock found for the criteria";
			err = -1;
		} else {
			success = true;
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! Set the datablock current position
int MongoDBDriver::vfsSetHeartbeatOnDatablock(chaos_vfs::VFSFile *vfs_file,
											  chaos_vfs::DataBlock *data_block,
											  uint64_t timestamp) {
	int err = 0;
	mongo::BSONObjBuilder bson_block_query;
	mongo::BSONObjBuilder bson_block_update;
	try{
		bson_block_query << MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH << data_block->vfs_path;
		bson_block_query << MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		
		//compose udpate
		bson_block_update << "$set"<< BSON(MONGO_DB_FIELD_DATA_BLOCK_HB << mongo::Date_t(((timestamp == 0)?chaos::TimingUtil::getTimeStamp():timestamp)));
		
		//for heart beat we use unsecure write
		mongo::BSONObj q = bson_block_query.obj();
		mongo::WriteConcern wc = mongo::WriteConcern::unacknowledged;
		err = ha_connection_pool->update(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION), q, bson_block_update.obj(), &wc);
		DEBUG_CODE(MDBID_LDBG_ << "vfsSetHeartbeatOnDatablock query ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "Query: "  << q.jsonString();)
		DEBUG_CODE(MDBID_LDBG_ << "vfsSetHeartbeatOnDatablock query ---------------------------------------------";)

		if(err) {
			MDBID_LERR_ << "Error " << err << " updating state on datablock";
		}
	}catch(const mongo::DBException &e) {
		MDBID_LERR_ << e.what();
		err = -2;
	}
	return err;
}

//! Set the datablock current position
int MongoDBDriver::vfsUpdateDatablockCurrentWorkPosition(chaos_vfs::VFSFile *vfs_file,
														 chaos_vfs::DataBlock *data_block) {
	int err = 0;
	mongo::BSONObjBuilder bson_search;
	mongo::BSONObjBuilder bson_block_query;
	mongo::BSONObjBuilder bson_block_update;
	mongo::BSONObj search_result;
	try{
		bson_search.append(MONGO_DB_FIELD_FILE_VFS_PATH, vfs_file->getVFSFileInfo()->vfs_fpath);
		bson_search.append(MONGO_DB_FIELD_FILE_VFS_DOMAIN, vfs_file->getVFSFileInfo()->vfs_domain);
		err = ha_connection_pool->findOne(search_result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), bson_search.obj());
		
		if(err) {
			MDBID_LERR_ << "Error " << err << " searching vfs on vfat";
			return err;
		}
		if(search_result.isEmpty()) {
			//cant be here..anyway give error
			MDBID_LERR_ << "[CAN'T BE HERE]Error getting file information";
			
		}
		
		//compose query
		bson_block_query << MONGO_DB_FIELD_FILE_PRIMARY_KEY << search_result["_id"].OID();
		bson_block_query << MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH << data_block->vfs_path;
		bson_block_query << MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		
		//compose udpate
		bson_block_update << "$set"<< BSON(MONGO_DB_FIELD_DATA_BLOCK_CURRENT_WORK_POSITION << (long long)data_block->current_work_position);
		
		err = ha_connection_pool->update(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION), bson_block_query.obj(), bson_block_update.obj());
		if(err) {
			MDBID_LERR_ << "Error " << err << " updating state on datablock";
		}
	}catch(const mongo::DBException &e) {
		MDBID_LERR_ << e.what();
		err = -2;
	}
	return err;
}

//! fill a complete datablock
chaos_vfs::DataBlock *MongoDBDriver::fillDatablock(const mongo::BSONObj& full_datablock_query_result, chaos_vfs::DataBlock *data_block) {
	if(!data_block) return data_block;
	
	//set the field
	(*data_block).index_driver_uinique_id = full_datablock_query_result["_id"].OID().toString();
	(*data_block).creation_time = full_datablock_query_result.getField(MONGO_DB_FIELD_DATA_BLOCK_CREATION_TS).numberLong();
	(*data_block).invalidation_timestamp = full_datablock_query_result.getField(MONGO_DB_FIELD_DATA_BLOCK_VALID_UNTIL_TS).numberLong();
	(*data_block).max_reacheable_size = full_datablock_query_result.getField(MONGO_DB_FIELD_DATA_BLOCK_MAX_BLOCK_SIZE).numberLong();
	(*data_block).vfs_domain = full_datablock_query_result.getField(MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN).String();
	(*data_block).vfs_path = full_datablock_query_result.getField(MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH).String();
	return data_block;
}

//! Return the next available datablock created since timestamp
int MongoDBDriver::vfsFindSinceTimeDataBlock(chaos_vfs::VFSFile *vfs_file,
											 uint64_t timestamp,
											 bool direction,
											 int state,
											 chaos_vfs::DataBlock **data_block) {
	int err = 0;
	mongo::BSONObjBuilder query_master;
	mongo::BSONObj result;
	try{
		//search for domain
		query_master << MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN << vfs_file->getVFSFileInfo()->vfs_domain;
		
		//search on file path, the datablock is always the end token of the path
		
		query_master << MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH << BSON("$regex" << boost::str(boost::format("%1%%2%") % vfs_file->getVFSFileInfo()->vfs_fpath % ".*"));

		//search for state
		query_master << MONGO_DB_FIELD_DATA_BLOCK_STATE << state;
		//search on the timestamp
		query_master << MONGO_DB_FIELD_DATA_BLOCK_CREATION_TS << mongo::BSONObjBuilder().appendDate("$gt", timestamp).obj();
		
		mongo::BSONObj q = query_master.obj();
		DEBUG_CODE(MDBID_LDBG_ << "vfsFindSinceTimeDataBlock query ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "Query: "  << q.jsonString();)
		DEBUG_CODE(MDBID_LDBG_ << "vfsFindSinceTimeDataBlock query ---------------------------------------------";)

		
		err = ha_connection_pool->findOne(result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION), q);
		if(err) {
			MDBID_LERR_ << "Error " << err << " creting vfs file entry";
		} else if(result.isEmpty()){
			MDBID_LERR_ << "No datablock found for the criteria";
		} else {
			// read the block element
			
			//get new empty datablock
			*data_block = fillDatablock(result, new chaos_vfs::DataBlock());
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! Return the datablock identified by path
int MongoDBDriver::vfsFindFromPathDataBlock(const std::string& data_block_domain,
											const std::string& data_block_path,
											chaos_vfs::DataBlock **data_block) {
	int err = 0;
	mongo::BSONObj result;
	mongo::BSONObjBuilder query_data_blocks;
	try{
		//search for domain
		query_data_blocks << MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN << data_block_domain;
		
		//search on file path, the datablock is always the end token of the path
		
		query_data_blocks << MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH << data_block_path;
		
		//search for state
		query_data_blocks << MONGO_DB_FIELD_DATA_BLOCK_STATE << chaos_vfs::data_block_state::DataBlockStateQuerable;
		
		mongo::BSONObj q = query_data_blocks.obj();
		DEBUG_CODE(MDBID_LDBG_ << "vfsFindFromPathDataBlock query ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "Query: "  << q.jsonString();)
		DEBUG_CODE(MDBID_LDBG_ << "vfsFindFromPathDataBlock query ---------------------------------------------";)
		
		if((err = ha_connection_pool->findOne(result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VBLOCK_COLLECTION), q))) {
			MDBID_LERR_ << "Error " << err << " creting vfs file entry";
		} else if(result.isEmpty()){
			MDBID_LERR_ << "No datablock found for the criteria";
		} else {
			// read the block element
			
			//get new empty datablock
			*data_block = fillDatablock(result, new chaos_vfs::DataBlock());
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! Check if the vfs file exists
int MongoDBDriver::vfsFileExist(VFSFile *vfs_file, bool& exists_flag) {
	int err = 0;
	mongo::BSONObjBuilder b;
	mongo::BSONObj result;
	try{
		//compose file search criteria
		b.append(MONGO_DB_FIELD_FILE_VFS_PATH, vfs_file->getVFSFileInfo()->vfs_fpath);
		b.append(MONGO_DB_FIELD_FILE_VFS_DOMAIN, vfs_file->getVFSFileInfo()->vfs_domain);
		
		err = ha_connection_pool->findOne(result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), b.obj());
		if(err) {
			MDBID_LERR_ << "Error " << err << " creting vfs file entry";
		} else {
			exists_flag = !result.isEmpty();
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! Create a file entry into the vfat
int MongoDBDriver::vfsCreateFileEntry(chaos_vfs::VFSFile *vfs_file) {
	int err = 0;
	mongo::BSONObjBuilder b;
	try{
		//compose file search criteria
		b.append(MONGO_DB_FIELD_FILE_VFS_PATH, vfs_file->getVFSFileInfo()->vfs_fpath);
		b.append(MONGO_DB_FIELD_FILE_VFS_DOMAIN, vfs_file->getVFSFileInfo()->vfs_domain);
		
		err = ha_connection_pool->insert(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), b.obj());
		if(err) {
			if(err != 11000) {
				MDBID_LERR_ << "Error " << err << " creting vfs file entry";
			} else {
				err = 0;
			}
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! Return a list of vfs path of the file belong to a domain
int MongoDBDriver::vfsGetFilePathForDomain(std::string vfs_domain, std::string prefix_filter, std::vector<std::string>& result_vfs_file_path, int limit_to_size) {
	int err = 0;
	mongo::BSONObjBuilder query_master;
	mongo::BSONObjBuilder return_field;
	std::vector<mongo::BSONObj> results;
	try{
		
		//after the prefix we need to add the regex
		prefix_filter.append(".*");
		
		query_master << "$query" << BSON(MONGO_DB_FIELD_FILE_VFS_DOMAIN << vfs_domain << MONGO_DB_FIELD_FILE_VFS_PATH << BSON("$regex" << prefix_filter))
					<< "$orderby" << BSON(MONGO_DB_FIELD_DATA_BLOCK_CREATION_TS << 1);
		//search for domain
		//query_master << MONGO_DB_FIELD_FILE_VFS_DOMAIN << vfs_domain;

		
		//query_master << MONGO_DB_FIELD_FILE_VFS_PATH << BSON("$regex" << prefix_filter);
		
		return_field << MONGO_DB_FIELD_FILE_VFS_PATH << 1 << "_id" << 0;
		
		mongo::BSONObj q_obj = query_master.obj();
		mongo::BSONObj f_obj = return_field.obj();
		
		DEBUG_CODE(MDBID_LDBG_ << "vfsGetFilePathForDomain query ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "Domain: " << vfs_domain;)
		DEBUG_CODE(MDBID_LDBG_ << "Prefix: " << prefix_filter;)
		DEBUG_CODE(MDBID_LDBG_ << "Query: "  << q_obj.jsonString();)
		DEBUG_CODE(MDBID_LDBG_ << "Selected Fileds: "  << f_obj.jsonString();)
		DEBUG_CODE(MDBID_LDBG_ << "vfsGetFilePathForDomain query ---------------------------------------------";)
		
		ha_connection_pool->findN(results, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_VFS_VFAT_COLLECTION), q_obj, limit_to_size, 0, &f_obj);
		for(std::vector<mongo::BSONObj>::iterator it = results.begin();
			it != results.end();
			it++) {
			result_vfs_file_path.push_back(it->getField(MONGO_DB_FIELD_FILE_VFS_PATH).String());
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! add the default index for a unique instrument identification and a timestamp
int MongoDBDriver::idxAddDataPackIndex(const DataPackIndex& index) {
	int err = 0;
	mongo::BSONObjBuilder index_builder;
	mongo::BSONObjBuilder index_data_block_info_builder;
	try{
		//add default index information
		index_builder << MONGO_DB_IDX_DATA_PACK_DID << index.did;
		index_builder << MONGO_DB_IDX_DATA_PACK_ACQ_TS << mongo::Date_t(index.acquisition_ts);
		index_builder << MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC << (long long)index.acquisition_ts;
		index_builder << MONGO_DB_IDX_DATA_PACK_DATA_BLOCK_DST_DOMAIN << getDataBlockFromFileLocation(index.dst_location)->vfs_domain;
		index_builder << MONGO_DB_IDX_DATA_PACK_DATA_BLOCK_DST_PATH << getDataBlockFromFileLocation(index.dst_location)->vfs_path;
		index_builder << MONGO_DB_IDX_DATA_PACK_DATA_BLOCK_DST_OFFSET << (long long)getDataBlockOffsetFromFileLocation(index.dst_location);
		index_builder << MONGO_DB_IDX_DATA_PACK_SIZE << (int32_t)index.datapack_size;
		
		DEBUG_CODE(MDBID_LDBG_ << "idxAddDataPackIndex insert ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "did: " << index.did;)
		DEBUG_CODE(MDBID_LDBG_ << "acq_ts_numeric: " << index.acquisition_ts;)
		DEBUG_CODE(MDBID_LDBG_ << "data_pack: " << getDataBlockFromFileLocation(index.dst_location)->vfs_path;)
		DEBUG_CODE(MDBID_LDBG_ << "data_pack_offset: " << getDataBlockOffsetFromFileLocation(index.dst_location);)
		DEBUG_CODE(MDBID_LDBG_ << "idxAddDataPackIndex insert ---------------------------------------------";)
		
		err = ha_connection_pool->insert(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION), index_builder.obj());
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = e.getCode();
	}
	return err;
}

//! add the default index for a unique instrument identification and a timestamp
int MongoDBDriver::idxDeleteDataPackIndex(const DataPackIndex& index) {
	int err = 0;
	mongo::BSONObjBuilder index_search_builder;
	try{
		//add default index information
		index_search_builder << MONGO_DB_IDX_DATA_PACK_DID << index.did;
		index_search_builder << MONGO_DB_IDX_DATA_PACK_ACQ_TS << mongo::Date_t(index.acquisition_ts);
		mongo::BSONObj q = index_search_builder.obj();
		DEBUG_CODE(MDBID_LDBG_ << "idxDeleteDataPackIndex insert ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "query: " << q.jsonString());
		DEBUG_CODE(MDBID_LDBG_ << "idxDeleteDataPackIndex insert ---------------------------------------------";)
		
		err = ha_connection_pool->remove(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION), q);
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! perform a search on data pack indexes
int MongoDBDriver::idxStartSearchDataPack(const DataPackIndexQuery& data_pack_index_query, DBIndexCursor **index_cursor) {
	int err=-1;
	*index_cursor = new MongoDBIndexCursor(this, data_pack_index_query);
	if(*index_cursor) {
		err = 0;
		((MongoDBIndexCursor*)*index_cursor)->computeTimeLapsForPage();
	}
	return err;
}

//-------------------------- protected method------------------------------
//! protected methdo that perform the real paged query on index called by the cursor
int MongoDBDriver::idxSearchResultCountDataPack(const DataPackIndexQuery& data_pack_index_query,  uint64_t& num_of_result) {
	int err = 0;
	mongo::BSONObjBuilder	index_search_builder;
	mongo::BSONObjBuilder	return_field;
	try{
		//add default index information
		index_search_builder << MONGO_DB_IDX_DATA_PACK_DID << data_pack_index_query.did;
		index_search_builder << MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC << BSON("$gte" << (long long)data_pack_index_query.start_ts <<
																	  "$lte" << (long long)data_pack_index_query.end_ts);
		
		mongo::BSONObj q = index_search_builder.obj();
		DEBUG_CODE(MDBID_LDBG_ << "idxSearchResultCountDataPack insert ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "query: " << q.jsonString());
		DEBUG_CODE(MDBID_LDBG_ << "idxSearchResultCountDataPack insert ---------------------------------------------";)
		
		mongo::BSONObj r = return_field.obj();
		err = ha_connection_pool->count(num_of_result, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION), q);
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! perform a search on data pack indexes
int MongoDBDriver::idxSearchDataPack(const DataPackIndexQuery& data_pack_index_query, std::auto_ptr<mongo::DBClientCursor>& cursor) {
	int err = 0;
	mongo::BSONObjBuilder	index_search_builder;
	mongo::BSONObjBuilder	return_field;
	try{
		//add default index information
		index_search_builder << MONGO_DB_IDX_DATA_PACK_DID << data_pack_index_query.did;
		index_search_builder << MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC << BSON("$gte" << (long long)data_pack_index_query.start_ts <<
																			  "$lte" << (long long)data_pack_index_query.end_ts);
		
		return_field << MONGO_DB_IDX_DATA_PACK_DATA_BLOCK_DST_DOMAIN << 1
					 << MONGO_DB_IDX_DATA_PACK_DATA_BLOCK_DST_PATH << 1
					 << MONGO_DB_IDX_DATA_PACK_DATA_BLOCK_DST_OFFSET << 1
					 << MONGO_DB_IDX_DATA_PACK_SIZE << 1;
		
		mongo::BSONObj q = index_search_builder.obj();
		DEBUG_CODE(MDBID_LDBG_ << "idxDeleteDataPackIndex insert ---------------------------------------------";)
		DEBUG_CODE(MDBID_LDBG_ << "query: " << q.jsonString());
		DEBUG_CODE(MDBID_LDBG_ << "idxDeleteDataPackIndex insert ---------------------------------------------";)
		
		mongo::BSONObj r = return_field.obj();
		cursor = ha_connection_pool->query(MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION), q, 0, 0, &r);
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}

//! protected methdo that perform the real paged query on index called by the cursor
int MongoDBDriver::idxMaxAndMInimumTimeStampForDataPack(const DataPackIndexQuery & data_pack_index_query, uint64_t& min_ts, uint64_t& max_ts) {
	int err = 0;
	
	mongo::BSONObj r_max;
	mongo::BSONObj r_min;
	mongo::BSONObjBuilder	return_field;
	try{

		return_field << MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC << 1;
		
		mongo::BSONObj p = return_field.obj();
		if((err = ha_connection_pool->findOne(r_max, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION), BSON("$orderby"<<BSON(MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC<<-1)), &p))){
			MDBID_LDBG_ << "Error getting maximum timestamp on index of " << data_pack_index_query.did;
		} else if((err = ha_connection_pool->findOne(r_min, MONGO_DB_COLLECTION_NAME(db_name, MONGO_DB_IDX_DATA_PACK_COLLECTION), BSON("$orderby"<<BSON(MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC<<1)), &p))) {
			MDBID_LDBG_ << "Error getting minimum timestamp on index of " << data_pack_index_query.did;
		} else {
			min_ts = (uint64_t)r_min.getField(MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC).Long();
			max_ts = (uint64_t)r_max.getField(MONGO_DB_IDX_DATA_PACK_ACQ_TS_NUMERIC).Long();
		}
	} catch( const mongo::DBException &e ) {
		MDBID_LERR_ << e.what();
		err = -1;
	}
	return err;
}