    //
    //  ControlUnit.h
    //  ChaosFramework
    //
    //  Created by Claudio Bisegni on 09/03/11.
    //  Copyright 2011 INFN. All rights reserved.
    //

#ifndef ControlUnit_H
#define ControlUnit_H

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <chaos/common/data/CUSchemaDB.h>
#include <chaos/common/general/Configurable.h>
#include <chaos/common/action/ActionDescriptor.h>
#include <chaos/common/exception/CException.h>
#include <chaos/common/action/DeclareAction.h>
#include <chaos/common/utility/ArrayPointer.h>
#include <chaos/common/thread/CThread.h>
#include <chaos/cu_toolkit/DataManager/KeyDataStorage.h>
#include <chaos/common/pqueue/CObjectHandlerProcessingQueue.h>

namespace chaos{
    
#define CU_IDENTIFIER_C_STREAM getCUName() << "_" << getCUInstance() 
    
#define INIT_STATE      0    
#define START_STATE     1
    
#define CHECK_INITIALIZED(deviceName)  if(deviceStateMap[deviceName] != INIT_STATE){\
LSANDBOX_ << "Control Unit already initialized";\
continue;\
}\
deviceStateMap[deviceName]++;
    
#define CHECK_STARTED(deviceName)  if(deviceStateMap[deviceName] != START_STATE){\
LSANDBOX_ << "Control Unit already started";\
throw CException(2, "Control Unit already started", "ControlUnitSandbox");\
}\
deviceStateMap[deviceName]++;
    
#define CHECK_NOT_STARTED(deviceName)  if(deviceStateMap[deviceName] != START_STATE+1){\
LSANDBOX_ << "Control Unit not started";\
throw CException(3, "Control Unit not started", "ControlUnitSandbox");\
}\
deviceStateMap[deviceName]--;

#define CHECK_NOT_INITIALIZED(deviceName)  if(deviceStateMap[deviceName] != INIT_STATE+1){\
LSANDBOX_ << "Control Unit not initialized";\
throw CException(1, "Control Unit not initialized", "ControlUnitSandbox");\
}\
deviceStateMap[deviceName]--;

    
    class ControlUnitThread;
        //class ControlUnitInstantiator;
    using namespace std;
    
    class ActionData {
        
    public:
        void purge(){
            if(actionData) delete(actionData);
        }
        CDataWrapper *actionData;
        boost::function<CDataWrapper*(CDataWrapper*, bool)> actionDef;
    };
    
    /*
     Base class for control unit execution task
     */
    class AbstractControlUnit:public DeclareAction, public CUSchemaDB {
        friend class ControlUnitSandbox;
        int32_t scheduleDelay;
        string jsonSetupFilePath;
        
        
            //!mutex for multithreading managment of sand box
        /*!
         The muthex is needed because the call to the action can occours in different thread
         */
        recursive_mutex managing_cu_mutex;
        
            //
        map<string, KeyDataStorage*>  keyDataStorageMap;
        
        map<string, CObjectHandlerProcessingQueue<ActionData*>* >  actionParamForDeviceMap;
        
        map<string, CThread* >  schedulerDeviceMap;
        
        map<string, int >  deviceStateMap;
        
        /*
         Add a new KeyDataStorage for a specific key
         */
        void addKeyDataStorage(const char *, KeyDataStorage*);
        
        void _sharedInit();
    public:
        /*
         Construct a new CU with an identifier
         */
        AbstractControlUnit(const char *);
        /*
         Construct a new CU with an identifier
         */
        AbstractControlUnit();
        /*
         Destructor a new CU with an identifier
         */
        virtual ~AbstractControlUnit();
        
        /*
         return the CU name
         */
        const char * getCUName();
        /*
         return the CU instance
         */
        const char * getCUInstance();      
        /*
         Define the control unit DataSet and Action into
         a CDataWrapper
         */
        void _defineActionAndDataset(CDataWrapper&) throw(CException);
        /*
         Define the control unit DataSet and Action into
         a CDataWrapper
         */
        void _undefineActionAndDataset() throw(CException);       
        /*
         Initialize the Custom Contro Unit and return the configuration
         */
        CDataWrapper* _init(CDataWrapper*, bool& detachParam) throw(CException);        
        
        /*
         Deinit the Control Unit
         */
        CDataWrapper* _deinit(CDataWrapper*, bool& detachParam) throw(CException);
        
        /*
         Starto the  Control Unit scheduling for device
         */
        CDataWrapper* _start(CDataWrapper*, bool& detachParam) throw(CException);    
        /*
         Stop the Custom Control Unit scheduling for device
         */
        CDataWrapper* _stop(CDataWrapper*, bool& detachParam) throw(CException);   
        
        /*
         Execute the scehduling for the device
         */
        void _run(const string&) throw(CException); 
    protected:
            //CU Identifier
        string cuName;
            //CU instance, this te fine the current isntance code
            //it's dynamically assigned
        string cuInstance;
        
        shared_ptr<CDataWrapper> _internalSetupConfiguration;
        
        /*
         Return the tart configuration for the Control Unit instance
         */
        virtual void defineActionAndDataset(CDataWrapper&) throw(CException)   = 0;
        
        /*
         Initialize the Custom Contro Unit and return the configuration
         */
        virtual void init(CDataWrapper*) throw(CException) = 0;
        
        /*
         Execute the Control Unit work
         */
        virtual void run(const string&) throw(CException) = 0;
        
        /*
         Execute the Control Unit work
         */
        virtual void stop(const string&) throw(CException) = 0;
        
        /*
         Deinit the Control Unit
         */
        virtual void deinit(const string&) throw(CException) = 0;
        /*
         Receive the evento for set the dataset input element
         */
        virtual CDataWrapper* setDatasetAttribute(CDataWrapper*, bool&) throw (CException) = 0;
        
        /*
         Event for update some CU configuration
         */
        virtual CDataWrapper* updateConfiguration(CDataWrapper*, bool&) throw (CException);
        
        /*
         Receive the evento for set the dataset input element
         */
        virtual CDataWrapper* _setDatasetAttribute(CDataWrapper*, bool&) throw (CException);
        
        /*
         Create a new action description, return the description for let the user to add parameter
         */
        template<typename T>
        AbstActionDescShrPtr addActionDescritionInstance(T* actonObjectPointer, typename ActionDescriptor<T>::ActionPointerDef actionHandler, const char*const actionAliasName, const char*const actionDescription) {
                //call the DeclareAction action register method, the domain will be associated to the control unit isntance
            return DeclareAction::addActionDescritionInstance(actonObjectPointer, actionHandler, cuInstance.c_str(), actionAliasName, actionDescription);
        }
        
            //--------------Contro Unit Service Method----------------
        /*
         Init the dataset ad other values by a json file
         */
        void initWithJsonFilePath(const char*const _jsonSetupFilePath) {
            jsonSetupFilePath.assign(_jsonSetupFilePath);
        }
        
        /*
         Set the default schedule delay for the sandbox
         */
        void setDefaultScheduleDelay(int32_t _sDelay){scheduleDelay = _sDelay;};
        
        /*
         load the json file setupped into jsonSetupFilePath class attributed
         */
        void loadCDataWrapperForJsonFile(CDataWrapper&)  throw (CException);
        
        /*
         Send device data to output buffer
         */
        void pushDataSetForKey(const char *key, CDataWrapper*);
        
        /*
         get latest device data 
         */
        ArrayPointer<CDataWrapper> *getLastDataSetForKey(const char *key);
        
        /*
         return a new instance of CDataWrapper filled with a mandatory data
         according to key
         */
        CDataWrapper *getNewDataWrapperForKey(const char*);        
    };
}
#endif
