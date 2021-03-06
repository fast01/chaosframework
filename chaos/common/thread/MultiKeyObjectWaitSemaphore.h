/*	
 *	MultiKeyObjectWaitSemaphore.h
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

#ifndef ChaosFramework_MultiKeyObjectWaitSemaphore_h
#define ChaosFramework_MultiKeyObjectWaitSemaphore_h

#include <map>

#include <chaos/common/global.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <chaos/common/thread/ObjectWaitSemaphore.h>

namespace chaos {
    
    using namespace std;
    using namespace boost;
    
        //! MultiKeyObjectWaitSemaphore An implementation for a multi semaphore associate to a generic keys
    /*! \class MultiKeyObjectWaitSemaphore
     \brief
     This semaphore implementation work on concept that at every semaphore is associated a generic key. A key can represent an event, with an object associated to it,
     for which a thread wait. 
     \tparam T is the type of the key
     \tparam A is the type of the object that will be associated at the event identified by the key
     */
    template<typename T, typename A>
    class MultiKeyObjectWaitSemaphore {
            //! mutex for regulate the multithreading map access
        boost::mutex mapAccessMutex;
        
            //! Represent the mapping between the key and it semaphore data structure
        map<T, ObjectWaitSemaphore<A> *> keySemaphoreMap;
        
    public:
            //!MultiKeyObjectWaitSemaphore
        /*! 
         Base constructor fro initialize all structure
         */
        MultiKeyObjectWaitSemaphore(){
        }
        
            //!~MultiKeyObjectWaitSemaphore
        /*! 
         Base destructor for deinitialize all structure
         */
        ~MultiKeyObjectWaitSemaphore(){
            for ( typename map<T, ObjectWaitSemaphore<A>* >::iterator iter = keySemaphoreMap.begin(); 
                 iter != keySemaphoreMap.end();
                 iter++) {
                delete(iter->second);
            }
        }
        
            //!initKey
        /*! 
         *  \param keyToInit key for wich the semaphore structure need to be initialized
         */
        void initKey(T keyToInit){
            if(keySemaphoreMap.count(keyToInit)) return;
            
            boost::unique_lock< boost::mutex >  mapLock(mapAccessMutex);
            
            keySemaphoreMap.insert(make_pair(keyToInit, new ObjectWaitSemaphore<A>()));
        }
            //!setWaithedObjectForKey
        /*! 
         * set the waiting object for the waiting thread
         *  \key is the key that represent the waiting thread for the object. Afther the unlock operation
         the struccure is removed from map
         */
        bool setWaithedObjectForKey(T key, A obj){
            boost::unique_lock< boost::mutex >  mapLock(mapAccessMutex);
            if(!keySemaphoreMap.count(key)) return false;
            ObjectWaitSemaphore<A> *ks = keySemaphoreMap[key];
            if(!ks) {
                //before i need to dealloc pointed data because noone wait for it
                return false;
            }
            //set the waiting object
            ks->setWaithedObject(obj);
            ks->unlock();
            return true;
        }
        
            //!wait
        /*! 
         \param waitOnKey identify the key event for wich the current call thread need to wait
         \return the object taht the thread was waiting for
         */
        A wait(T waitOnKey) {
            boost::unique_lock< boost::mutex >  mapLock(mapAccessMutex);
            
            A result = NULL;
            
            ObjectWaitSemaphore<A> *ks = keySemaphoreMap[waitOnKey]; mapLock.unlock();
            if(!ks) return NULL;
            result =  ks->wait();
            
            keySemaphoreMap.erase(waitOnKey);
            delete(ks);
            return result;
        }
        
            //!wait
        /*! 
         \param waitOnKey identify the key event for wich the current call thread need to wait
         \return the object taht the thread was waiting for
         */
        A wait(T waitOnKey, unsigned int millisecToWait) {
            ObjectWaitSemaphore<A> *ks = NULL;
            
            boost::unique_lock< boost::mutex >  lockMap(mapAccessMutex);
            ks = keySemaphoreMap[waitOnKey]; 
            lockMap.unlock();
            
            if(!ks) return NULL;
            
            A result = ks->wait(millisecToWait);
            lockMap.lock();
            if(keySemaphoreMap.count(waitOnKey)){
                keySemaphoreMap.erase(waitOnKey);
                delete(ks);
            }
            lockMap.unlock();
            return result;
        }
    };
}
#endif
