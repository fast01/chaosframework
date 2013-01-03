//
//  main.cpp
//  CachingSystem
//
//  Created by Flaminio Antonucci on 26/07/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include <iostream>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <map>
#include <sys/stat.h>

//includes for caching system
#include <chaos/common/caching_system/caching_thread/trackers/TransformDeviceTracker.h>
#include <chaos/common/caching_system/caching_thread/trackers/AgregateDeviceTracker.h>
#include <chaos/common/caching_system/common_buffer/CommonBuffer.h>
#include <chaos/common/caching_system/caching_thread/trackers/ConcreteDeviceTracker.h>


#include <chaos/common/caching_system/common_buffer/helper/MillisTime.h>



//implemented interfaces
#include "SimpleDataFetcher.h"
#include "SimpleDataTransform.h"
#include "SimpleDataAggregator.h"
#include "LocalDataFetcher.h"
#include "SimpleReader.h"
#include "AggregateReader.h"

//data structure
#include "MioElemento.h"
#include "DataAggregate.h"



using namespace boost::interprocess;
using namespace caching_system::caching_thread;
using namespace caching_system;

using namespace std;

string basePath="./log";

bool processInput(int argc, const char * argv[]);
void printHelp();
//fdf

//questo test crea 4 lettori in alta risoluzione, 4 in bassa risoluzione, un sub lettore , un tracker che crea un dato
//aggregato e 4 lettori che leggono il dato aggragato
int run(int argc, const char * argv[]);

int main(int argc, const char * argv[]){
    
    
    run(argc,argv);
}







int run(int argc, const char * argv[]){
    
    //duration time of the run
    int timeOfSimulation=60;

    // hertz to get data
    long hzGetData =2;
    
    //caching time for the hig resolution buffer
    long millisTimeCaching= 500;
    
    //device id
    string dev="dev01";
    
    //number of high resolution reader
    int numberOfReaders=1;
    
    // frame per second simulated for reading
    int fpsLettura=10;

    //logging to file
    std::stringstream path;
    path<<basePath;
    time_t startTime =time(0);
    path<<"_"<<startTime<<"/";
    basePath=path.str();
    mkdir(basePath.c_str(),0777);
    
    
    
    
    //first, create your data fetcher
    caching_system::DataFetcherInterface<Magnete>* fetcher=new SimpleDataFetcher();
    
    // then, create an istance of your tracker, passing all needed parameters and the fetcher
    caching_thread::ConcreteDeviceTracker<Magnete>* myTracker=new ConcreteDeviceTracker<Magnete>(fetcher,hzGetData,millisTimeCaching,dev);
    
    std::cout<<"Starting tracker\n";
    boost::thread threadTracker(boost::ref(*myTracker));
    threadTracker.detach();
    
    //now it will start some threads
    vector<SimpleDataReader*> threadReaders ;

    // this will create an iterator and then it will passed as argument to implemented reader thread
    for(int i=0;i<numberOfReaders;i++){
        IteratorReader<Magnete>* lett=myTracker->openMaxResBuffer();
        SimpleDataReader* threadLettore=new SimpleDataReader(lett,fpsLettura,basePath,i,myTracker);
        threadReaders.push_back(threadLettore);
        
    }
    std::cout<<"Staring thread readers\n";

    //Starting thread
    for(int i=0;i<threadReaders.size();i++){
        
        boost::thread reader(boost::ref(* threadReaders.at(i)));
        reader.detach();
        
        
        
    }
    
    
    std::cout<<"now i will sleep for about "<<timeOfSimulation<<" seconds... \n";
    
    // now thread main will sleep for designed time
    
    boost::posix_time::seconds timeFakeWork(timeOfSimulation+5);
    boost::this_thread::sleep(timeFakeWork);
    
 
    
    std::cout <<"end of simulation time, i'm trying to close tracker\n";
    // try to close, if false, it means that there is some thread holding the buffer
    if(myTracker->tryInterruptTracking()){
        std::cout<<"closed with success\n";
    }else{
        
        std::cout<<"something goes wrong, maybe there are some thread that are holding buffers..\n";
    }
    
    std::cout<<"finished, you can check the result in you execution path\n";
    return 0;
    
}


