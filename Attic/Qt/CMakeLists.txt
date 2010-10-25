PROJECT(SlicerBaseGUIQt)

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

slicer3_get_persistent_property(Slicer_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseGUIQtConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseGUIQtConfigure.h
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseGUIQtConfigure.h
  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
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
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)


slicer3_get_persistent_property(Slicer_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer_Base_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  # no tests yet...
  #  add_test(Slicer3GUITest1 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
endif(BUILD_TESTING)

file(GLOB PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/*.png")
foreach(pngfile ${PNGFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${pngfile}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${pngfile}
    COPYONLY
    )
endforeach(pngfile)

install(
  FILES ${PNGFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources
  )
