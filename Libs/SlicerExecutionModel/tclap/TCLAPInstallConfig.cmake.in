# tclap could be installed anywhere, so set all paths based on where 
# this file was found (which should be the lib/tclap directory of the 
# installation)
GET_FILENAME_COMPONENT(tclap_CONFIG_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# The TCLAP include directories.
#    tclap include files referenced as "tclap/foo.h"
SET(TCLAP_INCLUDE_DIRS "${tclap_CONFIG_DIR}/../../include/")

# The TCLAP USE file
SET(TCLAP_USE_FILE "${tclap_CONFIG_DIR}/UseTCLAP.cmake")
