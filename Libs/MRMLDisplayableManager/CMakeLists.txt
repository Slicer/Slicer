PROJECT(MRMLDisplayableManager)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# --------------------------------------------------------------------------
# Include dirs

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  )

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
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
  
# --------------------------------------------------------------------------
# Create Python wrappers if VTK is wrapped in Python
#

IF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

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
  IF(Slicer3_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES
       ${Slicer3_LIBRARY_PROPERTIES}
    )
    SET_TARGET_PROPERTIES(${lib_name}PythonD PROPERTIES
       ${Slicer3_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)

  INSTALL(TARGETS ${lib_name}PythonD ${lib_name}Python
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development)

ENDIF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
