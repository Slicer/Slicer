#
# Slicer3_build_module_logic
#

MACRO(Slicer3_build_module_logic)
  SLICER3_PARSE_ARGUMENTS(MODULELOGIC
    "NAME;EXPORT_DIRECTIVE;SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES"
    ""
    ${ARGN}
    )

  # Sanity checks
  IF(NOT DEFINED MODULELOGIC_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF(NOT DEFINED MODULELOGIC_NAME)

  IF(NOT DEFINED MODULELOGIC_EXPORT_DIRECTIVE)
    MESSAGE(SEND_ERROR "EXPORT_DIRECTIVE is mandatory")
  ENDIF(NOT DEFINED MODULELOGIC_EXPORT_DIRECTIVE)

  # Define library name
  SET(lib_name ${MODULELOGIC_NAME})
  
  # --------------------------------------------------------------------------
  # Find Slicer3

  IF(NOT Slicer3_SOURCE_DIR)
    FIND_PACKAGE(Slicer3 REQUIRED)
    INCLUDE(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  ENDIF(NOT Slicer3_SOURCE_DIR)

  # --------------------------------------------------------------------------
  # Include dirs

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer3_Libs_INCLUDE_DIRS}
    ${Slicer3_Base_INCLUDE_DIRS}
    ${MODULELOGIC_INCLUDE_DIRECTORIES}
    )

  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${MODULELOGIC_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX ${MODULELOGIC_NAME})
  SET(MY_LIBNAME ${lib_name})

  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/qSlicerExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  SET(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")


  # --------------------------------------------------------------------------
  # TCL Wrapping
  
  SET(MODULELOGIC_TCL_SRCS "")
  IF (VTK_WRAP_TCL)
    INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
    vtk_wrap_tcl3(${lib_name} 
                  MODULELOGIC_TCL_SRCS
                  "${MODULELOGIC_SRCS}" "")
  ENDIF(VTK_WRAP_TCL)

  #-----------------------------------------------------------------------------
  # Source group(s)
  
  SOURCE_GROUP("Generated" FILES
    ${MODULELOGIC_TCL_SRCS}
    ${dynamicHeaders}
    )

  # --------------------------------------------------------------------------
  # Build the library

  ADD_LIBRARY(${lib_name}
    ${MODULELOGIC_SRCS}
    ${MODULELOGIC_TCL_SRCS}
    )

  # Set loadable modules output path
  SET_TARGET_PROPERTIES(${lib_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}"
    )

  # HACK Since we don't depend on qSlicerBaseQT{Base, Core, CLI, CoreModules, GUI},
  # let's remove them from the list
  SET(Slicer3_ModuleLogic_Base_LIBRARIES ${Slicer3_Base_LIBRARIES})
  LIST(REMOVE_ITEM Slicer3_ModuleLogic_Base_LIBRARIES qSlicerBaseQTBase)
  LIST(REMOVE_ITEM Slicer3_ModuleLogic_Base_LIBRARIES qSlicerBaseQTCore)
  LIST(REMOVE_ITEM Slicer3_ModuleLogic_Base_LIBRARIES qSlicerBaseQTCLI)
  LIST(REMOVE_ITEM Slicer3_ModuleLogic_Base_LIBRARIES qSlicerBaseQTCoreModules)
  LIST(REMOVE_ITEM Slicer3_ModuleLogic_Base_LIBRARIES qSlicerBaseQTGUI)
  # Let's also remove dependency on SlicerBaseGUI
  LIST(REMOVE_ITEM Slicer3_ModuleLogic_Base_LIBRARIES SlicerBaseGUI)

  TARGET_LINK_LIBRARIES(${lib_name}
    ${Slicer3_Libs_LIBRARIES}
    ${Slicer3_ModuleLogic_Base_LIBRARIES}
    ${MODULELOGIC_TARGET_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  IF(Slicer3_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
      ${Slicer3_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)
  
  # Install rules
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
    )

  # Install headers
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES
    ${headers}
    ${dynamicHeaders}
    DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${MODULELOGIC_NAME} COMPONENT Development
    )
    
ENDMACRO(Slicer3_build_module_logic)
