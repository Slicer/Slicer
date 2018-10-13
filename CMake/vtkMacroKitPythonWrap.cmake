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

# Based on VTK/CMake/KitCommonWrapBlock.cmake

# Add <dep> as a dependency of <module_name> and recurse on <dep>'s dependencies.
# Appends dependencies and their include directories to lists:
#   - _<module_name>_wrap_depends
#   - _<module_name>_wrap_include_dirs
# Ignores VTK dependencies.
macro(_get_dependencies_recurse module_name dep)
  string(REGEX REPLACE "(.+)PythonD\$" "\\1" _dep_base ${dep})
  if(${_dep_base}_WRAP_HIERARCHY_FILE)
    list(APPEND _${module_name}_wrap_depends ${_dep_base})
  endif()

  set(_wrap_include_dirs ${${_dep_base}_INCLUDE_DIRS})
  if(_wrap_include_dirs)
    list(APPEND _${module_name}_wrap_include_dirs ${_wrap_include_dirs})
  endif()

  list(FIND ${_dep_base}_WRAP_DEPENDS "${_dep_base}" _index)
  if(NOT _index EQUAL -1)
    message(FATAL_ERROR "${_dep_base} can NOT depends on itself [${_dep_base}_WRAP_DEPENDS: ${${_dep_base}_WRAP_DEPENDS}]")
  endif()

  foreach(_dep ${${_dep_base}_WRAP_DEPENDS})
    list(FIND VTK_LIBRARIES "${_dep}" _index)
    if(_index EQUAL -1)
      _get_dependencies_recurse(${module_name} "${_dep}")
    endif()
  endforeach()
endmacro()

#!
#! vtkMacroKitPythonWrap(
#!
#!     # Single value arguments
#!     KIT_NAME <name>
#!     KIT_INSTALL_BIN_DIR <dir>
#!     KIT_INSTALL_LIB_DIR <dir>
#!     [KIT_MODULE_INSTALL_BIN_DIR <dir>]
#!     [KIT_MODULE_INSTALL_LIB_DIR <dir>]
#!
#!     # Multi-value arguments
#!     KIT_SRCS <src> [<src> [...]]
#!     [MY_KIT_PYTHON_EXTRA_SRCS <src> [<src> ...]]
#!     [KIT_WRAP_HEADERS <header> [<header ...]]
#!     [KIT_PYTHON_LIBRARIES <lib> [<lib> ...]]
#!
#!   )
#!
#! Variables that Change Behavior:
#!
#!   Slicer_VTK_WRAP_HIERARCHY_DIR:
#!
#!     Directory where to output the hierarchy files
#!     Default is ${Slicer_BINARY_DIR}
#!
#!   Slicer_VTK_WRAP_MODULE_INSTALL_COMPONENT_IDENTIFIER:
#!
#!     Install component associated with "Python" module. This does not include
#!     the "PythonD" libraries.
#!     Default is "RuntimeLibraries" which is the same as the component associated
#!     with the "PythonD" lilbraries.
#!
#!   Slicer_VTK_WRAP_HIERARCHY_TARGETS_PROPERTY_NAME:
#!
#!     Name of the global property associated with the list of all wrapped library
#!     names.
#!     Default is "SLICER_WRAP_HIERARCHY_TARGETS"
#!
macro(vtkMacroKitPythonWrap)
  set(options)
  set(oneValueArgs KIT_NAME KIT_INSTALL_BIN_DIR KIT_INSTALL_LIB_DIR KIT_MODULE_INSTALL_BIN_DIR KIT_MODULE_INSTALL_LIB_DIR)
  set(multiValueArgs KIT_SRCS KIT_PYTHON_EXTRA_SRCS KIT_WRAP_HEADERS KIT_PYTHON_LIBRARIES)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  set(expected_defined_vars
    VTK_CMAKE_DIR VTK_WRAP_PYTHON BUILD_SHARED_LIBS VTK_LIBRARIES)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "error: ${var} CMake variable is not defined !")
    endif()
  endforeach()

  set(expected_nonempty_vars KIT_NAME KIT_INSTALL_BIN_DIR KIT_INSTALL_LIB_DIR)
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  # Default arguments
  if("${MY_KIT_MODULE_INSTALL_BIN_DIR}" STREQUAL "")
    set(MY_KIT_MODULE_INSTALL_BIN_DIR ${MY_KIT_INSTALL_BIN_DIR})
  endif()
  if("${MY_KIT_MODULE_INSTALL_LIB_DIR}" STREQUAL "")
    set(MY_KIT_MODULE_INSTALL_LIB_DIR ${MY_KIT_INSTALL_LIB_DIR})
  endif()
  # Default global variables
  if(NOT DEFINED Slicer_VTK_WRAP_HIERARCHY_DIR)
    set(Slicer_VTK_WRAP_HIERARCHY_DIR ${Slicer_BINARY_DIR})
  endif()
  if(NOT DEFINED Slicer_VTK_WRAP_MODULE_INSTALL_COMPONENT_IDENTIFIER)
    set(Slicer_VTK_WRAP_MODULE_INSTALL_COMPONENT_IDENTIFIER "RuntimeLibraries")
  endif()
  if(NOT DEFINED Slicer_VTK_WRAP_HIERARCHY_TARGETS_PROPERTY_NAME)
    set(Slicer_VTK_WRAP_HIERARCHY_TARGETS_PROPERTY_NAME "SLICER_WRAP_HIERARCHY_TARGETS")
  endif()

  if(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

    # Tell vtkWrapPython.cmake to set VTK_PYTHON_LIBRARIES for us.
    set(VTK_WRAP_PYTHON_FIND_LIBS 1)
    include(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)

    set(TMP_WRAP_FILES ${MY_KIT_SRCS} ${MY_KIT_WRAP_HEADERS})
    set(_wrap_hierarchy_stamp_file)

    # Create list of wrapping dependencies for generating the hierarchy file.
    set(_kit_wrap_depends)
    set(_kit_wrap_include_dirs ${VTK_INCLUDE_DIRS})

    # Add kit include dirs
    list(APPEND _kit_wrap_include_dirs ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    if(DEFINED Slicer_Base_INCLUDE_DIRS)
      list(APPEND _kit_wrap_include_dirs ${Slicer_Base_INCLUDE_DIRS})
    endif()
    set(_kit_include_dirs ${${MY_KIT_NAME}_INCLUDE_DIRS})
    if(_kit_include_dirs)
      list(APPEND _kit_wrap_include_dirs ${_kit_include_dirs})
    endif()

    # Add VTK dependencies
    foreach(_dep ${VTK_LIBRARIES})
      list(APPEND _kit_wrap_depends ${_dep})
    endforeach()

    # Recursively add dependencies and get their include directories
    foreach(_dep ${MY_KIT_PYTHON_LIBRARIES})
      set(_${MY_KIT_NAME}_wrap_depends)
      set(_${MY_KIT_NAME}_wrap_include_dirs)
      _get_dependencies_recurse("${MY_KIT_NAME}" "${_dep}")
      list(APPEND _kit_wrap_depends ${_${MY_KIT_NAME}_wrap_depends})
      list(APPEND _kit_wrap_include_dirs ${_${MY_KIT_NAME}_wrap_include_dirs})
    endforeach()

    if(_kit_wrap_depends)
      list(REMOVE_DUPLICATES _kit_wrap_depends)
    endif()
    if(_kit_wrap_include_dirs)
      list(REMOVE_DUPLICATES _kit_wrap_include_dirs)
    endif()

    # Update list of include directories for wrapper tool command lines
    list(APPEND VTK_WRAP_INCLUDE_DIRS ${_kit_wrap_include_dirs})

    # Generate hierarchy files for VTK8 and later
    if(NOT ${VTK_VERSION_MAJOR} VERSION_LESS 8)
      include(${VTK_CMAKE_DIR}/vtkWrapHierarchy.cmake)

      # Set variables for this and future runs of vtk_wrap_hierarchy:
      #  - <module_name>_WRAP_DEPENDS
      #  - <module_name>_WRAP_HIERARCHY_FILE
      set(${MY_KIT_NAME}_WRAP_DEPENDS "${_kit_wrap_depends}" CACHE INTERNAL "${MY_KIT_NAME} wrapping dependencies" FORCE)
      set(_wrap_hierarchy_file "${Slicer_VTK_WRAP_HIERARCHY_DIR}/${MY_KIT_NAME}Hierarchy.txt")
      if(${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR} VERSION_LESS "8.2")
        set(_wrap_hierarchy_stamp_file ${CMAKE_CURRENT_BINARY_DIR}/${MY_KIT_NAME}Hierarchy.stamp.txt)
      endif()
      set(${MY_KIT_NAME}_WRAP_HIERARCHY_FILE "${_wrap_hierarchy_file}" CACHE INTERNAL "${MY_KIT_NAME} wrap hierarchy file" FORCE)

      set_property(GLOBAL APPEND PROPERTY ${Slicer_VTK_WRAP_HIERARCHY_TARGETS_PROPERTY_NAME} ${MY_KIT_NAME})

      # Set variables for vtk_wrap_python3:
      #   - KIT_HIERARCHY_FILE
      set(KIT_HIERARCHY_FILE "${_wrap_hierarchy_file}")

      # Generate hierarchy files
      vtk_wrap_hierarchy(${MY_KIT_NAME} ${Slicer_VTK_WRAP_HIERARCHY_DIR} "${TMP_WRAP_FILES}")
    endif()

    VTK_WRAP_PYTHON3(${MY_KIT_NAME}Python KitPython_SRCS "${TMP_WRAP_FILES}")

    include_directories("${PYTHON_INCLUDE_PATH}")

    # Create a python module that can be loaded dynamically.  It links to
    # the shared library containing the wrappers for this kit.
    add_library(${MY_KIT_NAME}PythonD ${KitPython_SRCS} ${MY_KIT_PYTHON_EXTRA_SRCS})

    # Include the hierarchy stamp file in the main kit library to ensure
    # hierarchy file is created.
    # XXX Use target_sources if cmake_minimum_required >= 3.1
    get_target_property(_kit_srcs ${MY_KIT_NAME} SOURCES)
    if(${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR} VERSION_LESS "8.2")
      list(APPEND _kit_srcs ${_wrap_hierarchy_stamp_file})
    else()
      list(APPEND _kit_srcs ${_wrap_hierarchy_file})
    endif()
    set_target_properties(${MY_KIT_NAME} PROPERTIES SOURCES "${_kit_srcs}")

    set(VTK_KIT_PYTHON_LIBRARIES)
    # XXX Hard-coded list of VTK kits available when building
    #     with VTK_ENABLE_KITS set to 1
    set(vtk_kits
      vtkCommonKit
      vtkFiltersKit
      vtkImagingKit
      vtkRenderingKit
      vtkIOKit
      vtkOpenGLKit
      vtkInteractionKit
      vtkViewsKit
      vtkParallelKit
      vtkWrappingKit
      )
    foreach(c ${VTK_LIBRARIES} ${vtk_kits})
      if(${c} MATCHES "^vtk.+" AND TARGET ${c}PythonD) # exclude system libraries
        list(APPEND VTK_KIT_PYTHON_LIBRARIES ${c}PythonD)
      endif()
    endforeach()
    set(VTK_PYTHON_CORE vtkWrappingPythonCore)
    target_link_libraries(
      ${MY_KIT_NAME}PythonD
      ${MY_KIT_NAME}
      ${VTK_PYTHON_CORE}
      ${VTK_PYTHON_LIBRARIES}
      ${VTK_KIT_PYTHON_LIBRARIES}
      ${MY_KIT_PYTHON_LIBRARIES}
      )

    install(TARGETS ${MY_KIT_NAME}PythonD
      RUNTIME DESTINATION ${MY_KIT_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${MY_KIT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION ${MY_KIT_INSTALL_LIB_DIR} COMPONENT Development
      )

    # Add a top-level dependency on the main kit library.  This is needed
    # to make sure no python source files are generated until the
    # hierarchy file is built (it is built when the kit library builds)
    add_dependencies(${MY_KIT_NAME}PythonD ${MY_KIT_NAME})

    # Add dependencies that may have been generated by VTK_WRAP_PYTHON3 to
    # the python wrapper library.  This is needed for the
    # pre-custom-command hack in Visual Studio 6.
    if(KIT_PYTHON_DEPS)
      add_dependencies(${MY_KIT_NAME}PythonD ${KIT_PYTHON_DEPS})
    endif()

    # Create a python module that can be loaded dynamically.  It links to
    # the shared library containing the wrappers for this kit.
    add_library(${MY_KIT_NAME}Python MODULE ${MY_KIT_NAME}PythonInit.cxx)
    target_link_libraries(${MY_KIT_NAME}Python
      ${MY_KIT_NAME}PythonD
      )

    # Python extension modules on Windows must have the extension ".pyd"
    # instead of ".dll" as of Python 2.5.  Older python versions do support
    # this suffix.
    if(WIN32 AND NOT CYGWIN)
      set_target_properties(${MY_KIT_NAME}Python PROPERTIES SUFFIX ".pyd")
    endif()

    # Make sure that no prefix is set on the library
    set_target_properties(${MY_KIT_NAME}Python PROPERTIES PREFIX "")

    install(TARGETS ${MY_KIT_NAME}Python
      RUNTIME DESTINATION ${MY_KIT_MODULE_INSTALL_BIN_DIR} COMPONENT ${Slicer_VTK_WRAP_MODULE_INSTALL_COMPONENT_IDENTIFIER}
      LIBRARY DESTINATION ${MY_KIT_MODULE_INSTALL_LIB_DIR} COMPONENT ${Slicer_VTK_WRAP_MODULE_INSTALL_COMPONENT_IDENTIFIER}
      ARCHIVE DESTINATION ${MY_KIT_MODULE_INSTALL_LIB_DIR} COMPONENT Development
      )
  endif()

endmacro()
