/*
 *	AsioEventForwarder.cpp
 *	CHAOSFramework
 *	Created by Claudio Bisegni on 26/08/12.
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
#include <string.h>
#include <chaos/common/event/AsioEventForwarder.h>
#include <chaos/common/global.h>
#include <chaos/common/utility/UUIDUtil.h>
using namespace std;
using namespace boost;
using namespace chaos;
using namespace chaos::event;

AsioEventForwarder::AsioEventForwarder(const boost::asio::ip::address& multicast_address,
                                       unsigned short mPort,
                                       boost::asio::io_service& io_service
                                       ) :_endpoint(multicast_address, mPort), _socket(io_service, _endpoint.protocol()) {
    hanlderID = UUIDUtil::generateUUIDLite();
    sent = false;
}

/*
 init the event adapter
 */
void AsioEventForwarder::init() throw(CException) {
    CObjectProcessingPriorityQueue<EventDescriptor>::init(1);
}

/*
 deinit the event adapter
 */
void AsioEventForwarder::deinit() throw(CException) {
    CObjectProcessingPriorityQueue<EventDescriptor>::clear();
    CObjectProcessingPriorityQueue<EventDescriptor>::deinit();
}

void AsioEventForwarder::handle_send_to(const boost::system::error_code& error) {
    if (!error) {
       
    }
        //we need to delete the last event object
    delete(currentEventForwarded);
    currentEventForwarded = NULL;
    sent = true;
    boost::unique_lock<boost::mutex> lock( wait_answer_mutex );
}

void AsioEventForwarder::submitEventAsync(EventDescriptor *event) {
    CObjectProcessingPriorityQueue<EventDescriptor>::push(event, event->getEventPriority());
}

void AsioEventForwarder::processBufferElement(EventDescriptor *priorityElement, ElementManagingPolicy& policy) throw(CException) {
    boost::unique_lock<boost::mutex> lock( wait_answer_mutex );
    while ( ! sent ) wait_answer_condition.wait(lock);
    policy.elementHasBeenDetached = true;
    currentEventForwarded = priorityElement;
    _socket.async_send_to(boost::asio::buffer(currentEventForwarded->getEventData(), currentEventForwarded->getEventDataLength()), _endpoint,
                          boost::bind(&AsioEventForwarder::handle_send_to, this,
                                      boost::asio::placeholders::error));

}