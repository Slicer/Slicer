project(SlicerLibs)

# Enable this option to avoid unnecessary re-compilation associated with command line module
set(GENERATECLP_USE_MD5 ON)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------

set(dirs
# FileWatcher
  )

list(APPEND dirs
  vtkTeem
  vtkITK
  FreeSurfer
  MRML
  MRMLCLI
  RemoteIO
  MRMLLogic
  MRMLDisplayableManager
  MRMLIDImageIO
  MGHImageIO
  IGT
  #SIGN
  qMRMLWidgets
  )

#-----------------------------------------------------------------------------
# CUDA
#-----------------------------------------------------------------------------
#include(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

#if(FOUND_CUDA AND FOUND_CUDART AND FOUND_CUT)
#   option(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" ON)
#else()
#   option(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" OFF)
#endif()

#if(CUDA_SUPPORT_ENABLED)
#  list(APPEND dirs CudaSupport)
#endif()

#-----------------------------------------------------------------------------
# Set library properties
#-----------------------------------------------------------------------------

# Python support within ModuleDescriptionParser is used only within ModuleFactory.cxx
# Considering the ModuleFactory isn't used anymore to discover modules, let's disable the
# associated python support so that the python library aren't a dependency of
# either ModuleDescriptionParser or program/library depending on ModuleDescriptionParser.
set(ModuleDescriptionParser_USE_PYTHON OFF)

# ITKFactories directories
set(MGHImageIO_ITKFACTORIES_DIR ${Slicer_ITKFACTORIES_DIR})
set(MGHImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
set(MRMLIDImageIO_ITKFACTORIES_DIR ${Slicer_ITKFACTORIES_DIR})
set(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})

set(IGT_USE_IGSTK ${Slicer_USE_IGSTK})
set(IGT_USE_NAVITRACK ${Slicer_USE_NAVITRACK})

# vtkITK contains tests that uses MRML's test data.
set(MRML_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MRML/Testing/TestData)

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  message(STATUS "Configuring library: ${dir}")
  set(${dir}_INSTALL_BIN_DIR ${Slicer_INSTALL_LIB_DIR})
  set(${dir}_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})
  set(${dir}_INSTALL_SHARE_DIR ${Slicer_INSTALL_ROOT}share/${dir})
  set(${dir}_INSTALL_NO_DEVELOPMENT ${Slicer_INSTALL_NO_DEVELOPMENT})
  set(${dir}_EXPORT_FILE ${Slicer_BINARY_DIR}/SlicerTargets.cmake)
  add_subdirectory(${dir})
endforeach()

#-----------------------------------------------------------------------------
# Generate list of include directories
#-----------------------------------------------------------------------------
set(include_dirs)
foreach(dir ${dirs})
  list(APPEND include_dirs ${${dir}_INCLUDE_DIRS})
endforeach()

# Set variable Slicer_Libs_INCLUDE_DIRS
set(Slicer_Libs_INCLUDE_DIRS ${include_dirs} CACHE INTERNAL "Slicer Libs includes" FORCE)

#-----------------------------------------------------------------------------
# Set MRML_LIBRARIES
#-----------------------------------------------------------------------------
# Anticipating the move of MRML related libraries to a standalone project,
# let's group these libraries.
# Note also that since MRMLIDImageIO is an ITK plugin it is NOT added to that list.
set(MRML_LIBRARIES
  MRML MRMLCLI MRMLLogic MRMLDisplayableManager
  CACHE INTERNAL "MRML libraries" FORCE)

#-----------------------------------------------------------------------------
# Set variable Slicer_Libs_VTK_WRAPPED_LIBRARIES
#-----------------------------------------------------------------------------
set(Slicer_Libs_VTK_WRAPPED_LIBRARIES
  vtkTeem
  vtkITK
  FreeSurfer
  MRML
  MRMLCLI
  MRMLLogic
  MRMLDisplayableManager
  RemoteIO
  CACHE INTERNAL "Slicer Libs VTK wrapped" FORCE)
