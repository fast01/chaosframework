/*
 *	DirectIOEndpointHandler.h
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
#ifndef CHAOSFramework_DirectIOEndpointHandler_h
#define CHAOSFramework_DirectIOEndpointHandler_h

#include <chaos/common/direct_io/DirectIODataPack.h>

namespace chaos {
	namespace common {
		namespace direct_io {
			class DirectIOEndpointHandler {
			public:
				virtual void consumeDataPack(DirectIODataPack *dataPack) = 0;
			};
		}
	}
}

#endif