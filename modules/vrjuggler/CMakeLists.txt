cmake_minimum_required(VERSION 2.6.2)
project(VRJuggler)

###
# Version
###
set(CPACK_PACKAGE_VENDOR "Iowa State University")
set(CPACK_PACKAGE_CONTACT "Ryan Pavlik <rpavlik@iastate.edu>")
set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "0")
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

if(TARGET vpr AND TARGET jccl AND TARGET gadget AND TARGET sonix)
	set(GADGET_LIBRARIES gadget)
	set(SNX_LIBRARIES sonix)
	set(JCCL_LIBRARIES jccl)
	set(VPR_LIBRARIES vpr)
else()
	message(FATAL_ERROR "Building VR Juggler separate from the rest of the build has not yet been implemented!")
	#find_package(VPR REQUIRED) # TODO make this right
	#find_package(CPPDOM REQUIRED)
	#find_package(Boost REQUIRED COMPONENTS filesystem system signals) # TODO check this.
	#set(VPR_INCLUDE_DIRECTORIES ${VPR_INCLUDE_DIRS})
endif()

find_package(Boost REQUIRED COMPONENTS filesystem system signals program_options)

include(CTest)

set(VRJ_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${GADGET_INCLUDE_DIRECTORIES} ${SNX_INCLUDE_DIRECTORIES})
if(NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
	set(VRJ_INCLUDE_DIRECTORIES ${VRJ_INCLUDE_DIRECTORIES} PARENT_SCOPE)
endif()

###
# Configuration
###

include_directories(${VRJ_INCLUDE_DIRECTORIES})

if(BUILD_VERSIONED_DIRECTORIES)
	set(VRJ_SHARE_DIR vrjuggler-${MAJOR_VER_NUMBER}.${MINOR_VER_NUMBER})
	set(VRJ_LIB_DIR vrjuggler-${MAJOR_VER_NUMBER}.${MINOR_VER_NUMBER})
	set(VRJ_INCLUDE_PREFIX include/vrjuggler-${MAJOR_VER_NUMBER}.${MINOR_VER_NUMBER})
	set(VRJ_VERSION_DOT "${MAJOR_VER_NUMBER}.${MINOR_VER_NUMBER}")
	set(VRJ_USE_VERSIONING 1)
else()
	set(VRJ_SHARE_DIR share/vrjuggler)
	set(VRJ_LIB_DIR vrjuggler)
	set(VRJ_INCLUDE_PREFIX include)
endif()

add_subdirectory(vrj)
add_subdirectory(vrj/Draw)

install(DIRECTORY vrj
	DESTINATION ${VRJ_INCLUDE_PREFIX}
	COMPONENT vrj-dev
	FILES_MATCHING PATTERN *.h)

install(DIRECTORY data
	DESTINATION ${VRJ_SHARE_DIR}
	COMPONENT vrj-runtime)

install(DIRECTORY samples
	DESTINATION ${VRJ_SHARE_DIR}
	COMPONENT vrj-samples)

install(DIRECTORY tools
	DESTINATION ${VRJ_SHARE_DIR}
	COMPONENT vrj-tools)

install(DIRECTORY test
	DESTINATION ${VRJ_SHARE_DIR}
	COMPONENT vrj-test)

if(BUILD_TESTING)
#	add_subdirectory(test)
endif()


