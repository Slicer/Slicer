PROJECT(qSlicerTractographyDisplayWidgets)



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


IF(NOT DEFINED BUILD_SHARED_LIBS)
  OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ON)
ENDIF(NOT DEFINED BUILD_SHARED_LIBS)

# CMake module
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${qSlicerTractographyDisplayWidget_SOURCE_DIR}/CMake")

#-----------------------------------------------------------------------------
# Sources
#
SET(qSlicerTractographyDisplayWidgets_SRCS
  qSlicerTractographyDisplayWidget.cxx
  qSlicerTractographyDisplayWidget.h
  qSlicerTractographyDisplayModuleWidget.cxx
  qSlicerTractographyDisplayModuleWidget.h
)

# Headers that should run through moc
SET(qSlicerTractographyDisplayWidgets_MOC_SRCS
  qSlicerTractographyDisplayWidget.h
  qSlicerTractographyDisplayModuleWidget.h
  )

# UI files
SET(qSlicerTractographyDisplayWidgets_UI_SRCS
  Resources/UI/qSlicerTractographyDisplayWidget.ui
  Resources/UI/qSlicerTractographyDisplayModule.ui
  )

SET(qSlicerTractographyDisplayWidgets_QRC_SRCS
  )

QT4_WRAP_CPP(qSlicerTractographyDisplayWidgets_MOC_CXX ${qSlicerTractographyDisplayWidgets_MOC_SRCS})
QT4_WRAP_UI(qSlicerTractographyDisplayWidgets_UI_CXX ${qSlicerTractographyDisplayWidgets_UI_SRCS})
QT4_ADD_RESOURCES(qSlicerTractographyDisplayWidgets_QRC_CXX ${qSlicerTractographyDisplayWidgets_QRC_SRCS})

SOURCE_GROUP("Resources" FILES
  ${qSlicerTractographyDisplayWidgets_UI_SRCS}
  ${qSlicerTractographyDisplayWidgets_QRC_SRCS}
  )

SOURCE_GROUP("Generated" FILES
  ${qSlicerTractographyDisplayWidgets_MOC_CXX}
  ${qSlicerTractographyDisplayWidgets_UI_CXX}
  ${qSlicerTractographyDisplayWidgets_QRC_CXX}
  )

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
  ${MRMLDisplayableManager_SOURCE_DIR}
  ${MRMLDisplayableManager_BINARY_DIR}
  ${qMRMLWidgets_SOURCE_DIR}
  ${qMRMLWidgets_BINARY_DIR}
  )

SET(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR}
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTXML_INCLUDE_DIR}
  )
SET(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${include_dirs})

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
SET(lib_name ${PROJECT_NAME})

ADD_LIBRARY(${lib_name}
  ${qSlicerTractographyDisplayWidgets_SRCS}
  ${qSlicerTractographyDisplayWidgets_MOC_CXX}
  ${qSlicerTractographyDisplayWidgets_UI_CXX}
  ${qSlicerTractographyDisplayWidgets_QRC_CXX}
  )

SET(qSlicerTractographyDisplayWidgets_LIBRARIES
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  ${CTK_EXTERNAL_LIBRARIES}
  vtkSlicerTractographyDisplayModuleLogic
  qMRMLWidgets
# Comment these out to avoid recompile every time
#  MRML
#  MRMLLogic
#  MRMLDisplayableManager
  )

TARGET_LINK_LIBRARIES(${lib_name} ${qSlicerTractographyDisplayWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()


# --------------------------------------------------------------------------
# PythonQt wrapping
# --------------------------------------------------------------------------
IF(Slicer_USE_PYTHONQT) 
  SET(qSlicerTractographyDisplayWidgets_WRAP_PYTHON ON)
ENDIF()

IF(qSlicerTractographyDisplayWidgets_WRAP_PYTHON)
  SET(KIT_PYTHONQT_SRCS) # Clear variable
  ctkMacroWrapPythonQt("org.slicer.libs" ${lib_name}
    KIT_PYTHONQT_SRCS "${qSlicerTractographyDisplayWidgets_SRCS}" FALSE)
  ADD_LIBRARY(${lib_name}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
  TARGET_LINK_LIBRARIES(${lib_name}PythonQt ${lib_name})
  IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    SET_TARGET_PROPERTIES(${lib_name}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
  ENDIF()
ENDIF()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  SET(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
ENDIF()
EXPORT(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

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

