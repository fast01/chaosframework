/*
 *	LogManager.cpp
 *	!CHAOS
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

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/filters.hpp>
#include <boost/log/utility/init/to_file.hpp>
#include <boost/log/utility/init/to_console.hpp>
#include <boost/log/utility/init/common_attributes.hpp>
#include <boost/log/formatters.hpp>

#include <chaos/common/global.h>
#include <chaos/common/configuration/GlobalConfiguration.h>

#include "LogManager.h"

using namespace chaos;
namespace logging = boost::BOOST_LOG_NAMESPACE;
namespace fmt = boost::log::formatters;

void LogManager::init() throw(CException) {
    bool logOnConsole = GlobalConfiguration::getInstance()->getConfiguration()->getBoolValue(InitOption::OPT_LOG_ON_CONSOLE);
    bool logOnFile = GlobalConfiguration::getInstance()->getConfiguration()->getBoolValue(InitOption::OPT_LOG_ON_FILE);
    string logFileName = GlobalConfiguration::getInstance()->getConfiguration()->getStringValue(InitOption::OPT_LOG_FILE);
    
    
    if(logOnConsole){
        logging::init_log_to_console(std::clog,
                                     logging::keywords::format = "[%TimeStamp%]: %_%"); //(format=\"%y-%m-%d %H:%M:%S\")
    }
    
    if(logOnFile){
        logging::init_log_to_file
        (
         logging::keywords::file_name = logFileName,                  // file name pattern
         logging::keywords::rotation_size = 10 * 1024 * 1024,         // rotate files every 10 MiB...
         // ...or at midnight
         logging::keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0),
         logging::keywords::format = "[%TimeStamp%]: %_%");//(format=\"%y-%m-%d %H:%M:%S\")
    }
    
        //enable the log in case of needs
        //boost::log::add_common_attributes();
    boost::log::core::get()->set_logging_enabled(logOnConsole || logOnFile);
    boost::log::core::get()->add_global_attribute("TimeStamp", boost::make_shared< logging::attributes::local_clock >());
    /*
     if(logOnConsole || logOnFile) {
     logging::core::get()->set_filter (logging::filters::attr< logging::trivial::severity_level >("Severity") >= logging::trivial::info);
     }*/
 }