project(EMSegmentAlgorithm)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# --------------------------------------------------------------------------
# Find Slicer3

#if(NOT Slicer3_SOURCE_DIR)
#  find_package(Slicer3 REQUIRED)
#  #include(${Slicer3_USE_FILE})
#  include(${Slicer3_CMAKE_DIR}/Slicer3Macros.cmake)
#  slicer3_set_default_install_prefix_for_external_projects()
#endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Find VTK

#if(NOT VTK_FOUND)
#  find_package(VTK REQUIRED)
#  include(${VTK_USE_FILE})
#endif()

# --------------------------------------------------------------------------
# Find ITK

#if(NOT ITK_FOUND)
#  find_package(ITK REQUIRED)
#  include(${ITK_USE_FILE})
#endif(NOT ITK_FOUND)

# --------------------------------------------------------------------------
# Include dirs

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentAlgorithmConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentAlgorithmConfigure.h
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

file(GLOB headers
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.txx"
  )

install(FILES
  ${headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentAlgorithmConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(KIT_SRCS
  EMLocalInterface.cxx
  EMLocalRegistrationCostFunction.cxx
  EMLocalShapeCostFunction.cxx
  vtkDataDef.cxx
  vtkFileOps.cxx
  vtkImageEMGeneral.cxx
  vtkImageEMGenericClass.cxx
  vtkImageEMLocalClass.cxx
  vtkImageEMLocalGenericClass.cxx
  vtkImageEMLocalSegmenter.cxx
  vtkImageEMLocalSuperClass.cxx
  vtkSimonParameterReaderWriter.cxx
  vtkTimeDef.cxx
  vtkImageMeanIntensityNormalization.cxx
  # Not used in current implementation---used in slicer2 modules
  # vtkImageEMAtlasClass.cxx
  # vtkImageEMAtlasSegmenter.cxx
  # vtkImageEMAtlasSuperClass.cxx
  # vtkImageEMMarkov.cxx
  )

set_source_files_properties(
  # Algorithm sources (temporary exclusion)
  EMLocalInterface.cxx
  EMLocalRegistrationCostFunction.cxx
  EMLocalShapeCostFunction.cxx
  vtkDataDef.cxx
  vtkFileOps.cxx
  vtkSimonParameterReaderWriter.cxx
  vtkTimeDef.cxx
  vtkImageMeanIntensityNormalization.cxx
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Wrapping
set(KIT_TCL_SRCS)
if(VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(${PROJECT_NAME}
    KIT_TCL_SRCS
    "${KIT_SRCS}" "")
endif()


# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ${PROJECT_NAME})
add_library(${lib_name}
  ${KIT_SRCS}
  ${KIT_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

set(libs
  ITKNumerics
  )

IF (VTK_WRAP_TCL)
  SET(libs
    ${libs}
    vtkImagingTCL
    vtkIOTCL
    )
ELSE (VTK_WRAP_TCL)
  SET(libs
    ${libs}
    vtkCommon
    vtkFiltering
    vtkImaging
    vtkIO
    )
ENDIF (VTK_WRAP_TCL)

target_link_libraries(${lib_name}
  ${libs}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
    ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install the testing data

#install(DIRECTORY
#  Testing/TestData
#  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#  PATTERN ".svn" EXCLUDE
#  )
