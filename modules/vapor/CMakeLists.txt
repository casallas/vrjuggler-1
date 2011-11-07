cmake_minimum_required(VERSION 2.6.2)
project(VPR)

set(CPACK_PACKAGE_VENDOR "Iowa State University")
set(CPACK_PACKAGE_CONTACT "Ryan Pavlik <rpavlik@iastate.edu>")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION
	"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(MAJOR_VER_NUMBER ${CPACK_PACKAGE_VERSION_MAJOR})
set(MINOR_VER_NUMBER ${CPACK_PACKAGE_VERSION_MINOR})
set(PATCH_VER_NUMBER ${CPACK_PACKAGE_VERSION_PATCH})
find_package(Boost REQUIRED COMPONENTS filesystem system signals)

math(EXPR VER_NUMBER "${MAJOR_VER_NUMBER} * 1000000 + ${MINOR_VER_NUMBER} * 1000 + ${PATCH_VER_NUMBER}")
set(VER_STRING "\"${CPACK_PACKAGE_VERSION}\"")

option(VPR_SIMULATOR "Use simulated interface." OFF)


# OS define
if(WIN32)
	set(VPR_OS_Win32 1)
	set(VPR_OS VPR_OS_Win32)
else()
	set(VPR_OS_${CMAKE_SYSTEM_NAME} 1)
	set(VPR_OS VPR_OS_${CMAKE_SYSTEM_NAME})
endif()
message(STATUS "VPR: Platform ${VPR_OS}")

# System domain
if(WIN32)
	set(VPR_DOMAIN VPR_DOMAIN_WIN32)
else()
	option(VPR_USE_NSPR "Use NSPR system instead of POSIX." OFF)
	if(VPR_USE_NSPR)
		set(VPR_DOMAIN VPR_DOMAIN_NSPR)
		set(VPR_USE_POSIX OFF) # Not actually used in the header
	else()
		set(VPR_DOMAIN VPR_DOMAIN_POSIX)
		set(VPR_USE_POSIX ON) # Not actually used in the header
	endif()
endif()

if(WIN32)
	set(BOOST_DEFAULT ON)
else()
	set(BOOST_DEFAULT OFF)
endif()
option(VPR_USE_BOOST "Use Boost IO " ${BOOST_DEFAULT})

message(STATUS "VPR: Using ${VPR_DOMAIN} system - Boost IO: ${VPR_USE_BOOST}.")

# Threading
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
	message(STATUS "VPR: Using pthreads.")
	set(VPR_USE_PTHREADS 1)
elseif(CMAKE_USE_WIN32_THREADS_INIT)
	message(STATUS "VPR: Using Win32 threads.")
	set(VPR_USE_WINTHREADS 1)
else()
	message(FATAL_ERROR "VPR: Apparently unsupported thread type! ${CMAKE_THREADS_INIT}")
endif()


# Type sizes
if(NOT VPR_USE_BOOST)
	include(CheckTypeSize)
	macro(vpr_type_size _type _var)
		check_type_size("${_type}" SIZEOF_${_var})
		set(VPR_SIZEOF_MACROS "${VPR_SIZEOF_MACROS}\n${SIZEOF_${_var}_CODE}")
	endmacro()
	vpr_type_size(char CHAR)
	vpr_type_size(int INT)
	vpr_type_size(long LONG)
	vpr_type_size("long long" LONG_LONG)
	vpr_type_size(short SHORT)
endif()

set(VPR_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${VPR_INCLUDE_DIRECTORIES})

add_subdirectory(vpr)
