PROJECT(SlicerLibs)

# Enable this option to avoid unnecessary re-compilation associated with command line module
SET(GENERATECLP_USE_MD5 ON)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------

SET(dirs
  SlicerExecutionModel # Bundled subdirectory that can easily be included in other packages.
# FileWatcher
  )

LIST(APPEND dirs
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
#INCLUDE(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

#IF(FOUND_CUDA AND FOUND_CUDART AND FOUND_CUT)
#   OPTION(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" ON)
#ELSE()
#   OPTION(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" OFF)
#ENDIF()

#IF(CUDA_SUPPORT_ENABLED)
#  LIST(APPEND dirs CudaSupport)
#ENDIF()

#-----------------------------------------------------------------------------
# Set library properties
#-----------------------------------------------------------------------------

# Python support within ModuleDescriptionParser is used only within ModuleFactory.cxx
# Considering the ModuleFactory isn't used anymore to discover modules, let's disable the 
# associated python support so that the python library aren't a dependency of 
# either ModuleDescriptionParser or program/library depending on ModuleDescriptionParser.
SET(ModuleDescriptionParser_USE_PYTHON OFF)

SET(IGT_USE_IGSTK ${Slicer_USE_IGSTK})
SET(IGT_USE_NAVITRACK ${Slicer_USE_NAVITRACK})
SET(MGHImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
SET(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
# vtkITK contains tests that uses MRML's test data.
SET(MRML_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MRML/Testing/TestData)

#-----------------------------------------------------------------------------
# Generate list of corresponding libraries and include directories
#-----------------------------------------------------------------------------
SET(libs)
SET(include_dirs)
FOREACH(dir ${dirs})
  LIST(APPEND include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
    ${CMAKE_CURRENT_BINARY_DIR}/${dir}
    )
  LIST(APPEND libs ${dir})
ENDFOREACH()

# Since there is inconsistency between library name and project/dir name, 
# let's tweak the list of generated library names

#STRING(REPLACE MGHImageIO MGHIO libs "${libs}")
#STRING(REPLACE MRMLIDImageIO MRMLIDIO libs "${libs}")
#LIST(REMOVE_ITEM libs SlicerExecutionModel) # Not a library

# Anticipating the move of MRML related libraries to a standalone project, 
# let's group these libraries.
# Note also that since MRMLIDImageIO is an ITK plugin it is NOT added to that list.
SET(MRML_LIBRARIES 
  MRML MRMLCLI MRMLLogic MRMLDisplayableManager 
  CACHE INTERNAL "MRML libraries" FORCE)

#-----------------------------------------------------------------------------
# Set variable Slicer_Libs_INCLUDE_DIRS
#-----------------------------------------------------------------------------
SET(Slicer_Libs_INCLUDE_DIRS ${include_dirs} CACHE INTERNAL "Slicer Libs includes" FORCE)

#-----------------------------------------------------------------------------
# Set variable Slicer_Libs_VTK_WRAPPED_LIBRARIES
#-----------------------------------------------------------------------------
SET(Slicer_Libs_VTK_WRAPPED_LIBRARIES 
  vtkTeem
  vtkITK
  FreeSurfer
  MRML
  MRMLCLI
  MRMLLogic
  MRMLDisplayableManager
  RemoteIO
  CACHE INTERNAL "Slicer Libs VTK wrapped" FORCE)

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
FOREACH(dir ${dirs})
  SET(${dir}_INSTALL_BIN_DIR ${Slicer_INSTALL_LIB_DIR})
  SET(${dir}_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})
  SET(${dir}_INSTALL_SHARE_DIR ${Slicer_INSTALL_ROOT}share/${dir})
  SET(${dir}_INSTALL_NO_DEVELOPMENT ${Slicer_INSTALL_NO_DEVELOPMENT})
  SET(${dir}_EXPORT_FILE ${Slicer_BINARY_DIR}/SlicerTargets.cmake)
  ADD_SUBDIRECTORY(${dir})
ENDFOREACH()
