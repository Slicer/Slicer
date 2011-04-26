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


# Calling PROJECT() defines projectname_SOURCE_DIR and projectname_BINARY_DIR
PROJECT(SlicerBase)

INCLUDE(${Slicer_CMAKE_DIR}/SlicerMacroBuildBaseQtLibrary.cmake)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/QTCore
  ${CMAKE_CURRENT_BINARY_DIR}/QTCore
  )

ADD_SUBDIRECTORY(Logic)
ADD_SUBDIRECTORY(QTCore)
ADD_SUBDIRECTORY(QTGUI)
ADD_SUBDIRECTORY(QTCLI)
ADD_SUBDIRECTORY(QTCoreModules)
ADD_SUBDIRECTORY(GUI)
ADD_SUBDIRECTORY(CLI)

IF(Slicer_USE_PYTHONQT)
  ADD_SUBDIRECTORY(Python)
ENDIF(Slicer_USE_PYTHONQT)
