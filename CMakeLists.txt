cmake_minimum_required(VERSION 3.12)
project(SimLink C CXX)

list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a")

if(MSVC)
  add_definitions("/MP")
endif()

if(MSVC)
  message("-- Using MSVC version " ${MSVC_VERSION})
endif()

IF(MSVC)
  add_definitions("/EHsc")
ENDIF(MSVC)

if(APPLE)
  add_compile_options("-std=c++17")
  #set(CMAKE_CXX_FLAGS "-lstdc++fs")
endif()

if(NO_LUA)
  add_definitions("-DNO_LUA")
endif()

link_directories("lib")
include_directories("lib/include" "inc")

set(SOURCES "src/SimLink.cpp"
            "src/EventMapper.cpp"
            "src/comm/CommManager.cpp"
            "src/comm/SerialDevice.cpp"
            "src/comm/NetworkDevice.cpp"
            "src/conf/Configuration.cpp"
            "src/conf/ConfigurationTypedef.cpp"
            "src/conf/Devices.cpp"
            "src/conf/Mappings.cpp"
            "src/conf/Simulator.cpp"
            "src/simconnect/SimConnectDataReceiver.cpp"
            "src/simconnect/SimConnectHandler.cpp"
            "src/simconnect/SimConnectNotificationReceiver.cpp"
            "src/iface/LuaInterface.cpp"
            "src/xp/xplane.cpp"
            "src/xp/xpnetconn.cpp"
            "lib/include/sago/platform_folders.cpp"
            )

if(MSVC)
  set(SOURCES ${SOURCES} "src/AppIcon.rc")
endif()

add_executable(${PROJECT_NAME} "${SOURCES}")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

add_definitions(-DWIN32_LEAN_AND_MEAN)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "./lib")
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_DATE_TIME_NO_LIB)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME ON) 
find_package(Boost REQUIRED COMPONENTS filesystem program_options) 

if(Boost_FOUND)
  message("-- Found Boost libraries: ${Boost_LIBRARIES}")
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
endif()

include(FindLua)
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} PRIVATE lua53)

set(MACOSX_BUNDLE_ICON_FILE "assets/logo.icns")

set(SimLink_ICON ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns)
set_source_files_properties(${SimLink_ICON} PROPERTIES
       MACOSX_PACKAGE_LOCATION "Resources")

# Apple requires linking to CoreServices
# Check sys name instead of "APPLE" for cross-compilation
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# Find the framework
	# ref: https://developer.apple.com/library/content/documentation/MacOSX/Conceptual/OSX_Technology_Overview/SystemFrameworks/SystemFrameworks.html
	find_library(_CoreServices_FRAMEWORK
		NAMES "CoreServices.framework" "CoreServices"
		PATH_SUFFIXES "CoreServices"
		PATHS "/System/Library/Frameworks"
	)
	# Make sure it was found
	if(NOT _CoreServices_FRAMEWORK)
		message(FATAL_ERROR "Could not find the CoreServices framework!")
	endif()
	# Link to the CoreServices framework. This also sets the correct linking options
	# "If the library file is in a Mac OSX framework, the Headers directory of the framework will also be processed as a usage requirement."
	target_link_libraries(${PROJECT_NAME} PRIVATE "${_CoreServices_FRAMEWORK}")
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE SimConnect)
endif()

