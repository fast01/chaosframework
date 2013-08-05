/*
 *	AttributeSetting.h
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

#ifndef __CHAOSFramework__AttributeSetting__
#define __CHAOSFramework__AttributeSetting__

#include <map>
#include <string>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>

#include <chaos/common/chaos_constants.h>
#include <chaos/common/utility/InizializableService.h>

#include <chaos/cu_toolkit/ControlManager/slow_command/SlowCommandTypes.h>

namespace chaos{
    namespace cu {
        namespace control_manager {
            namespace slow_command {
                
                using namespace std;
                
                class AttributeSetting;

                typedef void * SettingValuePtr;
                
            
                struct ValueSetting {
                    friend class AttributeSetting;
                    const uint32_t                      index;
                    const uint32_t                      size;
                    const chaos::DataType::DataType     type;
                    boost::mutex mutextAccessSetting;
                    
                    SettingValuePtr currentValue;
                    SettingValuePtr nextValue;
                    boost::dynamic_bitset<BitBlockDimension> * sharedBitmapChangedAttribute;

                    ValueSetting(uint32_t _size, uint32_t _index, chaos::DataType::DataType type);
                    
                    ~ValueSetting();
                    
                    void completed();
                    
                    void completedWithError();
                    
                    bool setNextValue(const void* valPtr, uint32_t _size);
                    
                    bool setDefaultValue(const void* valPtr, uint32_t _size);
                    
                    template<typename T>
                    T* getNextValue() {
                        return static_cast<T*>(nextValue);
                    }
                    
                    template<typename T>
                    T* getCurrentValue() {
                        return static_cast<T*>(currentValue);
                    }
                };

                //-----------------------------------------------------------------------------------------------------------------------------

                
                class AttributeSetting : public utility::InizializableService {
                    
                    AttributeIndexType index;

                    boost::dynamic_bitset<BitBlockDimension> bitmapChangedAttribute;
                    
                    map<string, AttributeIndexType> mapAttributeNameIndex;
                    map<AttributeIndexType, boost::shared_ptr<ValueSetting> > mapAttributeIndexSettings;
                public:
                    
                    //! This field point to a custom memory shared by cu and all command
                    void *customData;
                    
                    AttributeSetting();
                    
                    ~AttributeSetting();
                    
                    void addAttribute(string name, uint32_t size, chaos::DataType::DataType type);
                    
                    void setValueForAttribute(AttributeIndexType n, const void * value, uint32_t size);
                    void setDefaultValueForAttribute(AttributeIndexType n, const void * value, uint32_t size);
                    
                    AttributeIndexType getIndexForName( string name );
                    
                    void getAttributeNames(std::vector<std::string>& names);
                    
                    //! Initialize instance
                    void init(void *initData) throw(chaos::CException);
                    
                    //! Deinit the implementation
                    void deinit() throw(chaos::CException);
                    
                    void getChangedIndex(std::vector<AttributeIndexType>& changedIndex);
                    
                    ValueSetting *getValueSettingForIndex(AttributeIndexType index);
                };
                
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__AttributeSetting__) */