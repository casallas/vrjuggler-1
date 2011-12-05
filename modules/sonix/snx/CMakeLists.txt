
add_definitions(-D_SNX_BUILD_  "-DSNX_SHARE_DIR=\"${SNX_SHARE_DIR}\"") # "-DJCCL_ROOT_DIR=\"${CMAKE_INSTALL_PREFIX}\""

set(HEADERS
	AutoLink.h
	Endian.h
	ISoundImplementation.h
	PluginConfig.h
	snxConfig.h
	sonix.h
	SoundAPIInfo.h
	SoundFactory.h
	SoundHandle.h
	SoundImplementation.h
	SoundInfo.h
	StubSoundImplementation.h
	Util/Debug.h
	Util/Version.h)

set(SOURCES
	snxmain.cpp
	sonix.cpp
	SoundFactory.cpp
	SoundHandle.cpp
	SoundImplementation.cpp
	StubSoundImplementation.cpp
	Util/Version.cpp)

if(WIN32)
	add_definitions(-DCPPDOM_DYN_LINK -DBOOST_ALL_DYN_LINK)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/snxParam.h.in" "${CMAKE_CURRENT_BINARY_DIR}/snxParam.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/snxDefines.h.cmake_in" "${CMAKE_CURRENT_BINARY_DIR}/snxDefines.h")

add_library(sonix SHARED ${HEADERS} ${SOURCES})
target_link_libraries(sonix
	${JCCL_LIBRARIES})

if(WIN32)
	set_property(TARGET sonix
		PROPERTY OUTPUT_NAME sonix-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH})
else()
	set_property(TARGET sonix
		PROPERTY OUTPUT_NAME sonix-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR})
endif()
