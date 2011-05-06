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
# SlicerMacroBuildModuleLibrary
#

MACRO(SlicerMacroBuildModuleLibrary)
  SLICER_PARSE_ARGUMENTS(MODULELIBRARY
    "NAME;EXPORT_DIRECTIVE;SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES"
    "DISABLE_WRAP_PYTHON"
    ${ARGN}
    )

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------  
  SET(expected_nonempty_vars NAME EXPORT_DIRECTIVE SRCS)
  FOREACH(var ${expected_nonempty_vars})
    IF("${MODULELIBRARY_${var}}" STREQUAL "")
      MESSAGE(FATAL_ERROR "error: ${var} CMake variable is empty !")
    ENDIF()
  ENDFOREACH()
  
  # --------------------------------------------------------------------------
  # Define library name
  # --------------------------------------------------------------------------
  SET(lib_name ${MODULELIBRARY_NAME})
  
  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MODULELIBRARY_INCLUDE_DIRECTORIES}
    )

  #-----------------------------------------------------------------------------
  # Configure export header
  #-----------------------------------------------------------------------------
  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${MODULELIBRARY_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX ${MODULELIBRARY_NAME})
  SET(MY_LIBNAME ${lib_name})
  
  # Sanity checks
  IF (NOT EXISTS ${Slicer_EXPORT_HEADER_TEMPLATE})
    MESSAGE(FATAL_ERROR "error: Slicer_EXPORT_HEADER_TEMPLATE doesn't exist: ${Slicer_EXPORT_HEADER_TEMPLATE}")
  ENDIF()
  
  CONFIGURE_FILE(
    ${Slicer_EXPORT_HEADER_TEMPLATE}
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  SET(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

  #-----------------------------------------------------------------------------
  # Source groups
  # --------------------------------------------------------------------------
  SOURCE_GROUP("Generated" FILES
    ${dynamicHeaders}
    )

  # --------------------------------------------------------------------------
  # Build library
  # --------------------------------------------------------------------------
  ADD_LIBRARY(${lib_name}
    ${MODULELIBRARY_SRCS}
    )

  # Set loadable modules output path
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
    ${MODULELIBRARY_TARGET_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  IF(Slicer_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  ENDIF()
  
  # --------------------------------------------------------------------------
  # Install library
  # --------------------------------------------------------------------------
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR} COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT Development
    )

  # --------------------------------------------------------------------------
  # Install headers
  # --------------------------------------------------------------------------
  IF(DEFINED Slicer_DEVELOPMENT_INSTALL)
    IF(NOT DEFINED ${MODULELIBRARY_NAME}_DEVELOPMENT_INSTALL)
      SET(${MODULELIBRARY_NAME}_DEVELOPMENT_INSTALL ${Slicer_DEVELOPMENT_INSTALL})
    ENDIF()
  ELSE()
    IF (NOT DEFINED ${MODULELIBRARY_NAME}_DEVELOPMENT_INSTALL)
      SET(${MODULELIBRARY_NAME}_DEVELOPMENT_INSTALL OFF)
    ENDIF()
  ENDIF()
  
  IF(${MODULELIBRARY_NAME}_DEVELOPMENT_INSTALL)
    FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    INSTALL(FILES
      ${headers}
      ${dynamicHeaders}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${MODULELIBRARY_NAME} COMPONENT Development
      )
  ENDIF()
  
  # --------------------------------------------------------------------------
  # Export target
  # --------------------------------------------------------------------------
  SET_PROPERTY(GLOBAL APPEND PROPERTY Slicer_TARGETS ${MODULELIBRARY_NAME})
    
ENDMACRO()
