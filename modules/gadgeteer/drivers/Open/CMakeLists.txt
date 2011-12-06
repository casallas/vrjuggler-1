
#TODO DTK
#todo Trackd

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_subdirectory(LinuxJoydev)
endif()

find_package(VRPN)
if(VRPN_FOUND)
	add_subdirectory(VRPN)
else()
	message(STATUS "Gadgeteer: VRPN plugin disabled - dependency not found")
endif()

