/*	
 *	IOMemcachedIODriver.cpp
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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "IOMemcachedIODriver.h"
#include <chaos/common/chaos_constants.h>
#include "../global.h"

#define LMEMDRIVER_ LAPP_ << "[Memcached IO Driver] - "

namespace chaos_data = chaos::common::data;

namespace chaos{ 
    using namespace std;
    using namespace boost;
    using namespace boost::algorithm;

	
    //using namespace memcache;

    /*
     * Driver constructor
     */
    IOMemcachedIODriver::IOMemcachedIODriver(std::string alias):NamedService(alias) {
        memClient = NULL;
    }
    
    /*
     * Driver distructor
     */
    IOMemcachedIODriver::~IOMemcachedIODriver() {
        //delete memcache client instance
        if(memClient) {
            memcached_free(memClient);
        }
    }
    
    /*
     * Init method, the has map has all received value for configuration
     * every implemented driver need to get all needed configuration param
     */
    void IOMemcachedIODriver::init() throw(CException) {
        IODataDriver::init();
        LMEMDRIVER_ << "Initializing Driver with libmemcache: " << LIBMEMCACHED_VERSION_STRING;
        
        //memcached_return_t configResult = MEMCACHED_SUCCESS;
        
        memClient = memcached(NULL, 0);
        if(!memClient) throw CException(0, "LibMemcached structure has not been allocated", "IOMemcachedIODriver::init");
    }
    
    /*
     * Deinitialization of memcached driver
     */
    void IOMemcachedIODriver::deinit() throw(CException) {
        IODataDriver::deinit();
        if(memClient){
            memcached_free(memClient);
            memClient = NULL;
        }
    }
    
    /*
     * This method retrive the cached object by CSDawrapperUsed as query key and
     * return a pointer to the class ArrayPointer of CDataWrapper type
     */
    void IOMemcachedIODriver::storeRawData(size_t dataDim, const char * buffer)  throw(CException) {
        boost::mutex::scoped_lock lock(useMCMutex);
        memcached_return_t mcSetResult = MEMCACHED_SUCCESS;
        mcSetResult = memcached_set(memClient, dataKey.c_str(), dataKey.length(), buffer, dataDim, 0, 0);
            //for debug
        if(mcSetResult!=MEMCACHED_SUCCESS) {
#if DEBUG
            LMEMDRIVER_ << "cache data submition error";
#endif
        }

    }
    
    /*
     * This method retrive the cached object by CSDawrapperUsed as query key and
     * return a pointer to the class ArrayPointer of CDataWrapper type
     */
    char* IOMemcachedIODriver::retriveRawData(size_t *dim)  throw(CException) {
        uint32_t flags= 0;
        size_t value_length= 0;
        memcached_return_t mcSetResult = MEMCACHED_SUCCESS;
        boost::mutex::scoped_lock lock(useMCMutex);
        char* result =  memcached_get(memClient, dataKey.c_str(), dataKey.length(), &value_length, &flags,  &mcSetResult);
        if(dim) *dim = value_length;
        return result;
    }    
    
    /*
     Update the driver configuration
     */
    chaos_data::CDataWrapper* IOMemcachedIODriver::updateConfiguration(chaos_data::CDataWrapper* newConfigration) {
        memcached_return_t configResult = MEMCACHED_SUCCESS;
        boost::mutex::scoped_lock lock(useMCMutex);
        LMEMDRIVER_ << "Update Configuration";
        
        if(!memClient) throw CException(0, "Write memcached structure not allocated", "IOMemcachedIODriver::updateConfiguration");
        
            //checkif someone has passed us the device indetification
        if(newConfigration->hasKey(DatasetDefinitionkey::DEVICE_ID)){
            dataKey = newConfigration->getStringValue(DatasetDefinitionkey::DEVICE_ID);
            LMEMDRIVER_ << "The key for memory cache is: " << dataKey;
        }
        
        if(newConfigration->hasKey(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS) && memClient){
            LMEMDRIVER_ << "Get the DataManager LiveData address value";
            auto_ptr<chaos_data::CMultiTypeDataArrayWrapper> liveMemAddrConfig(newConfigration->getVectorValue(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS));
            //update the live data address
            
            //we need forst to reset all the server list 
            memcached_servers_reset(memClient);
            
            
            //const char *config_string= "--SERVER=192.168.1.2";
            //configResult = memcached_parse_configuration(memClient, config_string, strlen(config_string));
            //iterate the server name
            size_t numerbOfserverAddressConfigured = liveMemAddrConfig->size();
            for ( int idx = 0; idx < numerbOfserverAddressConfigured; idx++ ){
                vector<string> serverTokens;
                string serverDesc = liveMemAddrConfig->getStringElementAtIndex(idx);
                algorithm::split(serverTokens, serverDesc, is_any_of(":"));
                if(serverTokens.size()==2){
                    in_port_t port = lexical_cast<in_port_t>(serverTokens[1]);
                    LMEMDRIVER_ << "trye to configure " << serverTokens[0] << "on port "<<port;
                    configResult = memcached_server_add(memClient, serverTokens[0].c_str(), port);
                    if(configResult != MEMCACHED_SUCCESS) 
                        LMEMDRIVER_ << "Error Configuration server '" << liveMemAddrConfig->getStringElementAtIndex(idx) << "' with error: " << memcached_strerror(NULL, configResult);
                    else
                        LMEMDRIVER_ << "The server '" << liveMemAddrConfig->getStringElementAtIndex(idx) << "' has been configurated";
                }else{
                    LMEMDRIVER_ << "Wrong Server Description '" << liveMemAddrConfig->getStringElementAtIndex(idx) << "'";
                }
            }
            LMEMDRIVER_ << "write param";

            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_NO_BLOCK, (uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_NOREPLY, (uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_KETAMA, (uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_KETAMA,(uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_TCP_NODELAY,(uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, (uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, (uint64_t)1);
            configResult = memcached_behavior_set(memClient, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT,(uint64_t)1);
        }
        return NULL; 
    }
}