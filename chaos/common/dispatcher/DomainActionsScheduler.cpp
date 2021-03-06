/*
 *	DomainActionsScheduler.cpp
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
#include "../global.h"
#include "DomainActionsScheduler.h"
#include <chaos/common/chaos_constants.h>

using namespace chaos;

namespace chaos_data = chaos::common::data;

DomainActionsScheduler::DomainActionsScheduler(boost::shared_ptr<DomainActions> _domainActionsContainer):armed(false){
    domainActionsContainer = _domainActionsContainer;
}

/*!
 Default destructor
 */
DomainActionsScheduler::~DomainActionsScheduler() {
    
}
/*
 Initialization method for output buffer
 */
void DomainActionsScheduler::init(int threadNumber) throw(CException) {
    LAPP_ << "Initializing Domain Actions Scheduler for domain:" << domainActionsContainer->getDomainName();
    CObjectProcessingQueue<chaos_data::CDataWrapper>::init(threadNumber);
    armed = true;
}

/*
 Deinitialization method for output buffer
 */
void DomainActionsScheduler::deinit() throw(CException) {
    LAPP_ << "Deinitializing Domain Actions Scheduler for domain:" << domainActionsContainer->getDomainName();
	//mutex::scoped_lock lockAction(actionAccessMutext);
    CObjectProcessingQueue<chaos_data::CDataWrapper>::clear();
    CObjectProcessingQueue<chaos_data::CDataWrapper>::deinit();
    armed = false;
}

/*
 override the push method for ObjectProcessingQueue<CDataWrapper> superclass
 */
bool DomainActionsScheduler::push(chaos_data::CDataWrapper *actionParam) throw(CException) {
    if(!armed) throw CException(0, "Action can't be submitted, scheduler is not armed", "DomainActionsScheduler::push");
    return CObjectProcessingQueue<chaos_data::CDataWrapper>::push(actionParam);
}

/*
 
 */
string& DomainActionsScheduler::getManagedDomainName() {
    return domainActionsContainer->getDomainName();
}

/*
 
 */
void DomainActionsScheduler::setDispatcher(AbstractCommandDispatcher *newDispatcher) {
    dispatcher = newDispatcher;
}

/*
 process the element action to be executed
 */
void DomainActionsScheduler::processBufferElement(chaos_data::CDataWrapper *actionDescription, ElementManagingPolicy& elementPolicy) throw(CException) {
	//the domain is securely the same is is mandatory for submition so i need to get the name of the action
    chaos_data::CDataWrapper            *responsePack = NULL;
    chaos_data::CDataWrapper            *subCommand = NULL;
    auto_ptr<chaos_data::CDataWrapper>  actionMessage;
    auto_ptr<chaos_data::CDataWrapper>  remoteActionResult;
    auto_ptr<chaos_data::CDataWrapper>  actionResult;
	//keep track for the retain of the message of the aciton description
    ElementManagingPolicy               action_elementPolicy = {false};
    bool    needAnswer = false;
    //bool    detachParam = false;
    int     answerID;
    string  answerIP;
    string  answerDomain;
    string  answerAction;
    string  actionName = actionDescription->getStringValue( RpcActionDefinitionKey::CS_CMDM_ACTION_NAME );
    
    if(!domainActionsContainer->hasActionName(actionName)) {
        LAPP_ << "The action " << actionName << " is not present for domain " << domainActionsContainer->getDomainName();
        return;
    }
	//get the action reference
    AbstActionDescShrPtr actionDescriptionPtr = domainActionsContainer->getActionDescriptornFormActionName(actionName);
    
	//lock the action for write, so we can schedule it
    ActionReadLock readLockForActionExecution(actionDescriptionPtr->actionAccessMutext);
    
	//set hte action as fired
    bool canFire = actionDescriptionPtr->setFired(true);
    
	//if we can't fire we exit
    if(!canFire) return;
    
    try {
		//get the action message
        if( actionDescription->hasKey( RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE ) ) {
			//there is a subcommand to submit
            actionMessage.reset(actionDescription->getCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE));
        }
        
		//get sub command if present
		//check if we need to submit a sub command
        if( actionDescription->hasKey( RpcActionDefinitionKey::CS_CMDM_SUB_CMD ) ) {
			//there is a subcommand to submit
            subCommand = actionDescription->getCSDataValue(RpcActionDefinitionKey::CS_CMDM_SUB_CMD);
        }
        
		//check if request has the rigth key to let chaos lib can manage the answer send operation
        if(actionDescription->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_ID) &&
           actionDescription->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP) ) {
            needAnswer = true;
			//get infor for answer form the request
            answerID = actionDescription->getInt32Value(RpcActionDefinitionKey::CS_CMDM_ANSWER_ID);
            answerIP = actionDescription->getStringValue(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP);
            
			//we must check this only if we have a destination ip to send the answer
            if(actionDescription->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_DOMAIN) &&
               actionDescription->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_ACTION) ) {
				//fill the action doma and name for the answer
                answerDomain = actionDescription->getStringValue(RpcActionDefinitionKey::CS_CMDM_ANSWER_DOMAIN);
                answerAction = actionDescription->getStringValue(RpcActionDefinitionKey::CS_CMDM_ANSWER_ACTION);
            }
        }
        
        try{
			//call function core part
            if(needAnswer){
				//we need a response, so allocate the memory for it
                remoteActionResult.reset(new chaos_data::CDataWrapper());
            }
			//syncronously call the action in the current thread
            actionResult.reset(actionDescriptionPtr->call(actionMessage.get(), action_elementPolicy.elementHasBeenDetached));
			
			//check if we need to submit a sub command
            if( subCommand ) {
				//we can submit sub command
                auto_ptr<chaos_data::CDataWrapper> dispatchSubCommandResult(dispatcher->dispatchCommand(subCommand));
            }
            
            if(needAnswer){
				//we need an answer so add the submition result
				//if(actionResult.get()) remoteActionResult->addCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_SUBMISSION_RESULT, *actionResult.get());
				//put the submissione result error to 0(all is gone well)
                if(actionResult.get()) remoteActionResult->addCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE, *actionResult.get());
				
                remoteActionResult->addInt32Value(RpcActionDefinitionKey::CS_CMDM_ACTION_SUBMISSION_ERROR_CODE, 0);
            }
        } catch (CException& ex) {
            LAPP_ << "Error during action execution";
            DECODE_CHAOS_EXCEPTION(ex)
			//set error in response is it's needed
            if(needAnswer && remoteActionResult.get()) {
				DECODE_CHAOS_EXCEPTION_IN_CDATAWRAPPERPTR(remoteActionResult, ex)
			}
		} catch(...){
                    LAPP_ << "General error during action execution";
					//set error in response is it's needed
                    if(needAnswer) remoteActionResult->addInt32Value(RpcActionDefinitionKey::CS_CMDM_ACTION_SUBMISSION_ERROR_CODE, 1);
		}
        
        
        if( needAnswer && remoteActionResult.get() ) {
			//we need to construct the response pack
            responsePack = new chaos_data::CDataWrapper();
            
			//fill answer with data for remote ip and request id
            remoteActionResult->addInt32Value(RpcActionDefinitionKey::CS_CMDM_MESSAGE_ID, answerID);
			//set the answer host ip as remote ip where to send the answere
            responsePack->addStringValue(RpcActionDefinitionKey::CS_CMDM_REMOTE_HOST_IP, answerIP);
            
			//check this only if we have a destinantion
            if(answerDomain.size() && answerAction.size()){
				//set the domain for the answer
                responsePack->addStringValue(RpcActionDefinitionKey::CS_CMDM_ACTION_DOMAIN, answerDomain);
                
				//set the name of the action for the answer
                responsePack->addStringValue(RpcActionDefinitionKey::CS_CMDM_ACTION_NAME, answerAction);
            }
            
			//add the action message
            responsePack->addCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE, *remoteActionResult.get());
			//in any case this result must be LOG
			//the result of the action action is sent using this thread
            if(!dispatcher->submitMessage(answerIP, responsePack, false)){
				//the response has not been sent
                DELETE_OBJ_POINTER(responsePack);
            }
        }
    } catch (CException& ex) {
        CHK_AND_DELETE_OBJ_POINTER(responsePack);
		//these exception need to be logged
        DECODE_CHAOS_EXCEPTION(ex);
    }
    
    
	//check if we need to detach the action message
    if(action_elementPolicy.elementHasBeenDetached){
        actionMessage.release();
    }
    
	//set hte action as no fired
    actionDescriptionPtr->setFired(false);
}
