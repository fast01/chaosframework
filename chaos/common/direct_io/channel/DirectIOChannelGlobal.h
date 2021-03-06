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
#ifndef CHAOSFramework_ChannelGlobal_h
#define CHAOSFramework_ChannelGlobal_h
namespace chaos {
	namespace common {
		namespace direct_io {
			namespace channel {
				//index of all channel
				#define DIOCDC_Channel_Index			1
				#define DIODataset_Channel_Index		2
				#define DIOPerformance_Channel_Index	3
				#define DIOSystemAPI_Channel_Index		4
				
				
#define DELETE_HEADER_DATA(h,d)\
if(h) free(h);\
if(d) free(d);
				
#define DELETE_HEADER(h)\
if(h) free(h);
            }
        }
    }
}
#endif
