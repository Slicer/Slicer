#
# 
#

#
# Build a qtdesigner plugin
#
MACRO(qctk_build_designer_plugin)
  QCTK_PARSE_ARGUMENTS(MY
    "NAME;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_FORMS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES;LIBRARY_TYPE"
    ""
    ${ARGN}
    )

  # Sanity checks
  IF(NOT DEFINED MY_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF()
  IF(NOT DEFINED MY_LIBRARY_TYPE)
    SET(MY_LIBRARY_TYPE "SHARED")
  ENDIF()

  # Define library name
  SET(lib_name ${MY_NAME})

  # --------------------------------------------------------------------------
  # Include dirs
  SET(my_includes
    #${CTK_BASE_INCLUDE_DIRS}
    ${QT_QTDESIGNER_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MY_INCLUDE_DIRECTORIES}
    )
  INCLUDE_DIRECTORIES(
    ${my_includes}
    )

  SET(MY_LIBNAME ${lib_name})

  # Make sure variable are cleared
  SET(MY_UI_CXX)
  SET(MY_QRC_SRCS)

  # Wrap
  QT4_WRAP_CPP(MY_MOC_CXX ${MY_MOC_SRCS})
  QT4_WRAP_UI(MY_UI_CXX ${MY_UI_FORMS})
  SET(MY_QRC_SRCS "")
  IF(DEFINED MY_RESOURCES)
    QT4_ADD_RESOURCES(MY_QRC_SRCS ${MY_RESOURCES})
  ENDIF()

  SOURCE_GROUP("Resources" FILES
    ${MY_RESOURCES}
    ${MY_UI_FORMS}
    )

  SOURCE_GROUP("Generated" FILES
    ${MY_MOC_CXX}
    ${MY_QRC_SRCS}
    ${MY_UI_CXX}
    )

  ADD_LIBRARY(${lib_name} ${MY_LIBRARY_TYPE}
    ${MY_SRCS}
    ${MY_MOC_CXX}
    ${MY_UI_CXX}
    ${MY_QRC_SRCS}
    )

  # Apply properties to the library target.
  SET_TARGET_PROPERTIES(${lib_name}  PROPERTIES
    COMPILE_FLAGS "-DQT_PLUGIN"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/designer"
    )

  SET(my_libs
    ${MY_TARGET_LIBRARIES}
    ${QT_QTDESIGNER_LIBRARY}
    )
  TARGET_LINK_LIBRARIES(${lib_name} ${my_libs})

  # Install the library
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR}/designer COMPONENT Runtime
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR}/designer COMPONENT Runtime
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR}/designer COMPONENT Development)

  # Install headers - Are headers required ?
  #FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  #INSTALL(FILES
  #  ${headers}
  #  DESTINATION ${CTK_INSTALL_INCLUDE_DIR} COMPONENT Development
  #  )


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

ENDMACRO()
