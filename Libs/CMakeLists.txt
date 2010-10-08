project(SlicerLibs)

## Delegate the building of the SlicerExecutionModel to
## a bundled subdirectory that can easily be included in
## other packages.
set(ModuleDescriptionParser_USE_PYTHON ${Slicer_USE_PYTHON})
ADD_SUBDIRECTORY(SlicerExecutionModel)

set(dirs
# FileWatcher
  )

IF(Slicer_USE_KWWIDGETS)
  SET(dirs ${dirs}
    LoadableModule
    )
ENDIF(Slicer_USE_KWWIDGETS)

# Note: Order of of SUBDIRS is important if your projects depend
# on code in other projects (put dependent project later).

if(Teem_FOUND)
  set(dirs ${dirs} vtkTeem)
endif(Teem_FOUND)

SET(dirs ${dirs}
  vtkITK
  FreeSurfer
  MRML
  MRMLCLI
  MRMLLogic
  MRMLDisplayableManager
  )

IF(Slicer_USE_KWWIDGETS)
  SET(dirs ${dirs}
    GenerateLM
    SlicerIO
    ITKCommandIO
    )
ENDIF(Slicer_USE_KWWIDGETS)

SET(dirs ${dirs}
  MRMLIDImageIO
  MGHImageIO
  IGT
  RemoteIO
  #SIGN
  )



#-----------------------------------------------------------------------------
# QT related libraries
#
IF(Slicer_USE_QT)
set(dirs ${dirs}
  qMRMLWidgets
  )
ENDIF(Slicer_USE_QT)

#-----------------------------------------------------------------------------
# CUDA
#
INCLUDE(${CMAKE_SOURCE_DIR}/CMake/cuda/FindCuda.cmake)

IF(FOUND_CUDA AND FOUND_CUDART AND FOUND_CUT)
   OPTION(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" ON)
ELSE(FOUND_CUDA AND FOUND_CUDART AND FOUND_CUT)
   OPTION(CUDA_SUPPORT_ENABLED "If Cuda Support should be integrated into Slicer. You have to configure once again to see the other availiable options" OFF)
ENDIF(FOUND_CUDA AND FOUND_CUDART AND FOUND_CUT)

IF(CUDA_SUPPORT_ENABLED)
  set (dirs ${dirs} CudaSupport)
ENDIF(CUDA_SUPPORT_ENABLED)

#-----------------------------------------------------------------------------
# Set library properties
#
set(include_dirs)
set(libs)

set(LoadableModule_USE_PYTHON ${Slicer_USE_PYTHON})

set(IGT_USE_IGSTK ${Slicer_USE_IGSTK})
set(IGT_USE_NAVITRACK ${Slicer_USE_NAVITRACK})
set(ITKCommandIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
set(MGHImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})
set(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer_INSTALL_ITKFACTORIES_DIR})

foreach(dir ${dirs})

  set(include_dirs ${include_dirs}
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
    ${CMAKE_CURRENT_BINARY_DIR}/${dir}
    )

  set(libs ${libs} ${dir})
endforeach(dir)

# vtkITK contains tests that uses MRML's test data.
set(MRML_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MRML/Testing/TestData)

slicer3_get_persistent_property(Slicer_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})
# Inconsistent library name vs. project/dir name :(

string(REPLACE ITKCommandIO CommandIO libs "${libs}")
IF(Slicer_USE_KWWIDGETS)
  string(REPLACE SlicerIO slicerio libs "${libs}")
ENDIF(Slicer_USE_KWWIDGETS)
string(REPLACE MGHImageIO MGHIO libs "${libs}")
string(REPLACE MRMLIDImageIO MRMLIDIO libs "${libs}")

# Those are not libs

list(REMOVE_ITEM libs GenerateLM slicerio)

slicer3_get_persistent_property(Slicer_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer_Libs_LIBRARIES ${tmp} ${libs})

FOREACH(dir ${dirs})
  ADD_SUBDIRECTORY(${dir})
ENDFOREACH(dir)
