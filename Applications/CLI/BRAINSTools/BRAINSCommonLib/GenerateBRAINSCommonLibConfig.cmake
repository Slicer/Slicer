# Generate the BRAINSCommonLibConfig.cmake file in the build tree and configure
# one the installation tree.

# Settings specific to build trees
#
#
set(BRAINSCommonLib_USE_FILE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/UseBRAINSCommonLib.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UseBRAINSCommonLib.cmake.in
  ${BRAINSCommonLib_USE_FILE_CONFIG}
  @ONLY)

set(BRAINSCommonLib_DATA_DIRS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
set(BRAINSCommonLib_INCLUDE_DIRS_CONFIG ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set(BRAINSCommonLib_LIBRARY_DIRS_CONFIG ${CMAKE_CURRENT_BINARY_DIR})
set(ITK_DIR_CONFIG ${ITK_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BRAINSCommonLibConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLibConfig.cmake @ONLY)

# Settings specific for installation trees
#
#
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UseBRAINSCommonLib.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UseBRAINSCommonLib.cmake_install
  @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BRAINSCommonLibInstallConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLibConfig.cmake_install @ONLY)
