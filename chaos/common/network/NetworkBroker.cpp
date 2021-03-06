/*
 *	NetworkBroker.cpp
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
#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/global.h>
#include <boost/lexical_cast.hpp>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/message/DeviceMessageChannel.h>
#include <chaos/common/message/MDSMessageChannel.h>
#include <chaos/common/message/MessageChannel.h>
#include <chaos/common/message/PerformanceNodeChannel.h>
#include <chaos/common/event/EventServer.h>
#include <chaos/common/event/EventClient.h>
#include <chaos/common/dispatcher/AbstractCommandDispatcher.h>
#include <chaos/common/dispatcher/AbstractEventDispatcher.h>
#include <chaos/common/event/channel/AlertEventChannel.h>
#include <chaos/common/event/channel/InstrumentEventChannel.h>
#include <chaos/common/utility/InetUtility.h>

#define MB_LAPP LAPP_ << "[NetworkBroker]- "

#define INIT_STEP   0
#define DEINIT_STEP 1
using namespace chaos;
using namespace chaos::event;
using namespace chaos::common::data;
using namespace chaos::common::network;

/*!
 
 */
NetworkBroker::NetworkBroker():performance_session_managment(this){
    eventClient = NULL;
    eventServer = NULL;
    rpcServer = NULL;
    rpcClient = NULL;
    commandDispatcher = NULL;
    
    directIODispatcher = NULL;
    directIOServer = NULL;
    canUseMetadataServer = GlobalConfiguration::getInstance()->isMEtadataServerConfigured();
    if(canUseMetadataServer){
        metadataServerAddress = GlobalConfiguration::getInstance()->getMetadataServerAddress();
    }
}

/*!
 */
NetworkBroker::~NetworkBroker() {
    
}

/*!
 * Initzialize the Message Broker. In this step are taken the configured implementation
 * for the rpc client and server and for the dispatcher. All these are here initialized
 */
void NetworkBroker::init(void *initData) throw(CException) {
    MB_LAPP << "Init phase";
	//get global configuration reference
    CDataWrapper *globalConfiguration = GlobalConfiguration::getInstance()->getConfiguration();
    
    
    if(!globalConfiguration) {
        throw CException(1, "No global configuraiton found", "NetworkBroker::init");
    }
    
	//---------------------------- D I R E C T I/O ----------------------------
	if(globalConfiguration->hasKey(common::direct_io::DirectIOConfigurationKey::DIRECT_IO_IMPL_TYPE)) {
        string direct_io_impl = globalConfiguration->getStringValue(common::direct_io::DirectIOConfigurationKey::DIRECT_IO_IMPL_TYPE);
		//construct the rpc server and client name
        string directIOServerImpl = direct_io_impl+"DirectIOServer";
        directIOClientImpl = direct_io_impl+"DirectIOClient";
        MB_LAPP  << "DirectIO Client implementation is configured has : " << directIOClientImpl;
        MB_LAPP  << "Trying to initilize DirectIO Server: " << directIOServerImpl;
        directIOServer = ObjectFactoryRegister<common::direct_io::DirectIOServer>::getInstance()->getNewInstanceByName(directIOServerImpl);
		if(!directIOServer) throw CException(1, "Error creating direct io server implementation", __FUNCTION__);
		
		//allocate the dispatcher
		MB_LAPP  << "Allocate DirectIODispatcher DirectIODispatcher";
		directIOServer->setHandler(directIODispatcher = new common::direct_io::DirectIODispatcher());
		
		//initialize direct io server
		chaos::utility::StartableService::initImplementation(directIOServer, static_cast<void*>(globalConfiguration), directIOServer->getName(), __FUNCTION__);
		
		//init the my_ip variable for all client
		common::direct_io::DirectIOClientConnection::my_str_ip = GlobalConfiguration::getInstance()->getLocalServerAddress();
		common::direct_io::DirectIOClientConnection::my_i64_ip = STRIP_TO_UI64(common::direct_io::DirectIOClientConnection::my_str_ip).to_ulong();
    }
	//---------------------------- D I R E C T I/O ----------------------------
	
	//---------------------------- E V E N T ----------------------------
    if(globalConfiguration->hasKey(event::EventConfiguration::OPTION_KEY_EVENT_ADAPTER_IMPLEMENTATION)) {
        eventDispatcher = ObjectFactoryRegister<AbstractEventDispatcher>::getInstance()->getNewInstanceByName("DefaultEventDispatcher");
        if(!eventDispatcher)
            throw CException(2, "Event dispatcher implementation not found", "NetworkBroker::init");
        
        if(!chaos::utility::StartableService::initImplementation(eventDispatcher, static_cast<void*>(globalConfiguration), "DefaultEventDispatcher", "NetworkBroker::init"))
            throw CException(3, "Event dispatcher has not been initialized due an error", "NetworkBroker::init");
        
        
        string eventAdapterType = globalConfiguration->getStringValue(event::EventConfiguration::OPTION_KEY_EVENT_ADAPTER_IMPLEMENTATION);
		//construct the rpc server and client name
        string eventServerName = eventAdapterType+"EventServer";
        string eventClientName = eventAdapterType+"EventClient";
        
        MB_LAPP  << "Trying to initilize Event Server: " << eventServerName;
        eventServer = ObjectFactoryRegister<EventServer>::getInstance()->getNewInstanceByName(eventServerName);
        if(chaos::utility::StartableService::initImplementation(eventServer, static_cast<void*>(globalConfiguration), eventServer->getName(), "NetworkBroker::init")){
			//register the root handler on event server
            eventServer->setEventHanlder(eventDispatcher);
        }
        
        
        MB_LAPP  << "Trying to initilize Event Client: " << eventClientName;
        eventClient = ObjectFactoryRegister<EventClient>::getInstance()->getNewInstanceByName(eventClientName);
        chaos::utility::StartableService::initImplementation(eventClient, static_cast<void*>(globalConfiguration), eventClientName.c_str(), "NetworkBroker::init");
    }
	//---------------------------- E V E N T ----------------------------
    
	//---------------------------- R P C ----------------------------
    if(globalConfiguration->hasKey(RpcConfigurationKey::CS_CMDM_RPC_ADAPTER_TYPE)){
		//get the dispatcher
        MB_LAPP  << "Get DefaultCommandDispatcher implementation";
        commandDispatcher = ObjectFactoryRegister<AbstractCommandDispatcher>::getInstance()->getNewInstanceByName("DefaultCommandDispatcher");
        if(!commandDispatcher)
            throw CException(2, "Command dispatcher implementation not found", "NetworkBroker::init");
        
        if(!chaos::utility::StartableService::initImplementation(commandDispatcher, static_cast<void*>(globalConfiguration), "DefaultCommandDispatcher", "NetworkBroker::init"))
            throw CException(3, "Command dispatcher has not been initialized due an error", "NetworkBroker::init");
        
        
		// get the rpc type to instantiate
        string rpcRapterType = globalConfiguration->getStringValue(RpcConfigurationKey::CS_CMDM_RPC_ADAPTER_TYPE);
		//construct the rpc server and client name
        string rpcServerName = rpcRapterType+"Server";
        string rpcClientName = rpcRapterType+"Client";
        
        MB_LAPP  << "Trying to initilize RPC Server: " << rpcServerName;
        rpcServer = ObjectFactoryRegister<RpcServer>::getInstance()->getNewInstanceByName(rpcServerName);
		if(!rpcServer) throw CException(4, "Error allocating rpc server implementation", "NetworkBroker::init");
		
        if(chaos::utility::StartableService::initImplementation(rpcServer, static_cast<void*>(globalConfiguration), rpcServer->getName(), "NetworkBroker::init")) {
			//set the handler on the rpc server
            rpcServer->setCommandDispatcher(commandDispatcher);
        }
        
        
        MB_LAPP  << "Trying to initilize RPC Client: " << rpcClientName;
        rpcClient = ObjectFactoryRegister<RpcClient>::getInstance()->getNewInstanceByName(rpcClientName);
		if(!rpcClient) throw CException(4, "Error allocating rpc client implementation", "NetworkBroker::init");

        if(chaos::utility::StartableService::initImplementation(rpcClient, static_cast<void*>(globalConfiguration), rpcClient->getName(), "NetworkBroker::init")) {
			//set the forwarder into dispatcher for answere
            if(commandDispatcher) commandDispatcher->setRpcForwarder(rpcClient);
        }
    } else {
        throw CException(4, "No RPC Adapter type found in configuration", "NetworkBroker::init");
    }
	//---------------------------- R P C ----------------------------
	MB_LAPP  << "Initialize performance session manager";
	chaos::utility::StartableService::initImplementation(performance_session_managment, static_cast<void*>(globalConfiguration), "PerformanceManagment",  __PRETTY_FUNCTION__);
}

/*!
 * All rpc adapter and command siaptcer are deinitilized
 */
void NetworkBroker::deinit() throw(CException) {
	MB_LAPP  << "Deinitialize performance session manager";
	chaos::utility::StartableService::deinitImplementation(performance_session_managment, "PerformanceManagment",  __PRETTY_FUNCTION__);

	//---------------------------- D I R E C T I/O ----------------------------
	MB_LAPP  << "Deinit DirectIO server: " << directIOServer->getName();
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(directIOServer, directIOServer->getName(), "NetworkBroker::deinit");)
	DELETE_OBJ_POINTER(directIOServer);
	//---------------------------- D I R E C T I/O ----------------------------
	
	//---------------------------- E V E N T ----------------------------
    MB_LAPP  << "Deallocate all event channel";
    for (map<string, event::channel::EventChannel*>::iterator channnelIter = activeEventChannel.begin();
         channnelIter != activeEventChannel.end();
         channnelIter++) {
        
        event::channel::EventChannel *eventChannelToDispose = channnelIter->second;
        
		//deinit channel
        eventChannelToDispose->deinit();
        
		//dispose it
        delete(eventChannelToDispose);
    }
    MB_LAPP  << "Clear event channel map";
    activeEventChannel.clear();

    MB_LAPP  << "Deinit event client: " << eventClient->getName();
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(eventClient, eventClient->getName(), "NetworkBroker::deinit");)
    DELETE_OBJ_POINTER (eventClient);
    
    MB_LAPP  << "Deinit event server: " << eventServer->getName();
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(eventServer, eventServer->getName(), "NetworkBroker::deinit");)
    DELETE_OBJ_POINTER(eventServer);
    
    MB_LAPP  << "Deinit Event dispatcher";
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(eventDispatcher, "DefaultEventDispatcher", "NetworkBroker::deinit");)
    DELETE_OBJ_POINTER(eventDispatcher);
	//---------------------------- E V E N T ----------------------------
	
    
	//---------------------------- R P C ----------------------------
    MB_LAPP  << "Deallocate all rpc channel";
    for (map<string, MessageChannel*>::iterator channnelIter = activeRpcChannel.begin();
         channnelIter != activeRpcChannel.end();
         channnelIter++) {
        
        MessageChannel *messageChannelToDispose = channnelIter->second;
        
		//deinit channel
        messageChannelToDispose->deinit();
        
		//dispose it
        delete(messageChannelToDispose);
    }
    MB_LAPP  << "Clear rpc channel map";
    activeRpcChannel.clear();
    
    MB_LAPP  << "Deinit rpc client: " << rpcClient->getName();
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(rpcClient, rpcClient->getName(), "NetworkBroker::deinit");)
    DELETE_OBJ_POINTER(rpcClient);
    
    MB_LAPP  << "Deinit rpc server: " << rpcServer->getName();
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(rpcServer, rpcServer->getName(), "NetworkBroker::deinit");)
	DELETE_OBJ_POINTER(rpcServer);
    
    MB_LAPP  << "Deinit Command Dispatcher";
    CHAOS_NOT_THROW(chaos::utility::StartableService::deinitImplementation(commandDispatcher, "DefaultCommandDispatcher", "NetworkBroker::deinit");)
    DELETE_OBJ_POINTER(commandDispatcher);
	//---------------------------- R P C ----------------------------
	
}

/*!
 * all part are started
 */
void NetworkBroker::start() throw(CException){
	MB_LAPP  << "Start DirectIO server: " << directIOServer->getName();
    chaos::utility::StartableService::startImplementation(directIOServer, directIOServer->getName(), "NetworkBroker::start");
	
    MB_LAPP  << "Start event dispatcher ";
    chaos::utility::StartableService::startImplementation(eventDispatcher, "DefaultEventDispatcher", "NetworkBroker::start");
    
    MB_LAPP  << "Start event server: " << eventServer->getName();
    chaos::utility::StartableService::startImplementation(eventServer, eventServer->getName(), "NetworkBroker::start");
    
    MB_LAPP  << "Start event client: " << eventClient->getName();
    chaos::utility::StartableService::startImplementation(eventClient, eventClient->getName(), "NetworkBroker::start");
    
    MB_LAPP  << "Start command dispatcher ";
    chaos::utility::StartableService::startImplementation(commandDispatcher, "DefaultCommandDispatcher", "NetworkBroker::start");
	
    MB_LAPP  << "Start rpc server: " << rpcServer->getName();
    chaos::utility::StartableService::startImplementation(rpcServer, rpcServer->getName(), "NetworkBroker::start");
    
    MB_LAPP  << "Start rpc server: " << rpcClient->getName();
    chaos::utility::StartableService::startImplementation(rpcClient, rpcClient->getName(), "NetworkBroker::start");
    
    MB_LAPP << "get the published host and port from rpc server";
    getPublishedHostAndPort(publishedHostAndPort);
    MB_LAPP << "Rpc server has been published in: " << publishedHostAndPort;
	
	MB_LAPP  << "Start performance session manager";
	chaos::utility::StartableService::startImplementation(performance_session_managment, "PerformanceManagment",  __PRETTY_FUNCTION__);

}

/*!
 * all part are started
 */
void NetworkBroker::stop() throw(CException) {

    MB_LAPP  << "Stop rpc server: " << rpcClient->getName();
    chaos::utility::StartableService::stopImplementation(rpcClient, rpcClient->getName(), "NetworkBroker::stop");
	
    MB_LAPP  << "Stop rpc server: " << rpcServer->getName();
    chaos::utility::StartableService::stopImplementation(rpcServer, rpcServer->getName(), "NetworkBroker::stop");
	
    MB_LAPP  << "Stop command dispatcher ";
    chaos::utility::StartableService::stopImplementation(commandDispatcher, "DefaultCommandDispatcher", "NetworkBroker::stop");
	
    MB_LAPP  << "Stop event client: " << eventClient->getName();
    chaos::utility::StartableService::stopImplementation(eventClient, eventClient->getName(), "NetworkBroker::stop");
	
    MB_LAPP  << "Stop event server: " << eventServer->getName();
    chaos::utility::StartableService::stopImplementation(eventServer, eventServer->getName(), "NetworkBroker::stop");
	
    MB_LAPP  << "Stop event dispatcher ";
    chaos::utility::StartableService::stopImplementation(eventDispatcher, "DefaultEventDispatcher", "NetworkBroker::stop");
	
	MB_LAPP  << "Stop DirectIO server: " << directIOServer->getName();
    chaos::utility::StartableService::stopImplementation(directIOServer, directIOServer->getName(), "NetworkBroker::Stop");
	
	MB_LAPP  << "Stop performance session manager";
	chaos::utility::StartableService::stopImplementation(performance_session_managment, "PerformanceManagment",  __PRETTY_FUNCTION__);

}

/*!
 Return the port where the rpc server has been published
 */
int NetworkBroker::getPublishedPort() {
    CHAOS_ASSERT(rpcServer);
    return rpcServer->getPublishedPort();
}

/*!
 Fill the parameter withe rigth value of host and port for the internale
 rpc server of message broker
 */
void NetworkBroker::getPublishedHostAndPort(string& hostAndPort) {
    CHAOS_ASSERT(rpcServer);
    
    hostAndPort = GlobalConfiguration::getInstance()->getLocalServerAddress();
    hostAndPort.append(":");
    hostAndPort.append(lexical_cast<string>(rpcServer->getPublishedPort()));
}

std::string NetworkBroker::getRPCUrl() {
	std::string rpc_endpoint;
	getPublishedHostAndPort(rpc_endpoint);
	return rpc_endpoint;
}

std::string NetworkBroker::getDirectIOUrl() {
	return directIOServer->getUrl();
}

#pragma mark Event Registration and forwarding
//! event Action registration for the current instance of NetworkBroker
/*!
 Register an event actions defined for a detgerminated event type
 \param eventAction the actio to register
 \param eventType a type for the event for which the user want to register
 */
void NetworkBroker::registerEventAction(EventAction *eventAction, event::EventType eventType, const char * const identification) {
    CHAOS_ASSERT(eventDispatcher && eventAction);
    eventDispatcher->registerEventAction(eventAction, eventType, identification);
}

//!Event Action deregistration
/*!
 Deregister an event action
 */
void NetworkBroker::deregisterEventAction(EventAction *eventAction) {
    CHAOS_ASSERT(eventDispatcher && eventAction);
    eventDispatcher->deregisterEventAction(eventAction);
}

//!Event channel creation
/*!
 Performe the creation of an event channel of a desidered type
 \param eventType is one of the value listent in EventType enum that specify the
 type of the eventfor wich we want a channel
 */
event::channel::EventChannel *NetworkBroker::getNewEventChannelFromType(event::EventType  eventType) {
    event::channel::EventChannel *newEventChannel = NULL;
    switch (eventType) {
        case event::EventTypeAlert:
            newEventChannel = new event::channel::AlertEventChannel(this);
            break;
        case event::EventTypeInstrument:
            newEventChannel = new event::channel::InstrumentEventChannel(this);
            break;
        default:
            break;
    }
	//check if the channel has been created
    if(newEventChannel){
        newEventChannel->init();
        boost::mutex::scoped_lock lock(mapEventChannelAccess);
        activeEventChannel.insert(make_pair(newEventChannel->channelID, newEventChannel));
    }
    
    return newEventChannel;
}
//!Device channel creation
/*!
 Performe the creation of device channel
 \param deviceNetworkAddress device node address
 */
event::channel::AlertEventChannel *NetworkBroker::getNewAlertEventChannel() {
    return static_cast<event::channel::AlertEventChannel*>(NetworkBroker::getNewEventChannelFromType(event::EventTypeAlert));
}

//!Device channel creation
/*!
 Performe the creation of device channel
 \param deviceNetworkAddress device node address
 */
event::channel::InstrumentEventChannel *NetworkBroker::getNewInstrumentEventChannel() {
    return static_cast<event::channel::InstrumentEventChannel*>(NetworkBroker::getNewEventChannelFromType(event::EventTypeInstrument));
}

//!Event channel deallocation
/*!
 Perform the event channel deallocation
 */
void NetworkBroker::disposeEventChannel(event::channel::EventChannel *eventChannelToDispose) {
    if(!eventChannelToDispose) return;
    
    boost::mutex::scoped_lock lock(mapEventChannelAccess);
    
	//check if the channel is active
    if(activeEventChannel.count(eventChannelToDispose->channelID) == 0) return;
    
	//remove the channel as active
    activeEventChannel.erase(eventChannelToDispose->channelID);
    
	//deallocate it
    eventChannelToDispose->deinit();
    
	//dispose it
    delete(eventChannelToDispose);
}

//!message event
/*!
 Submit an event
 \param event the new evento to submit
 */
bool NetworkBroker::submitEvent(event::EventDescriptor *event) {
    CHAOS_ASSERT(eventClient)
    bool result = true;
    try{
        eventClient->submitEvent(event);
    } catch(CException& ex) {
        result = false;
        DECODE_CHAOS_EXCEPTION(ex);
    }
    return result;
}


#pragma mark Action Registration
/*
 Register actions defined by AbstractActionDescriptor instance contained in the array
 */
void NetworkBroker::registerAction(DeclareAction* declareActionClass) {
    CHAOS_ASSERT(commandDispatcher)
    commandDispatcher->registerAction(declareActionClass);
}

/*
 Deregister actions for a determianted domain
 */
void NetworkBroker::deregisterAction(DeclareAction* declareActionClass) {
    CHAOS_ASSERT(commandDispatcher)
    commandDispatcher->deregisterAction(declareActionClass);
}

#pragma mark Message Submission

/*!
 Submit a message specifing the destination
 */
bool NetworkBroker::submitMessage(string& serverAndPort, CDataWrapper *message, NetworkErrorHandler handler, const char * senderIdentifier, int64_t senderTag, bool onThisThread) {
    CHAOS_ASSERT(message && rpcClient)
    NetworkForwardInfo *nfi = new NetworkForwardInfo();
    nfi->destinationAddr = serverAndPort;
    nfi->message = message;
	//add answer id to datawrapper
    return rpcClient->submitMessage(nfi, onThisThread);
}

/*!
 Submite a new request to send to the remote host
 */
bool NetworkBroker::submiteRequest(string& serverAndPort,  CDataWrapper *request, NetworkErrorHandler handler, const char * senderIdentifier, int64_t senderTag, bool onThisThread) {
    CHAOS_ASSERT(request && rpcClient)
    request->addStringValue(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP, publishedHostAndPort);
    NetworkForwardInfo *nfi = new NetworkForwardInfo();
    nfi->destinationAddr = serverAndPort;
    nfi->message = request;
    return rpcClient->submitMessage(nfi, onThisThread);
}

/*
 */
MessageChannel *NetworkBroker::getNewMessageChannelForRemoteHost(CNetworkAddress *nodeNetworkAddress, EntityType type) {
    CHAOS_ASSERT(nodeNetworkAddress)
    MessageChannel *channel = NULL;
    switch (type) {
        case RAW:
            channel = new MessageChannel(this, nodeNetworkAddress->ipPort);
            delete(nodeNetworkAddress);
            break;
            
        case MDS:
            channel = new MDSMessageChannel(this, static_cast<CNodeNetworkAddress*>(nodeNetworkAddress));
            break;
            
        case DEVICE:
            channel = new DeviceMessageChannel(this, static_cast<CDeviceNetworkAddress*>(nodeNetworkAddress));
            break;
		case PERFORMANCE:
			channel = new common::message::PerformanceNodeChannel(this, nodeNetworkAddress, performance_session_managment.getLocalDirectIOClientInstance());
            break;
    }
	//check if the channel has been created
    if(channel){
        channel->init();
        boost::mutex::scoped_lock lock(mapRpcChannelAcces);
        activeRpcChannel.insert(make_pair(channel->channelID, channel));
    }
    return channel;
}

//!Metadata server channel creation
/*!
 Performe the creation of metadata server
 */
MDSMessageChannel *NetworkBroker::getMetadataserverMessageChannel() {
    CNodeNetworkAddress *mdsNodeAddr = new CNodeNetworkAddress();
    mdsNodeAddr->ipPort = GlobalConfiguration::getInstance()->getMetadataServerAddress();
    mdsNodeAddr->nodeID = ChaosSystemDomainAndActionLabel::SYSTEM_DOMAIN;
    return static_cast<MDSMessageChannel*>(getNewMessageChannelForRemoteHost(mdsNodeAddr, MDS));
}

//!Device channel creation
/*!
 Performe the creation of device channel
 \param deviceNetworkAddress device node address
 */
DeviceMessageChannel *NetworkBroker::getDeviceMessageChannelFromAddress(CDeviceNetworkAddress *deviceNetworkAddress) {
    return static_cast<DeviceMessageChannel*>(getNewMessageChannelForRemoteHost(deviceNetworkAddress, DEVICE));
}

//!performance channel creation
chaos::common::message::PerformanceNodeChannel *NetworkBroker::getPerformanceChannelFromAddress(CNetworkAddress  *node_network_address) {
	return static_cast<chaos::common::message::PerformanceNodeChannel*>(getNewMessageChannelForRemoteHost(node_network_address, PERFORMANCE));
}

//!Channel deallocation
/*!
 Perform the message channel deallocation
 */
void NetworkBroker::disposeMessageChannel(MessageChannel *messageChannelToDispose) {
    if(!messageChannelToDispose) return;
    
    boost::mutex::scoped_lock lock(mapRpcChannelAcces);
    
	//check if the channel is active
    if(activeRpcChannel.count(messageChannelToDispose->channelID) == 0) return;
    
	//remove the channel as active
    activeRpcChannel.erase(messageChannelToDispose->channelID);
    
	//deallocate it
    messageChannelToDispose->deinit();
    
	//dispose it
    delete(messageChannelToDispose);
}
//!Channel deallocation
/*!
 Perform the message channel deallocation
 */
void NetworkBroker::disposeMessageChannel(NodeMessageChannel *messageChannelToDispose) {
    NetworkBroker::disposeMessageChannel((MessageChannel*)messageChannelToDispose);
}
//Allocate a new endpoint in the direct io server
chaos_direct_io::DirectIOServerEndpoint *NetworkBroker::getDirectIOServerEndpoint() {
    chaos_direct_io::DirectIOServerEndpoint *result_endpoint = directIODispatcher->getNewEndpoint();
    return result_endpoint;
}
//Dispose an endpoint of the direct io server
void NetworkBroker::releaseDirectIOServerEndpoint(chaos_direct_io::DirectIOServerEndpoint *end_point) {
    directIODispatcher->releaseEndpoint(end_point);
}
//Return a new direct io client instance
chaos_direct_io::DirectIOClient *NetworkBroker::getDirectIOClientInstance() {
    MB_LAPP  << "Allocate a new DirectIOClient of type " << directIOClientImpl;
    return ObjectFactoryRegister<common::direct_io::DirectIOClient>::getInstance()->getNewInstanceByName(directIOClientImpl);
}
