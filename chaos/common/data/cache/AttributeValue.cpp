/*
 *	AttributesValue.cpp
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

#include <chaos/common/global.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/data/cache/AttributeValue.h>


#define AVLAPP_ LAPP_ << "[AttributeValue -" << "] - "
#define AVLDBG_ LDBG_ << "[AttributeValue -" << "] " << __PRETTY_FUNCTION__ << " - "
#define AVLERR_ LERR_ << "[AttributeValue -" << "] " << __PRETTY_FUNCTION__ << "(" << __LINE__ << ") - "

using namespace chaos::common::data;
using namespace chaos::common::data::cache;

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeValue::AttributeValue(const std::string& _name,
							   uint32_t _index,
							   uint32_t _size,
							   chaos::DataType::DataType _type):
value_buffer(NULL),
size(_size),
name(_name),
index(_index),
type(_type) {
	if(size) {
		value_buffer = std::calloc(size, 1);
		if(!value_buffer) {
			AVLERR_ << "error allcoating current_value memory";
		}
	}
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeValue::~AttributeValue() {
	if(value_buffer) free(value_buffer);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::setValue(const void* value_ptr,
							uint32_t value_size,
							bool tag_has_changed) {
	if(value_size>size) {
		value_size = size;
	}
	CHAOS_ASSERT(value_buffer)
	
	//copy the new value
	std::memcpy(value_buffer, value_ptr, value_size);
	
	//set the relative field for set has changed
	if(tag_has_changed) sharedBitmapChangedAttribute->set(index);
	return true;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::setNewSize(uint32_t _new_size) {
	bool result = true;
	switch(type) {
		case chaos::DataType::TYPE_BYTEARRAY:
		case chaos::DataType::TYPE_STRING:
			value_buffer = (double*)realloc(value_buffer, (size = _new_size)*2);
			if((result = (value_buffer != NULL))) {
				std::memset(value_buffer, 0, size);
			}
			break;
		default:
			break;
	}
	return result;
}


/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeValue::markAsChanged() {
	sharedBitmapChangedAttribute->set(index);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeValue::markAsUnchanged() {
	sharedBitmapChangedAttribute->reset(index);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::isGood() {
	return value_buffer!= NULL;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
CDataWrapper *AttributeValue::getValueAsCDatawrapperPtr(bool from_json) {
	CDataWrapper *result = NULL;
	if(!from_json) {
		result = new CDataWrapper((const char *)value_buffer);
	} else {
		result = new CDataWrapper();
		result->setSerializedJsonData((const char *)value_buffer);
	}
	return result;
}