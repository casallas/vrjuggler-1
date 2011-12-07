

set(CMAKE_SHARED_MODULE_PREFIX "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${GADGET_LIB_DIR}/drivers")

# Target to build all configured drivers
add_custom_target(gadgeteer_drivers)

function(gadgeteer_add_driver _driver_name)
	add_library(${_driver_name}_drv
		MODULE ${ARGN})

	target_link_libraries(${_driver_name}_drv gadget)

	install(TARGETS ${_driver_name}_drv
		LIBRARY DESTINATION lib/${GADGET_LIB_DIR}/drivers COMPONENT gadgeteer-runtime-drivers
		RUNTIME DESTINATION lib/${GADGET_LIB_DIR}/drivers COMPONENT gadgeteer-runtime-drivers)
	add_dependencies(gadgeteer_drivers ${_driver_name}_drv)
endfunction()

add_subdirectory(3Dconnexion/SpaceBall)
add_subdirectory(5DT/DataGlove)
add_subdirectory(5DT/DataGloveUltraWireless)
add_subdirectory(ART/DTrack)
add_subdirectory(Ascension/Flock)
add_subdirectory(Ascension/MotionStar)
add_subdirectory(Ascension/Wanda)
add_subdirectory(Elexol/Ether24)
#add_subdirectory(EssentialReality/P5Glove)
add_subdirectory(Fakespace/PinchGlove)
add_subdirectory(Immersion/IBox)
#add_subdirectory(Intersense/IntersenseAPI)
add_subdirectory(Intersense/IS900)
add_subdirectory(Logitech/ThreeDMouse)
#add_subdirectory(Microsoft/DirectXJoystick)
#add_subdirectory(Microsoft/SpeechRecognition)
#add_subdirectory(MotionNode/MotionNode)
#add_subdirectory(NaturalPoint/OptiTrack)
#add_subdirectory(noDNA/X-IST)
#add_subdirectory(Open/DTK)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_subdirectory(Open/LinuxJoydev)
endif()

include(CheckIncludeFileCXX)
check_include_file_cxx(sys/ipc.h HAVE_SYS_IPC_H)
check_include_file_cxx(sys/shm.h HAVE_SYS_SHM_H)
if(HAVE_SYS_IPC_H AND HAVE_SYS_SHM_H)
	add_subdirectory(Open/Trackd)
endif()

find_package(VRPN)
if(VRPN_FOUND)
	add_subdirectory(Open/VRPN)
else()
	message(STATUS "Gadgeteer: VRPN plugin disabled - dependency not found")
endif()

add_subdirectory(Polhemus/Fastrak)
add_subdirectory(USDigital/SerialEncoder)
#add_subdirectory(VirtualTechnologies/CyberGlove)
#add_subdirectory(VRCO/TrackdAPI)

