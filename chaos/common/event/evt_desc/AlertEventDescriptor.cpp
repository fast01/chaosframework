/*
 *	AlertEventDescriptor.cpp
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

#include <chaos/common/utility/endianess.h>
#include <chaos/common/event/evt_desc/AlertEventDescriptor.h>

using namespace chaos;
using namespace chaos::event;
using namespace chaos::event::alert;


AlertEventDescriptor::AlertEventDescriptor():EventDescriptor(EventTypeAlert, EVT_ALERT_DEFAULT_PRIORITY)  {

}

    //!Set the code of the alert
/*
 Set the allert code for this event
 \param alertCode thecode of the alert
 */
void AlertEventDescriptor::setAlertCode(EventAlertCode alertCode) {
    setSubCode(alertCode);
}

    //!Return the code of this alert
/*
 Return the alert code identified bythis event
 \return the code of the alert
 */
EventAlertCode AlertEventDescriptor::getAlertCode(){
    return (EventAlertCode)getSubCode();
}

    //!Set the code of the alert
/*
 Set the allert code for this event
 \param alertCode thecode of the alert
 */
void AlertEventDescriptor::setAlertCustomCode(uint16_t alertCustomCode) {
    setSubCode(alertCustomCode + EventAlertLastCodeNumber);
}

    //!Return the code of this alert
/*
 Return the alert code identified bythis event
 \return the code of the alert
 */
uint16_t AlertEventDescriptor::getAlertCustomCode(){
    return (getSubCode() - EventAlertLastCodeNumber);
}

/*!
 Set the Value for the type
 \param valueType the enumeration that descrive the type of the value
 \param valuePtr a pointer to the value
 \param valueSizethe size of the value
 */
void AlertEventDescriptor::setAlert(const char * const indetifier, uint8_t identifierLength, uint16_t alertCode, uint16_t priority, EventDataType valueType, const void *valuePtr, uint16_t valueSize) {
        //2 byte
    setAlertCode((EventAlertCode)alertCode);
        //2 byte
    setSubCodePriority(priority);
        //set the dimension, 10 is the fixed size of all information for alert pack
    EventDescriptor::setIdentificationAndValueWithType(indetifier, identifierLength, valueType, valuePtr, valueSize);
}

/*!
 Set the Value for the custom type
 \param valueType the enumeration that descrive the type of the value with EventDataType constant
 \param valuePtr a pointer to the value
 \param valueSizethe size of the value
 */
void AlertEventDescriptor::setCustomAlert(const char * const indetifier, uint8_t identifierLength, uint16_t alertCustomCode, uint16_t priority, EventDataType valueType, const void *valuePtr, uint16_t valueSize) {
        //2 byte
    setAlertCustomCode(alertCustomCode);
        //2 byte
    setSubCodePriority(priority);
        //set the dimension, 10 is the fixed size of all information for alert pack
    EventDescriptor::setIdentificationAndValueWithType(indetifier, identifierLength, valueType, valuePtr, valueSize);
}
