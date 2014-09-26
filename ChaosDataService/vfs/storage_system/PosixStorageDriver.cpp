/*
 *	PosixStorageDriver.cpp
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

#include "PosixStorageDriver.h"

#include <iostream>
#include <sys/stat.h>
#include <memory>

#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/TimingUtil.h>
#include <chaos/common/utility/UUIDUtil.h>

#include <boost/format.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#define POSIX_DOMAIN_FOLDER					".ChaosDomain"
#define POSIX_DOMAIN_DESCRIPTOR_FILE		"chaos_domain.data"
#define POSIX_DOMAIN_DESCRIPTOR_FILE_LOCK	"chaos_domain.lock"

#define PosixStorageDriver_LOG_HEAD "[PosixStorageDriver] - "
#define PSDLAPP_ LAPP_ << PosixStorageDriver_LOG_HEAD
#define PSDLDBG_ LDBG_ << PosixStorageDriver_LOG_HEAD << __FUNCTION__ << " - "
#define PSDLERR_ LERR_ << PosixStorageDriver_LOG_HEAD << __FUNCTION__ << " - "


using namespace chaos::data_service::vfs::storage_system;
namespace boost_fs = boost::filesystem;
namespace chaos_data = chaos::common::data;

//---------------------------------------------------------------------------

PosixStorageDriver::PosixStorageDriver(std::string alias):StorageDriver(alias) {
	
}

PosixStorageDriver::~PosixStorageDriver() {
	
}

//! init
void PosixStorageDriver::init(void *init_data) throw (chaos::CException) {
	StorageDriver::init(init_data);
	//if(!PSD_CASTED_SETTING->fs_driver_domain_path.size()) throw CException(-1, "No root path setupped", __PRETTY_FUNCTION__);
}

//! deinit
void PosixStorageDriver::deinit() throw (chaos::CException) {
	
}

std::string PosixStorageDriver::getAbsolutePath(std::string vfs_path) {
	return boost::str(boost::format("%1%/%2%") % fs_driver_domain_path % vfs_path);
}

void PosixStorageDriver::initDomain() throw (chaos::CException) {
	//domain file stream ptr
	boost_fs::fstream domain_file_stream;
	
	if(setting->key_value_custom_param.count("posix_root_path")) {
		fs_driver_domain_path = setting->key_value_custom_param["posix_root_path"];
	}
	
	//get domain folder path
	boost_fs::path domain_folder = getAbsolutePath(POSIX_DOMAIN_FOLDER);
	try{
		
		//check for the presence of the domain folder
		if(!boost_fs::exists(domain_folder)) {
			if (!boost_fs::create_directory(domain_folder) && !boost_fs::exists(domain_folder)) {
				throw chaos::CException(-1, boost::str(boost::format("Error creating %1% directory") % domain_folder.string()), __PRETTY_FUNCTION__);
			}
		} else {
			if(boost_fs::is_regular_file(domain_folder)) {
				throw chaos::CException(-1, boost::str(boost::format("%1% exist but is a file") % domain_folder.string()), __PRETTY_FUNCTION__);
			}
		}
		
		//now we can create or read domain file
		boost_fs::path domain_file = domain_folder / POSIX_DOMAIN_DESCRIPTOR_FILE;
		boost_fs::path domain_file_lock = domain_folder / POSIX_DOMAIN_DESCRIPTOR_FILE_LOCK;
		
		PSDLDBG_ << "Domain data file " << domain_file;
		PSDLDBG_ << "Domain lock file " << domain_file_lock;
		
		//open lock file
		boost_fs::fstream domain_file_lock_stream(domain_file_lock, std::ios_base::out | std::ios_base::binary);
		
		//check if we have got the lock
		boost::interprocess::file_lock flock(domain_file_lock.string().c_str());
		boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(flock);
		
		//we've got the lock, so we can write or read the domain
		uint32_t domain_file_size = 0;
		bool read_already_initialized_domain =  false;
		
		if(boost_fs::exists(domain_file)) {
			domain_file_size = (uint32_t)boost_fs::file_size(domain_file);
			read_already_initialized_domain =  domain_file_size > 0;
		}
		
		domain_file_stream.open(domain_file, fstream::in | fstream::out | fstream::binary | fstream::app);
		
		if(!read_already_initialized_domain) {
			createDomain(domain_file_stream);
		} else {
			readDomain(domain_file_stream, domain_file_size);
		}
		//
	} catch (const boost_fs::filesystem_error& e) {
		PSDLERR_ << e.what() << std::endl;
	}
	domain_file_stream.close();
}

void PosixStorageDriver::createDomain(boost::filesystem::fstream& domain_file_Stream) throw (chaos::CException) {
	chaos_data::CDataWrapper domain_data;
	
	//compose the pack with the given domain name and a generated unique key
	domain_data.addStringValue("domain.name", setting->domain.name);
	domain_data.addStringValue("domain.unique_code", (setting->domain.unique_code = UUIDUtil::generateUUID()));
	//get serialization
	std::auto_ptr<chaos_data::SerializationBuffer> bson_serialization(domain_data.getBSONData());
	
	//write on stream
	domain_file_Stream.seekp(0);
	domain_file_Stream.write(bson_serialization->getBufferPtr(), bson_serialization->getBufferLen());
	domain_file_Stream.flush();
	
	if (domain_file_Stream.fail()) {
		PSDLERR_ << "domain_file_Stream is failed.." << endl;
		PSDLERR_ << domain_file_Stream.rdstate() << endl;
	}
}

void PosixStorageDriver::readDomain(boost::filesystem::fstream& domain_file_Stream, uint32_t domain_file_size) throw (chaos::CException) {
	//get the stream size
	char memblock[domain_file_size];
	
	//read all data
	//rollback the file
	domain_file_Stream.seekg(0, ios::beg);
	domain_file_Stream.read(memblock, domain_file_size);
	
	//deserialize buffer
	chaos_data::CDataWrapper domain_data(memblock);
	if(domain_data.hasKey("domain.name") && domain_data.hasKey("domain.unique_code")) {
		//the key is present so we get the alread wrote domain name also if it is different from gived one
		setting->domain.name = domain_data.getStringValue("domain.name");
		setting->domain.unique_code = domain_data.getStringValue("domain.unique_code");
	} else {
		//rollback the file
		domain_file_Stream.seekg(0, ios::beg);
		
		//rewrite the correct data with our domain information
		createDomain(domain_file_Stream);
	}
}

int  PosixStorageDriver::blockIsPresent(chaos_vfs::DataBlock *data_block, bool &presence) {
	int err = 0;
	boost_fs::path _path = getAbsolutePath(data_block->vfs_path);
	if(boost::filesystem::exists(_path)) {
		if(boost::filesystem::is_directory(_path)) {
			err = -2;
		}
	} else {
		err = -1;
	}
	return err;
}

//! Open a block
int PosixStorageDriver::openBlock(chaos_vfs::DataBlock *data_block, unsigned int flags) {
	CHAOS_ASSERT(data_block)
	boost_fs::fstream *fstream_ptr = NULL;
	try {
		boost_fs::path _path = getAbsolutePath(data_block->vfs_path);
		boost_fs::file_status _status = boost_fs::status(_path);
		if(boost_fs::is_directory(_status)) {
			return -1;
		}
		
		std::ios_base::openmode mode = std::ios_base::binary;
		if(flags & chaos_vfs::block_flag::BlockFlagWriteble) {
			mode |= fstream::app | std::ios_base::out;
		}
		
		if(flags & chaos_vfs::block_flag::BlockFlagReadeable) {
			mode |= std::ios_base::in;
		}
		
		//set the default value
		data_block->max_reacheable_size = boost_fs::file_size(_path);
		data_block->driver_private_data = fstream_ptr = new boost_fs::fstream(_path, mode);
		data_block->vfs_domain = setting->domain.name;
		if(fstream_ptr->rdstate() & std::ifstream::failbit) {
			delete(fstream_ptr);
			data_block->driver_private_data = NULL;
			return -2;
		}
	} catch (boost_fs::filesystem_error &e) {
		if(fstream_ptr) delete(fstream_ptr);
		data_block->driver_private_data = NULL;
		//delete the allocated stream
		PSDLERR_ << e.what() << std::endl;
		return -3;
	}
	return 0;
}

// open a block of a determinated type with
int PosixStorageDriver::openBlock(const std::string& vfs_path, unsigned int flags, chaos_vfs::DataBlock **data_block) {
	CHAOS_ASSERT(data_block)
	boost_fs::path _path = getAbsolutePath(vfs_path);
	boost_fs::fstream *ofs = NULL;
	
	//reset memory (if the user use an allocated handle here there is memory leak)
	*data_block = NULL;
	
	try {
		boost_fs::file_status _status = boost_fs::status(_path);
		
		if(boost_fs::is_directory(_status)) {
			//a directory is present with the same name
			return -1;
		}
		
		if(boost_fs::is_symlink(_status)) {
			//a directory is present with the same name
			return -2;
		}
		
		//we can create a file that match a new data block
		
		
		std::ios_base::openmode mode = std::ios_base::binary;
		if(flags & chaos_vfs::block_flag::BlockFlagWriteble) {
			mode |= std::ios_base::out | fstream::app;
		}
		
		if(flags & chaos_vfs::block_flag::BlockFlagReadeable) {
			mode |= std::ios_base::in;
		}
		
		ofs = new boost_fs::fstream(_path, mode);
		if(ofs->rdstate() & std::ifstream::failbit) {
			delete(ofs);
			return -3;
		}
		
		//no we can create the block
		*data_block = new chaos_vfs::DataBlock();
		
		if(!data_block) {
			if(ofs) {
				delete(ofs);
			}
			return -4;
		}
		(*data_block)->vfs_path = vfs_path;
		(*data_block)->vfs_domain = setting->domain.name;
		(*data_block)->driver_private_data = ofs;
		(*data_block)->flags = flags;
		(*data_block)->invalidation_timestamp = 0;
		(*data_block)->max_reacheable_size = boost_fs::file_size(_path);
		(*data_block)->creation_time = TimingUtil::getTimeStamp();
	} catch(boost_fs::filesystem_error &e) {
		//delete the allcoated stream
		if(*data_block) {
			free(*data_block);
			*data_block = NULL;
		} else if(ofs) {
			delete(ofs);
		}
		PSDLERR_ << e.what() << std::endl;
		return -5;
	}
	return 0;
}

//! return the block current size
int PosixStorageDriver::getBlockSize(chaos_vfs::DataBlock *data_block) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	bool good = true;
	boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
	try {
		//write data
		fstream_ptr->seekg(0, fstream_ptr->end);
		good = fstream_ptr->good();
		data_block->current_work_position = (uint32_t)fstream_ptr->tellg();
		good = fstream_ptr->good();
		fstream_ptr->seekg(0, fstream_ptr->beg);
		good = fstream_ptr->good();
	}catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -1;
	}
	return 0;
}

//! close the block of data
int PosixStorageDriver::closeBlock(chaos_vfs::DataBlock *data_block) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	int result = 0;
	boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
	try {
		fstream_ptr->flush();
		fstream_ptr->close();
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		result = -1;
	}
	delete(fstream_ptr);
	delete(data_block);
	return result;
}

//! write an amount of data into a DataBlock
int PosixStorageDriver::write(chaos_vfs::DataBlock *data_block, void * data, uint32_t data_len) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		//write data
		static_cast<boost_fs::fstream *>(data_block->driver_private_data)->write((const char*)data, data_len);
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! read an amount of data from a DataBlock
int PosixStorageDriver::read(chaos_vfs::DataBlock *data_block, void * buffer, uint32_t buffer_len, uint32_t& readed_byte) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
		fstream_ptr->read((char*)buffer, buffer_len);
		if(!fstream_ptr->good()) {
			return -1;
		}
		//update the current work position
		readed_byte = (uint32_t)fstream_ptr->gcount();
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! change the block pointer for read or write
int PosixStorageDriver::seekg(chaos_vfs::DataBlock *data_block, uint64_t offset, chaos_vfs::block_seek_base::BlockSeekBase base_direction) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
		fstream_ptr->seekg(offset, (boost_fs::fstream::seekdir)base_direction);
		if(!fstream_ptr->good()) {
			return -1;
		}
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! change the block pointer for read or write
int PosixStorageDriver::seekp(chaos_vfs::DataBlock *data_block, uint64_t offset, chaos_vfs::block_seek_base::BlockSeekBase base_direction) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
		fstream_ptr->seekp(offset, (boost_fs::fstream::seekdir)base_direction);
		if(!fstream_ptr->good()) {
			return -1;
		}
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! get the current block data pointer position
int PosixStorageDriver::tellg(chaos_vfs::DataBlock *data_block, uint64_t& offset) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
		offset = fstream_ptr->tellg();
		if(!*fstream_ptr) {
			return -1;
		}
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! get the current block data pointer position
int PosixStorageDriver::tellp(chaos_vfs::DataBlock *data_block, uint64_t& offset) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
		offset = fstream_ptr->tellp();
		if(!*fstream_ptr) {
			return -1;
		}
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! truncate the file to the new size
int PosixStorageDriver::resize(chaos_vfs::DataBlock *data_block, uint64_t new_size) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	try {
		system::error_code ec;
		boost_fs::resize_file(getAbsolutePath(data_block->vfs_path), new_size, ec);
		if(ec) {
			return -1;
		}
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//! flush data on file
int PosixStorageDriver::flush(chaos_vfs::DataBlock *data_block) {
	CHAOS_ASSERT(data_block)
	CHAOS_ASSERT(data_block->driver_private_data)
	int err = 0;
	try {
		boost_fs::fstream *fstream_ptr = static_cast<boost_fs::fstream *>(data_block->driver_private_data);
		fstream_ptr->flush();
		if(!*fstream_ptr) {
			err = -1;
		}
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		err = -2;
	}
	return err;
}

//! create a directory
int PosixStorageDriver::createDirectory(const std::string& vfs_path) {
	//get absolute path
	int err = 0;
	boost::filesystem::path fs_path = getAbsolutePath(vfs_path);
	try {
		if(boost::filesystem::exists(fs_path)) {
			if(!boost::filesystem::is_directory(fs_path)) {
				err = -1;
			}
		} else {
			//create a directory
			if(!boost::filesystem::create_directory(fs_path)) {
				err = -2;
			}
		}
		
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -3;
	}
	return err;
}

//! create a directory
int PosixStorageDriver::createPath(const std::string& vfs_path) {
	int err = 0;
	system::error_code error_code;
	boost::filesystem::path fs_path = getAbsolutePath(vfs_path);
	try {
		if(!boost::filesystem::create_directories(fs_path, error_code)) {
			PSDLDBG_ << "Path "<< vfs_path << " already present" << std::endl;
		}
		
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		return -1;
	}
	return err;
}

//! delete a directory
int PosixStorageDriver::deletePath(const std::string& vfs_path, bool all) {
	int err = 0;
	system::error_code error_code;
	boost::filesystem::path fs_path = getAbsolutePath(vfs_path);
	try {
		if(all) {
			if(!boost::filesystem::remove_all(fs_path, error_code)) {
				PSDLERR_ << "Error deleting all from " << vfs_path << std::endl;
				err = -1;
			}
		} else {
			if(!boost::filesystem::remove(fs_path, error_code)) {
				PSDLERR_ << "Error deleting " << vfs_path << std::endl;
				err = -22;
			}
		}
		
	} catch (boost_fs::filesystem_error &e) {
		PSDLERR_ << e.what() << std::endl;
		err = -3;
	}
	return err;
}