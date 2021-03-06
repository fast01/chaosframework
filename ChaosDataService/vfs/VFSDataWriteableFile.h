/*
 *	VFSDataWriteableFile.h
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

#ifndef __CHAOSFramework__VFSDataWriteableFile__
#define __CHAOSFramework__VFSDataWriteableFile__

#include "VFSDataFile.h"

namespace chaos {
	namespace data_service {
		namespace vfs {
			//forward declaration
			class VFSManager;
			
			//! Data indexed writeable file
			/*!
			 This calss is used on the indexing process of a stage datafile. Every datafile contains datapack from an unique
			 instrument id. During stage file indexing process, this file is fille. Quen this instace close a datablock
			 it need to update all index belong to it in the querable state. In this qay data can be found by serach query
			 */
			class VFSDataWriteableFile: public VFSDataFile {
				friend class VFSManager;

				//! check if datablock is valid according to internal logic
				inline bool isDataBlockValid(DataBlock *new_data_block_ptr);
				
				VFSDataWriteableFile(storage_system::StorageDriver *_storage_driver_ptr,
									 db_system::DBDriver *_db_driver_ptr,
									 std::string data_vfs_relative_path);
				
				int releaseDataBlock(DataBlock *data_block_ptr, int closed_state = data_block_state::DataBlockStateNone);
				
			public:
				//! force new data block creation
				int switchDataBlock(bool close_only = false);
				
				//! ensure that a datablock is not null
				/*!
					usefullt to get the current lcoation before write the first data pack.
				 */
				int prefetchData();
				
				// write data on the current data block
				int write(void *data, uint32_t data_len);
			};
			
		}
	}
}

#endif /* defined(__CHAOSFramework__VFSDataWriteableFile__) */
