#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

include(${Slicer_CMAKE_DIR}/SlicerMacroBuildBaseQtLibrary.cmake)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/QTCore
  ${CMAKE_CURRENT_BINARY_DIR}/QTCore
  )

add_subdirectory(Logic)
add_subdirectory(QTCore)
add_subdirectory(QTGUI)
add_subdirectory(QTCLI)
add_subdirectory(QTCoreModules)
add_subdirectory(GUI)
add_subdirectory(CLI)

if(Slicer_USE_PYTHONQT)
  add_subdirectory(Python)
endif(Slicer_USE_PYTHONQT)
