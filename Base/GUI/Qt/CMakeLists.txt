PROJECT(SlicerBaseGUIQt)

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
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )


set(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR} 
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR} 
  ${QT_QTCORE_INCLUDE_DIR} 
  )
QT4_WRAP_CPP(MOC_SRCS 
   QtSlicerNodeSelectorWidget.h QtSlider.h QtSlicerWebKit.h qvtkfactory.h vtkview.h
   slicerview.h qslicerfactory.h
   )
SET(SRCS ${SRCS} ${MOC_SRCS} 
         QtSlicerNodeSelectorWidget.cxx QtSlicerNodeSelectorWidget.h 
         QtSlicerWebKit.cxx QtSlicerWebKit.h 
         vtkview.cxx vtkview.h
         qvtkfactory.cxx qvtkfactory.h
         slicerview.cxx slicerview.h
         qslicerfactory.cxx qslicerfactory.h
         QtSlider.cxx QtSlider.h)
SET(QT_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTWEBKIT_LIBRARY} QVTK
  )

set(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseGUIQtConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseGUIQtConfigure.h
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseGUIQtConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  QtSlicerNodeSelectorWidget.cxx 
  moc_QtSlicerNodeSelectorWidget.cxx
  QtSlicerNodeSelectorWidget.h 
  QtSlicerWebKit.cxx 
  moc_QtSlicerWebKit.cxx
  QtSlicerWebKit.h 
  QtSlider.cxx
  moc_QtSlider.cxx
  QtSlider.h
  vtkview.cxx vtkview.h
  qvtkfactory.cxx qvtkfactory.h
  WRAP_EXCLUDE
  )




# --------------------------------------------------------------------------
# Build and install the library

set(lib_name "SlicerBaseGUIQt")

add_library(${lib_name} 
  ${SRCS}
)
target_link_libraries(${lib_name} 
  ${QT_LIBRARIES}
  SlicerBaseLogic
  ${KWWidgets_LIBRARIES}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)


slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  # no tests yet...
  #  add_test(Slicer3GUITest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
endif(BUILD_TESTING)

file(GLOB PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/*.png")
foreach(pngfile ${PNGFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${pngfile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${pngfile}
    COPYONLY
    )
endforeach(pngfile)

install(
  FILES ${PNGFILES}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources
  )
