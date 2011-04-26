PROJECT(SlicerBaseLogic)

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
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBase_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRMLCLI_SOURCE_DIR}
  ${MRMLCLI_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  ${RemoteIO_SOURCE_DIR}
  ${RemoteIO_BINARY_DIR}
  ${LibArchive_INCLUDE_DIR}
  )

IF(Slicer_USE_PYTHONQT)
  LIST(APPEND include_dirs
    ${PYTHON_INCLUDE_DIR}
    )
ENDIF()

include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Update Slicer_Base_INCLUDE_DIRS
# --------------------------------------------------------------------------
SET(Slicer_Base_INCLUDE_DIRS ${Slicer_Base_INCLUDE_DIRS} ${include_dirs} CACHE INTERNAL "Slicer Base includes" FORCE)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkSlicerBaseLogicConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in 
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(SlicerBaseLogic_SRCS
  # Logic classes (data management and calculation)
  vtkSlicerApplicationLogic.cxx
  vtkSlicerModuleLogic.cxx 
  vtkSlicerTask.cxx
  vtkSlicerModelsLogic.cxx 
  vtkSlicerModelHierarchyLogic.cxx
  vtkSlicerFiducialsLogic.cxx
  vtkSlicerColorLogic.cxx
  vtkDataIOManagerLogic.cxx
  # slicer's vtk extensions (filters)
  vtkSlicerExtensionsLogic.cxx
  vtkArchive.cxx
  vtkImageSlicePaint.cxx
  vtkImageFillROI.cxx
  vtkSlicerGlyphSource2D.cxx
  vtkSlicerROILogic.cxx
  vtkImageConnectivity.cxx 
  vtkImageLabelChange.cxx 
  vtkImageErode.cxx
  vtkSlicerTransformLogic.cxx
  vtkPichonFastMarching.cxx
  vtkPichonFastMarchingPDF.cxx
  vtkImageRectangularSource.cxx
  vtkSystemInformation.cxx
  vtkImageStash.cxx
  )

IF(Slicer_USE_PYTHONQT)
  LIST(APPEND SlicerBaseLogic_SRCS
    vtkSlicerScriptedLoadableModuleLogic.cxx
    )
ENDIF()

SET_SOURCE_FILES_PROPERTIES(
  vtkPichonFastMarchingPDF.cxx
  vtkArchive.cxx
  WRAP_EXCLUDE
)

# --------------------------------------------------------------------------
## Create an instantiator for these classes
# SET(VTK_USE_INSTANTIATOR_NEW 1)
# VTK_MAKE_INSTANTIATOR3("${lib_name}Instantiator"
#   Instantiator_SRCS
#   "${SlicerBaseLogic_SRCS}"
#   "VTK_SLICER_BASE_LOGIC_EXPORT"
#   ${Slicer_BINARY_DIR}
#   "vtkSlicerBaseLogic.h")

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name SlicerBaseLogic)

#SET(srcs ${SlicerBaseLogic_SRCS} ${Instantiator_SRCS})
SET(srcs ${SlicerBaseLogic_SRCS})

ADD_LIBRARY(${lib_name} ${srcs})

SET(libs
  MRML
  MRMLCLI
  MRMLLogic
  vtkITK
  ${OPENTRACKER_LIB}
  FreeSurfer
  ${VTK_LIBRARIES}
  RemoteIO
  ${LibArchive_LIBRARY}
  )

IF(Slicer_USE_PYTHONQT)
  LIST(APPEND libs
    vtkCommonPythonD # For vtkPythonUtils
    ${PYTHON_LIBRARY} # At some point, VTK should be updated to expose VTK_PYTHON_LIBRARY
    )
ENDIF()

TARGET_LINK_LIBRARIES(${lib_name} ${libs})
SET_TARGET_PROPERTIES(${lib_name} PROPERTIES LABELS ${lib_name})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

# --------------------------------------------------------------------------
# Update Slicer_Base_LIBRARIES
# --------------------------------------------------------------------------
SET(Slicer_Base_LIBRARIES ${Slicer_Base_LIBRARIES} ${lib_name} CACHE INTERNAL "Slicer Base libraries" FORCE)

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )
  
# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
IF(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${SlicerBaseLogic_SRCS}
    KIT_PYTHON_LIBRARIES MRMLLogicPythonD
    KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
    )
ENDIF()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------

# Should this location change, update Base/Logic/vtkSlicerColorLogic.cxx

file(GLOB COLORFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/ColorFiles/*.txt")

foreach(colorfile ${COLORFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${colorfile}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${colorfile}
    COPYONLY
    )
endforeach()
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources/ColorFiles
  )
