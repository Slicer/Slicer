################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# SlicerMacroBuildBaseQtLibrary
#

#
# Parameters:
#
#   NAME .................: Name of the library
#
#   EXPORT_DIRECTIVE .....: Export directive that should be used to export symbol
#
#   SRCS .................: List of source files
#
#   MOC_SRCS .............: Optional list of headers to run through the meta object compiler (moc)
#                           using QT4_WRAP_CPP CMake macro
#
#   UI_SRCS ..............: Optional list of UI file to run through UI compiler (uic) using
#                           QT4_WRAP_UI CMake macro
#
#   INCLUDE_DIRECTORIES ..: Optional list of extra folder that should be included. See implementation
#                           for the list of folder included by default.
#
#   TARGET_LIBRARIES .....: Optional list of target libraries that should be used with TARGET_LINK_LIBRARIES
#                           CMake macro. See implementation for the list of libraries added by default.
#
#   RESOURCES ............: Optional list of files that should be converted into resource header
#                           using QT4_ADD_RESOURCES
#
# Options:
#
#   WRAP_PYTHONQT ........: If specified, the sources (SRCS) will be 'PythonQt' wrapped and a static
#                           library named <NAME>PythonQt will be built.
#

MACRO(SlicerMacroBuildBaseQtLibrary)
  SLICER_PARSE_ARGUMENTS(SLICERQTBASELIB
    "NAME;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES"
    "WRAP_PYTHONQT"
    ${ARGN}
    )

  # Sanity checks
  IF(NOT DEFINED SLICERQTBASELIB_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF()

  IF(NOT DEFINED SLICERQTBASELIB_EXPORT_DIRECTIVE)
    MESSAGE(SEND_ERROR "EXPORT_DIRECTIVE is mandatory")
  ENDIF()

  # Define library name
  SET(lib_name ${SLICERQTBASELIB_NAME})

  # --------------------------------------------------------------------------
  # Include dirs

  SET(QT_INCLUDE_DIRS
    ${QT_INCLUDE_DIR}
    ${QT_QTWEBKIT_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    )

  SET(include_dirs
    ${QT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SlicerBaseLogic_SOURCE_DIR}
    ${SlicerBaseLogic_BINARY_DIR}
    ${qMRMLWidgets_SOURCE_DIR}
    ${qMRMLWidgets_BINARY_DIR}
    ${MRML_SOURCE_DIR}
    ${MRML_BINARY_DIR}
    ${SLICERQTBASELIB_INCLUDE_DIRECTORIES}
    )

  INCLUDE_DIRECTORIES(${include_dirs})

  slicer3_get_persistent_property(Slicer_Base_INCLUDE_DIRS tmp)
  slicer3_set_persistent_property(Slicer_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

  #-----------------------------------------------------------------------------
  # Configure
  #
  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${SLICERQTBASELIB_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX ${SLICERQTBASELIB_NAME})
  SET(MY_LIBNAME ${lib_name})

  CONFIGURE_FILE(
    ${Slicer_SOURCE_DIR}/qSlicerExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  SET(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

  #-----------------------------------------------------------------------------
  # Sources
  #

  QT4_WRAP_CPP(SLICERQTBASELIB_MOC_OUTPUT ${SLICERQTBASELIB_MOC_SRCS})
  QT4_WRAP_UI(SLICERQTBASELIB_UI_CXX ${SLICERQTBASELIB_UI_SRCS})
  IF(DEFINED SLICERQTBASELIB_RESOURCES)
    QT4_ADD_RESOURCES(SLICERQTBASELIB_QRC_SRCS ${SLICERQTBASELIB_RESOURCES})
  ENDIF(DEFINED SLICERQTBASELIB_RESOURCES)

  QT4_ADD_RESOURCES(SLICERQTBASELIB_QRC_SRCS ${Slicer_SOURCE_DIR}/Resources/qSlicerLogos.qrc)

  SET_SOURCE_FILES_PROPERTIES(
    ${SLICERQTBASELIB_UI_CXX}
    ${SLICERQTBASELIB_MOC_OUTPUT}
    ${SLICERQTBASELIB_QRC_SRCS}
    WRAP_EXCLUDE
    )

  SOURCE_GROUP("Resources" FILES
    ${SLICERQTBASELIB_UI_SRCS}
    ${Slicer_SOURCE_DIR}/Resources/qSlicerLogos.qrc
    ${SLICERQTBASELIB_RESOURCES}
  )

  SOURCE_GROUP("Generated" FILES
    ${SLICERQTBASELIB_UI_CXX}
    ${SLICERQTBASELIB_MOC_OUTPUT}
    ${SLICERQTBASELIB_QRC_SRCS}
    ${dynamicHeaders}
  )

  # --------------------------------------------------------------------------
  # Build the library

  slicer3_get_persistent_property(Slicer_Base_LIBRARIES tmp)
  slicer3_set_persistent_property(Slicer_Base_LIBRARIES ${tmp} ${lib_name})

  ADD_LIBRARY(${lib_name}
    ${SLICERQTBASELIB_SRCS}
    ${SLICERQTBASELIB_MOC_OUTPUT}
    ${SLICERQTBASELIB_UI_CXX}
    ${SLICERQTBASELIB_QRC_SRCS}
    )
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES LABELS ${lib_name})

  # Apply user-defined properties to the library target.
  IF(Slicer_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  ENDIF(Slicer_LIBRARY_PROPERTIES)

  SET(QT_LIBRARIES
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    )

  TARGET_LINK_LIBRARIES(${lib_name}
    ${QT_LIBRARIES}
    ${CTK_EXTERNAL_LIBRARIES}
    ${SLICERQTBASELIB_TARGET_LIBRARIES}
    )

  # Install rules
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

  # Install headers
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES
    ${headers}
    ${dynamicHeaders}
    DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
    )

  # PythonQt wrapping
  IF(Slicer_USE_PYTHONQT AND SLICERQTBASELIB_WRAP_PYTHONQT)
    SET(KIT_PYTHONQT_SRCS) # Clear variable
    ctkMacroWrapPythonQt("org.slicer.base" ${lib_name}
      KIT_PYTHONQT_SRCS "${SLICERQTBASELIB_SRCS}" FALSE)
    ADD_LIBRARY(${lib_name}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
    TARGET_LINK_LIBRARIES(${lib_name}PythonQt ${lib_name})
    IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
      SET_TARGET_PROPERTIES(${lib_name}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
    ENDIF()
  ENDIF()

ENDMACRO()
