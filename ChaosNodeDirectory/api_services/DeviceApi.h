//
//  DeviceApi.h
//  CHAOSFramework
//
//  Created by Claudio Bisegni on 2/26/13.
//  Copyright (c) 2013 INFN. All rights reserved.
//

#ifndef __CHAOSFramework__DeviceApi__
#define __CHAOSFramework__DeviceApi__

#include "ApiProvider.h"

namespace chaos_data = chaos::common::data;

namespace chaos {
    namespace cnd {
        namespace api {
            
            /*!
             
             */
            class DeviceApi: public ApiProvider {
               
                
            public:
                DeviceApi();
                ~DeviceApi();
                
                /*!
                 Register a device
                 */
				chaos_data::CDataWrapper* registerDevice(chaos_data::CDataWrapper *registrationdData, bool& detachParam) throw(CException);
            };
        }
    }
}

#endif /* defined(__CHAOSFramework__DeviceApi__) */
