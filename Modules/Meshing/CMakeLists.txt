project(MimxMeshing)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
endif(NOT Slicer3_SOURCE_DIR)

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
if(Slicer3_BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(Slicer3_BUILD_TESTING)
