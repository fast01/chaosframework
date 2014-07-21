/*
 *	ControlManager.h
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
#ifndef ChaosFramework_ControlManager_h
#define ChaosFramework_ControlManager_h

#include <queue>
#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

#include <chaos/common/utility/Singleton.h>
#include <chaos/common/action/DeclareAction.h>
#include <chaos/common/exception/CException.h>
#include <chaos/common/general/Configurable.h>
#include <chaos/common/message/MDSMessageChannel.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/thread/WaitSemaphore.h>
#include <chaos/common/async_central/async_central.h>

#include <chaos/cu_toolkit/ControlManager/AbstractControlUnit.h>
#include <chaos/cu_toolkit/ControlManager/WorkUnitManagement.h>

#include <boost/thread.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>

//startup parameter for unit server
#define CONTROL_MANAGER_UNIT_SERVER_ENABLE							"unit_server_enable"
#define CONTROL_MANAGER_UNIT_SERVER_ENABLE_desc						"Enable the unit server on this instance of process"

#define CONTROL_MANAGER_UNIT_SERVER_ALIAS							"unit_server_alias"
#define CONTROL_MANAGER_UNIT_SERVER_ALIAS_desc						"Alias used to publish the unit server"
#define CONTROL_MANAGER_UNIT_SERVER_REGISTRATION_RETRY_MSEC			"unit_server_retry_ms"
#define CONTROL_MANAGER_UNIT_SERVER_REGISTRATION_RETRY_MSEC_desc	"Delay in milliseconds for the registration retry"
#define CONTROL_MANAGER_UNIT_SERVER_REGISTRATION_RETRY_MSEC_DEFAULT	30000
//define the type for the Control Unit isntancer

namespace chaos {
    namespace cu {
		namespace control_manager {
			using namespace std;
			using namespace boost;
			
			typedef chaos::common::utility::ObjectInstancerP3<AbstractControlUnit,
			const std::string&,
			const std::string&,
			const AbstractControlUnit::ControlUnitDriverList&> CUObjectInstancer;
			
			
			namespace unit_server_state_machine {
				
				//Unit server state machine
				namespace UnitServerEventType {
					// unit server need to register hiself to the mds
					struct UnitServerEventTypeUnpublished {};
					// unit server is registering and he's waiting for approval
					struct UnitServerEventTypePublishing {};
					// unti server is registered
					struct UnitServerEventTypePublished {};
					// unit server is registering and he's waiting for approval
					struct UnitServerEventTypeFailure {};
				}
				
				// Unit Server state
				struct Unpublished : public boost::msm::front::state<>{};
				
				struct Publishing : public boost::msm::front::state<> {};
				
				struct Published : public boost::msm::front::state<> {};
				
				struct PublishingFailure  : public boost::msm::front::state<> {};
				
				// front-end: define the FSM structure
				struct us_state_machine : public boost::msm::front::state_machine_def<us_state_machine> {
					
					// the initial state of the player SM. Must be defined
					typedef Unpublished initial_state;
					
					typedef boost::msm::front::Row <  Unpublished   ,  UnitServerEventType::UnitServerEventTypePublishing  , Publishing, boost::msm::front::none , boost::msm::front::none > unpub_pubing_row;
					typedef boost::msm::front::Row <  Publishing   ,  UnitServerEventType::UnitServerEventTypePublished  , Published, boost::msm::front::none , boost::msm::front::none > pubing_pub_row;
					typedef boost::msm::front::Row <  Publishing   ,  UnitServerEventType::UnitServerEventTypeFailure  , PublishingFailure, boost::msm::front::none , boost::msm::front::none > pubbing_pubfail_row;
					typedef boost::msm::front::Row <  Published   ,  UnitServerEventType::UnitServerEventTypeUnpublished  , Unpublished, boost::msm::front::none , boost::msm::front::none > pub_unpub_row;
					
					// Transition table for initialization services
					struct transition_table : boost::mpl::vector<
					unpub_pubing_row,
					pubing_pub_row,
					pubbing_pubfail_row,
					pub_unpub_row> {};
					
					template <class FSM,class Event>
					void no_transition(Event const& ,FSM&, int ){}
				};
			}
			//type definition of the state machine
			typedef boost::msm::back::state_machine< unit_server_state_machine::us_state_machine >	WorkUnitServerStateMachine;
			
			
			//!Manager for the Control Unit managment
			/*!
			 This class is the central point qhere control unit are managed. Two api are exposed
			 via RPC to load and unload the control unit giving some parameter.
			 */
			class ControlManager :
			public DeclareAction,
			public chaos::utility::StartableService,
			protected chaos::common::async_central::TimerHandler,
			public Singleton<ControlManager> {
				friend class Singleton<ControlManager>;
				mutable boost::shared_mutex mutex_registration;
				
				//unit server state machine
				bool						use_unit_server;
				std::string					unit_server_alias;
				boost::shared_mutex			unit_server_sm_mutex;
				WorkUnitServerStateMachine	unit_server_sm;
				
				bool thread_run;
				chaos::WaitSemaphore thread_waith_semaphore;
				boost::scoped_ptr<boost::thread> thread_registration;
				
				//! metadata server channel for control unit registration
				MDSMessageChannel *mdsChannel;
				
				//!queue for control unit waiting to be published
				queue< AbstractControlUnit* > submittedCUQueue;
				
				//! control unit instance mapped with their unique identifier
				map<string, shared_ptr<WorkUnitManagement> > map_control_unit_instance;
				
				//! association by alias and control unit instancer
				typedef std::map<string, boost::shared_ptr<CUObjectInstancer> >::iterator MapCUAliasInstancerIterator;
				std::map<string, boost::shared_ptr<CUObjectInstancer> > map_cu_alias_instancer;
				
				//----------private method-----------
				//! send register control unit to the mds.
				int sendConfPackToMDS(CDataWrapper&);
				
				/*
				 Constructor
				 */
				ControlManager();
				
				/*
				 Desctructor
				 */
				~ControlManager();
				
				void _loadWorkUnit();
				void _unloadControlUnit();
				//!prepare and send registration pack to the metadata server
				void sendUnitServerRegistration();
			protected:
				//! timer fire method
				void manageControlUnit();
				
				//TimerHandler callback
				void timeout();
				
				//! start control units state machine thread
				void startControlUnitSMThread();
				
				//! stop control unit state machien thread
				void stopControlUnitSMThread(bool whait = true);
			public:
				
				/*
				 Initialize the Control Manager
				 */
				void init(void *initParameter) throw(CException);
				
				/*
				 Start the Control Manager
				 */
				void start() throw(CException);
				
				
				/*!
				 Stop the control manager
				 */
				void stop() throw(CException);
				
				/*
				 Deinitialize the Control Manager
				 */
				void deinit() throw(CException);
				
				/*
				 Submit a new Control unit instance
				 */
				void submitControlUnit(AbstractControlUnit *control_unit_instance) throw(CException);
				
#define TO_STRING(x) #x
				//! control unit registration
				/*!
				 Register a control unit instancer associating it to an alias
				 \param control_unit_alias the alias associated to the tempalte class identification
				 */
				template<typename ControlUnitClass>
				void registerControlUnit() {
					map_cu_alias_instancer.insert(make_pair(CONTROL_UNIT_PUBLISH_NAME(ControlUnitClass),
															boost::shared_ptr<CUObjectInstancer>(ALLOCATE_INSTANCER_P3(ControlUnitClass,										//Control unit implementation
																													   AbstractControlUnit,										//Control unit base class
																													   const std::string&,										//Control Unit unique id param
																													   const std::string&,										//control unit load param
																													   const AbstractControlUnit::ControlUnitDriverList&))));	//Control unit driver list
				}
				
				
				/*!
				 Action that show the unit server registration result(success or failure)
				 */
				CDataWrapper* unitServerRegistrationACK(CDataWrapper *messageData, bool &detach) throw (CException);
				
				/*!
				 Action for loading a control unit
				 */
				CDataWrapper* loadControlUnit(CDataWrapper *messageData, bool &detach) throw (CException);
				
				/*!
				 Action for the unloading of a control unit
				 The unload operation, check that the target control unit is in deinit state
				 */
				CDataWrapper* unloadControlUnit(CDataWrapper *messageData, bool &detach) throw (CException);
				
				/*!
				 Configure the sandbox and all subtree of the CU
				 */
				CDataWrapper* updateConfiguration(CDataWrapper*, bool&);
			};
		}
	}
}
#endif
