/*
 *	PosixStorageDriver.h
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

#ifndef __CHAOSFramework__PosixStorageDriver__
#define __CHAOSFramework__PosixStorageDriver__


#include "StorageDriver.h"

#include <string>

#include <chaos/common/utility/ObjectFactoryRegister.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace chaos {
	namespace data_service {
		namespace storage_system {
			
			//forward declaration
			class PosixStorageDriver;
			
			//! storage driver setting
			struct PosixStorageDriverSetting :public StorageDriverSetting {
				std::string fsd_domain_path;
			};
			
			//! Posix implementation of the Storage Driver
			REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY(PosixStorageDriver, StorageDriver) {
				REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(PosixStorageDriver)
				
				PosixStorageDriverSetting *setting;
				
				PosixStorageDriver(std::string alias);
			protected:
				std::string getAbsolutePath(std::string vfs_path);
				int _openFile(std::string path);
				
			public:
				~PosixStorageDriver();
				
				//! init
				/*!
				 This implementation need a point to the PosixStorageDriverSetting struct
				 for his configuration;
				 */
				void init(void *init_data) throw (chaos::CException);
				
				//!deinit
				void deinit() throw (chaos::CException);
				
				//! Open a block
				int openBlock(chaos_vfs::DataBlock *data_block, unsigned int flags);
				
				// open a block of a determinated type with
				int openBlock(std::string vfs_path, unsigned int flags, chaos_vfs::DataBlock **data_block);
				
				//! close the block of data
				int closeBlock(chaos_vfs::DataBlock *data_block);

				//! return the block current size
				int getBlockSize(chaos_vfs::DataBlock *data_block);
				
				//! return all block of data found on the path, acocrding to the type
				int listBlock(std::string vfs_path, std::vector<chaos_vfs::DataBlock*>& bloks_found);
				
				//! write an amount of data into a DataBlock
                int write(chaos_vfs::DataBlock *data_block, void * data, uint32_t data_len);
				
				//! read an amount of data from a DataBlock
                int read(chaos_vfs::DataBlock *data_block, uint64_t offset, void * * data, uint32_t& data_len);
			};
			
		}
	}
}

#endif /* defined(__CHAOSFramework__PosixStorageDriver__) */
