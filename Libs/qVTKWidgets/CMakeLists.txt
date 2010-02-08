PROJECT(qVTKWidgets)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


IF(NOT DEFINED BUILD_SHARED_LIBS)
  OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ON)
ENDIF(NOT DEFINED BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# Sources
#
SET(qVTKWidgets_SRCS
  qVTKAbstractMatrixWidget.cxx
  qVTKAbstractMatrixWidget.h
  qVTKAbstractMatrixWidget_p.h
  qVTKCommandOptions.cxx
  qVTKCommandOptions.h
  qVTKConnection.cxx
  qVTKConnection.h
  qVTKMatrixWidget.cxx
  qVTKMatrixWidget.h
  qVTKObject.h
  qVTKObjectEventsObserver.cxx
  qVTKObjectEventsObserver.h
  qVTKRenderView.cxx
  qVTKRenderView.h
  )

# Headers that should run through moc
SET(qVTKWidgets_MOC_SRCS
  qVTKAbstractMatrixWidget_p.h
  qVTKConnection.h
  qVTKObjectEventsObserver.h
  qVTKMatrixWidget.h
  qVTKRenderView.h
  qVTKRenderView_p.h
  )
  
# UI files
SET(qVTKWidgets_UI_SRCS
)
  
QT4_WRAP_CPP(qVTKWidgets_SRCS ${qVTKWidgets_MOC_SRCS})
QT4_WRAP_UI(qVTKWidgets_UI_CXX ${qVTKWidgets_UI_SRCS})
QT4_ADD_RESOURCES(qVTKWidgets_QRC_SRCS Resources/qVTKWidgets.qrc)
   


# --------------------------------------------------------------------------
# Include dirs
#

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

SET(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR} 
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR} 
  ${QT_QTCORE_INCLUDE_DIR} 
  )

FOREACH(package 
    qCTKWidgets
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
ENDFOREACH(package)

SET(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${include_dirs})

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers} 
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the library
#

SET(lib_name "qVTKWidgets")

ADD_LIBRARY(${lib_name} 
  ${qVTKWidgets_SRCS}
  ${qVTKWidgets_UI_CXX}
  ${qVTKWidgets_QRC_SRCS}
)

SET(qVTKWidgets_LIBRARIES 
  ${QT_LIBRARIES}
  MRML
  qCTKWidgets
  QVTK
  )
  
TARGET_LINK_LIBRARIES(${lib_name} ${qVTKWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)


# --------------------------------------------------------------------------
# Install
#

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
#
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Sub directories
#
ADD_SUBDIRECTORY(Plugins)
