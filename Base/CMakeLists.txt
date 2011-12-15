
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildBaseQtLibrary.cmake)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/QTCore
  ${CMAKE_CURRENT_BINARY_DIR}/QTCore
  )

add_subdirectory(Logic)
add_subdirectory(QTCore)
add_subdirectory(QTGUI)
if(Slicer_BUILD_CLI_SUPPORT)
  add_subdirectory(QTCLI)
endif()
add_subdirectory(QTCoreModules)
add_subdirectory(GUI)
if(Slicer_BUILD_CLI_SUPPORT)
  add_subdirectory(CLI)
endif()

if(Slicer_USE_PYTHONQT)
  add_subdirectory(Python)
endif(Slicer_USE_PYTHONQT)
