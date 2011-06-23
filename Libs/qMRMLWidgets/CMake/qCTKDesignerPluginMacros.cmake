#
#
#

#
# Build a qtdesigner plugin
#
macro(qctk_build_designer_plugin)
  QCTK_PARSE_ARGUMENTS(MY
    "NAME;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_FORMS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES;LIBRARY_TYPE"
    ""
    ${ARGN}
    )

  # Sanity checks
  if(NOT DEFINED MY_NAME)
    message(SEND_ERROR "NAME is mandatory")
  endif()
  if(NOT DEFINED MY_LIBRARY_TYPE)
    set(MY_LIBRARY_TYPE "SHARED")
  endif()

  # Define library name
  set(lib_name ${MY_NAME})

  # --------------------------------------------------------------------------
  # Include dirs
  set(my_includes
    #${CTK_BASE_INCLUDE_DIRS}
    ${QT_QTDESIGNER_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MY_INCLUDE_DIRECTORIES}
    )
  include_directories(
    ${my_includes}
    )

  set(MY_LIBNAME ${lib_name})

  # Make sure variable are cleared
  set(MY_UI_CXX)
  set(MY_QRC_SRCS)

  # Wrap
  QT4_WRAP_CPP(MY_MOC_CXX ${MY_MOC_SRCS})
  QT4_WRAP_UI(MY_UI_CXX ${MY_UI_FORMS})
  set(MY_QRC_SRCS "")
  if(DEFINED MY_RESOURCES)
    QT4_ADD_RESOURCES(MY_QRC_SRCS ${MY_RESOURCES})
  endif()

  source_group("Resources" FILES
    ${MY_RESOURCES}
    ${MY_UI_FORMS}
    )

  source_group("Generated" FILES
    ${MY_MOC_CXX}
    ${MY_QRC_SRCS}
    ${MY_UI_CXX}
    )

  add_library(${lib_name} ${MY_LIBRARY_TYPE}
    ${MY_SRCS}
    ${MY_MOC_CXX}
    ${MY_UI_CXX}
    ${MY_QRC_SRCS}
    )
  # HACK - Replace Plugin with empty string to obtain Label name
  string(REPLACE "Plugin" "" label ${lib_name})
  set_target_properties(${lib_name} PROPERTIES LABELS ${label})

  # Apply properties to the library target.
  set_target_properties(${lib_name}  PROPERTIES
    COMPILE_FLAGS "-DQT_PLUGIN"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/designer"
    )

  set(my_libs
    ${MY_TARGET_LIBRARIES}
    ${QT_QTDESIGNER_LIBRARY}
    )
  target_link_libraries(${lib_name} ${my_libs})

  # Install the library
  install(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR}/designer COMPONENT Runtime
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR}/designer COMPONENT Runtime
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR}/designer COMPONENT Development)

  # Install headers - Are headers required ?
  #file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  #install(FILES
  #  ${headers}
  #  DESTINATION ${CTK_INSTALL_INCLUDE_DIR} COMPONENT Development
  #  )


  # Since QtDesigner expects plugin to be directly located under the
  # directory 'designer', let's copy them.

  if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
    get_target_property(FILE_PATH ${lib_name} LOCATION)
    get_target_property(DIR_PATH ${lib_name} LIBRARY_OUTPUT_DIRECTORY)

    add_custom_command(
      TARGET ${lib_name}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${FILE_PATH} ${DIR_PATH}/../designer/${CMAKE_SHARED_LIBRARY_PREFIX}${lib_name}${CMAKE_BUILD_TYPE}${CMAKE_SHARED_LIBRARY_SUFFIX}
      )
  endif()

endmacro()
