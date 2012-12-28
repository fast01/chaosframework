/*
 *	Entity.h
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

#ifndef __CHAOSFramework__Entity__
#define __CHAOSFramework__Entity__

#include <vector>
#include <chaos/common/data/entity/Record.h>
#include <chaos/common/utility/Atomic.h>
namespace chaos {

    namespace edb {
        //forward declaration
        class EntityDB;
        struct KeyIdAndValue;
    }
    
    namespace entity {
       
        
        /*!
         This represent the base class for the entity
         */
        class Entity  : protected Record {
            friend class edb::EntityDB;
            
            atomic_int_type instanceID;
            
            int32_t entityID;
            
            Entity(edb::EntityDB *_database, atomic_int_type _instanceID);
            
            ~Entity();
            
            int32_t setEntityKeyAndInfo(chaos::edb::KeyIdAndValue& keyInfo);
        public:

            int32_t addProperty(chaos::edb::KeyIdAndValue& keyInfo);

            int32_t reset();
            
            int32_t getAllProperty(chaos::ArrayPointer<chaos::edb::KeyIdAndValue>& propertys);
        };
        
    }
    
}

#endif /* defined(__CHAOSFramework__Entity__) */
