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

macro(slicerMacroBuildLoadableModule)
  set(options
    NO_INSTALL
    NO_TITLE
    WITH_GENERIC_TESTS
    )
  set(oneValueArgs
    NAME
    TITLE
    EXPORT_DIRECTIVE
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    RESOURCES
    )
  cmake_parse_arguments(LOADABLEMODULE
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(LOADABLEMODULE_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerMacroBuildLoadableModule(): \"${LOADABLEMODULE_UNPARSED_ARGUMENTS}\"")
  endif()

  set(MY_EXPORT_HEADER_PREFIX qSlicer${LOADABLEMODULE_NAME}Module)
  message(STATUS "Configuring Loadable module: ${LOADABLEMODULE_NAME} [${MY_EXPORT_HEADER_PREFIX}Export.h]")

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  set(expected_defined_vars NAME EXPORT_DIRECTIVE)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED LOADABLEMODULE_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  if(NOT DEFINED LOADABLEMODULE_TITLE)
    set(LOADABLEMODULE_TITLE ${LOADABLEMODULE_NAME})
  endif()

  # --------------------------------------------------------------------------
  # Define library name
  # --------------------------------------------------------------------------
  set(lib_name qSlicer${LOADABLEMODULE_NAME}Module)

  # --------------------------------------------------------------------------
  # Define Module title
  # --------------------------------------------------------------------------
  if(NOT LOADABLEMODULE_NO_TITLE)
    add_definitions(
      -DMODULE_TITLE="${LOADABLEMODULE_TITLE}"
      -DQTMODULE_TITLE="${LOADABLEMODULE_TITLE}" # For backward compatibility
      )
  endif()

  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------

  if(NOT DEFINED ${lib_name}_SOURCE_DIR)
    set(${lib_name}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
  endif()

  if(NOT DEFINED ${lib_name}_BINARY_DIR)
    set(${lib_name}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" FORCE)
  endif()

  set(${lib_name}_INCLUDE_DIRS ${${lib_name}_SOURCE_DIR} ${${lib_name}_BINARY_DIR} CACHE INTERNAL "" FORCE)

  include_directories(
    ${${lib_name}_INCLUDE_DIRS}
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_Base_INCLUDE_DIRS}
    ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    ${LOADABLEMODULE_INCLUDE_DIRECTORIES}
    )

  #-----------------------------------------------------------------------------
  # Configure export header
  #-----------------------------------------------------------------------------
  set(MY_LIBRARY_EXPORT_DIRECTIVE ${LOADABLEMODULE_EXPORT_DIRECTIVE})
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
  set(LOADABLEMODULE_MOC_OUTPUT)
  set(LOADABLEMODULE_UI_CXX)
  set(LOADABLEMODULE_QRC_SRCS)
  if(NOT EXISTS ${Slicer_LOGOS_RESOURCE})
    message("Warning, Slicer_LOGOS_RESOURCE doesn't exist: ${Slicer_LOGOS_RESOURCE}")
  endif()

    set(_moc_options OPTIONS -DSlicer_HAVE_QT5)
    QT5_WRAP_CPP(LOADABLEMODULE_MOC_OUTPUT ${LOADABLEMODULE_MOC_SRCS} ${_moc_options})
    QT5_WRAP_UI(LOADABLEMODULE_UI_CXX ${LOADABLEMODULE_UI_SRCS})
    if(DEFINED LOADABLEMODULE_RESOURCES)
      QT5_ADD_RESOURCES(LOADABLEMODULE_QRC_SRCS ${LOADABLEMODULE_RESOURCES})
    endif()
    QT5_ADD_RESOURCES(LOADABLEMODULE_QRC_SRCS ${Slicer_LOGOS_RESOURCE})

  set_source_files_properties(
    ${LOADABLEMODULE_SRCS} # For now, let's prevent the module widget from being wrapped
    ${LOADABLEMODULE_UI_CXX}
    ${LOADABLEMODULE_MOC_OUTPUT}
    ${LOADABLEMODULE_QRC_SRCS}
    WRAP_EXCLUDE
    )

  # --------------------------------------------------------------------------
  # Source groups
  # --------------------------------------------------------------------------
  source_group("Resources" FILES
    ${LOADABLEMODULE_UI_SRCS}
    ${Slicer_LOGOS_RESOURCE}
    ${LOADABLEMODULE_RESOURCES}
    )

  source_group("Generated" FILES
    ${LOADABLEMODULE_UI_CXX}
    ${LOADABLEMODULE_MOC_OUTPUT}
    ${LOADABLEMODULE_QRC_SRCS}
    ${dynamicHeaders}
    )

  # --------------------------------------------------------------------------
  # Translation
  # --------------------------------------------------------------------------
  if(Slicer_BUILD_I18N_SUPPORT)
    set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Translations/")
    get_property(Slicer_LANGUAGES GLOBAL PROPERTY Slicer_LANGUAGES)

    include(SlicerMacroTranslation)
    SlicerMacroTranslation(
      SRCS ${LOADABLEMODULE_SRCS}
      UI_SRCS ${LOADABLEMODULE_UI_SRCS}
      TS_DIR ${TS_DIR}
      TS_BASEFILENAME ${LOADABLEMODULE_NAME}
      TS_LANGUAGES ${Slicer_LANGUAGES}
      QM_OUTPUT_DIR_VAR QM_OUTPUT_DIR
      QM_OUTPUT_FILES_VAR QM_OUTPUT_FILES
      )
    set_property(GLOBAL APPEND PROPERTY Slicer_QM_OUTPUT_DIRS ${QM_OUTPUT_DIR})

  else()
    set(QM_OUTPUT_FILES )
  endif()

  # --------------------------------------------------------------------------
  # Build library
  # --------------------------------------------------------------------------
  add_library(${lib_name}
    ${LOADABLEMODULE_SRCS}
    ${LOADABLEMODULE_MOC_OUTPUT}
    ${LOADABLEMODULE_UI_CXX}
    ${LOADABLEMODULE_QRC_SRCS}
    ${QM_OUTPUT_FILES}
    )

  # Set loadable modules output path
  set_target_properties(${lib_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
    )
  set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})

  target_link_libraries(${lib_name}
    # The two PUBLIC keywords are not a duplication, they allow developers to 
    # include PRIVATE/INTERFACE keywords in their library list
    PUBLIC
      ${LOADABLEMODULE_TARGET_LIBRARIES}
    PUBLIC
      ${Slicer_GUI_LIBRARY}
    )

  # Apply user-defined properties to the library target.
  if(Slicer_LIBRARY_PROPERTIES)
    set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  endif()

  # Folder
  if(NOT DEFINED LOADABLEMODULE_FOLDER AND DEFINED MODULE_NAME)
    set(LOADABLEMODULE_FOLDER "Module-${MODULE_NAME}")
  endif()
  if(NOT "${LOADABLEMODULE_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name} PROPERTIES FOLDER ${LOADABLEMODULE_FOLDER})
  endif()

  set_property(GLOBAL APPEND PROPERTY SLICER_MODULE_TARGETS ${lib_name})

  # --------------------------------------------------------------------------
  # Install library
  # --------------------------------------------------------------------------
  if(NOT LOADABLEMODULE_NO_INSTALL)
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
    if(NOT DEFINED ${LOADABLEMODULE_NAME}_DEVELOPMENT_INSTALL)
      set(${LOADABLEMODULE_NAME}_DEVELOPMENT_INSTALL ${Slicer_DEVELOPMENT_INSTALL})
    endif()
  else()
    if(NOT DEFINED ${LOADABLEMODULE_NAME}_DEVELOPMENT_INSTALL)
      set(${LOADABLEMODULE_NAME}_DEVELOPMENT_INSTALL OFF)
    endif()
  endif()

  if(NOT LOADABLEMODULE_NO_INSTALL AND ${LOADABLEMODULE_NAME}_DEVELOPMENT_INSTALL)
    # Install headers
    file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    install(FILES
      ${headers}
      ${dynamicHeaders}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${LOADABLEMODULE_NAME} COMPONENT Development
      )
  endif()

  if(BUILD_TESTING AND LOADABLEMODULE_WITH_GENERIC_TESTS)

    set(KIT ${lib_name})

    set(KIT_GENERIC_TEST_SRCS)
    set(KIT_GENERIC_TEST_NAMES)
    set(KIT_GENERIC_TEST_NAMES_CXX)
    SlicerMacroConfigureGenericCxxModuleTests(${LOADABLEMODULE_NAME}
      KIT_GENERIC_TEST_SRCS
      KIT_GENERIC_TEST_NAMES
      KIT_GENERIC_TEST_NAMES_CXX
      )

    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
    create_test_sourcelist(Tests ${KIT}CxxTests.cxx
      ${KIT_GENERIC_TEST_NAMES_CXX}
      EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
      )

    ctk_add_executable_utf8(${KIT}GenericCxxTests ${Tests})
    set_target_properties(${KIT}GenericCxxTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR})
    target_link_libraries(${KIT}GenericCxxTests ${KIT})
    if(NOT "${LOADABLEMODULE_FOLDER}" STREQUAL "")
      set_target_properties(${KIT}GenericCxxTests PROPERTIES FOLDER ${LOADABLEMODULE_FOLDER})
    endif()

    foreach(testname ${KIT_GENERIC_TEST_NAMES})
      add_test(NAME ${testname}
               COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}GenericCxxTests> ${testname})
      set_property(TEST ${testname} PROPERTY LABELS ${KIT})
    endforeach()
  endif()

endmacro()


#
# Deprecated macro - Will be removed after 4.8
#
macro(slicerMacroBuildQtModule)
  set(options
    NO_INSTALL
    NO_TITLE
    WITH_GENERIC_TESTS
    )
  set(oneValueArgs
    NAME
    TITLE
    EXPORT_DIRECTIVE
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    RESOURCES
    )
  cmake_parse_arguments(QTMODULE
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # See http://www.na-mic.org/Bug/view.php?id=3332
  #message(AUTHOR_WARNING
  #  "Macro 'slicerMacroBuildQtModule' is deprecated: Use 'slicerMacroBuildLoadableModule' instead."
  #  )

  set(QTMODULE_ARGS)
  foreach(arg ${oneValueArgs} ${multiValueArgs})
    list(APPEND QTMODULE_ARGS ${arg} ${QTMODULE_${arg}})
  endforeach()

  set(QTMODULE_OPTIONS)
  foreach(option ${options})
    if(${QTMODULE_${option}})
      list(APPEND QTMODULE_OPTIONS ${option})
    endif()
  endforeach()

  slicerMacroBuildLoadableModule(${QTMODULE_ARGS} ${QTMODULE_OPTIONS})
endmacro()
