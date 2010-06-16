#
# 
#

#
# Build a qtdesigner plugin
#
MACRO(qctk_build_designer_plugin)
  QCTK_PARSE_ARGUMENTS(QCTK_DESIGNERPLUGIN
    "NAME;SRCS;MOC_SRCS;TARGET_LIBRARIES;RESOURCES"
    ""
    ${ARGN}
    )
  
  # Sanity checks
  IF(NOT DEFINED QCTK_DESIGNERPLUGIN_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF(NOT DEFINED QCTK_DESIGNERPLUGIN_NAME)
  
  # Define library name
  SET(lib_name ${QCTK_DESIGNERPLUGIN_NAME})

  # --------------------------------------------------------------------------
  # Include dirs
  #
  SET(QT_INCLUDE_DIRS
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTDESIGNER_INCLUDE_DIR}
    )
  
  SET(include_dirs
    ${QT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    )
  
  INCLUDE_DIRECTORIES(${include_dirs})
  
    
  #----------------------------------------------------------------------------
  # Sources
  #
  
  QT4_WRAP_CPP(QCTK_DESIGNERPLUGIN_SRCS ${QCTK_DESIGNERPLUGIN_MOC_SRCS})
  IF(DEFINED QCTK_DESIGNERPLUGIN_RESOURCES)
    QT4_ADD_RESOURCES(QCTK_DESIGNERPLUGIN_QRC_SRCS ${QCTK_DESIGNERPLUGIN_RESOURCES})
  ENDIF(DEFINED QCTK_DESIGNERPLUGIN_RESOURCES)
  
  
  # --------------------------------------------------------------------------
  # Build the library

  ADD_LIBRARY(${lib_name} MODULE
    ${QCTK_DESIGNERPLUGIN_SRCS}
    ${QCTK_DESIGNERPLUGIN_QRC_SRCS})
  
  # Apply properties to the library target.
  SET_TARGET_PROPERTIES(${lib_name}  PROPERTIES
                      COMPILE_FLAGS "-DQT_PLUGIN")
  
  SET(QT_LIBRARIES 
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDESIGNER_LIBRARY}
    )
  
  TARGET_LINK_LIBRARIES(${lib_name}
    ${QT_LIBRARIES}
    ${QCTK_DESIGNERPLUGIN_TARGET_LIBRARIES}
    )

ENDMACRO(qctk_build_designer_plugin)

#
# Install qtdesigner plugin
#
MACRO(qctk_install_designer_plugin)
  QCTK_PARSE_ARGUMENTS(QCTK_INSTALLDESIGNERPLUGIN
    "NAME;INSTALL_BIN_DIR;INSTALL_LIB_DIR;INSTALL_HEADERS_DIR"
    ""
    ${ARGN}
    )

  #----------------------------------------------------------------------------
  # Sanity checks
  #
  IF(NOT DEFINED QCTK_INSTALLDESIGNERPLUGIN_INSTALL_BIN_DIR)
    MESSAGE(SEND_ERROR "INSTALL_BIN_DIR is mandatory")
  ENDIF(NOT DEFINED QCTK_INSTALLDESIGNERPLUGIN_INSTALL_BIN_DIR)
  
  IF(NOT DEFINED QCTK_INSTALLDESIGNERPLUGIN_INSTALL_LIB_DIR)
    MESSAGE(SEND_ERROR "INSTALL_LIB_DIR is mandatory")
  ENDIF(NOT DEFINED QCTK_INSTALLDESIGNERPLUGIN_INSTALL_LIB_DIR)

  IF(NOT DEFINED QCTK_INSTALLDESIGNERPLUGIN_INSTALL_HEADERS_DIR)
    MESSAGE(SEND_ERROR "INSTALL_HEADERS_DIR is mandatory")
  ENDIF(NOT DEFINED QCTK_INSTALLDESIGNERPLUGIN_INSTALL_HEADERS_DIR)

  # --------------------------------------------------------------------------
  # Install the library

  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES 
    ${headers}
    DESTINATION ${QCTK_INSTALLDESIGNERPLUGIN_INSTALL_HEADERS_DIR}/${PROJECT_NAME} COMPONENT Development
    )
  
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${QCTK_INSTALLDESIGNERPLUGIN_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${QCTK_INSTALLDESIGNERPLUGIN_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${QCTK_INSTALLDESIGNERPLUGIN_INSTALL_LIB_DIR} COMPONENT Development
  )
  
  # Since QtDesigner expects plugin to be directly located under the
  # directory 'designer', let's copy them. 

  IF(NOT CMAKE_CFG_INTDIR STREQUAL ".")
    GET_TARGET_PROPERTY(FILE_PATH ${lib_name} LOCATION)
    GET_TARGET_PROPERTY(DIR_PATH ${lib_name} LIBRARY_OUTPUT_DIRECTORY)
  
    ADD_CUSTOM_COMMAND(
      TARGET ${lib_name}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${FILE_PATH} ${DIR_PATH}/../designer/${CMAKE_SHARED_LIBRARY_PREFIX}${lib_name}${CMAKE_BUILD_TYPE}${CMAKE_SHARED_LIBRARY_SUFFIX}
      )
  ENDIF()
  
ENDMACRO(qctk_install_designer_plugin)
