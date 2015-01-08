/*
 *	AbstractWANInterface.h
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
#ifndef __CHAOSFramework__AbstractWANInterface__
#define __CHAOSFramework__AbstractWANInterface__

#include <chaos/common/utility/UrlAccessibleService.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/utility/NamedService.h>
namespace chaos {
	namespace wan_proxy {
		namespace wan_interface {
			
			
			class AbstractWANInterface:
			public NamedService,
			public utility::StartableService,
			public common::utility::UrlAccessibleService {
			protected:
				int service_port;
				
				std::string service_url;
				
				AbstractWANInterface(const std::string& alias);
				
				~AbstractWANInterface();
				
			public:
				// inherited method
				void init(void *init_data) throw(chaos::CException);
				
				// inherited method
				void deinit() throw(chaos::CException);
				
				// inherited method
				void start() throw(chaos::CException);
				
				// inherited method
				void stop() throw(chaos::CException);
				
				//inherited method
				const std::string& getUrl();
			};
		}
	}
}

#endif /* defined(__CHAOSFramework__AbstractWANInterface__) */
