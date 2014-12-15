/*
 *	AttributeCache.cpp
 *	!CHAOS
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

#include <chaos/common/data/cache/AttributeCache.h>

using namespace chaos::common::data::cache;

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeCache::AttributeCache():
index(0),
mutex(new boost::shared_mutex()){
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeCache::~AttributeCache() {
}


/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::init(void *initData) throw(chaos::CException) {
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::deinit() throw(chaos::CException) {
	//remove all ValueSetting instance
	index = 0;
	mapAttributeNameIndex.clear();
	mapAttributeIndex.clear();
	bitmapChangedAttribute.clear();
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::addAttribute(const string& name,
								  uint32_t size,
								  chaos::DataType::DataType type) {
	if(mapAttributeNameIndex.count(name)) return;
	
	VariableIndexType tmpIndex;
	
	//add name nad his index
	mapAttributeNameIndex.insert(make_pair<string, VariableIndexType>(name, (tmpIndex=index++)));
	
	//add channel setting
	switch(type) {
		case chaos::DataType::TYPE_BOOLEAN:
			size = sizeof(bool);
			break;
		case chaos::DataType::TYPE_DOUBLE:
			size = sizeof(double);
			break;
		case chaos::DataType::TYPE_INT32:
			size = sizeof(int32_t);
			break;
		case chaos::DataType::TYPE_INT64:
			size = sizeof(int64_t);
			break;
			
		default:
			break;
	}
	boost::shared_ptr<AttributeValue> tmpSP(new AttributeValue(name, tmpIndex, size, type));
	
	//add the relative bit
	bitmapChangedAttribute.push_back(false);
	tmpSP->sharedBitmapChangedAttribute = &bitmapChangedAttribute;
	
	mapAttributeIndex.insert(make_pair<VariableIndexType, boost::shared_ptr<AttributeValue> >(tmpIndex, tmpSP));
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::getAttributeNames(std::vector<std::string>& names) {
	
	//get all names
	for(map<string, VariableIndexType>::iterator it = mapAttributeNameIndex.begin();
		it != mapAttributeNameIndex.end();
		it++) {
		names.push_back(it->first);
	}
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::setValueForAttribute(VariableIndexType n,
											 const void * value,
											 uint32_t size) {
	mapAttributeIndex[n]->setValue(value, size);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
VariableIndexType AttributeCache::getIndexForName(const std::string& name ) {
	if(!mapAttributeNameIndex.count(name)) {
		throw chaos::CException(-1, "No name present in Attribute setting", __PRETTY_FUNCTION__);
	}
	return mapAttributeNameIndex[name];
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeValue *AttributeCache::getValueSettingForIndex(VariableIndexType index) {
	if(!mapAttributeIndex.count(index)) {
		throw chaos::CException(-1, "No index present in Attribute setting", __PRETTY_FUNCTION__);
	}
	return mapAttributeIndex[index].get();
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
VariableIndexType AttributeCache::getNumberOfAttributes() {
	return index;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::getChangedIndex(std::vector<VariableIndexType>& changed_index) {
	size_t index = 0;
	index = bitmapChangedAttribute.find_first();
	while(index != boost::dynamic_bitset<BitBlockDimension>::npos) {
		changed_index.push_back(index);
		index = bitmapChangedAttribute.find_next(index);
	}
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::resetChangedIndex() {
	bitmapChangedAttribute.reset();
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeCache::markAllAsChanged() {
	bitmapChangedAttribute.set();
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeCache::hasChanged() {
	return bitmapChangedAttribute.any();
}
/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeCache::setNewSize(VariableIndexType attribute_index, uint32_t new_size) {
	return mapAttributeIndex[attribute_index]->setNewSize(new_size);
}