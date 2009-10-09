project(SlicerBaseGUI2)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#-----------------------------------------------------------------------------
# Sources
#
SET(qSlicerGUI2_SRCS
  qSlicerAbstractModule.h
  qSlicerAbstractModule.cxx
  qSlicerModuleTransform.h
  qSlicerModuleTransform.cxx
  qSlicerApplication.h
  qSlicerApplication.cxx
  qSlicerIOManager.h
  qSlicerIOManager.cxx
  #qSlicerKwUIWrapper.h
  #qSlicerKwUIWrapper.cxx
  )

# Headers that should run through moc
SET(qSlicerGUI2_MOC_SRCS
  qSlicerAbstractModule.h
  qSlicerModuleTransform.h
  qSlicerApplication.h
  #qSlicerKwUIWrapper.h
  )
  
# UI files
SET(qSlicerGUI2_UI_SRCS
  Resources/UI/qSlicerModuleTransform.ui
)
  
QT4_WRAP_CPP(qSlicerGUI2_SRCS ${qSlicerGUI2_MOC_SRCS})
QT4_WRAP_UI(qSlicerGUI2_UI_CXX ${qSlicerGUI2_UI_SRCS})
QT4_ADD_RESOURCES(qSlicerGUI2_QRC_SRCS Resources/qSlicerGUI2.qrc)

SET_SOURCE_FILES_PROPERTIES(
  ${qSlicerGUI2_UI_CXX}
  ${qSlicerGUI2_SRCS}
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${QCTKWidgets_SOURCE_DIR}
  ${QCTKWidgets_BINARY_DIR}
  ${QMRMLWidgets_SOURCE_DIR}
  ${QMRMLWidgets_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

set(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR} 
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR} 
  ${QT_QTCORE_INCLUDE_DIR} 
  )

set(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/qSlicerBaseGUI2Configure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qSlicerBaseGUI2Configure.h
  )


# --------------------------------------------------------------------------
# Build the library

set(lib_name "SlicerBaseGUI2")

slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

add_library(${lib_name} 
  ${qSlicerGUI2_SRCS}
  ${qSlicerGUI2_UI_CXX}
  ${qSlicerGUI2_QRC_SRCS}
)

SET(QT_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTWEBKIT_LIBRARY} 
  )
  
SET(GUI2_LIBRARIES
  SlicerBaseLogic
  ${QT_LIBRARIES}
  QVTK
  QCTKWidgets
  QMRMLWidgets
  )
  
target_link_libraries(${lib_name} 
  ${GUI2_LIBRARIES}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)


# --------------------------------------------------------------------------
# Install the library

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/qSlicerBaseGUI2Configure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

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

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  # no tests yet...
  #  add_test(Slicer3GUITest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
endif(BUILD_TESTING)

