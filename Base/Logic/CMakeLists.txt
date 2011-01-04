PROJECT(SlicerBaseLogic)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

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

slicer3_get_persistent_property(Slicer_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

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
  vtkSlicerSceneViewLogic.cxx
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
#   ${Slicer_BINARY_DIR}
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

IF(Slicer_USE_KWWIDGETS)
  TARGET_LINK_LIBRARIES(${lib_name} ${KWWidgets_LIBRARIES})
ENDIF()

IF (VTK_WRAP_TCL)
  FOREACH(c ${VTK_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name} ${c}TCL)
  ENDFOREACH(c)
ENDIF()

slicer3_get_persistent_property(Slicer_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer_Base_LIBRARIES ${tmp} ${lib_name})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )
  
# --------------------------------------------------------------------------
# Create Python wrappers if VTK is wrapped in Python
#

IF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

  SET(KIT_PYTHON_LIBRARIES ${VTK_LIBRARIES})

  INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
  VTK_WRAP_PYTHON3(${lib_name}Python PYTHON_SRCS "${SlicerBaseLogic_SRCS}")
  ADD_LIBRARY(${lib_name}PythonD ${PYTHON_SRCS})
  SET_TARGET_PROPERTIES(${lib_name}PythonD PROPERTIES LABELS ${lib_name})
  ADD_LIBRARY(${lib_name}Python MODULE ${lib_name}PythonInit.cxx)
  SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES LABELS ${lib_name})
  
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
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development)

ENDIF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

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
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${colorfile}
    COPYONLY
    )
endforeach(colorfile)
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources/ColorFiles
  )
