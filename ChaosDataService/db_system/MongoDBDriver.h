/*
 *	MongoDBDriver.h
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
#ifndef __CHAOSFramework__MongoDBDriver__
#define __CHAOSFramework__MongoDBDriver__

#include "DBDriver.h"
#include "mongo_db_types.h"
#include "MongoDBHAConnectionManager.h"

#include <chaos/common/utility/ObjectFactoryRegister.h>

#include <boost/format.hpp>

#include <mongo/client/dbclient.h>

namespace chaos {
	namespace data_service {
			namespace db_system {
				class MongoDBIndexCursor;
				
				//! Mongodb implementation for the index driver
				REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY(MongoDBDriver, DBDriver) {
					REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(MongoDBDriver)
					MongoDBDriver(std::string alias);
					friend class MongoDBIndexCursor;
					std::string db_name;
				protected:
					//! ha mongodb driver
					MongoDBHAConnectionManager *ha_connection_pool;
					
					//! protected methdo that perform the real paged query on index called by the cursor
					int idxSearchDataPack(const DataPackIndexQuery & data_pack_index_query, std::auto_ptr<mongo::DBClientCursor>& cursor);
					
					//! protected methdo that perform the real paged query on index called by the cursor
					int idxSearchResultCountDataPack(const DataPackIndexQuery & data_pack_index_query, uint64_t& num_of_result);
					
					//! protected methdo that perform the real paged query on index called by the cursor
					int idxMaxAndMInimumTimeStampForDataPack(const DataPackIndexQuery & data_pack_index_query, uint64_t& min_ts, uint64_t& max_ts);
					
					//! fill a complete datablock
					chaos_vfs::DataBlock* fillDatablock(const mongo::BSONObj& full_datablock_query_result, chaos_vfs::DataBlock *data_block);
				public:
					~MongoDBDriver();
					
					//! init
					void init(void *init_data) throw (chaos::CException);
					
					//!deinit
					void deinit() throw (chaos::CException);
					//! Register a new domain
					int vfsAddDomain(vfs::VFSDomain domain);
					
					//! Give an heart beat for a domain
					int vfsDomainHeartBeat(vfs::VFSDomain domain);
					
					//! Register a new data block wrote on stage area
					int vfsAddNewDataBlock(chaos_vfs::VFSFile *vfs_file,
										   chaos_vfs::DataBlock *data_block,
										   vfs::data_block_state::DataBlockState new_block_state = vfs::data_block_state::DataBlockStateNone);
					
					//! Delete a virtual file datablock
					int vfsDeleteDataBlock(chaos_vfs::VFSFile *vfs_file,
												   chaos_vfs::DataBlock *data_block);
					
					//! Set the state for a stage datablock
					int vfsSetStateOnDataBlock(chaos_vfs::VFSFile *vfs_file,
											   chaos_vfs::DataBlock *data_block,
											   int state);
					
					//! Set the state for a stage datablock
					int vfsSetStateOnDataBlock(chaos_vfs::VFSFile *vfs_file,
											   chaos_vfs::DataBlock *data_block,
											   int cur_state,
											   int new_state,
											   bool& success);
					
					//! Set the datablock current position
					int vfsSetHeartbeatOnDatablock(chaos_vfs::VFSFile *vfs_file,
												   chaos_vfs::DataBlock *data_block,
												   uint64_t timestamp = 0);
					
					//! update the current datablock size
					int vfsUpdateDatablockCurrentWorkPosition(chaos_vfs::VFSFile *vfs_file,
															  chaos_vfs::DataBlock *data_block);
					
					//! Return the next available datablock created since timestamp
					int vfsFindSinceTimeDataBlock(chaos_vfs::VFSFile *vfs_file,
												  uint64_t timestamp,
												  bool direction,
												  int state,
												  chaos_vfs::DataBlock **data_block);
					
					//! Return the datablock identified by path
					int vfsFindFromPathDataBlock(const std::string& data_block_domain,
												 const std::string& data_block_path,
												 chaos_vfs::DataBlock **data_block);
					
					//! Check if the vfs file exists
					int vfsFileExist(chaos_vfs::VFSFile *vfs_file,
									 bool& exists_flag);
					
					//! Create a file entry into the vfat
					int vfsCreateFileEntry(chaos_vfs::VFSFile *vfs_file);
					
					//! Return a list of vfs path of the file belong to a domain
					int vfsGetFilePathForDomain(std::string vfs_domain, std::string prefix_filter, std::vector<std::string>& result_vfs_file_path, int limit_to_size);
					
					//! add the default index for a unique instrument identification and a timestamp
					int idxAddDataPackIndex(const DataPackIndex& index);
					
					//! remove the index for a unique instrument identification and a timestamp
					int idxDeleteDataPackIndex(const DataPackIndex& index);
					
					//! perform a search on data pack indexes
					int idxStartSearchDataPack(const DataPackIndexQuery& _query, DBIndexCursor **index_cursor);
				};
			}
	}
}

#endif /* defined(__CHAOSFramework__MongoDBDriver__) */