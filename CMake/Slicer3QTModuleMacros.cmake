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

  # Define Module title
  ADD_DEFINITIONS(-DQTMODULE_TITLE="${QTMODULE_TITLE}")

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
    ${QTMODULE_INCLUDE_DIRECTORIES}
    )

  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${QTMODULE_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX qSlicer${QTMODULE_NAME}Module)
  SET(MY_LIBNAME ${lib_name})

  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/qSlicerExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  SET(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")


  #file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h")
  #install(FILES
  #  ${files}
  #  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  #  )

  #-----------------------------------------------------------------------------
  # Sources
  #

  QT4_WRAP_CPP(QTMODULE_MOC_OUTPUT ${QTMODULE_MOC_SRCS})
  QT4_WRAP_UI(QTMODULE_UI_CXX ${QTMODULE_UI_SRCS})
  IF(DEFINED QTMODULE_RESOURCES)
    QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${QTMODULE_RESOURCES})
  ENDIF(DEFINED QTMODULE_RESOURCES)

  QT4_ADD_RESOURCES(QTMODULE_QRC_SRCS ${Slicer3_SOURCE_DIR}/Resources/qSlicerLogos.qrc)
  
  SET_SOURCE_FILES_PROPERTIES(
    ${QTMODULE_UI_CXX}
    ${QTMODULE_SRCS}
    WRAP_EXCLUDE
    )

  SOURCE_GROUP("Resources" FILES
    ${QTMODULE_UI_SRCS}
    ${Slicer3_SOURCE_DIR}/Resources/qSlicerLogos.qrc
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
    #${ITK_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  IF(Slicer3_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
      ${Slicer3_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)

  # Install rules
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT Development
    )

  # Install headers
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES
    ${headers}
    ${dynamicHeaders}
    DESTINATION ${Slicer3_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${QTMODULE_NAME} COMPONENT Development
    )

ENDMACRO(Slicer3_build_qtmodule)
