project(SlicerBaseGUIQT)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#-----------------------------------------------------------------------------
# Sources
#
SET(qSlicerGUIQT_SRCS
  qSlicerWidget.h
  qSlicerWidget.cxx
  qSlicerAbstractModule.cxx
  qSlicerAbstractModule.h
  qSlicerAbstractCoreModule.h
  qSlicerAbstractCoreModule.cxx
  qSlicerAbstractLoadableModule.h
  qSlicerAbstractLoadableModule.cxx
  qSlicerApplication.cxx
  qSlicerApplication.h
  qSlicerIOManager.cxx
  qSlicerIOManager.h
  qSlicerModuleFactory.cxx
  qSlicerModuleFactory.h
  qSlicerModuleManager.cxx
  qSlicerModuleManager.h
  qSlicerTransformsModule.cxx
  qSlicerTransformsModule.h
  qSlicerCameraModule.h
  qSlicerCameraModule.cxx
  )

# Headers that should run through moc
SET(qSlicerGUIQT_MOC_SRCS
  qSlicerWidget.h
  qSlicerAbstractModule.h
  qSlicerAbstractCoreModule.h
  qSlicerAbstractLoadableModule.h
  qSlicerApplication.h
  qSlicerCameraModule.h
  qSlicerTransformsModule.h
  )
  
# UI files
SET(qSlicerGUIQT_UI_SRCS
  Resources/UI/qSlicerTransformsModule.ui
  Resources/UI/qSlicerCameraModule.ui
)
  
QT4_WRAP_CPP(qSlicerGUIQT_SRCS ${qSlicerGUIQT_MOC_SRCS})
QT4_WRAP_UI(qSlicerGUIQT_UI_CXX ${qSlicerGUIQT_UI_SRCS})
QT4_ADD_RESOURCES(qSlicerGUIQT_QRC_SRCS Resources/qSlicerGUIQT.qrc)

SET_SOURCE_FILES_PROPERTIES(
  ${qSlicerGUIQT_UI_CXX}
  ${qSlicerGUIQT_SRCS}
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${qCTKWidgets_SOURCE_DIR}
  ${qCTKWidgets_BINARY_DIR}
  ${qMRMLWidgets_SOURCE_DIR}
  ${qMRMLWidgets_BINARY_DIR}
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
  ${CMAKE_CURRENT_SOURCE_DIR}/qSlicerBaseGUIQTConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qSlicerBaseGUIQTConfigure.h
  )


# --------------------------------------------------------------------------
# Build the library

set(lib_name "SlicerBaseGUIQT")

slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

add_library(${lib_name}
  ${qSlicerGUIQT_SRCS}
  ${qSlicerGUIQT_UI_CXX}
  ${qSlicerGUIQT_QRC_SRCS}
)

SET(QT_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTWEBKIT_LIBRARY} 
  )
  
SET(GUIQT_LIBRARIES
  SlicerBaseLogic
  ${QT_LIBRARIES}
  QVTK
  qCTKWidgets
  qMRMLWidgets
  )
  
target_link_libraries(${lib_name} 
  ${GUIQT_LIBRARIES}
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
  ${CMAKE_CURRENT_BINARY_DIR}/qSlicerBaseGUIQTConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

#file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
#install(FILES 
#  ${files}
#  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
#  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

#file(GLOB PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/*.png")
#foreach(pngfile ${PNGFILES})
#  configure_file(
#    ${CMAKE_CURRENT_SOURCE_DIR}/${pngfile}
#    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${pngfile}
#    COPYONLY
#    )
#endforeach(pngfile)

#install(
#  FILES ${PNGFILES}
#  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources
#  )

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  # no tests yet...
  #  add_test(Slicer3GUITest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Subdirectories
#
ADD_SUBDIRECTORY(Plugins)
