/*
 *	Entity.cpp
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

#include <chaos/common/data/entity/Entity.h>
#include <chaos/common/data/entity_db/EntityDB.h>

using namespace chaos;
using namespace chaos::entity;

Entity::Entity(edb::EntityDB *_database,  atomic_int_type _instanceID):Record(_database),instanceID(_instanceID){
}

Entity::~Entity(){
    
}

int32_t Entity::setEntityKeyAndInfo(chaos::edb::KeyIdAndValue& keyInfo) {
    //register the information for the entity
    return database->getIDForEntity(keyInfo, entityID);
}

int32_t Entity::addProperty(edb::KeyIdAndValue& keyInfo) {
    int32_t error = 0;
    int32_t idNewProperty = 0;
    if(!(error = database->addNewPropertyForEntity(entityID, keyInfo, idNewProperty))) {
        return error;
    }
    return error;
}

int32_t Entity::getAllProperty(chaos::ArrayPointer<chaos::edb::KeyIdAndValue>& propertys) {
    int32_t error = 0;
    if(!(error = database->searchPropertyForEntity(entityID, propertys))) {
        return error;
    }
    return error;
}

int32_t Entity::reset() {
    return database->deleteAllPropertyForEntity(entityID);
}
