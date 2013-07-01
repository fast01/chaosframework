//
//  PluginTest.cpp
//  PluginTest
//
//  Created by Claudio Bisegni on 6/22/13.
//
//

#include "PluginTest.h"

//define the plugin
DEFINE_PLUGIN_CLASS(PluginAlias, 1.0, DRV, PluginTest)

DEFINE_CU_DRIVER_PLUGIN_CLASS(DriverAlias, 1.0, Sl7TcpDriver)


//register the plugin with his init attribute
OPEN_REGISTER_PLUGIN
    REGISTER_PLUGIN(PluginAlias)
    REGISTER_PLUGIN(DriverAlias)

    REGISTER_PLUGIN_INIT_ATTRIBUTE(PluginAlias,http_address)
    REGISTER_PLUGIN_INIT_ATTRIBUTE(PluginAlias,http_port)
    REGISTER_PLUGIN_INIT_ATTRIBUTE(DriverAlias,http_address)
    REGISTER_PLUGIN_INIT_ATTRIBUTE(DriverAlias,http_port)
CLOSE_REGISTER_PLUGIN

PluginTest::PluginTest() {
     std::cout << "PluginTest created" << std::endl;
}

void PluginTest::test(int num) {
    for(int idx = 0; idx < num; idx++) {
        std::cout << "Plugin in test output" << std::endl;
    }
}


DEFAULT_CU_DRIVER_PLUGIN_CONSTRUCTOR(Sl7TcpDriver) {
    std::cout << "Driver plugin created" << std::endl;
}