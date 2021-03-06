/*
 *	DriverManager.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2013 INFN, National Institute of Nuclear Physics
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
#ifndef __CHAOSFramework__DriverManager__
#define __CHAOSFramework__DriverManager__

#include <map>
#include <vector>

#include <chaos/common/utility/Singleton.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/ObjectInstancer.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/plugin/PluginInspector.h>

#include <chaos/cu_toolkit/driver_manager/driver/AbstractDriver.h>
#include <chaos/cu_toolkit/driver_manager/driver/DriverAccessor.h>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace chaos {
	namespace cu {
		namespace control_manager {
			//forward declaration
			class AbstractControlUnit;
		}
		namespace driver_manager {
			
			#define MATERIALIZE_INSTANCE_AND_INSPECTOR(DriverClass) \
			boost::shared_ptr< chaos::common::plugin::PluginInspector> DriverClass ## Inspector(GET_PLUGIN_CLASS_DEFINITION(DriverClass)); \
			boost::shared_ptr< chaos::common::utility::ObjectInstancer< chaos::cu::driver_manager::driver::AbstractDriver > > DriverClass ## Instancer(CU_DRIVER_INSTANCER(DriverClass));
			
			#define MATERIALIZE_INSTANCE_AND_INSPECTOR_WITH_NS(n, DriverClass) \
			boost::shared_ptr< chaos::common::plugin::PluginInspector> DriverClass ## Inspector(GET_PLUGIN_CLASS_DEFINITION(DriverClass)); \
			boost::shared_ptr< chaos::common::utility::ObjectInstancer< chaos::cu::driver_manager::driver::AbstractDriver > > DriverClass ## Instancer(CU_DRIVER_INSTANCER(n::DriverClass));
			
			#define CU_DRIVER_INSTANCER(DriverClass) new chaos::common::utility::TypedObjectInstancer<DriverClass, chaos::cu::driver_manager::driver::AbstractDriver>()

			typedef struct DriverPluginInfo {
				boost::shared_ptr< chaos::common::plugin::PluginInspector > sp_inspector;
				boost::shared_ptr< chaos::common::utility::ObjectInstancer<chaos::cu::driver_manager::driver::AbstractDriver> > sp_instancer;
			} DriverPluginInfo;
			
			//! !CHAOS Driver infrastructure managment
			class DriverManager : public chaos::utility::StartableService, public chaos::Singleton<DriverManager> {
				friend class chaos::Singleton<DriverManager>;
				friend class chaos::cu::control_manager::AbstractControlUnit;
				
				//! this map correlate the alias to the object instancer
				std::map<std::string, boost::shared_ptr< DriverPluginInfo > > mapDriverAliasVersionInstancer;
				
				//! this map correlate the param md5 to a live driver instance
				/*!
					the param hashing identify the CDataWrapper with the initilization param for the init pahse of the
					device driver. So in this way when a new initialization wil be requested, before a new instance of the
					is created, the hashing of the input parameter is checked if it is contained in the map. In case
					of successfull test a live instance is returned.
				 */
				std::map<string, chaos::cu::driver_manager::driver::AbstractDriver*> mapParameterLiveInstance;
				
				
				std::map<string, chaos::cu::driver_manager::driver::AbstractDriver*> mapDriverUUIDHashLiveInstance;
				
				//!Mutex for priority queue managment
				boost::shared_mutex    mutextMapAccess;
			protected:
				
				/*
				 Constructor
				 */
				DriverManager();
				
				/*
				 Desctructor
				 */
				~DriverManager();
				
                // Initialize instance
				void init(void *initParameter) throw(chaos::CException);
				
                // Start the implementation
				void start() throw(chaos::CException);
				
                // Stop the implementation
				void stop() throw(chaos::CException);
				
				
                // Deinit the implementation
				void deinit() throw(chaos::CException);
				
				
				//! Get a new driver accessor for a driver instance
				/*!
				 checking the hashing of the input parameter is created (or got one) device driver instance,
				 from this a new driver accessor is created
				 */
				chaos::cu::driver_manager::driver::DriverAccessor *getNewAccessorForDriverInstance(chaos::cu::driver_manager::driver::DrvRequestInfo& request_info) throw (chaos::CException);
				
				//! release the accessor instance
				void releaseAccessor(chaos::cu::driver_manager::driver::DriverAccessor *accessor);
			public:

				
				//! Register a new driver
				void registerDriver(boost::shared_ptr< chaos::common::utility::ObjectInstancer<chaos::cu::driver_manager::driver::AbstractDriver> > instancer,
									boost::shared_ptr< chaos::common::plugin::PluginInspector > description) throw(chaos::CException);

			};
		}
	}
}


#endif /* defined(__CHAOSFramework__DriverManager__) */
