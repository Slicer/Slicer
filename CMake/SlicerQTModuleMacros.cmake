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
#
#

MACRO(Slicer_build_qtmodule)
  SLICER_PARSE_ARGUMENTS(QTMODULE
    "NAME;TITLE;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES"
    ""
    ${ARGN}
    )

  # Sanity checks
  IF(NOT DEFINED QTMODULE_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF()

  IF(NOT DEFINED QTMODULE_EXPORT_DIRECTIVE)
    MESSAGE(SEND_ERROR "EXPORT_DIRECTIVE is mandatory")
  ENDIF()

  IF(NOT DEFINED QTMODULE_TITLE)
    SET(QTMODULE_TITLE ${QTMODULE_NAME})
  ENDIF()

  # Define library name
  SET(lib_name qSlicer${QTMODULE_NAME}Module)

  # Define Module title
  ADD_DEFINITIONS(-DQTMODULE_TITLE="${QTMODULE_TITLE}")

  # --------------------------------------------------------------------------
  # Find Slicer3

  IF(NOT Slicer_SOURCE_DIR)
    FIND_PACKAGE(Slicer3 REQUIRED)
    INCLUDE(${Slicer_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  ENDIF()

  # --------------------------------------------------------------------------
  # Include dirs

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_Base_INCLUDE_DIRS}
    ${QTMODULE_INCLUDE_DIRECTORIES}
    )

  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${QTMODULE_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX qSlicer${QTMODULE_NAME}Module)
  SET(MY_LIBNAME ${lib_name})

  CONFIGURE_FILE(
    ${Slicer_SOURCE_DIR}/qSlicerExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  SET(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")


  #file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h")
  #install(FILES
  #  ${files}
  #  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  #  )

  #-----------------------------------------------------------------------------
  # Sources
  #

  QT4_WRAP_CPP(QTMODULE_MOC_OUTPUT ${QTMODULE_MOC_SRCS})
  QT4_WRAP_UI(QTMODULE_UI_CXX ${QTMODULE_UI_SRCS})
  IF(DEFINED QTMODULE_RESOURCES)
    QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${QTMODULE_RESOURCES})
  ENDIF()

  QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${Slicer_SOURCE_DIR}/Resources/qSlicerLogos.qrc)
  
  SET_SOURCE_FILES_PROPERTIES(
    ${QTMODULE_UI_CXX}
    ${QTMODULE_SRCS}
    WRAP_EXCLUDE
    )

  SOURCE_GROUP("Resources" FILES
    ${QTMODULE_UI_SRCS}
    ${Slicer_SOURCE_DIR}/Resources/qSlicerLogos.qrc
    ${QTMODULE_RESOURCES}
    )

  SOURCE_GROUP("Generated" FILES
    ${QTMODULE_UI_CXX}
    ${QTMODULE_MOC_OUTPUT}
    ${QTMODULE_QRC_SRCS}
    ${dynamicHeaders}
    )
  
  # --------------------------------------------------------------------------
  # Build the library

  ADD_LIBRARY(${lib_name}
    ${QTMODULE_SRCS}
    ${QTMODULE_MOC_OUTPUT}
    ${QTMODULE_UI_CXX}
    ${QTMODULE_QRC_SRCS}
    #${qSlicerModule_TCL_SRCS}
    )

  # Set qt loadable modules output path
  SET_TARGET_PROPERTIES(${lib_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    )

  TARGET_LINK_LIBRARIES(${lib_name}
    ${Slicer_Libs_LIBRARIES}
    ${Slicer_Base_LIBRARIES}
    ${QTMODULE_TARGET_LIBRARIES}
    #${ITK_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  IF(Slicer_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
      ${Slicer_LIBRARY_PROPERTIES}
    )
  ENDIF()

  # Install rules
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT Development
    )

  # Install headers
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES
    ${headers}
    ${dynamicHeaders}
    DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${QTMODULE_NAME} COMPONENT Development
    )

ENDMACRO()
