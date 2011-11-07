foreach(APP first reader second writer)
	add_executable(serialport_${APP} ${APP}.cpp)
	target_link_libraries(serialport_${APP} vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})
endforeach()

