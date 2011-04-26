PROJECT(SlicerLibs)

#
# Order of project directory is important. Project should be topologically ordered
#

SET(dirs
  SlicerExecutionModel # Bundled subdirectory that can easily be included in other packages.
# FileWatcher
  )

IF(Teem_FOUND)
  LIST(APPEND dirs vtkTeem)
ENDIF()

LIST(APPEND dirs
  vtkITK
  FreeSurfer
  MRML
  MRMLCLI
  MRMLLogic
  MRMLDisplayableManager
  MRMLIDImageIO
  MGHImageIO
  IGT
  RemoteIO
  #SIGN
  qMRMLWidgets
  )

#-----------------------------------------------------------------------------
# CUDA
#-----------------------------------------------------------------------------
INCLUDE(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

IF(FOUND_CUDA AND FOUND_CUDART AND FOUND_CUT)
   OPTION(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" ON)
ELSE()
   OPTION(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" OFF)
ENDIF()

IF(CUDA_SUPPORT_ENABLED)
  LIST(APPEND dirs CudaSupport)
ENDIF()

#-----------------------------------------------------------------------------
# Set library properties
#-----------------------------------------------------------------------------

SET(ModuleDescriptionParser_USE_PYTHON ${Slicer_USE_PYTHONQT})

SET(IGT_USE_IGSTK ${Slicer_USE_IGSTK})
SET(IGT_USE_NAVITRACK ${Slicer_USE_NAVITRACK})
SET(ITKCommandIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
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

STRING(REPLACE ITKCommandIO CommandIO libs "${libs}")
  STRING(REPLACE SlicerIO slicerio libs "${libs}")
STRING(REPLACE MGHImageIO MGHIO libs "${libs}")
STRING(REPLACE MRMLIDImageIO MRMLIDIO libs "${libs}")
LIST(REMOVE_ITEM libs SlicerExecutionModel) # Not a library

#-----------------------------------------------------------------------------
# Update variable Slicer_Libs_LIBRARIES and Slicer_Libs_INCLUDE_DIRS
#-----------------------------------------------------------------------------

SET(Slicer_Libs_LIBRARIES ${Slicer_Libs_LIBRARIES} ${libs} CACHE INTERNAL "Slicer Libs libraries" FORCE)
SET(Slicer_Libs_INCLUDE_DIRS ${Slicer_Libs_INCLUDE_DIRS} ${include_dirs} CACHE INTERNAL "Slicer Libs includes" FORCE)

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------

FOREACH(dir ${dirs})
  SET(${dir}_INSTALL_BIN_DIR ${Slicer_INSTALL_LIB_DIR})
  SET(${dir}_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})
  ADD_SUBDIRECTORY(${dir})
ENDFOREACH()
