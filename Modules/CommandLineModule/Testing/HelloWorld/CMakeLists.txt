project(HelloWorld)

cmake_minimum_required(VERSION 2.6)

# We need Slicer3

find_package(Slicer3 REQUIRED)
include(${Slicer3_USE_FILE})

# Set reasonable default install prefix and output paths
# (after setting Slicer3_DIR, delete CMAKE_INSTALL_PREFIX and re-configure)

slicer3_set_default_plugins_install_prefix()
slicer3_set_default_plugins_output_path()

# The Command Line Module

set(CLP HelloWorld)
set(${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml)

# Build it

add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries(${CLP} ${ITK_LIBRARIES})

# Install it

install(TARGETS ${CLP}
  RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
  )

# Test it

enable_testing()
include(CTest)
set(UPDATE_TYPE svn)

add_test(HelloWorldTest1 ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${CLP} --help)

