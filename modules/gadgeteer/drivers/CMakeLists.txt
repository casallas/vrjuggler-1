

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

#add_subdirectory(3Dconnexion)
#add_subdirectory(5DT)
#add_subdirectory(ART)
#add_subdirectory(Ascension)
#add_subdirectory(Elexol)
#add_subdirectory(EssentialReality)
#add_subdirectory(Fakespace)
#add_subdirectory(Immersion)
#add_subdirectory(Intersense)
#add_subdirectory(Logitech)
#add_subdirectory(Microsoft)
#add_subdirectory(MotionNode)
#add_subdirectory(NaturalPoint)
#add_subdirectory(noDNA)
add_subdirectory(Open)
#add_subdirectory(Polhemus)
#add_subdirectory(USDigital)
#add_subdirectory(VirtualTechnologies)
#add_subdirectory(VRCO)

