foreach(APP tcp_client tcp_server udp_client udp_server)
	add_executable(socket_${APP} ${APP}.cpp)
	target_link_libraries(socket_${APP} vpr)
endforeach()

