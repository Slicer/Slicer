#
#
#

MACRO(Slicer3_build_qtmodule)
  SLICER3_PARSE_ARGUMENTS(QTMODULE
    "NAME;TITLE;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES"
    ""
    ${ARGN}
    )

  # Sanity checks
  IF(NOT DEFINED QTMODULE_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF(NOT DEFINED QTMODULE_NAME)

  IF(NOT DEFINED QTMODULE_EXPORT_DIRECTIVE)
    MESSAGE(SEND_ERROR "EXPORT_DIRECTIVE is mandatory")
  ENDIF(NOT DEFINED QTMODULE_EXPORT_DIRECTIVE)

  IF(NOT DEFINED QTMODULE_TITLE)
    SET(QTMODULE_TITLE ${QTMODULE_NAME})
  ENDIF(NOT DEFINED QTMODULE_TITLE)

  # Define library name
  SET(lib_name qSlicer${QTMODULE_NAME}Module)

  # --------------------------------------------------------------------------
  # Find Slicer3

  if(NOT Slicer3_SOURCE_DIR)
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  endif(NOT Slicer3_SOURCE_DIR)

  # --------------------------------------------------------------------------
  # Include dirs

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer3_Libs_INCLUDE_DIRS}
    ${Slicer3_Base_INCLUDE_DIRS}
    ${QTMODULE_INCLUDE_DIRECTORIES}
    )

  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${QTMODULE_EXPORT_DIRECTIVE})
  SET(MY_WIN32_HEADER_PREFIX qSlicer${QTMODULE_NAME}Module)
  SET(MY_LIBNAME ${lib_name})

  CONFIGURE_FILE(
    ${QTModules_SOURCE_DIR}/qSlicerQTModulesConfigure.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_WIN32_HEADER_PREFIX}Configure.h
    )

  CONFIGURE_FILE(
    ${QTModules_SOURCE_DIR}/qSlicerQTModulesWin32Header.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_WIN32_HEADER_PREFIX}Win32Header.h
    )

  # Install headers
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES
    ${headers}
    "${CMAKE_CURRENT_BINARY_DIR}/${MY_WIN32_HEADER_PREFIX}Configure.h"
    "${CMAKE_CURRENT_BINARY_DIR}/${MY_WIN32_HEADER_PREFIX}Win32Header.h"
    DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
    )

  #file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h")
  #install(FILES
  #  ${files}
  #  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  #  )

  #-----------------------------------------------------------------------------
  # Sources
  #

  QT4_WRAP_CPP(QTMODULE_SRCS ${QTMODULE_MOC_SRCS})
  QT4_WRAP_UI(QTMODULE_UI_CXX ${QTMODULE_UI_SRCS})
  IF(DEFINED QTMODULE_RESOURCES)
    QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${QTMODULE_RESOURCES})
  ENDIF(DEFINED QTMODULE_RESOURCES)
  #ENDIF(${QTMODULE_NO_RESOURCES} EQUAL FALSE)

  SET_SOURCE_FILES_PROPERTIES(
    ${QTMODULE_UI_CXX}
    ${QTMODULE_SRCS}
    WRAP_EXCLUDE
    )

  # --------------------------------------------------------------------------
  # Wrapping

  #include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  #vtk_wrap_tcl3(Volumes
  #  Volumes_TCL_SRCS
  #  "${Volumes_SRCS}" "")

  #---------------------------------------------------------------------------
  # Add Loadable Module support

  #generatelm(Volumes_SRCS SlicerVolumes.txt)

  # --------------------------------------------------------------------------
  # Build and install the library

  ADD_LIBRARY(${lib_name}
    ${QTMODULE_SRCS}
    ${QTMODULE_UI_CXX}
    ${QTMODULE_QRC_SRCS}
    #${qSlicerModule_TCL_SRCS}
    )

  # Set qt loadable modules output path
  SET_TARGET_PROPERTIES(${lib_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_QTLOADABLEMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    )

  TARGET_LINK_LIBRARIES(${lib_name}
    ${Slicer3_Libs_LIBRARIES}
    ${Slicer3_Base_LIBRARIES}
    ${QTMODULE_TARGET_LIBRARIES}
    #${KWWidgets_LIBRARIES}
    #${ITK_LIBRARIES}
    #CommandLineModule
    #SlicerTractographyDisplay
    #SlicerTractographyFiducialSeeding
    )

  # Apply user-defined properties to the library target.
  IF(Slicer3_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
      ${Slicer3_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)

  # Install qt loadable modules
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT Development
    )

ENDMACRO(Slicer3_build_qtmodule)
