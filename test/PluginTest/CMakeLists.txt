cmake_minimum_required(VERSION 2.6)
option(BUILD_FORCE_32 "Set to ON to enable 32 bit compilation" OFF)

IF( ($ENV{CHAOS32}) OR (BUILD_FORCE_32) )
    MESSAGE(STATUS "Enabling 32 bit Compilation")
    set (CMAKE_C_FLAGS "-m32")
    set (CMAKE_CXX_FLAGS "-m32")
    set (CMAKE_LINK_FLAGS "-m32")
ENDIF()

SET(plugin_src PluginTest.cpp )
SET(loadr_src main)

INCLUDE_DIRECTORIES(. ${PROJECT_SOURCE_DIR}/usr/local/include /usr/local/include /usr/include) 
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/usr/local/lib /usr/local/lib)

ADD_LIBRARY(PluginTest SHARED ${plugin_src})
set_target_properties(PluginTest PROPERTIES PREFIX "" SUFFIX ".extension")


ADD_EXECUTABLE(PluginLoader ${loadr_src})

SET(PluginTestLib $ENV{CHAOS_LINK_LIBRARY})
separate_arguments(PluginTestLib)


TARGET_LINK_LIBRARIES(PluginTest chaos_cutoolkit chaos_common ${PluginTestLib})
TARGET_LINK_LIBRARIES(PluginLoader chaos_cutoolkit chaos_common ${PluginTestLib})

INSTALL_TARGETS(/bin PluginTest)
INSTALL_TARGETS(/bin PluginLoader)