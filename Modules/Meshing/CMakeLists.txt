project(MimxMeshing)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
set(meshing_libs
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
foreach(lib ${meshing_libs})
  add_subdirectory(${lib})
endforeach(lib)


# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
