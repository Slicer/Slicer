# Generate the TCLAPConfig.cmake file in the build tree and configure one
# the installation tree.

# Settings specific to build trees
#
#    tclap include files referenced as "tclap/foo.h"
SET(TCLAP_INCLUDE_DIRS_CONFIG ${TCLAP_SOURCE_DIR}/include)
SET(TCLAP_USE_FILE_CONFIG ${TCLAP_BINARY_DIR}/UseTCLAP.cmake)


# Configure TCLAPConfig.cmake for the install tree.
CONFIGURE_FILE(${TCLAP_SOURCE_DIR}/TCLAPConfig.cmake.in
               ${TCLAP_BINARY_DIR}/TCLAPConfig.cmake @ONLY)



# Settings specific for installation trees
#
#    tclap include files referenced as "tclap/foo.h"


# Configure TCLAPInstallConfig.cmake for the install tree.
CONFIGURE_FILE(${TCLAP_SOURCE_DIR}/TCLAPInstallConfig.cmake.in
               ${TCLAP_BINARY_DIR}/install/TCLAPConfig.cmake @ONLY)
