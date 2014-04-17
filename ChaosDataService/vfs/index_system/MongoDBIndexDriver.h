/*
 *	MongoDBIndexDriver.h
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
#ifndef __CHAOSFramework__MongoDBIndexDriver__
#define __CHAOSFramework__MongoDBIndexDriver__

#include "IndexDriver.h"

#include <chaos/common/utility/ObjectFactoryRegister.h>

#include "mongo/client/dbclient.h"

namespace chaos {
	namespace data_service {
		namespace index_system {
			
/*!
 Collection for the storage for the vfs infroamtion of the virtual files across the domain
 
 //some index need to be setup on the mongo installation
 usedb chaos_vfs
 
 //unique index for the vfs colletion
 db.vfat.ensureIndex( { "vfs_path": 1, "vfs_domain":1 } , { unique: true } )
 */
#define MONGO_DB_VFS_VFAT_COLLECTION		"chaos_vfs.vfat"
#define MONGO_DB_VFS_VBLOCK_COLLECTION		"chaos_vfs.datablock"
			
			//file filed------------------------------------------------------
#define MONGO_DB_FIELD_FILE_PRIMARY_KEY				"fpk"
#define MONGO_DB_FIELD_FILE_VFS_PATH				"vfs_path"
#define MONGO_DB_FIELD_FILE_VFS_DOMAIN				"vfs_domain"
			
			//data block file-------------------------------------------------
#define MONGO_DB_FIELD_DATA_BLOCK_STATE				"state"
#define MONGO_DB_FIELD_DATA_BLOCK_CREATION_TS		"ct"
#define MONGO_DB_FIELD_DATA_BLOCK_VALID_UNTIL_TS	"vu"
#define MONGO_DB_FIELD_DATA_BLOCK_MAX_BLOCK_SIZE	"mbs"
#define MONGO_DB_FIELD_DATA_BLOCK_VFS_PATH			"vfs_path"
#define MONGO_DB_FIELD_DATA_BLOCK_VFS_DOMAIN		"vfs_domain"
			
			//! Mongodb implementation for the index driver
			REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY(MongoDBIndexDriver, IndexDriver) {
				REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(MongoDBIndexDriver)
				mongo::ConnectionString connection_string;
				MongoDBIndexDriver(std::string alias);
			protected:
				mongo::DBClientConnection mongo_client;
				
			public:
				~MongoDBIndexDriver();
				
				//! init
				void init(void *init_data) throw (chaos::CException);
				
				//!deinit
				void deinit() throw (chaos::CException);
				
				//! Register a new data block wrote on stage area
				int vfsAddNewDataBlock(chaos_vfs::VFSFile *vfs_file, chaos_vfs::DataBlock *data_block, DataBlockState new_block_state = DataBlockStateNone);
				
				//! Set the state for a stage datablock
				int vfsSetStateOnDataBlock(chaos_vfs::VFSFile *vfs_file, chaos_vfs::DataBlock *data_block, DataBlockState state);
				
				//! Heartbeat update stage block
				int vfsWorkHeartBeatOnDataBlock(chaos_vfs::VFSFile *vfs_file, chaos_vfs::DataBlock *data_block);
				
				//! Check if the vfs file exists
				int vfsFileExist(chaos_vfs::VFSFile *vfs_file, bool& exists_flag);
				
				//! Create a file entry into the vfat
				int vfsCreateFileEntry(chaos_vfs::VFSFile *vfs_file);
			};
		}
	}
}

#endif /* defined(__CHAOSFramework__MongoDBIndexDriver__) */