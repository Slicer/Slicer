PROJECT(MRMLDisplayableManager)

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

SET(MRMLDisplayableManager_USE_PYTHON ${Slicer_USE_PYTHONQT})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  )
IF(MRMLDisplayableManager_USE_PYTHON)
  LIST(APPEND include_dirs ${PYTHON_INCLUDE_DIR})
ENDIF()
INCLUDE_DIRECTORIES(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkMRMLDisplayableManagerConfigure.h)
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
SET(DisplayableManager_SRCS

  # DisplayableManager associated with ThreeDView
  vtkMRMLCameraDisplayableManager.cxx
  vtkMRMLModelDisplayableManager.cxx
  vtkMRMLViewDisplayableManager.cxx

  # DisplayableManager associated with SliceView
  vtkMRMLSliceModelDisplayableManager.cxx
  )

SET(KIT_SRCS
  ${DisplayableManager_SRCS}
  vtkMRMLAbstractDisplayableManager.cxx
  vtkMRMLDisplayableManagerGroup.cxx
  vtkMRMLDisplayableManagerFactory.cxx

  # ThreeDView factory and DisplayableManager
  vtkMRMLAbstractThreeDViewDisplayableManager.cxx
  vtkMRMLThreeDViewDisplayableManagerFactory.cxx

  # SliceView factory and DisplayableManager
  vtkMRMLAbstractSliceViewDisplayableManager.cxx
  vtkMRMLSliceViewDisplayableManagerFactory.cxx
  
  vtkSliceViewInteractorStyle.cxx
  vtkThreeDViewInteractorStyle.cxx
  )
SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLDisplayableManagerGroup.cxx
  vtkMRMLDisplayableManagerFactory.cxx
  vtkMRMLThreeDViewDisplayableManagerFactory.cxx
  vtkMRMLSliceViewDisplayableManagerFactory.cxx
  WRAP_EXCLUDE
)

IF(MRMLDisplayableManager_USE_PYTHON)
  LIST(APPEND KIT_SRCS
    vtkMRMLScriptedDisplayableManager.cxx
    )
  SET_SOURCE_FILES_PROPERTIES(
    vtkMRMLScriptedDisplayableManager.cxx
    WRAP_EXCLUDE
    )
ENDIF()

# Create the instantiator for DisplayableManager classes.
SET(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${PROJECT_NAME}Instantiator"
  DisplayableManagerInstantiator_SRCS
  "${DisplayableManager_SRCS}"
  "VTK_MRML_DISPLAYABLEMANAGER_EXPORT"
  ${PROJECT_BINARY_DIR}
  "vtk${PROJECT_NAME}Win32Header.h")

SET(srcs
  ${KIT_SRCS}
  ${DisplayableManagerInstantiator_SRCS}
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
SET(lib_name ${PROJECT_NAME})
ADD_LIBRARY(${lib_name} ${srcs})

SET(libs
  ${VTK_LIBRARIES}
  MRML
  MRMLLogic
  )

IF(MRMLDisplayableManager_USE_PYTHON)
  LIST(APPEND libs
    vtkCommonPythonD # For vtkPythonUtils
    ${PYTHON_LIBRARY} # At some point, VTK should be updated to expose VTK_PYTHON_LIBRARY
    )
ENDIF()

TARGET_LINK_LIBRARIES(${lib_name} ${libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )
  
# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
IF(MRMLDisplayableManager_USE_PYTHON AND VTK_WRAP_PYTHON) 
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${KIT_SRCS}
    KIT_PYTHON_LIBRARIES MRMLLogicPythonD
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
  ADD_SUBDIRECTORY(Python)
ENDIF()

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()
