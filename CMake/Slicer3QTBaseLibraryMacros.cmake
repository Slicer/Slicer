#
#
#

MACRO(Slicer3_build_slicer_qtbase_library)
  SLICER3_PARSE_ARGUMENTS(SLICERQTBASELIB
    "NAME;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES"
    ""
    ${ARGN}
    )

  # Sanity checks
  IF(NOT DEFINED SLICERQTBASELIB_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF(NOT DEFINED SLICERQTBASELIB_NAME)

  IF(NOT DEFINED SLICERQTBASELIB_EXPORT_DIRECTIVE)
    MESSAGE(SEND_ERROR "EXPORT_DIRECTIVE is mandatory")
  ENDIF(NOT DEFINED SLICERQTBASELIB_EXPORT_DIRECTIVE)

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

  slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
  slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

  #-----------------------------------------------------------------------------
  # Configure
  #
  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${SLICERQTBASELIB_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX ${SLICERQTBASELIB_NAME})
  SET(MY_LIBNAME ${lib_name})

  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/qSlicerExport.h.in
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

  QT4_ADD_RESOURCES(SLICERQTBASELIB_QRC_SRCS ${Slicer3_SOURCE_DIR}/Resources/qSlicerLogos.qrc)

  SET_SOURCE_FILES_PROPERTIES(
    ${SLICERQTBASELIB_UI_CXX}
    ${SLICERQTBASELIB_SRCS}
    WRAP_EXCLUDE
    )

  SOURCE_GROUP("Resources" FILES
    ${SLICERQTBASELIB_UI_SRCS}
    ${Slicer3_SOURCE_DIR}/Resources/qSlicerLogos.qrc
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

  slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
  slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

  ADD_LIBRARY(${lib_name}
    ${SLICERQTBASELIB_SRCS}
    ${SLICERQTBASELIB_MOC_OUTPUT}
    ${SLICERQTBASELIB_UI_CXX}
    ${SLICERQTBASELIB_QRC_SRCS}
    )

  # Apply user-defined properties to the library target.
  IF(Slicer3_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer3_LIBRARY_PROPERTIES})
  ENDIF(Slicer3_LIBRARY_PROPERTIES)

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
    RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

  # Install headers
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES
    ${headers}
    ${dynamicHeaders}
    DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
    )

ENDMACRO(Slicer3_build_slicer_qtbase_library)
