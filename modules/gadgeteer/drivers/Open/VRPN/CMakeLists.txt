include_directories(${VRPN_INCLUDE_DIRS})

gadgeteer_add_driver(VRPN vrpn.rc Vrpn.cpp Vrpn.h)
target_link_libraries(VRPN_drv ${VRPN_LIBRARIES})
