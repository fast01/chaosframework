/*
 *	CUSchemaDB.cpp
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

#include <string>
#include <boost/lexical_cast.hpp>

#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/global.h>
#include <chaos/common/data/CUSchemaDB.h>
#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/data/CDataWrapper.h>

using namespace std;
using namespace boost;
using namespace chaos;
using namespace chaos::common::data;



#define MAKE_KEY(key, tmp) entityDB->getIDForKey(key, tmp);\
                           mapDatasetKeyForID.insert(make_pair<const char *, uint32_t>(key, tmp));

void RangeValueInfo::reset() {
    defaultValue.clear();
    maxRange.clear();
    minRange.clear();
    maxSize = 0;
    valueType = (DataType::DataType)0;
}

CUSchemaDB::CUSchemaDB() {
    initDB("CUSchemaDB", false);
}

/*!
 Parametrized constructor
 */
CUSchemaDB::CUSchemaDB(bool onMemory) {
    initDB("CUSchemaDB", onMemory);
}

/*!
 Parametrized constructor
 */
CUSchemaDB::CUSchemaDB(const char *databaseName, bool onMemory) {
    initDB(databaseName, onMemory);
}

void CUSchemaDB::initDB(const char *name, bool onMemory) {
    //create a database
    uint32_t keyTmp = 0;
    entityDB = ObjectFactoryRegister<edb::EntityDB>::getInstance()->getNewInstanceByName("SQLiteEntityDB");
    if(!entityDB) return;
    std::string composeName(name);
    composeName.append(UUIDUtil::generateUUIDLite());
    entityDB->initDB(composeName.c_str(), onMemory);
    
    MAKE_KEY(DatasetDefinitionkey::DEVICE_ID, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::ATTRIBUTE_NAME, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::DESCRIPTION, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::TIMESTAMP, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::ATTRIBUTE_TYPE, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::ATTRIBUTE_DIRECTION, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::MAX_RANGE, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::MIN_RANGE, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::DEFAULT_VALUE, keyTmp);
    MAKE_KEY(DatasetDefinitionkey::VALUE_MAX_SIZE, keyTmp);
    MAKE_KEY(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS, keyTmp);
}

CUSchemaDB::~CUSchemaDB() {
        //remove all dataset
    for (EntityPtrMapIterator deviceEntityIter = deviceEntityMap.begin();
         deviceEntityIter != deviceEntityMap.end();
         deviceEntityIter++) {
        delete (deviceEntityIter->second);
    }
    
        //dispose internal database
    if(entityDB) {
        entityDB->deinitDB();
        delete(entityDB);
    }
}

/*
 Compose the attribute name
*/
void CUSchemaDB::composeAttributeName(const char *deviceID, const char *attributeName, string& composedName) {
    composedName.assign(deviceID).append(":").append(attributeName);
} 

/*
 Compose the attribute name
*/ 
const char * CUSchemaDB::decomposeAttributeName(string& deviceID, string& attributeName) {
    return attributeName.substr(deviceID.size()+1).c_str();
}

/*
 return the vector containing the atrtibute list for a domain
 */
entity::Entity* CUSchemaDB::getDeviceEntity(const string& deviceID) {
    if(deviceEntityMap.count(deviceID) > 0){
        return deviceEntityMap[deviceID];
    }
        //a new vector need to be added
    addDeviceId(deviceID);
    return deviceEntityMap[deviceID];
} 


/*
 return the vector containing the atrtibute list for a domain
 */
void CUSchemaDB::addDeviceId(const string& deviceID) {
    if(deviceEntityMap.count(deviceID) > 0){
        return;
    }
    edb::KeyIdAndValue kiv;
    kiv.keyID = mapDatasetKeyForID[DatasetDefinitionkey::DEVICE_ID];
    kiv.type = chaos::edb::KEY_STR_VALUE;
    strcpy(kiv.value.strValue, deviceID.c_str());
        //add the entity for device
    entity::Entity *dsEntity = entityDB->getNewEntityInstance(kiv);
    if(dsEntity) {
        deviceEntityMap.insert(make_pair<string, entity::Entity*>(deviceID, dsEntity));
        addUniqueAttributeProperty(dsEntity, mapDatasetKeyForID[DatasetDefinitionkey::TIMESTAMP], timingUtils.getTimeStamp(), false);
    }
}

entity::Entity *CUSchemaDB::getDatasetElement(entity::Entity *device, string& attributeName) {
    return getDatasetElement(device, attributeName.c_str());
}

entity::Entity *CUSchemaDB::getDatasetElement(entity::Entity *device, const char * attributeName) {
    bool alreadyAdded = false;
    string a_name;
    edb::KeyIdAndValue kiv;
    kiv.keyID = mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_NAME];
    kiv.type = chaos::edb::KEY_STR_VALUE;
    composeAttributeName(device->getKeyInfo().value.strValue, attributeName, a_name);
    
    strcpy(kiv.value.strValue, a_name.c_str());
    
    ptr_vector<entity::Entity> childs;
    device->getHasChildByKeyInfo(kiv, alreadyAdded);
    if(alreadyAdded) {
        device->getChildsWithKeyInfo(kiv, childs);
        if(childs.size())
            return childs.release(childs.begin()).release();
        else
            return NULL;
    } else {
        entity::Entity *elementDst = entityDB->getNewEntityInstance(kiv);
        if(elementDst) {
            device->addChild(*elementDst);
        }
        return elementDst;
    }
}

/*
 Add the new field to the dataset
 */
void CUSchemaDB::addAttributeToDataSet(const char*const attributeDeviceID,
                                       const char*const attributeName,
                                       const char*const attributeDescription,
                                       DataType::DataType attributeType,
                                       DataType::DataSetAttributeIOAttribute attributeDirection,
                                       uint32_t maxDimension) {
    
    uint32_t typeMaxDimension = 0;
    
    switch (attributeType) {
        case DataType::TYPE_BOOLEAN:
            typeMaxDimension = sizeof(bool);   //bson type
            break;
        case DataType::TYPE_DOUBLE:
            typeMaxDimension = sizeof(double);	//8 bytes (64-bit IEEE 754 floating point)
            break;
        case DataType::TYPE_INT32:
            typeMaxDimension = sizeof(int32_t);
            break;
        case DataType::TYPE_INT64:
            typeMaxDimension = sizeof(int64_t);
            break;
        case DataType::TYPE_STRING:
            if(maxDimension == 0)
                throw CException(1, "for string type need to be specified the max value", "CUSchemaDB::addAttributeToDataSet");
            typeMaxDimension = maxDimension;
            break;
        case DataType::TYPE_BYTEARRAY:
            if(maxDimension == 0)
                throw CException(1, "for byte array type need to be specified the max value", "CUSchemaDB::addAttributeToDataSet");
            typeMaxDimension = maxDimension;
            break;
        default:
                throw CException(2, "unmanaged type", "CUSchemaDB::addAttributeToDataSet");
            break;
    }
    
    bool isChild = false;
    entity::Entity *device = getDeviceEntity(attributeDeviceID);
    
        //add the attribute
    string a_name;
    edb::KeyIdAndValue kiv;
    kiv.keyID = mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_NAME];
    kiv.type = chaos::edb::KEY_STR_VALUE;
    
    composeAttributeName(attributeDeviceID, attributeName, a_name);
    
    strcpy(kiv.value.strValue, a_name.c_str());
    
    auto_ptr<entity::Entity> elementDst(entityDB->getNewEntityInstance(kiv));
    
    if(elementDst.get()) {
        addUniqueAttributeProperty(elementDst.get(), mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION], attributeDescription);
        addUniqueAttributeProperty(elementDst.get(), mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_TYPE], (int64_t)attributeType);
        addUniqueAttributeProperty(elementDst.get(), mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DIRECTION], (int64_t)attributeDirection);
        addUniqueAttributeProperty(elementDst.get(), mapDatasetKeyForID[DatasetDefinitionkey::VALUE_MAX_SIZE], (int64_t)typeMaxDimension);
        device->isChild(*elementDst.get(), isChild);
        if(!isChild) device->addChild(*elementDst);
    }
}

void CUSchemaDB::clearAllAttributeForProperty(entity::Entity *attributeEntity, uint32_t keyIDToAdd) {
	ptr_vector<edb::KeyIdAndValue> properties_for_entity;
	attributeEntity->getPropertyByKeyID(keyIDToAdd, properties_for_entity);
	
	for (int idx = 0; idx < properties_for_entity.size(); idx++) {
		attributeEntity->deleteProrperty((&properties_for_entity[idx])->elementID);
	}
}

void CUSchemaDB::addUniqueAttributeProperty(entity::Entity *attributeEntity, uint32_t keyIDToAdd, const char * attributeValue, bool checkValueForUnicity) {
    ptr_vector<edb::KeyIdAndValue> keysAndValues;
    attributeEntity->getPropertyByKeyID(keyIDToAdd, keysAndValues);
    int idx = 0;
    bool found = keysAndValues.size() != 0;
    
    if(found && checkValueForUnicity) {
        found = false;
        for (; idx< keysAndValues.size(); idx++) {
            if(!strcmp((&keysAndValues[idx])->value.strValue, attributeValue)) {
                found = true;
                break;
            } else {
				attributeEntity->deleteProrperty((&keysAndValues[idx])->elementID);
			}
        }
    }
    
    if(!found) {
        attributeEntity->addProperty(keyIDToAdd, attributeValue);
    }
	//else {
      //  attributeEntity->updateProperty((&keysAndValues[idx])->elementID, keyIDToAdd, attributeValue);
 //   }
}

void CUSchemaDB::addUniqueAttributeProperty(entity::Entity *attributeEntity, uint32_t keyIDToAdd, string& attributeValue, bool checkValueForUnicity) {
    ptr_vector<edb::KeyIdAndValue> keysAndValues;
    attributeEntity->getPropertyByKeyID(keyIDToAdd, keysAndValues);
    int idx = 0;
    bool found = keysAndValues.size() != 0;
    
    if(found && checkValueForUnicity) {
        found = false;
        for (; idx< keysAndValues.size(); idx++) {
            if(!strcmp((&keysAndValues[idx])->value.strValue, attributeValue.c_str())) {
                found = true;
                break;
            } else {
				attributeEntity->deleteProrperty((&keysAndValues[idx])->elementID);
			}
        }
    }
    
    //if(!found) attributeEntity->addProperty(keyIDToAdd, attributeValue);
    if(!found) {
        attributeEntity->addProperty(keyIDToAdd, attributeValue);
    } /*else {
        attributeEntity->updateProperty((&keysAndValues[idx])->elementID, keyIDToAdd, attributeValue);
    }*/
}

void CUSchemaDB::addUniqueAttributeProperty(entity::Entity *attributeEntity, uint32_t keyIDToAdd, int64_t attributeValue, bool checkValueForUnicity) {
    ptr_vector<edb::KeyIdAndValue> keysAndValues;
    attributeEntity->getPropertyByKeyID(keyIDToAdd, keysAndValues);
    int idx = 0;
    bool found = keysAndValues.size() != 0;
    
    if(found && checkValueForUnicity) {
        found = false;
        for (; idx< keysAndValues.size(); idx++) {
            if((&keysAndValues[idx])->value.numValue == attributeValue) {
                found = true;
                break;
            } else {
				attributeEntity->deleteProrperty((&keysAndValues[idx])->elementID);
			}
        }
    }
    
    //if(!found)  attributeEntity->addProperty(keyIDToAdd, attributeValue);
    if(!found) {
        attributeEntity->addProperty(keyIDToAdd, attributeValue);
    } /*else {
        attributeEntity->updateProperty((&keysAndValues[idx])->elementID, keyIDToAdd, attributeValue);
    }*/
}

void CUSchemaDB::addUniqueAttributeProperty(entity::Entity *attributeEntity, uint32_t keyIDToAdd, double attributeValue, bool checkValueForUnicity) {
    ptr_vector<edb::KeyIdAndValue> keysAndValues;
    attributeEntity->getPropertyByKeyID(keyIDToAdd, keysAndValues);
    int idx = 0;
    bool found = keysAndValues.size() != 0;
    
    if(found && checkValueForUnicity) {
        found = false;
        for (; idx< keysAndValues.size(); idx++) {
            if((&keysAndValues[idx])->value.doubleValue == attributeValue) {
                found = true;
                break;
            } else {
				attributeEntity->deleteProrperty((&keysAndValues[idx])->elementID);
			}
        }
    }
    
    //if(!found)  attributeEntity->addProperty(keyIDToAdd, attributeValue);
    if(!found) {
        attributeEntity->addProperty(keyIDToAdd, attributeValue);
    } /*else {
        attributeEntity->updateProperty((&keysAndValues[idx])->elementID, keyIDToAdd, attributeValue);
    }*/
}
/*
 Add the new field at the CU dataset from the CDataWrapper
 */
void CUSchemaDB::addAttributeToDataSetFromDataWrapper(CDataWrapper& attributeDataWrapper) {
        //if(!attributeDataWrapper) return;
    
    string attributeDeviceID;
    string attributeName;
    string attributeDescription;
    auto_ptr<CDataWrapper> elementDescription;
    auto_ptr<CMultiTypeDataArrayWrapper> elementsDescriptions;
    
    if(!attributeDataWrapper.hasKey(DatasetDefinitionkey::DEVICE_ID)) return;
    attributeDeviceID = attributeDataWrapper.getStringValue(DatasetDefinitionkey::DEVICE_ID);
    //get the entity for device
    entity::Entity *deviceEntity = getDeviceEntity(attributeDeviceID);
    
    if(attributeDataWrapper.hasKey(DatasetDefinitionkey::DESCRIPTION)){
            //get the entity for device
        entity::Entity *deviceEntity = getDeviceEntity(attributeDeviceID);
        
        elementsDescriptions.reset(attributeDataWrapper.getVectorValue(DatasetDefinitionkey::DESCRIPTION));
        
        for (int idx = 0; idx < elementsDescriptions->size(); idx++) {
            
            
                //next element in dataset
            elementDescription.reset(elementsDescriptions->getCDataWrapperElementAtIndex(idx));
                //attribute name
            
            if(!elementDescription->hasKey(DatasetDefinitionkey::ATTRIBUTE_NAME))
                continue;
            
            
            string attrName = elementDescription->getStringValue(DatasetDefinitionkey::ATTRIBUTE_NAME);
            
                //get the attribute
            auto_ptr<entity::Entity> attributeEntity(getDatasetElement(deviceEntity, attrName));
            
                //attribute description
            if(elementDescription->hasKey(DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION)){
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION], elementDescription->getStringValue(DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION).c_str());
            }
                //attribute type
            if(elementDescription->hasKey(DatasetDefinitionkey::ATTRIBUTE_TYPE)) {
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_TYPE], (int64_t)elementDescription->getInt32Value(DatasetDefinitionkey::ATTRIBUTE_TYPE));
            }
                //attribute direction
            if(elementDescription->hasKey(DatasetDefinitionkey::ATTRIBUTE_DIRECTION)){
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DIRECTION], (int64_t)elementDescription->getInt32Value(DatasetDefinitionkey::ATTRIBUTE_DIRECTION));
            }
            
            if(elementDescription->hasKey(DatasetDefinitionkey::MAX_RANGE)) {
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::MAX_RANGE], elementDescription->getStringValue(DatasetDefinitionkey::MAX_RANGE).c_str());
            }
            
            if(elementDescription->hasKey(DatasetDefinitionkey::MIN_RANGE)){
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::MIN_RANGE], elementDescription->getStringValue(DatasetDefinitionkey::MIN_RANGE).c_str());
            }
            
            if(elementDescription->hasKey(DatasetDefinitionkey::DEFAULT_VALUE)){
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::DEFAULT_VALUE], elementDescription->getStringValue(DatasetDefinitionkey::DEFAULT_VALUE).c_str());
            }
            
            if(elementDescription->hasKey(DatasetDefinitionkey::VALUE_MAX_SIZE)){
                addUniqueAttributeProperty(attributeEntity.get(), mapDatasetKeyForID[DatasetDefinitionkey::VALUE_MAX_SIZE], (int64_t)elementDescription->getInt32Value(DatasetDefinitionkey::VALUE_MAX_SIZE));
            }
        }
     }
    
    //add now the server address for this device if sent
    if(attributeDataWrapper.hasKey(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS)) {
		//remove all stored server
		clearAllAttributeForProperty(deviceEntity, mapDatasetKeyForID[DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS]);
		
        //in the package has been sent the address where fir the data for this device
        auto_ptr<CMultiTypeDataArrayWrapper> serverVec(attributeDataWrapper.getVectorValue(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS));
        for (int idx = 0; idx < serverVec->size(); idx++) {
			//add new server
			deviceEntity->addProperty(mapDatasetKeyForID[DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS],  serverVec->getStringElementAtIndex(idx).c_str());
        }
    }
}

/*
 fill a CDataWrapper with the dataset decode
 */
void CUSchemaDB::fillDataWrapperWithDataSetDescription(CDataWrapper& datasetDescription) {
        //now i must describe the param for this action
        // map<string, entity::Entity*> deviceEntityMap
    if(deviceEntityMap.size()){
        boost::shared_ptr<CDataWrapper> datasetElementCDW;
        boost::shared_ptr<CDataWrapper> domainDatasetDescription;
        
            //there are some parameter for this action, need to be added to rapresentation
        for (EntityPtrMapIterator deviceEntityIter = deviceEntityMap.begin();
             deviceEntityIter != deviceEntityMap.end();
             deviceEntityIter++) {
                //get domain name
            string deviceId = deviceEntityIter->first;
            entity::Entity *deviceEntity = deviceEntityIter->second;

            auto_ptr<CDataWrapper> domainDatasetDescription(new CDataWrapper());
            
            fillDataWrapperWithDataSetDescription(deviceEntity, *domainDatasetDescription.get());
            
                // add parametere representation object to main action representation
            datasetDescription.appendCDataWrapperToArray(*domainDatasetDescription.get());
        }
        
        //close array for all device description [IN CASE DATA FOR INITIALIZE THE DEVICE ONLY one description need to be returne SO THIS FUNCTION NEED TO BE CHANGED TO SUPPORT ALSO ONE ONLY DEVICE DS CREATION]
        datasetDescription.finalizeArrayForKey(DatasetDefinitionkey::DESCRIPTION);
    }
}

/*!
 fill a CDataWrapper with the dataset decode
 */
void CUSchemaDB::fillDataWrapperWithDataSetDescription(string& deviceID, CDataWrapper& datasetDescription) {
    if(!deviceEntityMap.count(deviceID)) return;
    
    //get the netity for devceID
    entity::Entity *deviceEntity = deviceEntityMap[deviceID];
    
    fillDataWrapperWithDataSetDescription(deviceEntity, datasetDescription);
}

/*!
 fill a CDataWrapper with the dataset decode
 */
void CUSchemaDB::fillDataWrapperWithDataSetDescription(entity::Entity *deviceEntity, CDataWrapper& deviceDatasetDescription) {
    ptr_vector<edb::KeyIdAndValue> attrProperty;
    ptr_vector<entity::Entity> deviceDatasetAttribute;
    
    //add deviceID to description data
    deviceDatasetDescription.addStringValue(DatasetDefinitionkey::DEVICE_ID, deviceEntity->getKeyInfo().value.strValue);
    
    //set the registered timestamp
    deviceEntity->getPropertyByKeyID(mapDatasetKeyForID[DatasetDefinitionkey::TIMESTAMP], attrProperty);
    if(attrProperty.size()) {
        deviceDatasetDescription.addInt64Value(DatasetDefinitionkey::TIMESTAMP, (uint64_t)(&attrProperty[0])->value.numValue);
        attrProperty.release();
    }
    
    //try to get all dataset attribute for device entity
    deviceEntity->getChildsWithKeyID(mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_NAME], deviceDatasetAttribute);
    
    //get dataset attribute for domain name
    
    //convenient array for element porperty
    
    for (ptr_vector<entity::Entity>::iterator dstElmtIterator = deviceDatasetAttribute.begin();
         dstElmtIterator != deviceDatasetAttribute.end();
         dstElmtIterator++) {
        
        //get next dst element entity for get the attribute
        entity::Entity *dstAttrEntity = &(*dstElmtIterator);
        
        //get all entity property
        attrProperty.release();
        dstAttrEntity->getAllProperty(attrProperty);
        if(attrProperty.size() == 0) continue;
        
        //cicle all dataset element
        auto_ptr<CDataWrapper> datasetElementCDW(new CDataWrapper());
        
        fillCDataWrapperDSAtribute(datasetElementCDW.get(), deviceEntity, dstAttrEntity, attrProperty);
        
        // add parametere representation object to main action representation
        deviceDatasetDescription.appendCDataWrapperToArray(*datasetElementCDW.get());
        //CDataWrapper *data = *datasetIterator;
    }
    
    //close the dataset attribute array
    deviceDatasetDescription.finalizeArrayForKey(DatasetDefinitionkey::DESCRIPTION);
    
    //add now the server address for this device if sent
    attrProperty.clear();
    deviceEntity->getPropertyByKeyID(mapDatasetKeyForID[DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS], attrProperty);
    for (ptr_vector<edb::KeyIdAndValue>::iterator iter = attrProperty.begin();
         iter != attrProperty.end();
         iter++) {
        edb::KeyIdAndValue *kivPtr = &(*iter);
        deviceDatasetDescription.appendStringToArray(kivPtr->value.strValue);
    }
    deviceDatasetDescription.finalizeArrayForKey(DataProxyConfigurationKey::CS_DM_LD_SERVER_ADDRESS);
}

/*
 */
void CUSchemaDB::fillCDataWrapperDSAtribute(CDataWrapper *dsAttribute, entity::Entity *deviceIDEntity, entity::Entity *attrEntity, ptr_vector<edb::KeyIdAndValue>& attrProperty) {
    
    //add name
    edb::KeyIdAndValue *curKIV = NULL;
    string dID = deviceIDEntity->getKeyInfo().value.strValue;
    string attr = attrEntity->getKeyInfo().value.strValue;
    string attrDecomposed = attr.substr(dID.size()+1);
    
    dsAttribute->addStringValue(DatasetDefinitionkey::ATTRIBUTE_NAME, attrDecomposed);
    
    for (ptr_vector<edb::KeyIdAndValue>::iterator dstElmtIterator = attrProperty.begin();
         dstElmtIterator != attrProperty.end();
         dstElmtIterator++) {
        //cicle the property
        curKIV = &(*dstElmtIterator);
        if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION]) {
            dsAttribute->addStringValue(DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION, curKIV->value.strValue);
        } else if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DIRECTION]) {
            dsAttribute->addInt32Value(DatasetDefinitionkey::ATTRIBUTE_DIRECTION, (int32_t)curKIV->value.numValue);
        } else if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::MAX_RANGE]) {
            dsAttribute->addStringValue(DatasetDefinitionkey::MAX_RANGE, curKIV->value.strValue);
        } else if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::MIN_RANGE]) {
            dsAttribute->addStringValue(DatasetDefinitionkey::MIN_RANGE, curKIV->value.strValue);
        } else if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::DEFAULT_VALUE]){
            dsAttribute->addStringValue(DatasetDefinitionkey::DEFAULT_VALUE, curKIV->value.strValue);
        } else if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::VALUE_MAX_SIZE]){
            dsAttribute->addInt32Value(DatasetDefinitionkey::VALUE_MAX_SIZE, (int32_t)curKIV->value.numValue);
        } else if(curKIV->keyID == mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_TYPE]) {
            dsAttribute->addInt32Value(DatasetDefinitionkey::ATTRIBUTE_TYPE, (int32_t)curKIV->value.numValue);
        } else {
            //custom attribute
        }
    }
}

/*
 return al domain
 */
void CUSchemaDB::getAllDeviceId(vector<string>& deviceNames) {
    for (EntityPtrMapIterator deviceEntityIter = deviceEntityMap.begin();
         deviceEntityIter != deviceEntityMap.end();
         deviceEntityIter++) {
        
            //add domain name
        deviceNames.push_back(deviceEntityIter->first);
    }
}

/*
 return al domain
 */
bool CUSchemaDB::deviceIsPresent(const string& deviceID) {
    return deviceEntityMap.count(deviceID)==1;
}

void CUSchemaDB::getDeviceDatasetAttributesName(const string& deviceID,
                                                vector<string>& attributesName) {
    entity::Entity *deviceEntity = getDeviceEntity(deviceID);
    ptr_vector<entity::Entity> attrDst;
    deviceEntity->getChildsWithKeyID(mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DIRECTION], attrDst);
    for (ptr_vector<entity::Entity>::iterator attrEntityIter = attrDst.begin();
         attrEntityIter != attrDst.end();
         attrEntityIter++) {
        entity::Entity *e = &(*attrEntityIter);
        attributesName.push_back(e->getKeyInfo().value.strValue);
    }
}

void CUSchemaDB::getDeviceDatasetAttributesName(const string& deviceID,
                                                DataType::DataSetAttributeIOAttribute directionType,
                                                vector<string>& attributesName) {
    //get the device entity
    ptr_vector<entity::Entity> entityVec;
    entity::Entity *deviceEntity = getDeviceEntity(deviceID);
    
    edb::KeyIdAndValue kiv;
    kiv.keyID = mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DIRECTION];
    kiv.type = edb::KEY_NUM_VALUE;
    kiv.value.numValue = static_cast<int64_t>(directionType);
    
    deviceEntity->getChildWithPropertyKeyIDandValue(kiv, entityVec);
    //create all attribute datawrapper for all entity
    ptr_vector<edb::KeyIdAndValue> attrProperty;
    for (ptr_vector<entity::Entity>::iterator dstElmtIterator = entityVec.begin();
         dstElmtIterator != entityVec.end();
         dstElmtIterator++) {
        
        entity::Entity *e = &(*dstElmtIterator);
        string attrName = e->getKeyInfo().value.strValue;
        attributesName.push_back(attrName.substr(deviceID.length()+1));
    }
}

void CUSchemaDB::getDeviceAttributeDescription(const string& deviceID,
                                               const string& attributesName,
                                               string& attributeDescription) {
    ptr_vector<edb::KeyIdAndValue> attrPropertyVec;
    entity::Entity *deviceEntity = getDeviceEntity(deviceID);

    auto_ptr<entity::Entity> attributeDstEntity(getDatasetElement(deviceEntity, attributesName.c_str()));
    if(!attributeDstEntity.get()) return;
    
    attributeDstEntity->getPropertyByKeyID(mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DESCRIPTION], attrPropertyVec);
    if(!attrPropertyVec.size()) return;
    
    attributeDescription = (&attrPropertyVec[0])->value.strValue;
}

int CUSchemaDB::getDeviceAttributeRangeValueInfo(const string& deviceID,
                                                  const string& attributesName,
                                                  RangeValueInfo& rangeInfo) {
    
    string a_name;
    edb::KeyIdAndValue kiv;
    vector<uint32_t> keyToGot;
    ptr_vector<entity::Entity> attrEntityVec;
    ptr_vector<edb::KeyIdAndValue> attrPropertyVec;

    kiv.keyID = mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_NAME];
    kiv.type = edb::KEY_STR_VALUE;
    
    composeAttributeName(deviceID.c_str(), attributesName.c_str(), a_name);
    
    strcpy(kiv.value.strValue, a_name.c_str());
    entity::Entity *deviceEntity = getDeviceEntity(deviceID);
    
    //get all
    deviceEntity->getChildsWithKeyInfo(kiv, attrEntityVec);
    
    //check if we ha found the attribute
    if(!attrEntityVec.size()) return 1;
    
    
    uint32_t keyIdAttrMaxRng = mapDatasetKeyForID[DatasetDefinitionkey::MAX_RANGE];
    uint32_t keyIdAttrMinRng = mapDatasetKeyForID[DatasetDefinitionkey::MIN_RANGE];
    uint32_t keyIdAttrDefaultValue = mapDatasetKeyForID[DatasetDefinitionkey::DEFAULT_VALUE];
    uint32_t keyIdAttrType = mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_TYPE];
    uint32_t keyIdAttrMaxSize = mapDatasetKeyForID[DatasetDefinitionkey::VALUE_MAX_SIZE];
    
    keyToGot.push_back(keyIdAttrMaxRng);
    keyToGot.push_back(keyIdAttrMinRng);
    keyToGot.push_back(keyIdAttrDefaultValue);
    keyToGot.push_back(keyIdAttrType);
    keyToGot.push_back(keyIdAttrMaxSize);
    
    (&attrEntityVec[0])->getPropertyByKeyID(keyToGot, attrPropertyVec);
    if(!attrPropertyVec.size()) return 1;
    
    for (ptr_vector<edb::KeyIdAndValue>::iterator attrPropertyIterator = attrPropertyVec.begin();
         attrPropertyIterator != attrPropertyVec.end();
         attrPropertyIterator++) {
        
        edb::KeyIdAndValue *kivPtr = &(*attrPropertyIterator);
        
        if(kivPtr->keyID == keyIdAttrMaxSize) {
            rangeInfo.maxSize = (uint32_t)kivPtr->value.numValue;
        } else if(kivPtr->keyID == keyIdAttrMaxRng) {
            rangeInfo.maxRange = kivPtr->value.strValue;
        } else if(kivPtr->keyID == keyIdAttrMinRng) {
            rangeInfo.minRange = kivPtr->value.strValue;
        } else if(kivPtr->keyID == keyIdAttrType) {
            rangeInfo.valueType = (DataType::DataType)kivPtr->value.numValue;
        } else if(kivPtr->keyID == keyIdAttrDefaultValue) {
            switch (kivPtr->type) {
                case edb::KEY_DOUBLE_VALUE:
                    rangeInfo.defaultValue = lexical_cast<string>(kivPtr->value.doubleValue);
                    break;
                case edb::KEY_NUM_VALUE:
                    rangeInfo.defaultValue = lexical_cast<string>(kivPtr->value.numValue);
                break;
                case edb::KEY_STR_VALUE:
                    rangeInfo.defaultValue = kivPtr->value.strValue;
                break;
            }
            
        }
    }
	return 0;
}

//!Set the range values for an attribute
/*!
 set the range value for the attribute
 \param deviceID the identification of the device
 \param attributesName the name of the attribute
 \param rangeInfo the range and default value of the attribute, the fields
 of the struct are not cleaned, so if an attrbute doesn't has
 some finromation, relative field are not touched.
 */
void CUSchemaDB::setDeviceAttributeRangeValueInfo(const string& deviceID,
                                                  const string& attributesName,
                                                  RangeValueInfo& rangeInfo) {

    entity::Entity *deviceEntity = getDeviceEntity(deviceID);

    //get the attribute
    auto_ptr<entity::Entity> attributeEntity(getDatasetElement(deviceEntity, attributesName.c_str()));
    
    uint32_t keyIdAttrMaxRng = mapDatasetKeyForID[DatasetDefinitionkey::MAX_RANGE];
    uint32_t keyIdAttrMinRng = mapDatasetKeyForID[DatasetDefinitionkey::MIN_RANGE];
    uint32_t keyIdAttrDefaultValue = mapDatasetKeyForID[DatasetDefinitionkey::DEFAULT_VALUE];

    addUniqueAttributeProperty(attributeEntity.get(), keyIdAttrMaxRng, rangeInfo.maxRange);
    addUniqueAttributeProperty(attributeEntity.get(), keyIdAttrMinRng, rangeInfo.minRange);
    addUniqueAttributeProperty(attributeEntity.get(), keyIdAttrDefaultValue, rangeInfo.defaultValue);
}

int CUSchemaDB::getDeviceAttributeDirection(const string& deviceID,
                                            const string& attributesName,
                                            DataType::DataSetAttributeIOAttribute& directionType) {

    ptr_vector<edb::KeyIdAndValue> attrPropertyVec;
    entity::Entity *deviceEntity = getDeviceEntity(deviceID);
    //check if we ha found the attribute
    if(!deviceEntity) return 1;
        
    auto_ptr<entity::Entity> attributeDstEntity(getDatasetElement(deviceEntity, attributesName.c_str()));
    
    attributeDstEntity->getPropertyByKeyID(mapDatasetKeyForID[DatasetDefinitionkey::ATTRIBUTE_DIRECTION], attrPropertyVec);
    if(!attrPropertyVec.size()) return 1;
    
    directionType = static_cast<DataType::DataSetAttributeIOAttribute>((&attrPropertyVec[0])->value.numValue);
    return 0;
}
