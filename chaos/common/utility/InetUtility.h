/*	
 *	InetUtility.h
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

#ifndef CHAOSFramework_InetUtility_h
#define CHAOSFramework_InetUtility_h

#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <chaos/common/global.h>
#include <boost/regex.hpp>
#include <boost/asio.hpp>

namespace chaos {
    
        //! Regular expression for check server hostname and port
    static const boost::regex ServerHostNameRegExp("[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+:[0-9]{4,5}");
        //! Regular expression for check server ip and port
    static const boost::regex ServerIPAndPortRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b:[0-9]{4,5}");
        //! Regular expression for check server ip
    static const boost::regex ServerIPRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    
    class InetUtility {
        
    public:
        
            //! Network address scan
        /*! 
         This methdo will take care for network address scan process
         */
        static string scanForLocalNetworkAddress() {
			string ip_port;
            struct ifaddrs * ifAddrStruct=NULL;
            struct ifaddrs * ifa=NULL;
#if __APPLE__
            const char *ethInterfaceName  = "en";
#elif __linux__
            const char *ethInterfaceName = "eth";
#endif
            void * tmpAddrPtr=NULL;
            
            LAPP_ << "Scan for local network interface and ip";
            getifaddrs(&ifAddrStruct);
            
            for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
                if(strstr(ifa->ifa_name, ethInterfaceName)== NULL) continue;
                
                if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
                                                          // is a valid IP4 Address
                    tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                    char addressBuffer[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                    LAPP_ << "Interface " << ifa->ifa_name << " address " << addressBuffer << "<- this will be choosen";
                    ip_port = addressBuffer;
                    break;
                } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
                                                                 // is a valid IP6 Address
                    tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                    char addressBuffer[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                    LAPP_ << "Interface " << ifa->ifa_name << " address " << addressBuffer;
                } 
            }
            if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
            
            if(ip_port.size() == 0) {
                // no ip was found go to localhost
                ip_port.assign("127.0.0.1");
            }
			
			return ip_port;
        }
        
            //! Network port scan
        /*! 
         This methdo will take care for search nex available port starting from a base value
         */
        
        static int scanForLocalFreePort(int basePort) {
            int sockfd, portno;
            struct sockaddr_in serv_addr;
            portno = basePort;
            int err = 0;
            for (; err>=0;) {
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) return -1;
                bzero((char *) &serv_addr, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
                serv_addr.sin_port = htons(portno);
                err = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
                close(sockfd);
                if(err>=0)portno++;
            }
            
            return portno;
        }
        
        static bool checkWellFormedHostPort(std::string host_port) {
            return regex_match(host_port, ServerIPRegExp);
        }
		
		static void queryDns(std::string hostname, std::vector<std::string>& resolved_endpoints) {
			boost::asio::io_service io;
			boost::asio::ip::tcp::resolver resolver(io);
			boost::asio::ip::tcp::resolver::query q(hostname.c_str(), "");
			boost::system::error_code error;
			boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(q, error);
			boost::asio::ip::tcp::resolver::iterator end;
			while (iter != end) {
				resolved_endpoints.push_back(iter->endpoint().address().to_string());
				iter++;
			}
		}
		
#define		STRIP_TO_UI64(x)  boost::asio::ip::address_v4::from_string(x)
#define		UI64_TO_STRIP(i)  boost::asio::ip::address_v4(i).to_string()
    };
}
#endif
