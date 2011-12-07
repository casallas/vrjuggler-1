gadgeteer_add_driver(DataGloveUltraWireless
	DataGloveUltraWireless.cpp
	DataGloveUltraWireless.h
	data_glove_ultra_wireless.rc
	DataGloveUltraWirelessStandalone.cpp
	DataGloveUltraWirelessStandalone.h)

if(BUILD_TESTING)
	add_executable(gadgeteer_DataGloveUltraWireless_testapp
		glove.cpp)
	target_link_libraries(gadgeteer_DataGloveUltraWireless_testapp ${VPR_LIBRARIES})
endif()

