cmake_minimum_required(VERSION 2.6.2)
project(VRJugglerSuite)

###
# Version
###
set(CPACK_PACKAGE_VENDOR "Iowa State University")
set(CPACK_PACKAGE_CONTACT "Ryan Pavlik <rpavlik@iastate.edu>")
set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION
	"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

###
# Configuration
###
include(scripts/cmake/VRJugglerSuiteDefaultOptions.cmake)

option(BUILD_SONIX "Should we build the Sonix library?" ON)

add_subdirectory(modules)
