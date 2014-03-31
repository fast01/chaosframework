/*
 *	IndexDriver.h
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

#ifndef __CHAOSFramework__IndexDriver__
#define __CHAOSFramework__IndexDriver__

#include "../vfs/DataBlock.h"

#include <string>
#include <vector>

#include <chaos/common/utility/NamedService.h>
#include <chaos/common/utility/InizializableService.h>

namespace chaos_vfs = chaos::data_service::vfs;

namespace chaos {
	namespace data_service {
		
		namespace index_system {
			
			typedef std::vector<std::string>			IndexDriverServerList;
			typedef std::vector<std::string>::iterator	IndexDriverServerListIterator;
			
			//! index driver setting
			typedef struct IndexDriverSetting {
				//! vector for all index endpoint url (usualy database url, more is the driver permit the scalability of the service)
				IndexDriverServerList servers;
			} IndexDriverSetting;
			
			//! define a type of work that can occour on stage datablock
			typedef enum StageDataBlockState {
				WriteData = 0,
				ReadyForProcessing,
				Processed
			} StageWorkType;
			
			/*!
			 Base class for all driver that will manage the work on index database.
			 The base role of the index driver is to help the fast storage
			 of into stage area, coordinate the worker that migrate data between stage and data area
			 of the !CHAOS virtual filesystem. At the end it will help to achive the execution 
			 of query on memoryzed data
			 */
			class IndexDriver : public NamedService , public chaos::utility::InizializableService {
			protected:
				//protected constructor
				IndexDriver(std::string alias);
			protected:
				IndexDriverSetting *setting;
            public:
				
				//! public destructor
				virtual ~IndexDriver();
				
				//! init
				/*!
				 Need a point to a structure IndexDriverSetting for the setting
				*/
				void init(void *init_data) throw (chaos::CException);
				
				//!deinit
				void deinit() throw (chaos::CException);
				
				//! Register a new data block wrote on stage area
				/*!
					Registration of a new datablock in stage area is achieved directly to the DataService process
					after the block has been created.
					\param data_block Newly created data block
				 */
				virtual int addNewStageDataBlock(chaos_vfs::DataBlock *data_block) = 0;
				
				//! Set the state for a stage datablock
				/*!
				 Set the current state for a datablock in the stage area
				 \param data_block Data block for wich need to be changed the state
				 */
				virtual int setStateOnStageDataBlock(chaos_vfs::DataBlock *data_block, StageDataBlockState state) = 0;
				
				//! Heartbeat update stage block
				/*!
				 Registration of a new datablock in stage area is achieved directly to the DataService process
				 after the block has been created.
				 */
				virtual int workHeartBeatOnStageDataBlock(chaos_vfs::DataBlock *data_block) = 0;
				
				
				//! Retrive the path for all datablock in a determinate state
				/*!
				 Retrieve from the all DataBlock in stage area that match the state.
				 */
				virtual int getStageDataBlockPathByState(std::vector<std::string>& path_data_block, StageDataBlockState state) = 0;

			};
		}
	}
}

#endif /* defined(__CHAOSFramework__IndexDriver__) */
