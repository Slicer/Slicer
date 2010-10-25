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

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  )
IF(MRMLDisplayableManager_USE_PYTHON)
  LIST(APPEND include_dirs
    ${PYTHON_INCLUDE_DIR}
    )
ENDIF()

SET(packages
  MRML
  MRMLLogic
  )
  
FOREACH(package 
    ${packages}
    )
  # This is mostly to test building this lib as standalone outside Slicer3
  IF(NOT DEFINED ${package}_SOURCE_DIR)
    SET(${package}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${package}")
  ENDIF(NOT DEFINED ${package}_SOURCE_DIR)
  IF(NOT DEFINED ${package}_BINARY_DIR)
    STRING(REPLACE ${PROJECT_NAME} ${package}
      ${package}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    LINK_DIRECTORIES(${${package}_BINARY_DIR})
  ENDIF(NOT DEFINED ${package}_BINARY_DIR)

  SET(include_dirs ${include_dirs}
    ${${package}_SOURCE_DIR}
    ${${package}_BINARY_DIR}
    )
ENDFOREACH()

INCLUDE_DIRECTORIES(${include_dirs})

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLDisplayableManagerConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLDisplayableManagerConfigure.h
  )

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLDisplayableManagerConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

SET(DisplayableManager_SRCS

  # DisplayableManager associated with ThreeDView
  vtkMRMLCameraDisplayableManager.cxx
  vtkMRMLModelDisplayableManager.cxx
  vtkMRMLViewDisplayableManager.cxx

  # DisplayableManager associated with SliceView
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

# TCL Wrapping
if (VTK_WRAP_TCL)
   include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake") 
   vtk_wrap_tcl3(MRMLDisplayableManager  KIT_TCL_SRCS   "${KIT_SRCS}" "")
endif()

SET(srcs
 ${KIT_SRCS}
 ${DisplayableManagerInstantiator_SRCS}
 )

if (VTK_WRAP_TCL)
  set(srcs
    ${srcs}
    ${KIT_TCL_SRCS}
    )
endif()

# --------------------------------------------------------------------------
# Build and install the library

SET(lib_name MRMLDisplayableManager)
ADD_LIBRARY(${lib_name} 
  ${srcs}
 )

SET(libs
  MRML
  MRMLLogic
  )

IF(MRMLDisplayableManager_USE_PYTHON)
  LIST(APPEND libs
    vtkCommonPythonD # For vtkPythonUtils
    ${PYTHON_LIBRARY} # At some point, VTK should be updated to expose VTK_PYTHON_LIBRARY
    )
ENDIF()

TARGET_LINK_LIBRARIES(${lib_name}
  ${libs}
  )

SET(VTK_LIBRARIES
  vtkWidgets
  )

FOREACH(c ${VTK_LIBRARIES})
  SET(tcl_wrap)
  IF (VTK_WRAP_TCL)
    SET(tcl_wrap TCL)
  ENDIF()
  TARGET_LINK_LIBRARIES(${lib_name} ${c}${tcl_wrap})
ENDFOREACH(c)

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
  
# --------------------------------------------------------------------------
# Create Python wrappers if VTK is wrapped in Python
#

IF(MRMLDisplayableManager_USE_PYTHON AND VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

  SET(KIT_PYTHON_LIBRARIES ${VTK_LIBRARIES})

  INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
  VTK_WRAP_PYTHON3(${lib_name}Python PYTHON_SRCS "${KIT_SRCS}")
  ADD_LIBRARY(${lib_name}PythonD ${PYTHON_SRCS})
  ADD_LIBRARY(${lib_name}Python MODULE ${lib_name}PythonInit.cxx)
  
  TARGET_LINK_LIBRARIES(${lib_name}PythonD ${lib_name})
  TARGET_LINK_LIBRARIES(${lib_name}PythonD MRMLLogicPythonD)
  
  FOREACH(c ${KIT_PYTHON_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name}PythonD ${c}PythonD)
  ENDFOREACH(c)

  TARGET_LINK_LIBRARIES(${lib_name}Python ${lib_name}PythonD)

  ## Python modules on Windows must have the extension ".pyd"
  IF(WIN32 AND NOT CYGWIN)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES SUFFIX ".pyd")
  ENDIF(WIN32 AND NOT CYGWIN)

  # Apply user-defined properties to the library target.
  IF(Slicer_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES
       ${Slicer_LIBRARY_PROPERTIES}
    )
    SET_TARGET_PROPERTIES(${lib_name}PythonD PROPERTIES
       ${Slicer_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer_LIBRARY_PROPERTIES)

  INSTALL(TARGETS ${lib_name}PythonD ${lib_name}Python
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development)

  ADD_SUBDIRECTORY(Python)

ENDIF()

# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
