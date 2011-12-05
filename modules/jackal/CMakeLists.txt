cmake_minimum_required(VERSION 2.6.2)
project(JCCL)

###
# Version
###
set(CPACK_PACKAGE_VENDOR "Iowa State University")
set(CPACK_PACKAGE_CONTACT "Ryan Pavlik <rpavlik@iastate.edu>")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "4")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION
	"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(MAJOR_VER_NUMBER ${CPACK_PACKAGE_VERSION_MAJOR})
set(MINOR_VER_NUMBER ${CPACK_PACKAGE_VERSION_MINOR})
set(PATCH_VER_NUMBER ${CPACK_PACKAGE_VERSION_PATCH})

math(EXPR VER_NUMBER "${MAJOR_VER_NUMBER} * 1000000 + ${MINOR_VER_NUMBER} * 1000 + ${PATCH_VER_NUMBER}")
set(VER_STRING "\"${CPACK_PACKAGE_VERSION}\"")


# CMake modules
include("${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/VRJugglerSuiteDefaultOptions.cmake")

###
# Dependencies
###

if(TARGET vpr)
	set(VPR_LIBRARIES vpr)
else()
	message(FATAL_ERROR "Building JCCL separate from the rest of the build has not yet been implemented!")
	#find_package(VPR REQUIRED) # TODO make this right
	#find_package(CPPDOM REQUIRED)
	#find_package(Boost REQUIRED COMPONENTS filesystem system signals) # TODO check this.
	#set(VPR_INCLUDE_DIRECTORIES ${VPR_INCLUDE_DIRS})
endif()

include(CTest)

set(JCCL_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/common ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_CURRENT_SOURCE_DIR}/rtrc ${CMAKE_CURRENT_BINARY_DIR}/common ${VPR_INCLUDE_DIRECTORIES})
if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
	set(JCCL_INCLUDE_DIRECTORIES ${JCCL_INCLUDE_DIRECTORIES} PARENT_SCOPE)
endif()

###
# Configuration
###


include_directories(${JCCL_INCLUDE_DIRECTORIES})

if(BUILD_VERSIONED_DIRECTORIES)
	set(JCCL_SHARE_DIR jccl-${MAJOR_VER_NUMBER}.${MINOR_VER_NUMBER})
else()
	set(JCCL_SHARE_DIR jccl)
endif()

add_subdirectory(common/jccl)
add_subdirectory(data)
add_subdirectory(tools/xmlupdate)

###
# Library Build
###

set(SOURCES
	common/jccl/jcclmain.cpp
	common/jccl/Util/Version.cpp

	config/jccl/Config/ConfigDefinition.cpp
	config/jccl/Config/ConfigDefinitionReader.cpp
	config/jccl/Config/ConfigDefinitionRepository.cpp
	config/jccl/Config/ConfigElement.cpp
	config/jccl/Config/Configuration.cpp
	config/jccl/Config/ElementFactory.cpp
	config/jccl/Config/ParseUtil.cpp
	config/jccl/Config/PropertyDefinition.cpp

	rtrc/jccl/RTRC/ConfigElementHandler.cpp
	rtrc/jccl/RTRC/ConfigManager.cpp
	rtrc/jccl/RTRC/DepChecker.cpp
	rtrc/jccl/RTRC/DependencyManager.cpp

	jccl.rc)

set(HEADERS
	common/jccl/AutoLink.h
	common/jccl/jcclConfig.h
	common/jccl/Plugins/PluginConfig.h
	common/jccl/Util/Debug.h
	common/jccl/Util/Version.h

	config/jccl/Config/ConfigDefinition.h
	config/jccl/Config/ConfigDefinitionPtr.h
	config/jccl/Config/ConfigDefinitionReader.h
	config/jccl/Config/ConfigDefinitionRepository.h
	config/jccl/Config/ConfigElement.h
	config/jccl/Config/ConfigElementPtr.h
	config/jccl/Config/ConfigTokens.h
	config/jccl/Config/Configuration.h
	config/jccl/Config/ElementFactory.h
	config/jccl/Config/ParseUtil.h
	config/jccl/Config/PropertyDefinition.h

	rtrc/jccl/RTRC/ConfigElementHandler.h
	rtrc/jccl/RTRC/ConfigManager.h
	rtrc/jccl/RTRC/DepChecker.h
	rtrc/jccl/RTRC/DependencyManager.h
	rtrc/jccl/RTRC/RemoteReconfig.h
)

add_definitions(-D_JCCL_BUILD_ "-DJCCL_ROOT_DIR=\"${CMAKE_INSTALL_PREFIX}\"" "-DJCCL_SHARE_DIR=\"${JCCL_SHARE_DIR}\"")

if(WIN32)
	add_definitions(-DCPPDOM_DYN_LINK -DBOOST_ALL_DYN_LINK)
endif()

add_library(jccl SHARED ${HEADERS} ${SOURCES})
target_link_libraries(jccl
	${VPR_LIBRARIES})

if(WIN32)
	set_property(TARGET jccl
		PROPERTY OUTPUT_NAME jccl-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH})

	set_property(TARGET jccl
		PROPERTY OUTPUT_NAME_Debug jccl_d-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH})
else()
	set_property(TARGET jccl
		PROPERTY OUTPUT_NAME jccl-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR})

	set_property(TARGET jccl
		PROPERTY OUTPUT_NAME_Debug jccl_d-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR})
endif()

if(BUILD_TESTING)
#	add_subdirectory(test)
endif()


