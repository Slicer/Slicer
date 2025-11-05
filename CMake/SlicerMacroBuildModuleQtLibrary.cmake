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
# SlicerMacroBuildModuleQtLibrary
#

macro(SlicerMacroBuildModuleQtLibrary)
  set(options
    WRAP_PYTHONQT
    NO_INSTALL
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE
    FOLDER
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    RESOURCES
    )
  cmake_parse_arguments(MODULEQTLIBRARY
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(MODULEQTLIBRARY_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SlicerMacroBuildModuleQtLibrary(): \"${MODULEQTLIBRARY_UNPARSED_ARGUMENTS}\"")
  endif()

  set(expected_defined_vars NAME EXPORT_DIRECTIVE)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED MODULEQTLIBRARY_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  # --------------------------------------------------------------------------
  # Define library name
  # --------------------------------------------------------------------------
  set(lib_name ${MODULEQTLIBRARY_NAME})

  # --------------------------------------------------------------------------
  # Set <MODULEQTLIBRARY_NAME>_INCLUDE_DIRS
  # --------------------------------------------------------------------------
  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

  set(_include_dirs
    ${${MODULEQTLIBRARY_NAME}_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    # Ensure generated AUTOUIC headers (ui_*.h) are discoverable.
    #
    # By default CMake writes them to:
    #
    #   - Single-config generators (Ninja/Makefiles):
    #       <AUTOGEN_BUILD_DIR>/include
    #
    #   - Multi-config generators (VS, Xcode, Ninja Multi-Config):
    #       <AUTOGEN_BUILD_DIR>/include_<CONFIG>
    #
    # where AUTOGEN_BUILD_DIR defaults to:
    #   <target-binary-dir>/<target-name>_autogen
    #
    # References:
    # - https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html#autouic
    # - https://cmake.org/cmake/help/latest/prop_tgt/AUTOGEN_BUILD_DIR.html
    ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}_autogen/include$<$<BOOL:${_isMultiConfig}>:_$<CONFIG>>
    )
  # Since module developer may have already set the variable to some
  # specific values in the module CMakeLists.txt, we make sure to
  # consider the already set variable and remove duplicates.
  list(REMOVE_DUPLICATES _include_dirs)
  set(${MODULEQTLIBRARY_NAME}_INCLUDE_DIRS
    ${_include_dirs}
    CACHE INTERNAL "${MODULEQTLIBRARY_NAME} include directories" FORCE)

  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------
  include_directories(
    ${${MODULEQTLIBRARY_NAME}_INCLUDE_DIRS}
    ${MODULEQTLIBRARY_INCLUDE_DIRECTORIES}
    )

  #-----------------------------------------------------------------------------
  # Configure export header
  #-----------------------------------------------------------------------------
  set(MY_LIBRARY_EXPORT_DIRECTIVE ${MODULEQTLIBRARY_EXPORT_DIRECTIVE})
  set(MY_EXPORT_HEADER_PREFIX ${MODULEQTLIBRARY_NAME})
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
  if(NOT EXISTS ${Slicer_LOGOS_RESOURCE})
    message("Warning, Slicer_LOGOS_RESOURCE doesn't exist: ${Slicer_LOGOS_RESOURCE}")
  endif()

  # --------------------------------------------------------------------------
  # Source groups
  # --------------------------------------------------------------------------
  source_group("Resources" FILES
    ${MODULEQTLIBRARY_UI_SRCS}
    ${Slicer_LOGOS_RESOURCE}
    ${MODULEQTLIBRARY_RESOURCES}
    )

  source_group("Generated" FILES
    ${dynamicHeaders}
    )

  # --------------------------------------------------------------------------
  # Build library
  #-----------------------------------------------------------------------------
  add_library(${lib_name}
    ${MODULEQTLIBRARY_SRCS}
    ${MODULEQTLIBRARY_RESOURCES}
    ${Slicer_LOGOS_RESOURCE}
    )

  target_compile_definitions(${lib_name} PRIVATE
    $<$<BOOL:${Qt5_VERSION_MAJOR}>:Slicer_HAVE_QT5>
    )

  # Configure CMake Qt automatic code generation
  set(uic_search_paths)
  foreach(ui_src IN LISTS MODULEQTLIBRARY_UI_SRCS)
    if(NOT IS_ABSOLUTE ${ui_src})
      set(ui_src "${CMAKE_CURRENT_SOURCE_DIR}/${ui_src}")
    endif()
    get_filename_component(ui_path ${ui_src} PATH)
    list(APPEND uic_search_paths ${ui_path})
  endforeach()
  list(REMOVE_DUPLICATES uic_search_paths)

  set_target_properties(${lib_name} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS "${uic_search_paths}"
    )

  # Set qt loadable modules output path
  set_target_properties(${lib_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
    )
  set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})

  if(NOT "${MODULEQTLIBRARY_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name} PROPERTIES FOLDER ${MODULEQTLIBRARY_FOLDER})
  endif()

  target_link_libraries(${lib_name}
    ${MODULEQTLIBRARY_TARGET_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  if(Slicer_LIBRARY_PROPERTIES)
    set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  endif()

  # --------------------------------------------------------------------------
  # Install library
  # --------------------------------------------------------------------------
  if(NOT MODULEQTLIBRARY_NO_INSTALL)
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
    if(NOT DEFINED ${MODULEQTLIBRARY_NAME}_DEVELOPMENT_INSTALL)
      set(${MODULEQTLIBRARY_NAME}_DEVELOPMENT_INSTALL ${Slicer_DEVELOPMENT_INSTALL})
    endif()
  else()
    if(NOT DEFINED ${MODULEQTLIBRARY_NAME}_DEVELOPMENT_INSTALL)
      set(${MODULEQTLIBRARY_NAME}_DEVELOPMENT_INSTALL OFF)
    endif()
  endif()

  if(NOT MODULEQTLIBRARY_NO_INSTALL AND ${MODULEQTLIBRARY_NAME}_DEVELOPMENT_INSTALL)
    # Install headers
    file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    install(FILES
      ${headers}
      ${dynamicHeaders}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${MODULEQTLIBRARY_NAME} COMPONENT Development
      )
  endif()

  # --------------------------------------------------------------------------
  # Export target
  # --------------------------------------------------------------------------
  set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${MODULEQTLIBRARY_NAME})

  # --------------------------------------------------------------------------
  # PythonQt wrapping
  # --------------------------------------------------------------------------
  if(Slicer_USE_PYTHONQT AND MODULEQTLIBRARY_WRAP_PYTHONQT)
    if(MODULEQTLIBRARY_NO_INSTALL)
      set(MODULEQTLIBRARY_NO_INSTALL_OPTION "NO_INSTALL")
    endif()
    ctkMacroBuildLibWrapper(
      NAMESPACE "osm" # Use "osm" instead of "org.slicer.module" to avoid build error on windows
      TARGET ${lib_name}
      SRCS "${MODULEQTLIBRARY_SRCS}"
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
      INSTALL_BIN_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}
      INSTALL_LIB_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}
      ${MODULEQTLIBRARY_NO_INSTALL_OPTION}
      )
    if(NOT "${MODULEQTLIBRARY_FOLDER}" STREQUAL "")
      set_target_properties(${lib_name}PythonQt PROPERTIES FOLDER ${MODULEQTLIBRARY_FOLDER})
    endif()
  endif()

endmacro()
