
if(__juggler_suite_default_options)
	return()
endif()
set(__juggler_suite_default_options YES)

get_filename_component(VRJUGGLERSUITE_MODULE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
list(APPEND CMAKE_MODULE_PATH "${VRJUGGLERSUITE_MODULE_DIR}")

if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if(NOT SHARE_OUTPUT_DIRECTORY)
	set(SHARE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/share)
endif()

if(WIN32)
	set(DEFAULT_VERSIONED OFF)
else()
	set(DEFAULT_VERSIONED ON)
endif()
option(BUILD_VERSIONED_DIRECTORIES "Should we version the directories for plugins and data?" ${DEFAULT_VERSIONED})

if(CMAKE_VERSION VERSION_GREATER 2.8.5)
	option(BUILD_JAVA "Should we build the Java components?" ON)
else()
	set(BUILD_JAVA OFF)
endif()

if(BUILD_JAVA)
	find_package(Java)
	if(JAVA_FOUND)
		include(UseJava)
	else()
		message(FATAL_ERROR "BUILD_JAVA is on, but Java wasn't found!")
	endif()
endif()

if(APPLE)
	option(BUILD_WITH_COCOA "Should we build with Cocoa (Mac OS X native) code? (Alternative is X11)" ON)
endif()

if(MSVC)
	option(BUILD_WITHOUT_COMMON_WARNINGS "Should we disable some distracting build warnings?" ON)
	if(BUILD_WITHOUT_COMMON_WARNINGS)
		# Disable some warnings
		# 4275: non dll-interface class used as base for dll-interface class
		# 4251: needs to have dll-interface to be used by clients of class
		# 4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275 /wd4251 /wd4290")

		# Deprecation warnings for strerror, etc.
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)

		# Warning related to checked iterators
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	endif()
endif()

if(MSVC)
	option(BUILD_WITH_PROJECT_FOLDERS
		"Use project folders in build system - not compatible with Visual C++ Express editions!"
		OFF)
else()
	set(BUILD_WITH_PROJECT_FOLDERS ON)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ${BUILD_WITH_PROJECT_FOLDERS})

set(VRJUGGLERSUITE_CORELIB_PROJECT_FOLDER "Core VR Juggler libraries")
set(VRJUGGLERSUITE_META_PROJECT_FOLDER "Convenience targets")

if(WIN32)
	add_definitions(-DCPPDOM_DYN_LINK -DBOOST_ALL_DYN_LINK)
endif()

function(vrjugglersuite_create_version_rc)
	configure_file("${VRJUGGLERSUITE_MODULE_DIR}/version.rc.cmake_in" "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endfunction()
