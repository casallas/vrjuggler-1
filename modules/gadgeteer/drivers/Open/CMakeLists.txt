
#TODO DTK
#todo Trackd
#todo LinuxJoydev

find_package(VRPN)
if(VRPN_FOUND)
	add_subdirectory(VRPN)
else()
	message(STATUS "Gadgeteer: VRPN plugin disabled - dependency not found")
endif()

