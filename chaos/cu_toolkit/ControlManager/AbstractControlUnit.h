/*
 *	ControlUnit.h
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
#ifndef ControlUnit_H
#define ControlUnit_H
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <chaos/common/data/cache/AttributesSetting.h>
#include <chaos/common/general/Configurable.h>
#include <chaos/common/action/ActionDescriptor.h>
#include <chaos/common/exception/CException.h>
#include <chaos/common/action/DeclareAction.h>
#include <chaos/common/utility/ArrayPointer.h>
#include <chaos/common/utility/StartableService.h>

#include <chaos/common/data/DatasetDB.h>

#include <chaos/cu_toolkit/ControlManager/AttributeSharedCacheWrapper.h>
#include <chaos/cu_toolkit/DataManager/KeyDataStorage.h>
#include <chaos/cu_toolkit/driver_manager/DriverErogatorInterface.h>

#define INIT_STATE      0
#define START_STATE     1

#define CU_OPT_IN_MEMORY_DATABASE "cu_db_in_memory"

#define CONTROL_UNIT_PUBLISH_NAME(impl)\
impl::PublishName

#define PUBLISHABLE_CONTROL_UNIT_IMPLEMENTATION(impl)\
const char * const  impl::PublishName = #impl;\

#define PUBLISHABLE_CONTROL_UNIT_INTERFACE(impl)\
public:\
static const char * const PublishName;\
private:\
//class impl : public subclass

namespace chaos{
    
    //forward event channel declaration
    namespace event{
        namespace channel {
            class InstrumentEventChannel;
        }
    }
    
    namespace cu {
        namespace control_manager {
			using namespace chaos::common::data;
			using namespace chaos::common::data::cache;
			using namespace chaos::cu::driver_manager;
			using namespace chaos::cu::driver_manager::driver;
			
			class ControlManager;
			class WorkUnitManagement;
            //!  Base class for control unit !CHAOS node
			/*!
			 This is the abstraction of the contorl unit node of CHAOS. This class extends DeclareAction
			 class that permit to publish rpc method for control the control unit life cycle. Most of the API
			 that needs to be used to create device and his dataset are contained into the DeviceSchemaDB class.
			 */
			class AbstractControlUnit :
			public DriverErogatorInterface,
			public DeclareAction,
			protected DatasetDB,
			public utility::StartableService {
				//friendly class declaration
				friend class ControlManager;
				friend class WorkUnitManagement;
				friend class DomainActionsScheduler;
				friend class SCAbstractControlUnit;
				friend class RTAbstractControlUnit;
			public:
				//! definition of the type for the driver list
				typedef std::vector<DrvRequestInfo>				ControlUnitDriverList;
				typedef std::vector<DrvRequestInfo>::iterator	ControlUnitDriverListIterator;
			private:
				std::string  control_key;
				//! contains the description of the type of the control unit
				std::string control_unit_type;
				
				//! is the unique identification code associated to the control unit instance(rand benerated by contructor)
				std::string control_unit_id;
				
				//! control unit load param
				std::string control_unit_param;
				
				//! control unit driver information list
				ControlUnitDriverList control_unit_drivers;
				
				//! list of the accessor of the driver requested by the unit implementation
				std::vector< DriverAccessor *> accessorInstances;
				
				//! attributed value shared cache
				/*!
				 The data in every attribute is published automatically on the chaos data service
				 with somne triggering logic according to the domain one.
				 */
				SharedCacheInterface *attribute_value_shared_cache;
				
				//! the wrapper for the user to isolate the shared cache
				AttributeSharedCacheWrapper *attribute_shared_cache_wrapper;
				
				//! fast access for acquisition timestamp
				ValueSetting *timestamp_acq_cached_value;
				
				/*!
				 Add a new KeyDataStorage for a specific key
				 */
				void setKeyDataStorage(data_manager::KeyDataStorage *_keyDatStorage);
				
				/*!
				 Initialize the Custom Contro Unit and return the configuration
				 */
				virtual CDataWrapper* _init(CDataWrapper*, bool& detachParam) throw(CException);
				
				/*!
				 Deinit the Control Unit
				 */
				virtual CDataWrapper* _deinit(CDataWrapper*, bool& detachParam) throw(CException);
				
				/*!
				 Starto the  Control Unit scheduling for device
				 */
				virtual CDataWrapper* _start(CDataWrapper*, bool& detachParam) throw(CException);
				
				/*!
				 Stop the Custom Control Unit scheduling for device
				 */
				virtual CDataWrapper* _stop(CDataWrapper*, bool& detachParam) throw(CException);
				
				/*!
				 Define the control unit DataSet and Action into
				 a CDataWrapper
				 */
				void _undefineActionAndDataset() throw(CException);
				
				// Startable Service method
				void init(void *initData) throw(CException);
				
				// Startable Service method
				void start() throw(CException);
				
				// Startable Service method
				void stop() throw(CException);
				
				// Startable Service method
				void deinit() throw(CException);
				
				//! initialize the dataset attributes (input and output)
				void initAttributeOnSharedAttributeCache(AttributeValueSharedCache::SharedVariableDomain domain,
														std::vector<string>& attribute_names);
				//! complete the output dataset cached with mandatory attribute
				/*!
				 The mandatory attribute like timestamp and triggered id are added after the user defined output aattribute
				 */
				virtual void completeOutputAttribute();

				//! complete the input attribute
				/*!
				 this method can be overloaded by subclass to add 
				 automatically generated input attribute
				 */
				virtual void completeInputAttribute();
				
				//! fill the array with the cached value, the array is used for the fast access of cached data
				void fillCachedValueVector(AttributesSetting& attribute_cache,
										   std::vector<ValueSetting*>& cached_value);
				
				//! filel the dataset packet for the cached attribute in the array
				inline void fillCDatawrapperWithCachedValue(std::vector<ValueSetting*>& cached_attributes,
															CDataWrapper& dataset);
			protected:
				//  It's is the dynamically assigned instance of the CU. it will be used
				// as domain for the rpc action.
				string control_unit_instance;
				
				//! Momentary driver for push data into the central memory
				data_manager::KeyDataStorage*  keyDataStorage;
				
				//! Event channel to permit the fire of the device event
				event::channel::InstrumentEventChannel *device_event_channel;
				
				
				//! fast cached attribute vector accessor
				std::vector<ValueSetting*> cache_output_attribute_vector;
				std::vector<ValueSetting*> cache_input_attribute_vector;
				std::vector<ValueSetting*> cache_custom_attribute_vector;
				std::vector<ValueSetting*> cache_system_attribute_vector;

				//! initialize system attribute
				virtual void initSystemAttributeOnSharedAttributeCache();
				
				//! Define the dataset information of the device implementeted into the CU
				/*!
				 This method configure the CDataWrapper whit all th einromation for describe the implemented device
				 */
				virtual void _defineActionAndDataset(CDataWrapper&) throw(CException);
				
				//! Get all managed declare action instance
				/*!
				 Return all the instance that implements actions to be published into network broker.
				 Subclass can override this method to inclued their action instancer
				 */
				virtual void _getDeclareActionInstance(std::vector<const DeclareAction *>& declareActionInstance);
				
                //! Set the dataset input element values
				/*!
				 This method is called when the input attribute of the dataset need to be valorized,
				 subclass need to perform all the appropiate action to set these attribute
				 */
				CDataWrapper* _setDatasetAttribute(CDataWrapper*, bool&) throw (CException);
				
                //! Return the state of the control unit
				/*!
				 Return the current control unit state identifyed by ControlUnitState types
				 fitted into the CDatawrapper with the key CUStateKey::CONTROL_UNIT_STATE
				 */
				CDataWrapper* _getState(CDataWrapper*, bool& detachParam) throw(CException);
				
				//! Return the information about the type of the current instace of control unit
				/*!
				 Return unit fitted into cdata wrapper:
				 CU type: string type associated with the key @CUDefinitionKey::CS_CM_CU_TYPE
				 */
				CDataWrapper* _getInfo(CDataWrapper*, bool& detachParam) throw(CException);
				
                //! Abstract Method that need to be used by the sublcass to define the dataset
				/*!
				 Subclass, in this method can call the api to create the dataset, after this method
				 this class will collet all the information and send all to the MDS server.
				 */
				virtual void unitDefineActionAndDataset() throw(CException) = 0;
				
				//! Abstract method for the definition of the driver
				/*!
				 This method permit to the control unit implementation to define the necessary drivers it needs
				 \param neededDriver need to be filled with the structure DrvRequestInfo filled with the information
				 about the needed driver [name, version and initialization param if preset statically]
				 */
				virtual void unitDefineDriver(std::vector<DrvRequestInfo>& neededDriver);
				
				//! abstract method to permit to the control unit to define custom attribute
				/*!
				 Custom attirbute, into the shared cache can be accessed globally into the control unit environment
				 they are publishe on every value change commit
				 */
				virtual void unitDefineCustomAttribute();
				
                //! Abstract method for the initialization of the control unit
				/*!
				 This is where the subclass need to be inizialize their environment, usually the hardware initialization. An exception
				 will stop the Control Unit live.
				 */
				virtual void unitInit() throw(CException) = 0;
				
                //! Abstract method for the start of the control unit
				/*!
				 This is where the subclass need to be start all the staff needed by normal control process. An exception
				 will stop the Control Unit live and perform the deinitialization of the control unit.
				 */
				virtual void unitStart() throw(CException) = 0;
				
				//! Abstract method for the stop of the control unit
				/*!
				 This is where the subclass need to be stop all the staff needed for pause the control process. An exception
				 will stop the Control Unit live and perform the deinitialization of the control unit.
				 */
				virtual void unitStop() throw(CException) = 0;
				
				//! Abstract method for the deinit of the control unit
				/*!
				 This is where the subclass need to be deinit all the staff that has been allocatate into the init method.
				 Usually the hardware deallocation etc..
				 */
				virtual void unitDeinit() throw(CException) = 0;
				
				
				//! this andler is called befor the input attribute will be updated
				virtual void unitInputAttributePreChangeHandler() throw(CException);
				
				//! attribute change handler
				/*!
				 the handle is fired after the input attribute cache as been update triggere
				 by the rpc request for attribute change.
				 */
				virtual void unitInputAttributeChangedHandler() throw(CException);
				
                //Abstract method used to sublcass to set theri needs
				/*!
				 Receive the event for set the dataset input element, this virtual method
				 is empty because can be used by controlunit implementation
				 */
				virtual CDataWrapper* setDatasetAttribute(CDataWrapper*, bool& detachParam) throw (CException);
				
                // Infrastructure configuration update
				/*!
				 This methdo is called when some configuration is received. All the Control Unit subsystem is
				 checked to control waht is needed to update. Subclass that override this method need first inherited
				 the parent one and the check if the CDataWrapper contains something usefull for it.
				 */
				virtual CDataWrapper* updateConfiguration(CDataWrapper*, bool&) throw (CException);
				
				//! return the accessor by an index
				/*
				 The index parameter correspond to the order that the driver infromation are
				 added by the unit implementation into the function AbstractControlUnit::unitDefineDriver.
				 */
				driver_manager::driver::DriverAccessor * getAccessoInstanceByIndex(int idx);
				
				//!return the accessor for the sared attribute cache
				inline
				AttributeSharedCacheWrapper * const getAttributeCache() {
					return attribute_shared_cache_wrapper;
				}
				
			public:
				
				//! Default Contructor
				/*!
				 \param _control_unit_type the superclass need to set the control unit type for his implementation
				 \param _control_unit_id unique id for the control unit
				 \param _control_unit_param is a string that contains parameter to pass during the contorl unit creation
				 */
				AbstractControlUnit(const std::string& _control_unit_type,
									const std::string& _control_unit_id,
									const std::string& _control_unit_param);
				
				//! Default Contructor
				/*!
				 \param _control_unit_type the superclass need to set the control unit type for his implementation
				 \param _control_unit_id unique id for the control unit
				 \param _control_unit_param is a string that contains parameter to pass during the contorl unit creation
				 \param _control_unit_drivers driver information
				 */
				AbstractControlUnit(const std::string& _control_unit_type,
									const std::string& _control_unit_id,
									const std::string& _control_unit_param,
									const ControlUnitDriverList& _control_unit_drivers);
				
				//! default destructor
				virtual ~AbstractControlUnit();
				
				
                //![API] Api for publish a lass method as RPC action
				/*!
				 This is a convenient method to register actiona associated to this instance of the class, class pointer
				 is associated directly to the current instance of this class and domain is fixed to the Control Unit instance.
				 Developer can use this method or the more flexible DeclareAction::addActionDescritionInstance.
				 */
				template<typename T>
				AbstActionDescShrPtr addActionDescritionInstance(T* objectReference,
																 typename ActionDescriptor<T>::ActionPointerDef actionHandler,
																 const char*const actionAliasName,
																 const char*const actionDescription) {
					//call the DeclareAction action register method, the domain will be associated to the control unit isntance
					return DeclareAction::addActionDescritionInstance(objectReference, actionHandler, control_unit_instance.c_str(), actionAliasName, actionDescription);
				}
				
				//! Return the contro unit instance
				const char * getCUInstance();
				
				//! Return the contro unit instance
				const char * getCUID();
				
				//! getControlUnitParam
				const string& getCUParam();
				
				//push output dataset
				virtual void pushOutputDataset();
				
				//push system dataset
				virtual void pushInputDataset();
				
				//push system dataset
				virtual void pushCustomDataset();
				
				//push system dataset
				virtual void pushSystemDataset();
			};
		}
    }
}
#endif
