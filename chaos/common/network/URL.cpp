/*
 *	URL.cpp
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
#include <chaos/common/network/URL.h>

using namespace chaos::common::network;

    //! Default contructor
URL::URL() {
    
}

URL::URL(std::vector<boost::regex>& _url_reg, std::string _url):
url(_url),
url_reg(_url_reg) {
}

URL::URL(std::string& _url):
url(_url){
}

URL::URL(const URL & _orig_url) {
    url = _orig_url.url;
    url_reg = _orig_url.url_reg;
}

bool URL::ok() {
    bool result = false;
    for (std::vector<boost::regex>::iterator it = url_reg.begin();
         it != url_reg.end() && !result;
         it++) {
        result = regex_match(url, *it);
    }
    return result;
}

const std::string URL::getURL() const {
    return url;
}