/*
 *	VFSStageFile.h
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

#ifndef __CHAOSFramework__VFSStageFile__
#define __CHAOSFramework__VFSStageFile__

#include "VFSFile.h"

namespace chaos {
	namespace data_service {
		namespace vfs {
			//forward declaration
			class VFSManager;
			
			typedef enum VFSStageFileOpenMode {
				VFSStageFileOpenModeRead = VFSFileOpenModeRead,
				VFSStageFileOpenModeWrite = VFSFileOpenModeWrite
			} VFSStageFileOpenMode;
			
			//! Abstract the managment of the stage file
			/*!
			 Stage file represnt a file in the vfs that contain chaos heterogeneous
			 data pack colelcted by different data producer (for example Control Unit, etc...).
			 */
			class VFSStageFile : public VFSFile {
				friend class VFSManager;
			protected:
				VFSStageFile(storage_system::StorageDriver *_storage_driver_ptr,
							 chaos_index::DBDriver *_db_driver_ptr,
							 std::string stage_vfs_relative_path,
							 VFSStageFileOpenMode _open_mode);
			};
			
		}
	}
}

#endif /* defined(__CHAOSFramework__VFSStageFile__) */
