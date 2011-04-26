project(MimxMeshing)

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

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
SET(meshing_libs
  IO
  Adapter
  Common
  Filter
  #CommandLine
  Mesh
  Widgets
  BuildingBlock
  IA_FEMesh
  )
FOREACH(lib ${meshing_libs})
  ADD_SUBDIRECTORY(${lib})
ENDFOREACH(lib)


# --------------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
