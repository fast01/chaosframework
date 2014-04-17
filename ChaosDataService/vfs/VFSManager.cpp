/*
 *	VFSManager.cpp
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

#include "VFSManager.h"

#include <chaos/common/utility/ObjectFactoryRegister.h>

#include <boost/format.hpp>

using namespace chaos::data_service::vfs;
namespace chaos_data_index = chaos::data_service::index_system;
namespace chaos_data_storage = chaos::data_service::storage_system;

#define VFSManager_LOG_HEAD "[VFSManager] - "
#define VFSFM_LAPP_ LAPP_ << VFSManager_LOG_HEAD
#define VFSFM_LDBG_ LDBG_ << VFSManager_LOG_HEAD << __FUNCTION__ << " - "
#define VFSFM_LERR_ LERR_ << VFSManager_LOG_HEAD << __FUNCTION__ << " - "

#define VFSManager_MAX_BLOCK_SIZE		1024*1024*5			// 5 megabyte
#define VFSManager_MAX_BLOCK_LIFETIME	1000 * 60 * 5		// 5 minutes

VFSManager::VFSManager():setting(NULL) {
	
}

VFSManager::~VFSManager() {
	//deallocate the configuration
	deinit();
	
}

void VFSManager::init(void * init_data) throw (chaos::CException) {
	if(!init_data) throw chaos::CException(-1, "Invalid init data", __PRETTY_FUNCTION__);
	
	//associate the setting
	VFSFM_LAPP_ << "Get setting";
	setting = static_cast<VFSManagerSetting*>(init_data);
	
	//allocate index driver
	std::string index_driver_class_name = boost::str(boost::format("%1%IndexDriver") % setting->index_driver_impl);
	VFSFM_LAPP_ << "Allocate index driver of type "<<index_driver_class_name;
	index_driver_ptr = chaos::ObjectFactoryRegister<chaos_data_index::IndexDriver>::getInstance()->getNewInstanceByName(index_driver_class_name);
	if(!index_driver_ptr) throw chaos::CException(-1, "No index driver found", __PRETTY_FUNCTION__);
	chaos::utility::InizializableService::initImplementation(index_driver_ptr, &setting->index_driver_setting, index_driver_ptr->getName(), __PRETTY_FUNCTION__);
	
	std::string storage_driver_class_name = boost::str(boost::format("%1%StorageDriver") % setting->storage_driver_impl);
	VFSFM_LAPP_ << "Allocate storage driver of type " << storage_driver_class_name;
	storage_driver_ptr = chaos::ObjectFactoryRegister<chaos_data_storage::StorageDriver>::getInstance()->getNewInstanceByName(storage_driver_class_name);
	if(!storage_driver_ptr) throw chaos::CException(-1, "No storage driver found", __PRETTY_FUNCTION__);
	chaos::utility::InizializableService::initImplementation(storage_driver_ptr, &setting->storage_driver_setting, storage_driver_ptr->getName(), __PRETTY_FUNCTION__);
	
	if(!setting->max_block_size) {
		VFSFM_LAPP_ << "Use default value for max_block_size";
		setting->max_block_size = VFSManager_MAX_BLOCK_SIZE;
	}
	
	if(!setting->max_block_lifetime) {
		VFSFM_LAPP_ << "Use default value for max_block_lifetime";
		setting->max_block_lifetime = VFSManager_MAX_BLOCK_LIFETIME;
	}
	
	VFSFM_LDBG_ << "max_block_size configured with " << setting->max_block_size << " megabyte";
	VFSFM_LDBG_ << "max_block_lifetime configured with " << setting->max_block_lifetime << " milliseconds (" << (setting->max_block_lifetime/(1000*60)) << " minutes)";
}

void VFSManager::deinit() throw (chaos::CException) {
	if(index_driver_ptr) {
		VFSFM_LAPP_ << "Deallocate index driver";
		
		try {
			chaos::utility::InizializableService::deinitImplementation(index_driver_ptr, index_driver_ptr->getName(), __PRETTY_FUNCTION__);
		} catch (chaos::CException e) {
			VFSFM_LDBG_ << e.what();
		}
		
		delete (index_driver_ptr);
	}
	if(storage_driver_ptr) {
		VFSFM_LAPP_ << "Deallocate storage driver";
		try {
			chaos::utility::InizializableService::deinitImplementation(storage_driver_ptr, storage_driver_ptr->getName(), __PRETTY_FUNCTION__);
		}catch (chaos::CException e) {
			VFSFM_LDBG_ << e.what();
			delete (storage_driver_ptr);
		}
	}
}

void VFSManager::freeObject(std::string key, VFSFilesForPath *element) {
	//lock the files info for path
	boost::unique_lock<boost::mutex> lock(element->_mutex);
	
	//delete all file
	for(FileInstanceMapIterator iter = element->map_logical_files.begin();
		iter != element->map_logical_files.end();
		iter++) {
		VFSFM_LAPP_ << "delete file instance -> "<< iter->second->vfs_file_info.identify_key;
		delete(iter->second);
	}
	
	//remove all deleted element
	element->map_logical_files.clear();
}

int VFSManager::getFile(std::string vfs_fpath,  VFSFile **l_file) {
	DEBUG_CODE(VFSFM_LDBG_ << "Start getting new logical file with path->" << vfs_fpath;)
	
	VFSFilesForPath *files_for_path = NULL;
	
	VFSFile *logical_file = new VFSFile(vfs_fpath);
	if(!logical_file) return -1;
	
	//the vfs file is identified by a folder containing all data block
	if(storage_driver_ptr->createDirectory(vfs_fpath)) {
		return -2;
		delete logical_file;
	}
	
	//get or create the infro for logical file isntance
	if(VFSManagerKeyObjectContainer::hasKey(vfs_fpath)) {
		files_for_path = VFSManagerKeyObjectContainer::accessItem(vfs_fpath);
	} else {
		files_for_path =  new VFSFilesForPath();
		if(!files_for_path) return -3;
		VFSManagerKeyObjectContainer::registerElement(vfs_fpath, files_for_path);
	}

	//lock the files info for path
	boost::unique_lock<boost::mutex> lock(files_for_path->_mutex);

	//generate isntance unique key
	logical_file->vfs_file_info.identify_key = boost::str(boost::format("%1%_%2%") % vfs_fpath % files_for_path->instance_counter++);
	logical_file->vfs_file_info.vfs_domain = storage_driver_ptr->getStorageDomain();
	logical_file->vfs_file_info.max_block_size = setting-> max_block_size;
	logical_file->vfs_file_info.max_block_lifetime = setting-> max_block_lifetime;
	logical_file->storage_driver_ptr = storage_driver_ptr;
	logical_file->index_driver_ptr	= index_driver_ptr;

	files_for_path->map_logical_files.insert(make_pair(logical_file->vfs_file_info.identify_key, logical_file));
	
	*l_file = logical_file;
	return 0;
}

int VFSManager::releaseFile(VFSFile *l_file) {
	CHAOS_ASSERT(l_file)
	DEBUG_CODE(VFSFM_LDBG_ << "Delete vfs fiel with path->" << l_file->vfs_file_info.vfs_fpath;)
	
	VFSFilesForPath *files_for_path = NULL;

	//check if the file belong to this manager
	if(VFSManagerKeyObjectContainer::hasKey(l_file->vfs_file_info.vfs_fpath)) {
		files_for_path = VFSManagerKeyObjectContainer::accessItem(l_file->vfs_file_info.vfs_fpath);
	} else {
		return -1;
	}
	//lock the files info for path
	boost::unique_lock<boost::mutex> lock(files_for_path->_mutex);
	files_for_path->map_logical_files.erase(l_file->vfs_file_info.identify_key);
	delete l_file;
	
	//return
	return 0;
}

int VFSManager::createDirectory(std::string vfs_path, bool all_path) {
	DEBUG_CODE(VFSFM_LDBG_ << "Create directory path->" << vfs_path;)
	int err = 0;
	if(all_path)
		err = storage_driver_ptr->createPath(vfs_path);
	else
		err = storage_driver_ptr->createDirectory(vfs_path);
	return err;
}

int VFSManager::deleteDirectory(std::string vfs_path, bool all_path) {
	return storage_driver_ptr->deletePath(vfs_path, all_path);
}