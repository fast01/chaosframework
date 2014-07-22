/*
 *	ControlManager.cpp
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

#include <chaos/common/global.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/cu_toolkit/ControlManager/ControlManager.h>
#include <chaos/cu_toolkit/CommandManager/CommandManager.h>
#include <chaos/common/configuration/GlobalConfiguration.h>
#include <chaos/common/event/channel/InstrumentEventChannel.h>

#include <boost/scoped_ptr.hpp>

#define LCMAPP_ LAPP_ << "[Control Manager] - "
#define LCMDBG_ LDBG_ << "[Control Manager] - "
#define LCMERR_ LERR_ << "[Control Manager]"<<__LINE__<<" - "
#define WAITH_TIME_FOR_CU_REGISTRATION 2000000
#define WU_IDENTIFICATION(x) std::string(x->getCUInstance()) + std::string("-") + std::string(x->getCUID())



namespace chaos_data = chaos::common::data;
namespace chaos_async = chaos::common::async_central;
namespace cu_driver_manager = chaos::cu::driver_manager;

using namespace chaos;
using namespace chaos::cu::command_manager;
using namespace chaos::cu::control_manager;
using namespace std;

/*
 Constructor
 */
ControlManager::ControlManager() {}

/*
 Desctructor
 */
ControlManager::~ControlManager() {
}


/*
 Initialize the CU Instantiator
 */
void ControlManager::init(void *initParameter) throw(CException) {
	//control manager action initialization
    
	AbstActionDescShrPtr actionDescription;
	use_unit_server =	GlobalConfiguration::getInstance()->hasOption(CONTROL_MANAGER_UNIT_SERVER_ENABLE) &&
	GlobalConfiguration::getInstance()->getOption<bool>(CONTROL_MANAGER_UNIT_SERVER_ENABLE);
    if(use_unit_server) {
		LCMAPP_  << "Enable unit server";
		
		if(!GlobalConfiguration::getInstance()->hasOption(CONTROL_MANAGER_UNIT_SERVER_ALIAS)) {
			throw CException(-1, "No server alias param found", __PRETTY_FUNCTION__);
		}
		
		unit_server_alias = GlobalConfiguration::getInstance()->getOption<std::string>(CONTROL_MANAGER_UNIT_SERVER_ALIAS);
		
		//init CU action
		actionDescription = DeclareAction::addActionDescritionInstance<ControlManager>(this,
																					   &ControlManager::loadControlUnit,
																					   ChaosSystemDomainAndActionLabel::SYSTEM_DOMAIN,
																					   ChaosSystemDomainAndActionLabel::ACTION_LOAD_CONTROL_UNIT,
																					   "Control Unit load system action");
		//add parameter for control unit name
		actionDescription->addParam(ChaosSystemDomainAndActionLabel::PARAM_CU_LOAD_UNLOAD_ALIAS,
									DataType::TYPE_STRING,
									"Alias of the control unit to load");
		//add parameter for driver initialization string
		actionDescription->addParam(ChaosSystemDomainAndActionLabel::PARAM_CU_LOAD_DRIVER_PARAMS,
									DataType::TYPE_STRING,
									"Driver params (pipe separated '|') to pass to the control unit instance");
		
		//deinit CU action
		actionDescription = DeclareAction::addActionDescritionInstance<ControlManager>(this,
																					   &ControlManager::unloadControlUnit,
																					   ChaosSystemDomainAndActionLabel::SYSTEM_DOMAIN,
																					   ChaosSystemDomainAndActionLabel::ACTION_UNLOAD_CONTROL_UNIT,
																					   "Control Unit unload system action");
		//add parameter for control unit name
		actionDescription->addParam(ChaosSystemDomainAndActionLabel::PARAM_CU_LOAD_UNLOAD_ALIAS,
									DataType::TYPE_STRING,
									"Alias of the control unit to unload");
		
		actionDescription = DeclareAction::addActionDescritionInstance<ControlManager>(this,
																					   &ControlManager::unitServerRegistrationACK,
																					   ChaosSystemDomainAndActionLabel::SYSTEM_DOMAIN,
																					   ChaosSystemDomainAndActionLabel::ACTION_UNIT_SERVER_REG_ACK,
																					   "Unit server registration ack message");
	}
    
    actionDescription = DeclareAction::addActionDescritionInstance<ControlManager>(this,
                                                                                   &ControlManager::updateConfiguration,
                                                                                   "commandManager",
                                                                                   "updateConfiguration",
                                                                                   "Update Command Manager Configuration");
	actionDescription = DeclareAction::addActionDescritionInstance<ControlManager>(this,
                                                                                   &ControlManager::workUnitRegistrationACK,
                                                                                   ChaosSystemDomainAndActionLabel::SYSTEM_DOMAIN,
                                                                                   ChaosSystemDomainAndActionLabel::ACTION_WORK_UNIT_REG_ACK,
                                                                                   "Update Command Manager Configuration");
	//register command manager action
    CommandManager::getInstance()->registerAction(this);
    
    LCMAPP_ << "Get the Metadataserver channel";
    mds_channel = CommandManager::getInstance()->getMetadataserverChannel();
    if(mds_channel) LCMAPP_ << "Metadataserver has been allocated";
    else throw CException(-2, "Error allcoating metadata server channel", __PRETTY_FUNCTION__);
}

/*
 Initialize the CU Instantiator
 */
void ControlManager::start() throw(CException) {
    LCMAPP_  << "Start cu scan timer";
	if(use_unit_server){
		//add unit server registration managment timer
		chaos_async::AsyncCentralManager::getInstance()->addTimer(this, 0, GlobalConfiguration::getInstance()->getOption<uint64_t>(CONTROL_MANAGER_UNIT_SERVER_REGISTRATION_RETRY_MSEC));
	} else {
		startControlUnitSMThread();
	}
}

// start control units state machine thread
void ControlManager::startControlUnitSMThread() {
	thread_run = true;
	thread_registration.reset(new boost::thread(boost::bind(&ControlManager::manageControlUnit, this)));
}

// stop control unit state machien thread
void ControlManager::stopControlUnitSMThread(bool whait) {
	thread_run = false;
	thread_waith_semaphore.unlock();
	if(thread_registration.get() && whait) thread_registration->join();
}

/*
 Initialize the CU Instantiator
 */
void ControlManager::stop() throw(CException) {
    LCMAPP_  << "Stop cu scan timer";
	stopControlUnitSMThread();
}

/*
 Deinitialize the CU Instantiator
 */
void ControlManager::deinit() throw(CException) {
    bool detachFake = false;
    std::vector<const chaos::DeclareAction * > cuDeclareActionsInstance;
    vector<string> allCUDeviceIDToStop;
    LCMAPP_  << "Deinit the Control Manager";
    LCMAPP_  << "system action deinitialization";
	//deregistering the action
    CommandManager::getInstance()->deregisterAction(this);
    LCMAPP_  << "system action deinitialized";
    
    
    LCMAPP_  << "Deinit all the submitted Control Unit";
	
    for (map<string, shared_ptr<WorkUnitManagement> >::iterator cuIter = map_cuid_registered_instance.begin();
		 cuIter != map_cuid_registered_instance.end();
		 cuIter++ ){
        shared_ptr<WorkUnitManagement> cu = (*cuIter).second;
        
        LCMAPP_  << "Deregister RPC action for cu whith instance:" << WU_IDENTIFICATION(cu->work_unit_instance);
        cu->work_unit_instance->_getDeclareActionInstance(cuDeclareActionsInstance);
        for(int idx = 0; idx < cuDeclareActionsInstance.size(); idx++) {
            CommandManager::getInstance()->deregisterAction((chaos::DeclareAction *)cuDeclareActionsInstance[idx]);
        }
        
		//load all device id for this cu
        allCUDeviceIDToStop.push_back(cu->work_unit_instance->getDeviceID());
        
        for (vector<string>::iterator iter =  allCUDeviceIDToStop.begin();
             iter != allCUDeviceIDToStop.end();
             iter++) {
            
			//stop all itnerna device
            
            CDataWrapper fakeDWForDeinit;
            fakeDWForDeinit.addStringValue(DatasetDefinitionkey::DEVICE_ID, *iter);
            try{
                LCMAPP_  << "Stopping Control Unit: " << WU_IDENTIFICATION(cu->work_unit_instance);
                cu->work_unit_instance->_stop(&fakeDWForDeinit, detachFake);
            }catch (CException& ex) {
                if(ex.errorCode != 1){
					//these exception need to be logged
                    DECODE_CHAOS_EXCEPTION(ex);
                }
            }
            
            try{
                LCMAPP_  << "Deiniting Control Unit: " << WU_IDENTIFICATION(cu->work_unit_instance);
                cu->work_unit_instance->_deinit(&fakeDWForDeinit, detachFake);
            }catch (CException& ex) {
                if(ex.errorCode != 1){
					//these exception need to be logged
                    DECODE_CHAOS_EXCEPTION(ex);
                }
            }
            try{
				LCMAPP_  << "Undefine Action And Dataset for  Control Unit: " << WU_IDENTIFICATION(cu->work_unit_instance);
                cu->work_unit_instance->_undefineActionAndDataset();
            }  catch (CException& ex) {
                if(ex.errorCode != 1){
					//these exception need to be logged
                    DECODE_CHAOS_EXCEPTION(ex);
                }
            }
        }
        LCMAPP_  << "Dispose event channel for Control Unit Sanbox:" << WU_IDENTIFICATION(cu->work_unit_instance);
        CommandManager::getInstance()->deleteEventChannel(cu->work_unit_instance->deviceEventChannel);
        cu->work_unit_instance->deviceEventChannel = NULL;
        cuDeclareActionsInstance.clear();
        LCMAPP_  << "Unload" << cu->work_unit_instance->getCUInstance();
    }
    map_cuid_registered_instance.clear();
	map_cuid_registering_instance.clear();
}



/*
 Submit a new Control unit for operation
 */
void ControlManager::submitControlUnit(AbstractControlUnit *data) throw(CException) {
	//lock the hastable of cu instance and managmer
    boost::unique_lock<boost::shared_mutex> lock(mutex_queue_submitted_cu);
    queue_submitted_cu.push(data);
	
	//unlock thread
	thread_waith_semaphore.unlock();
}

void ControlManager::migrateStableAndUnstableSMCUInstance() {
	//shared access for the read of map
	//get the upgradeable lock
	UpgradeableLock registering_lock(mutex_map_cuid_registering_instance);
	UpgradeableLock registered_lock(mutex_map_cuid_registered_instance);
	
    for (map<string, shared_ptr<WorkUnitManagement> >::iterator i = map_cuid_registering_instance.begin();
		 i != map_cuid_registering_instance.end();
		 i++ ){
		
		if(!i->second->smNeedToSchedule()) {
			UpgradeReadToWriteLock registering_wlock(registering_lock);
			UpgradeReadToWriteLock registered_wlock(registered_lock);
			
			//now i can write on the two map
			map_cuid_registered_instance.insert(make_pair(i->first, i->second));
			
			//remove the iterator
			map_cuid_registering_instance.erase(i);
		}
	}
}

//! Make one steps in SM for all registring state machine
void ControlManager::makeSMSteps() {
	//lock for read the registering map
	ReadLock read_registering_lock(mutex_map_cuid_registering_instance);
	
	for (map<string, shared_ptr<WorkUnitManagement> >::iterator i = map_cuid_registering_instance.begin();
		 i != map_cuid_registering_instance.end();
		 i++ ){
		//make step
		if(i->second->smNeedToSchedule()) i->second->scheduleSM();
	}
}

void ControlManager::manageControlUnit() {
	//initialize the Control Unit
 	
	boost::unique_lock<boost::shared_mutex> lock(mutex_queue_submitted_cu, boost::defer_lock);
	while(thread_run) {
		//lock queue
		lock.lock();
		
		//try to consume all the submitted control unit instance (fter the lock no other thread can submit new on)
		while(!queue_submitted_cu.empty()) {
			//we have new instance to manage
			shared_ptr<WorkUnitManagement> wui(new WorkUnitManagement(queue_submitted_cu.front()));
			LCMAPP_  << "We have a new control unit isntance:" << WU_IDENTIFICATION(wui->work_unit_instance);
			
			//remove the oldest data
			queue_submitted_cu.pop();
			
			//activate the sm for register the control unit instance
			wui->turnOn();
			
			LCMAPP_  << "Create manager for new control unit:" << WU_IDENTIFICATION(wui->work_unit_instance);
			
			//! lock the hastable
			ReadLock read_registering_lock(mutex_map_cuid_registering_instance);
			LCMDBG_  << "Added to registering map" << WU_IDENTIFICATION(wui->work_unit_instance);
			
			// we can't have two different work unit with the same unique identifier within the same process
			if(map_cuid_registering_instance.count(wui->work_unit_instance->getCUInstance())) {
				LCMERR_  << "Duplicated control unit instance " << WU_IDENTIFICATION(wui->work_unit_instance);
				return;
			}
			
			//now we can proceed, add the network broker instance to the managment class of the work unit
			wui->mds_channel = mds_channel;
			
			//add sandbox to all map of running cu
			map_cuid_registering_instance.insert(make_pair(wui->work_unit_instance->getCUInstance(), wui));
		}
		lock.unlock();
		
		//migrate stable <-> unstable
		migrateStableAndUnstableSMCUInstance();
		
		//! lock the registering (unstable sm) hastable
		ReadLock read_registering_lock(mutex_map_cuid_registering_instance);
		//schedule unstable state machine steps
		if(map_cuid_registering_instance.size()) {
			//whe have control unit isntance with unstable state machine
			makeSMSteps();
			//waith some time to retry the state machine
			thread_waith_semaphore.wait(5000);
		} else {
			//we don'need to do anything else
			thread_waith_semaphore.wait();
		}
	}
}

/*
 
 */
int ControlManager::sendConfPackToMDS(CDataWrapper& dataToSend) {
	// dataToSend can't be sent because it is porperty of the CU
	//so we need to copy it
	
	auto_ptr<SerializationBuffer> serBuf(dataToSend.getBSONData());
	CDataWrapper *mdsPack = new CDataWrapper(serBuf->getBufferPtr());
	//add action for metadata server
	//add local ip and port
	
	mdsPack->addStringValue(CUDefinitionKey::CS_CM_CU_INSTANCE_NET_ADDRESS, GlobalConfiguration::getInstance()->getLocalServerAddressAnBasePort().c_str());
	
	//register CU from mds
	return mds_channel->sendUnitDescription(mdsPack, true, WAITH_TIME_FOR_CU_REGISTRATION);
}

#define CDW_HAS_KEY(x) message_data->hasKey(x)
#define CDW_STR_KEY(x) CDW_HAS_KEY(x)?message_data->getStringValue(x):""

//! message for load operation
CDataWrapper* ControlManager::loadControlUnit(CDataWrapper *message_data, bool& detach) throw (CException) {
	//check param
	IN_ACTION_PARAM_CHECK(!message_data, -1, "No param found")
	IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_ALIAS), -2, "No instancer alias")
	IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_UNLOAD_CONTROL_UNIT_DEVICE_ID), -2, "No id for the work unit instance found")
	
	std::string alias = message_data->getStringValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_ALIAS);
	LCMDBG_ << "Get new request for instance the work unit with alias:" << alias;
	
	WriteLock write_instancer_lock(mutex_map_cu_instancer);
	IN_ACTION_PARAM_CHECK(!map_cu_alias_instancer.count(alias), -2, "No work unit instancer's found for the alias")
	
	string device_id = message_data->getStringValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_UNLOAD_CONTROL_UNIT_DEVICE_ID);
	string load_options = CDW_STR_KEY(ChaosSystemDomainAndActionLabel::PARAM_LOAD_UNLOAD_CONTROL_UNIT_DEVICE_ID);
	
	
	LCMDBG_ << "instantiate work unit ->" << "device_id:" <<device_id<< " load_options:"<< load_options;

	//scan all the driver description forwarded
	std::vector<cu_driver_manager::driver::DrvRequestInfo> driver_params;
	if(message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC)) {
		LCMDBG_ << "Driver param has been supplyed";
		boost::scoped_ptr<CMultiTypeDataArrayWrapper> driver_descriptions(message_data->getVectorValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC));
		//scan all the driver description
		LCMDBG_ << "scan " << driver_descriptions->size() << " driver descriptions";
		for( int idx = 0; idx < driver_descriptions->size(); idx++) {
			LCMDBG_ << "scan " << idx << " driver";
			boost::scoped_ptr<CDataWrapper> driver_desc(driver_descriptions->getCDataWrapperElementAtIndex(idx));
			IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC_NAME), -3, "No driver name found")
			IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC_VERSION), -3, "No driver version found")
			IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC_INIT_PARAM), -3, "No driver init param name found")
			LCMDBG_ << "scan " << idx << " driver";
			cu_driver_manager::driver::DrvRequestInfo drv = {message_data->getStringValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC_NAME).c_str(),
																message_data->getStringValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC_VERSION).c_str(),
																message_data->getStringValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_CONTROL_UNIT_DRIVER_DESC_INIT_PARAM).c_str()};
			LCMDBG_ << "adding driver  " << drv.alias << "["<<drv.version << "-" << drv.init_parameter<<"]";
			driver_params.push_back(drv);
		}
	}
	
	//submit new instance of the requested control unit
	submitControlUnit(map_cu_alias_instancer[device_id]->getInstance(device_id, load_options, driver_params));
	return NULL;
}

//! message for unload operation
CDataWrapper* ControlManager::unloadControlUnit(CDataWrapper *message_data, bool& detach) throw (CException) {
	IN_ACTION_PARAM_CHECK(!message_data, -1, "No param found")
	IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_UNLOAD_CONTROL_UNIT_DEVICE_ID), -2, "No device id found")
	IN_ACTION_PARAM_CHECK(false, -3, "Not yet implemented")
	return NULL;
}


//! ack received for the registration of the uwork unit
CDataWrapper* ControlManager::workUnitRegistrationACK(CDataWrapper *message_data, bool &detach) throw (CException) {
	IN_ACTION_PARAM_CHECK(!message_data, -1, "No param found")
	IN_ACTION_PARAM_CHECK(!message_data->hasKey(ChaosSystemDomainAndActionLabel::PARAM_LOAD_UNLOAD_CONTROL_UNIT_DEVICE_ID), -2, "No device id found")
	
	//lock the registering control unit map
	ReadLock read_registering_lock(mutex_map_cuid_registering_instance);
	std::string device_id = message_data->getStringValue(ChaosSystemDomainAndActionLabel::PARAM_LOAD_UNLOAD_CONTROL_UNIT_DEVICE_ID);
	
	IN_ACTION_PARAM_CHECK(!map_cuid_registering_instance.count(device_id), -3, "No registering work unit found with the device id")

	//we can process the ack into the right manager
	map_cuid_registering_instance[device_id]->manageACKPack(*message_data);
	return NULL;
}

/*
 Configure the sandbox and all subtree of the CU
 */
CDataWrapper* ControlManager::updateConfiguration(CDataWrapper *message_data, bool& detach) {
	return NULL;
}

//---------------unit server state machine managment handler
void ControlManager::timeout() {
	boost::unique_lock<boost::shared_mutex> lock_sm(unit_server_sm_mutex);
	switch (unit_server_sm.current_state()[0]) {
			//Unpublished
		case 0:
			LCMDBG_ << "[Unpublished] Send first registration pack to mds";
			if(use_unit_server) {
				if(unit_server_sm.process_event(unit_server_state_machine::UnitServerEventType::UnitServerEventTypePublishing()) == boost::msm::back::HANDLED_TRUE){
					//gone to publishing
					sendUnitServerRegistration();
				} else {
					LCMERR_ << "[Unpublished] i can't be here";
				}
			} else {
				LCMDBG_ << "[Publishing] Unit server registration not sucessfull, turn off the timer";
				chaos_async::AsyncCentralManager::getInstance()->removeTimer(this);
			}
			break;
			//Publishing
		case 1:
			LCMDBG_ << "[Publishing] Send another registration pack to mds";
			sendUnitServerRegistration();
			break;
			//Published
		case 2:
			LCMDBG_ << "[Published] Unit server registration completed, turn off the timer";
			chaos_async::AsyncCentralManager::getInstance()->removeTimer(this);
			
			LCMDBG_ << "[Published] Start control units registration state machine";
			startControlUnitSMThread();
			break;
			//Published failed
		case 3:
			LCMDBG_ << "[Published failed] Perform Unpublishing state";
			chaos_async::AsyncCentralManager::getInstance()->removeTimer(this);
			use_unit_server = false;
			break;
	}
}

//!prepare and send registration pack to the metadata server
void ControlManager::sendUnitServerRegistration() {
	chaos_data::CDataWrapper *unit_server_registration_pack = new chaos_data::CDataWrapper();
	//set server alias
	unit_server_registration_pack->addStringValue(ChaosSystemDomainAndActionLabel::MDS_REGISTER_UNIT_SERVER_ALIAS, unit_server_alias);
	
	//add control unit alias
	for(MapCUAliasInstancerIterator iter = map_cu_alias_instancer.begin();
		iter != map_cu_alias_instancer.end();
		iter++) {
		unit_server_registration_pack->appendStringToArray(iter->first);
	}
	unit_server_registration_pack->finalizeArrayForKey(ChaosSystemDomainAndActionLabel::MDS_REGISTER_UNIT_SERVER_CONTROL_UNIT_ALIAS);
	mds_channel->sendUnitServerRegistration(unit_server_registration_pack);
}

// Server registration ack message
CDataWrapper* ControlManager::unitServerRegistrationACK(CDataWrapper *message_data, bool &detach) throw (CException) {
	//lock the sm access
	boost::unique_lock<boost::shared_mutex> lock_sm(unit_server_sm_mutex);
	LCMAPP_ << "Unit server registration ack message received";
	detach = false;
	if(!message_data->hasKey(ChaosSystemDomainAndActionLabel::MDS_REGISTER_UNIT_SERVER_ALIAS))
		throw CException(-1, "No alias forwarder", __PRETTY_FUNCTION__);
	
	string server_alias = message_data->getStringValue(ChaosSystemDomainAndActionLabel::MDS_REGISTER_UNIT_SERVER_ALIAS);
	if(server_alias.compare(unit_server_alias) != 0) {
		throw CException(-2, "Server alias not found", __PRETTY_FUNCTION__);
	}
	if(message_data->hasKey(ChaosSystemDomainAndActionLabel::MDS_REGISTER_UNIT_SERVER_RESULT)) {
		//registration has been ended
		switch(message_data->getInt32Value(ChaosSystemDomainAndActionLabel::MDS_REGISTER_UNIT_SERVER_RESULT)){
			case ErrorCode::EC_MDS_UNIT_SERV_REGISTRATION_OK:
				LCMAPP_ << "Registration is gone well";
				if(unit_server_sm.process_event(unit_server_state_machine::UnitServerEventType::UnitServerEventTypePublished()) == boost::msm::back::HANDLED_TRUE){
					//we are published and it is ok!
				} else {
					throw CException(ErrorCode::EC_MDS_UNIT_SERV_BAD_US_SM_STATE, "Bad state of the sm for published event", __PRETTY_FUNCTION__);
				}
				break;
				
			case ErrorCode::EC_MDS_UNIT_SERV_REGISTRATION_FAILURE_DUPLICATE_ALIAS:
				LCMERR_ << "The " << unit_server_alias << " is already used";
				//turn of unit server
				if(unit_server_sm.process_event(unit_server_state_machine::UnitServerEventType::UnitServerEventTypeFailure()) == boost::msm::back::HANDLED_TRUE){
					//we have problem
				} else {
					throw CException(ErrorCode::EC_MDS_UNIT_SERV_BAD_US_SM_STATE, "Bad state of the sm for unpublishing event", __PRETTY_FUNCTION__);
				}
				break;
				
			case ErrorCode::EC_MDS_UNIT_SERV_REGISTRATION_FAILURE_INVALID_ALIAS:
				LCMERR_ << "The " << unit_server_alias << " is invalid";
				//turn of unit server
				LCMDBG_ << "Turning of unit server";
				if(unit_server_sm.process_event(unit_server_state_machine::UnitServerEventType::UnitServerEventTypeFailure()) == boost::msm::back::HANDLED_TRUE){
					//we have problem
					
				} else {
					throw CException(ErrorCode::EC_MDS_UNIT_SERV_BAD_US_SM_STATE, "Bad state of the sm for unpublished event", __PRETTY_FUNCTION__);
				}
				break;
		}
		//repeat fast as possible the timer
		chaos_async::AsyncCentralManager::getInstance()->removeTimer(this);
		chaos_async::AsyncCentralManager::getInstance()->addTimer(this, 0, GlobalConfiguration::getInstance()->getOption<uint64_t>(CONTROL_MANAGER_UNIT_SERVER_REGISTRATION_RETRY_MSEC));
	} else {
		throw CException(-3, "No result received", __PRETTY_FUNCTION__);
	}
	return NULL;
}
