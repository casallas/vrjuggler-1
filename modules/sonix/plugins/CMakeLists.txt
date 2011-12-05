

set(CMAKE_SHARED_MODULE_PREFIX "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${SNX_LIB_DIR}")

option(SONIX_BUILD_OPENAL "Build the OpenAL backend plugin?" ON)
#option(SONIX_BUILD_AUDIERE "Build the Audiere backend plugin?" OFF)
#option(SONIX_BUILD_SUBSYNTH "Build the Subsynth backend plugin?" OFF)

if(SONIX_BUILD_OPENAL)
	add_subdirectory(OpenAL)
endif()

#if(SONIX_BUILD_AUDIERE)
#	add_subdirectory(Audiere)
#endif()

#if(SONIX_BUILD_SUBSYNTH)
#	add_subdirectory(Subsynth)
#endif()
