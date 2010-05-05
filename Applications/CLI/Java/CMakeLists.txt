project(JavaExamples Java )

FIND_PACKAGE ( Java )




slicer3_set_plugins_output_path()

set(build_type ".")
if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
  # Sadly, there is no way to know if the user picked Debug or Release
  # here, so we are going to have to stick to the value of CMAKE_BUILD_TYPE
  # if it has been set explicitly (by a dashboard for example), or the
  # first value in CMAKE_CONFIGURATION_TYPES (i.e. Debug)/
  kwwidgets_get_cmake_build_type(build_type)
endif(WIN32 AND CMAKE_CONFIGURATION_TYPES)

#####################

set (CLP HelloWorld)

# If have the java compiler, try to compile the .java files
IF ( JAVA_COMPILE )
# message (STATUS "Java run time = ${JAVA_RUNTIME}, compiler = ${JAVA_COMPILE}, jar = ${JAVA_ARCHIVE}")

  # source files
  SET ( ${CLP}_JAR_SRCS               Examples/${CLP}App.java)

  # class path
  IF ( WIN32 )
    SET  ( ${CLP}_CLASSPATH
      ".\;../Examples\;${CMAKE_CURRENT_BINARY_DIR}" )
  ELSE ( WIN32 )
    SET  ( ${CLP}_CLASSPATH
      ".:../Examples:${CMAKE_CURRENT_BINARY_DIR}" )
  ENDIF ( WIN32 )

# message (STATUS "Adding custom java compile target: ${JAVA_COMPILE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${${CLP}_CLASSPATH} ${${CLP}_JAR_SRCS}")
  ADD_CUSTOM_TARGET ( ${CLP}.jar.classes ALL
     ${JAVA_COMPILE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${${CLP}_CLASSPATH} ${${CLP}_JAR_SRCS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )

# message (STATUS "Adding a custom target to jar it up into the plug ins directory ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}")
  ADD_CUSTOM_TARGET ( ${CLP}.jar ALL
      ${JAVA_ARCHIVE} cvf  ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${CLP}.jar Examples
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

  ADD_DEPENDENCIES ( ${CLP}.jar ${CLP}.jar.classes )

  # install the jar file
  install (
     FILES ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${CLP}.jar
     DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
     )

  #####
  # install the tcl file that runs the jar file like an executable CLI
  #####
 
  if (Slicer3_SOURCE_DIR)
    # install each target in the production area (where it would appear in an 
    # installation) and install each target in the developer area (for running 
    # from a build)
#    message (STATUS "Installing ${CLP}.tcl")
    set(TARGETS 
      ${CLP}.tcl
     )

  foreach(file ${TARGETS})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${file}
      COPYONLY
      )
  endforeach(file)
#  install(
#    FILES ${TARGETS}
#    DESTINATION ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}
#   )
  install (
     FILES ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${CLP}.tcl
     DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
     )
  endif (Slicer3_SOURCE_DIR)

ELSE ( JAVA_COMPILE)
   message (STATUS "Unable to compile java code, skipping installation of ${CLP}.jar and .tcl")
ENDIF ( JAVA_COMPILE )

##############

# skip SPECTRE
if (0)

set (CLP SPECTRE)

#####
# install the tcl file that runs the jar file like an executable CLI
#####

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS 
     ${CLP}.tcl
     SPECTRE_2010-04-01.jar
    )

foreach(file ${TARGETS})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${file}
    COPYONLY
    )
endforeach(file)
install(
  FILES ${TARGETS}
  DESTINATION ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}
  )
endif (Slicer3_SOURCE_DIR)


endif (0)
