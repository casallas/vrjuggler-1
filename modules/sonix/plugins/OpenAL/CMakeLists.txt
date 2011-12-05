find_package(ALUT REQUIRED)

include_directories(${ALUT_INCLUDE_DIRS})

add_library(OpenAL_snd
	MODULE
	OpenALSoundImplementation.cpp
	OpenALSoundImplementation.h
	openal.rc)

target_link_libraries(OpenAL_snd ${ALUT_LIBRARIES})
