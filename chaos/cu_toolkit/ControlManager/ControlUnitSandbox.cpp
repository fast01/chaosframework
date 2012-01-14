//
//  ControlUnitSandbox.cpp
//  ChaosFramework
//
//  Created by Claudio Bisegni on 21/06/11.
//  Copyright 2011 INFN. All rights reserved.
//

#include "../../Common/global.h"
#include "ControlUnitSandbox.h"
#include "../../Common/cconstants.h"
#include "../DataManager/DataManager.h"
#include "../ConfigurationManager/ConfigurationManager.h"
#include "../CommandManager/CommandManager.h"


#define CHECK_INITIALIZED  if(initialized){\
LAPP_ << "Control Unit already initialized";\
throw CException(0, "Control Unit already initialized", "ControlUnitSandbox");\
}

#define CHECK_NOT_INITIALIZED  if(!initialized){\
LAPP_ << "Control Unit not initialized";\
throw CException(1, "Control Unit not initialized", "ControlUnitSandbox");\
}

#define CHECK_STARTED  if(started){\
LAPP_ << "Control Unit already started";\
throw CException(2, "Control Unit already started", "ControlUnitSandbox");\
}

#define CHECK_NOT_STARTED  if(!started){\
LAPP_ << "Control Unit not started";\
throw CException(3, "Control Unit not started", "ControlUnitSandbox");\
}



using namespace chaos;
using namespace std;
using namespace boost;
#pragma mark Public Method

/*
 Construct the CUSandBox with a Contorl Unit pointer
 */
ControlUnitSandbox::ControlUnitSandbox(AbstractControlUnit *newAcu) {
    started = false;
    autostart = false;
    initialized = false;
    acu = newAcu;
    csThread = 0L;
}


/*
 Contorl Unit Desctruction
 */
ControlUnitSandbox::~ControlUnitSandbox() {
    //thread deallocation
    deinitSandbox();
    
    if(acu){
        delete(acu);
        acu = 0L;
    }
}

/*
 Return the sandbox name
 */
string& ControlUnitSandbox::getSandboxName() {
    return sandboxName;
}


/*
 return the name of CU managed
 */
const char * ControlUnitSandbox::getCUName() {
    return acu? acu->getCUName():"No Control Unit In Sandbox";
}

const char * ControlUnitSandbox::getCUInstance() {
    return acu? acu->getCUInstance():"No Control Unit In Sandbox";
}

/*
 Define control unit
 */
void ControlUnitSandbox::defineSandboxAndControlUnit(CDataWrapper& masterConfiguration) throw (CException) {
    if(!acu) throw CException(0, "No control unit associated", "ControlUnitSandbox::initSandBox");
    //if(!masterConfiguration) throw CException(1, "No initial configuration passed", "ControlUnitSandbox::initSandBox");
    
    //DataWrapper for send cu templat to the metadataserver
    //associate new acs
    LAPP_ << "Init Control Unit Sandbox for Control Unit:" << CU_IDENTIFIER_C_STREAM;
    
    LAPP_ << "Init Thread for:" << CU_IDENTIFIER_C_STREAM;
    
    LAPP_ << "Start the Control Unit:" << CU_IDENTIFIER_C_STREAM << " setup";
    acu->_defineActionAndDataset(masterConfiguration);
    LAPP_ << "Control Unit:" << CU_IDENTIFIER_C_STREAM << " done";
#if DEBUG
    LDBG_ << "-------------START "<< CU_IDENTIFIER_C_STREAM << " SETUP------------";
    LDBG_ << masterConfiguration.getJSONString();
    LDBG_ << "---------------END "<< CU_IDENTIFIER_C_STREAM << " SETUP------------";
#endif  
    //decode the dataset
    LAPP_ << "Register the action exposed by Control Unit:" << CU_IDENTIFIER_C_STREAM;
    CommandManager::getInstance()->registerAction(acu);
    
    //expose updateConfiguration Methdo to rpc
    LAPP_ << "Register Sandbox with domain:" << acu->getCUInstance() << " updateConfiguration action";
    DeclareAction::addActionDescritionInstance<ControlUnitSandbox>(this, 
                                                                   &ControlUnitSandbox::updateConfiguration, 
                                                                   acu->getCUInstance(), 
                                                                   "updateConfiguration", 
                                                                   "Update Configuration");
    
    LAPP_ << "Register Sandbox with Domain:" << acu->getCUInstance() << " startControlUnit action";
    DeclareAction::addActionDescritionInstance<ControlUnitSandbox>(this, 
                                                                   &ControlUnitSandbox::startSandbox, 
                                                                   acu->getCUInstance(), 
                                                                   "startSandbox", 
                                                                   "Start the control unit manage by this Sandbox");
    
    LAPP_ << "Register Sandbox with Domain:" << acu->getCUInstance() << " stopControlUnit action";
    DeclareAction::addActionDescritionInstance<ControlUnitSandbox>(this, 
                                                                   &ControlUnitSandbox::stopSandbox, 
                                                                   acu->getCUInstance(), 
                                                                   "stopSandbox", 
                                                                   "Start the control unit manage by this Sandbox");
    
    sandboxName.append("Sandbox_");sandboxName.append(acu->getCUName());sandboxName.append("_");sandboxName.append(acu->getCUInstance());    
}

/*
 Sand box initialization, here is instantiated the thread for schedule 
 the Control Unit Run() method. Here is get the static inital CU configuration
 then this is sent to Metadata server waithing to receive the last configuration
 available. The first time will be the same as the initial
 */
void ControlUnitSandbox::initSandbox(CDataWrapper *masterConfiguration) throw (CException) {
    //the lock is need because it's possible initi ad deinit the Sandbox by 
    //system action
    recursive_mutex::scoped_lock  lock(managing_cu_mutex);
    CHECK_INITIALIZED
    
    //now ew can allocate the thread
    csThread = new CThread(this);
    if(!csThread) throw CException(0,"Thread inititalization error","ControlUnitSandbox::initSandBox");
    
    
    LAPP_ << "Init Control Unit:" << CU_IDENTIFIER_C_STREAM;
    //acu->_init(defaultInternalConf);
    acu->_init(masterConfiguration);
    
    LAPP_ << "Update Control Unit and Sandbox Configuration for:" << CU_IDENTIFIER_C_STREAM;
    updateConfiguration(masterConfiguration);
    
    //register the actions fot the control unit
    CommandManager::getInstance()->registerAction(this);
    //flag the sandbox initialiaztion status before the autostart
    initialized = true;
    
    //need to check if the CU wnat to be autostarted
    //check if cu want to be started without metadataserver consense
    if(masterConfiguration->hasKey(CUDefinitionKey::CS_CM_CU_AUTOSTART) &&
       masterConfiguration->getInt32Value(CUDefinitionKey::CS_CM_CU_AUTOSTART)){
        LAPP_  << "Starting Control Unit Sanbox:" << CU_IDENTIFIER_C_STREAM;
        startSandbox(masterConfiguration);
        LAPP_  << "Started Control Unit Sanbox:" << CU_IDENTIFIER_C_STREAM;   
    }
}

/*
 Sandbox deinitialization keep care to stop the sandbox, the thread and
 to deinit the CU
 */
void ControlUnitSandbox::deinitSandbox() throw (CException) {
    //the lock is need because it's possible initi ad deinit the Sandbox by 
    //system action
    recursive_mutex::scoped_lock  lock(managing_cu_mutex);
    //check the already deinitilaized status
    CHECK_NOT_INITIALIZED
    
    //check the thread
    CHAOS_ASSERT(csThread);
    
    
    LAPP_ << "Deinit Control Unit Sandbox for Control Unit:" << CU_IDENTIFIER_C_STREAM;
    CommandManager::getInstance()->deregisterAction(this);
    //deinit the thread and dispose it
    //stop the SandBox
    try{
        stopSandbox(NULL);
    }catch(CException& ex){
        if(ex.errorCode!=3) throw ex;
    }
    
    LAPP_ << "Deinit CU:" << CU_IDENTIFIER_C_STREAM;
    //deinit the cu
    acu->_deinit();
    
    //deallocate the thread
    if(csThread){
        delete(csThread);
        csThread = 0L;
    }
    
    //deallocate the control unit deinitializating it
    if(acu){
        //register the action implemented by CU(it extends DeclareaAction Class)
        LAPP_ << "Deregister the action exposed by Control Unit:" << CU_IDENTIFIER_C_STREAM;
        CommandManager::getInstance()->deregisterAction(acu);
    }
    
    //flag the sandbox initialiaztion status
    initialized = false;
}

/*
 Start the sandbox ad cu
 */
CDataWrapper* ControlUnitSandbox::startSandbox(CDataWrapper *startConfiguration) throw (CException){
    recursive_mutex::scoped_lock  lock(managing_cu_mutex);
    
    //check the initialization status
    CHECK_NOT_INITIALIZED
    //check the already start status
    CHECK_STARTED
    //check the thread
    CHAOS_ASSERT(csThread);
    
    
    if(!csThread) {
        LERR_ << "No trehad defined for sandbox "<< getSandboxName();
    }
    if(!csThread->isStopped()){
        LERR_ << "Sanbox "<< getSandboxName() << "already runnign";
    }
    
    LAPP_ << "Sanbox "<< getSandboxName() << " start event";
    
    LAPP_ << "Start Thread for:" << CU_IDENTIFIER_C_STREAM;
    csThread->start();
    csThread->setThreadPriorityLevel(sched_get_priority_max(SCHED_RR), SCHED_RR);
    
    //set started flag
    started = true;
    return NULL;
}

/*
 Stop the sandbox and cu
 */
CDataWrapper* ControlUnitSandbox::stopSandbox(CDataWrapper *stopConfiguration) throw (CException){
    recursive_mutex::scoped_lock  lock(managing_cu_mutex);
    
    //check the initialization status
    CHECK_NOT_INITIALIZED
    //check the already start status
    CHECK_NOT_STARTED
    
    //check the thread
    CHAOS_ASSERT(csThread);
    
    if(csThread->isStopped()){
        LERR_ << "Sanbox "<< getSandboxName() << "already stopped";
    }
    LAPP_ << "Stopping Control Unit:" << CU_IDENTIFIER_C_STREAM;
    if(acu) acu->stop();
    LAPP_ << "Stop Thread for:" << CU_IDENTIFIER_C_STREAM;
    if(csThread) csThread->stop();
    LAPP_ << "Stopped Thread for:" << CU_IDENTIFIER_C_STREAM;
    
    //set started flag
    started = false;
    return NULL;
}

/*
 Update the sandbox configuration. After SB has been configured the configData is sent to ControlUnit
 */
CDataWrapper* ControlUnitSandbox::updateConfiguration(CDataWrapper *configData) {
    if(!configData) return NULL;
#if DEBUG
    LDBG_ << "Update Configuration for Control Unit Sandbox:" << CU_IDENTIFIER_C_STREAM;
#endif
    if(configData->hasKey(CUDefinitionKey::CS_CM_THREAD_SCHEDULE_DELAY)){
        //we need to configure the delay  from a run() call and the next
        int32_t uSecdelay = configData->getInt32Value(CUDefinitionKey::CS_CM_THREAD_SCHEDULE_DELAY);
#if DEBUG
        LDBG_ << "THREAD_SCHEDULE_DELAY:" << uSecdelay << " uSecond";
#endif
        csThread->setDelayBeetwenTask(uSecdelay);
    }        
#if DEBUG
    LDBG_ << "Update Configuration for Internal Control Unit:" << CU_IDENTIFIER_C_STREAM;
#endif
    if(acu)acu->updateConfiguration(configData);
    return NULL;
}
#pragma mark Private Method

/*!
 Perform the heartbeat operation for managed Contorl Unit Instance
 */
void ControlUnitSandbox::performHeartbeat() {
    try{
        
    } catch(...) {
        
    }
}

/*
 Execute the scheduling of the run() method of the managed control unit
 */
void ControlUnitSandbox::executeOnThread() throw(CException) {    
    //call the run mehtod for cu
    acu->run();
    
    //check for heart beat time befor to going to sleep
    if (boost::chrono::steady_clock::now() >= lastHeartBeatTime + heartBeatDelayInMicroseconds) {
        performHeartbeat();
    }
    
}
