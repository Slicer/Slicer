project(HelloSlicer)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()


# We need Slicer3

find_package(Slicer3 REQUIRED)
include(${Slicer_USE_FILE})

# Set reasonable default install prefix and output paths
# (after setting Slicer_DIR, delete CMAKE_INSTALL_PREFIX and re-configure)

slicer3_set_default_install_prefix_for_external_projects()

# The Command Line Module

set(CLP HelloSlicer)
set(${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml)

# Build it

add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries(${CLP} ${ITK_LIBRARIES})

# Install it

slicer3_install_plugins(${CLP})

# Test it

# slicer3_enable_plugins_testing()

# slicer3_add_plugins_test(${CLP}Test1 ${CLP} --help)


