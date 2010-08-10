PROJECT(SlicerBaseLogic)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# --------------------------------------------------------------------------
# Include dirs

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
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerBaseLogic_SRCS
  # Logic classes (data management and calculation)
  vtkSlicerLogic.cxx 
  vtkSlicerApplicationLogic.cxx 
  vtkSlicerModuleLogic.cxx 
  vtkSlicerTask.cxx
  vtkSlicerModelsLogic.cxx 
  vtkSlicerModelHierarchyLogic.cxx
  vtkSlicerFiducialsLogic.cxx
  vtkSlicerColorLogic.cxx
  vtkDataIOManagerLogic.cxx
  # slicer's vtk extensions (filters)
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
  )

SET_SOURCE_FILES_PROPERTIES(
  vtkPichonFastMarchingPDF.cxx
  WRAP_EXCLUDE
)

# --------------------------------------------------------------------------
# TCL Wrapping
IF (VTK_WRAP_TCL)
  VTK_WRAP_TCL3(SlicerBaseLogic SlicerBaseLogic_TCL_SRCS "${SlicerBaseLogic_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
## Create an instantiator for these classes
# SET(VTK_USE_INSTANTIATOR_NEW 1)
# VTK_MAKE_INSTANTIATOR3("${lib_name}Instantiator"
#   Instantiator_SRCS
#   "${SlicerBaseLogic_SRCS}"
#   "VTK_SLICER_BASE_LOGIC_EXPORT"
#   ${Slicer3_BINARY_DIR}
#   "vtkSlicerBaseLogic.h")

# --------------------------------------------------------------------------
# Build the library

set(lib_name SlicerBaseLogic)

#SET(srcs ${SlicerBaseLogic_SRCS} ${Instantiator_SRCS})
SET(srcs ${SlicerBaseLogic_SRCS})

IF (VTK_WRAP_TCL)
  SET(srcs ${srcs} ${SlicerBaseLogic_TCL_SRCS})
ENDIF (VTK_WRAP_TCL)

ADD_LIBRARY(${lib_name} ${srcs})

SET(VTK_LIBRARIES
  vtkCommon
  vtkFiltering
  vtkGraphics
  vtkIO
  vtkHybrid
  )

SET(libs
  MRML
  MRMLCLI
  MRMLLogic
  vtkITK
  ${OPENTRACKER_LIB}
  FreeSurfer
  ${VTK_LIBRARIES}
  )
TARGET_LINK_LIBRARIES(${lib_name} ${libs})

IF(Slicer3_USE_KWWIDGETS)
  TARGET_LINK_LIBRARIES(${lib_name} ${KWWidgets_LIBRARIES})
ENDIF()

IF (VTK_WRAP_TCL)
  FOREACH(c ${VTK_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name} ${c}TCL)
  ENDFOREACH(c)
ENDIF()

slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )
  
# --------------------------------------------------------------------------
# Create Python wrappers if Slicer3_USE_PYTHONQT is ON and if VTK is wrapped in Python
#

IF(Slicer3_USE_PYTHONQT AND BUILD_SHARED_LIBS)

  SET(KIT_PYTHON_LIBRARIES ${VTK_LIBRARIES})

  INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
  VTK_WRAP_PYTHON3(${lib_name}Python PYTHON_SRCS "${SlicerBaseLogic_SRCS}")
  ADD_LIBRARY(${lib_name}PythonD ${PYTHON_SRCS})
  ADD_LIBRARY(${lib_name}Python MODULE ${lib_name}PythonInit.cxx)
  
  TARGET_LINK_LIBRARIES(${lib_name}PythonD ${lib_name})
  TARGET_LINK_LIBRARIES(${lib_name}PythonD MRMLLogicPythonD)
  
  FOREACH(c ${KIT_PYTHON_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name}PythonD ${c}PythonD)
  ENDFOREACH(c)

  TARGET_LINK_LIBRARIES(${lib_name}Python ${lib_name}PythonD)
  TARGET_LINK_LIBRARIES(${lib_name}Python MRMLLogicPythonD)

  ## Python modules on Windows must have the extension ".pyd"
  IF(WIN32 AND NOT CYGWIN)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES SUFFIX ".pyd")
  ENDIF(WIN32 AND NOT CYGWIN)

  INSTALL(TARGETS ${lib_name}PythonD ${lib_name}Python
    RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT Runtime
    LIBRARY DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development)

ENDIF(Slicer3_USE_PYTHONQT AND BUILD_SHARED_LIBS)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# Should this location change, update Base/Logic/vtkSlicerColorLogic.cxx

file(GLOB COLORFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/ColorFiles/*.txt")

foreach(colorfile ${COLORFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${colorfile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${colorfile}
    COPYONLY
    )
endforeach(colorfile)
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources/ColorFiles
  )
