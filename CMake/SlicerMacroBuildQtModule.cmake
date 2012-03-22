################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
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
include(SlicerMacroParseArguments)

macro(slicerMacroBuildQtModule)
  SLICER_PARSE_ARGUMENTS(QTMODULE
    "NAME;TITLE;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES"
    "NO_INSTALL;NO_TITLE"
    ${ARGN}
    )

  set(MY_EXPORT_HEADER_PREFIX qSlicer${QTMODULE_NAME}Module)
  message(STATUS "Configuring Qt loadable module: ${QTMODULE_NAME} [${MY_EXPORT_HEADER_PREFIX}Export.h]")

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  set(expected_defined_vars NAME EXPORT_DIRECTIVE)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED QTMODULE_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  if(NOT DEFINED QTMODULE_TITLE)
    set(QTMODULE_TITLE ${QTMODULE_NAME})
  endif()

  # --------------------------------------------------------------------------
  # Define library name
  # --------------------------------------------------------------------------
  set(lib_name qSlicer${QTMODULE_NAME}Module)

  # --------------------------------------------------------------------------
  # Define Module title
  # --------------------------------------------------------------------------
  if(NOT QTMODULE_NO_TITLE)
    add_definitions(-DQTMODULE_TITLE="${QTMODULE_TITLE}")
  endif()

  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_Base_INCLUDE_DIRS}
    ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    ${QTMODULE_INCLUDE_DIRECTORIES}
    )

  #-----------------------------------------------------------------------------
  # Configure export header
  #-----------------------------------------------------------------------------
  set(MY_LIBRARY_EXPORT_DIRECTIVE ${QTMODULE_EXPORT_DIRECTIVE})
  set(MY_LIBNAME ${lib_name})

  # Sanity checks
  if(NOT EXISTS ${Slicer_EXPORT_HEADER_TEMPLATE})
    message(FATAL_ERROR "error: Slicer_EXPORT_HEADER_TEMPLATE doesn't exist: ${Slicer_EXPORT_HEADER_TEMPLATE}")
  endif()

  configure_file(
    ${Slicer_EXPORT_HEADER_TEMPLATE}
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  set(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

  #-----------------------------------------------------------------------------
  # Sources
  #-----------------------------------------------------------------------------
  set(QTMODULE_MOC_OUTPUT)
  QT4_WRAP_CPP(QTMODULE_MOC_OUTPUT ${QTMODULE_MOC_SRCS})
  set(QTMODULE_UI_CXX)
  QT4_WRAP_UI(QTMODULE_UI_CXX ${QTMODULE_UI_SRCS})
  set(QTMODULE_QRC_SRCS)
  if(DEFINED QTMODULE_RESOURCES)
    QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${QTMODULE_RESOURCES})
  endif()

  if(NOT EXISTS ${Slicer_LOGOS_RESOURCE})
    message("Warning, Slicer_LOGOS_RESOURCE doesn't exist: ${Slicer_LOGOS_RESOURCE}")
  endif()
  QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${Slicer_LOGOS_RESOURCE})

  set_source_files_properties(
    ${QTMODULE_SRCS} # For now, let's prevent the module widget from being wrapped
    ${QTMODULE_UI_CXX}
    ${QTMODULE_MOC_OUTPUT}
    ${QTMODULE_QRC_SRCS}
    WRAP_EXCLUDE
    )

  # --------------------------------------------------------------------------
  # Source groups
  # --------------------------------------------------------------------------
  source_group("Resources" FILES
    ${QTMODULE_UI_SRCS}
    ${Slicer_LOGOS_RESOURCE}
    ${QTMODULE_RESOURCES}
    )

  source_group("Generated" FILES
    ${QTMODULE_UI_CXX}
    ${QTMODULE_MOC_OUTPUT}
    ${QTMODULE_QRC_SRCS}
    ${dynamicHeaders}
    )

  # --------------------------------------------------------------------------
  # Build library
  #-----------------------------------------------------------------------------
  add_library(${lib_name}
    ${QTMODULE_SRCS}
    ${QTMODULE_MOC_OUTPUT}
    ${QTMODULE_UI_CXX}
    ${QTMODULE_QRC_SRCS}
    )

  # Set loadable modules output path
  set_target_properties(${lib_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
    )
  set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})

  target_link_libraries(${lib_name}
    ${Slicer_GUI_LIBRARY}
    ${QTMODULE_TARGET_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  if(Slicer_LIBRARY_PROPERTIES)
    set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  endif()

  # --------------------------------------------------------------------------
  # Install library
  # --------------------------------------------------------------------------
  if(NOT QTMODULE_NO_INSTALL)
    install(TARGETS ${lib_name}
      RUNTIME DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT Development
      )
  endif()

  # --------------------------------------------------------------------------
  # Install headers
  # --------------------------------------------------------------------------
  if(DEFINED Slicer_DEVELOPMENT_INSTALL)
    if(NOT DEFINED ${QTMODULE_NAME}_DEVELOPMENT_INSTALL)
      set(${QTMODULE_NAME}_DEVELOPMENT_INSTALL ${Slicer_DEVELOPMENT_INSTALL})
    endif()
  else()
    if(NOT DEFINED ${QTMODULE_NAME}_DEVELOPMENT_INSTALL)
      set(${QTMODULE_NAME}_DEVELOPMENT_INSTALL OFF)
    endif()
  endif()

  if(NOT QTMODULE_NO_INSTALL AND ${QTMODULE_NAME}_DEVELOPMENT_INSTALL)
    # Install headers
    file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    install(FILES
      ${headers}
      ${dynamicHeaders}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${QTMODULE_NAME} COMPONENT Development
      )
  endif()

endmacro()
