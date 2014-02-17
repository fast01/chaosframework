/*
 *	Startab.hleService
 *	!CHOAS
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
#ifndef __CHAOSFramework__StartableService__
#define __CHAOSFramework__StartableService__

#include <chaos/common/utility/InizializableService.h>

namespace chaos {
    
    namespace utility {
        
        namespace service_state_machine {
            //SM Event
            namespace EventType {
                struct start {};
                struct stop {};
            }
            
            // States
            struct Started : public boost::msm::front::state<> {};
            struct Stopped : public boost::msm::front::state<> {};
            
            // front-end: define the FSM structure
            struct ss_state_machine : public boost::msm::front::state_machine_def<ss_state_machine>  {

                
                typedef boost::msm::front::Row <  service_state_machine::Deinitilized  ,  EventType::initialize    , service_state_machine::Initialized   , boost::msm::front::none , boost::msm::front::none > deinit_init_row;
                typedef boost::msm::front::Row <  service_state_machine::Initialized   ,  EventType::deinitialize  , service_state_machine::Deinitilized  , boost::msm::front::none , boost::msm::front::none > init_deinit_row;
                typedef boost::msm::front::Row <  service_state_machine::Initialized   ,  EventType::start         , service_state_machine::Started       , boost::msm::front::none , boost::msm::front::none > init_start_row;
                typedef boost::msm::front::Row <  service_state_machine::Started       ,  EventType::stop          , service_state_machine::Stopped       , boost::msm::front::none , boost::msm::front::none > start_stop_row;
                typedef boost::msm::front::Row <  service_state_machine::Stopped       ,  EventType::start         , service_state_machine::Started       , boost::msm::front::none , boost::msm::front::none > stop_start_row;
                typedef boost::msm::front::Row <  service_state_machine::Stopped       ,  EventType::deinitialize  , service_state_machine::Deinitilized  , boost::msm::front::none , boost::msm::front::none > stop_deinit_row;

                // the initial state of the player SM. Must be defined
                typedef Deinitilized initial_state;
                
                // Transition table for Startable services and his subclass
                struct transition_table : boost::mpl::vector< deinit_init_row, init_deinit_row, init_start_row, start_stop_row, stop_start_row, stop_deinit_row > {};
                
                template <class FSM,class Event>
                void no_transition(Event const& ,FSM&, int ) {}
            };
        }

        class StartableService : public InizializableService {
            boost::msm::back::state_machine< service_state_machine::ss_state_machine > state_machine;
		protected:
			
			//! Start the implementation
            virtual void start() throw(chaos::CException);
            
			//! Stop the implementation
            virtual void stop() throw(chaos::CException);
			
        public:
            StartableService();
            virtual ~StartableService();

            
            static bool startImplementation(StartableService& impl, const char * const implName,  const char * const domainString);
            static bool stopImplementation(StartableService& impl, const char * const implName,  const char * const domainString);
            
            static bool startImplementation(StartableService *impl, const char * const implName,  const char * const domainString);
            static bool stopImplementation(StartableService *impl, const char * const implName,  const char * const domainString);
            
            static bool initImplementation(StartableService& impl, void *initData, const char * const implName,  const char * const domainString);
            static bool deinitImplementation(StartableService& impl, const char * const implName,  const char * const domainString);
            
            static bool initImplementation(StartableService *impl, void *initData, const char * const implName,  const char * const domainString);
            static bool deinitImplementation(StartableService *impl, const char * const implName,  const char * const domainString);
        };
		
		template<typename T>
		class StartableServiceContainer {
			bool delete_on_dispose;
            const char * service_name;
			T *startable_service_instance;
		public:
			StartableServiceContainer(bool _delete_on_dispose, const char * instance_name):delete_on_dispose(_delete_on_dispose), startable_service_instance(new T()), service_name(instance_name) {}
			StartableServiceContainer(T *instance, bool _delete_on_dispose, const char * instance_name):startable_service_instance(instance), delete_on_dispose(_delete_on_dispose), service_name(instance_name) {}
			~StartableServiceContainer() {
				if(delete_on_dispose) delete(startable_service_instance);
			}
			
			bool init(void *init_data, const char * const domainString) {
				return StartableService::initImplementation(startable_service_instance, init_data, service_name, domainString);
			}
			
			bool start(const char * const domainString) {
				return StartableService::startImplementation(startable_service_instance, service_name, domainString);
			}
			
			bool stop(const char * const domainString) {
				return StartableService::stopImplementation(startable_service_instance, service_name, domainString);
			}
			
			bool deinit(const char * const domainString) {
				return StartableService::deinitImplementation(startable_service_instance, service_name, domainString);
			}
			
			inline T* getPointer() {
				return startable_service_instance;
			}
		};
        
        
        #define ALLOCATE_SS_CONTAINER(T, b) new chaos::utility::StartableServiceContainer<T>(b, #T)
        #define ALLOCATE_SS_CONTAINER_WI(T, i, b) new chaos::utility::StartableServiceContainer<T>(i, b, #T)
    }
    
}
#endif /* defined(__CHAOSFramework__StartableService__) */
