//
//  ReactorController.h
//  BenchTest
//
//  Created by Claudio Bisegni on 1/12/13.
//  Copyright (c) 2013 INFN. All rights reserved.
//

#ifndef __BenchTest__ReactorController__
#define __BenchTest__ReactorController__

#include <string>
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>
#include <chaos/common/thread/ChaosThread.h>
#include <chaos/ui_toolkit/LowLevelApi/LLRpcApi.h>
#include <chaos/ui_toolkit/HighLevelApi/HLDataApi.h>
#include <chaos/common/utility/TimingUtil.h>
#include "Batch_Controller.h"
#include "WriteFile.h"

using namespace chaos;
using namespace std;


typedef enum ControllerState {
    INIT,
    DEINIT
} ControllerState;

class ReactorController : public chaos::CThreadExecutionTask, private Batch_Controller {
    string reactorID;
    WriteFile *logFile;
    TimingUtil timingUtil;
    int64_t lastSampledTimestamp;
    chaos::event::channel::InstrumentEventChannel *instrumentEvtCh;
    boost::shared_ptr<CThread> chaosThread;
    chaos::ui::DeviceController *chaosReactorController;
    uint64_t simulationSpeed;
    ControllerState state;
    high_resolution_clock::time_point lastExecutionTime;
    high_resolution_clock::time_point currentExecutionTime;
    int cycleCount;
protected:
    virtual void executeOnThread() throw(CException);
    
public:
    ReactorController(string& _rName, vector<double> *refVec, int refIdx, uint64_t _simulationSpeed);
    ~ReactorController();
    void init();
    void deinit();
    void setScheduleDelay(int64_t scheduleDaly);
    void joinThread();
};

#endif /* defined(__BenchTest__ReactorController__) */
