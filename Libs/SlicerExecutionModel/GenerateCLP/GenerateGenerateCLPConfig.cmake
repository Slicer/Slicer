# Generate the GenerateCLPConfig.cmake file in the build tree and configure
# one the installation tree.

# Following the standard pattern, UseGenerateCLP.cmake is a configured file
#

get_target_property(GenerateCLP_EXE_PATH GenerateCLP LOCATION)

if(GenerateCLP_EXE_PATH)
  set(GENERATECLP_EXE "${GenerateCLP_EXE_PATH}")
else(GenerateCLP_EXE_PATH)
  if(Slicer_BINARY_DIR)
    set(USE_CONFIG_FILE_INSTALL_DIR ${Slicer_BINARY_DIR}/bin)
  else(Slicer_BINARY_DIR)
    set(USE_CONFIG_FILE_INSTALL_DIR ${GenerateCLP_BINARY_DIR})
  endif(Slicer_BINARY_DIR)
  set(GENERATECLP_EXE "${USE_CONFIG_FILE_INSTALL_DIR}/${CMAKE_CFG_INTDIR}/GenerateCLP")
endif(GenerateCLP_EXE_PATH)

# Settings specific to build trees
#
#
set(GenerateCLP_USE_FILE_CONFIG ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake)
configure_file(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
  ${GenerateCLP_USE_FILE_CONFIG}
  @ONLY)

set(GenerateCLP_INCLUDE_DIRS_CONFIG ${GenerateCLP_BINARY_DIR} ${GenerateCLP_SOURCE_DIR})
set(GenerateCLP_LIBRARY_DIRS_CONFIG ${GenerateCLP_BINARY_DIR})
set(GENERATECLP_EXE_CONFIG "${GENERATECLP_EXE}")
if(NOT WIN32)
  set(GENERATECLP_EXE_CONFIG "${GENERATECLP_EXE}Launcher")
endif(NOT WIN32)
set(ITK_DIR_CONFIG ${ITK_DIR})
configure_file(${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in
  ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake @ONLY)


# Settings specific for installation trees
#
#
configure_file(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
  ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
  @ONLY)

configure_file(${GenerateCLP_SOURCE_DIR}/GenerateCLPInstallConfig.cmake.in
  ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install @ONLY)
