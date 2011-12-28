# Note that on MacOS X, java can be found but not installed. You can prevent find_package to find
# Java by passing to CMake >2.8.6 CMAKE_DISABLE_FIND_PACKAGE_Java:BOOL=TRUE
find_package( Java )

if(NOT Java_JAVAC_EXECUTABLE)
  message(STATUS "Skipping installation of CLI/Application/Java - Set Java_JAVAC_EXECUTABLE variable to fix the problem")
  return()
endif()

project(JavaExamples Java )

slicer3_set_plugins_output_path()

set(build_type ".")
if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
  # Sadly, there is no way to know if the user picked Debug or Release
  # here, so we are going to have to stick to the value of CMAKE_BUILD_TYPE
  # if it has been set explicitly (by a dashboard for example), or the
  # first value in CMAKE_CONFIGURATION_TYPES (i.e. Debug)/
if (Slicer_USE_KWWIDGETS)
  kwwidgets_get_cmake_build_type(build_type)
endif (Slicer_USE_KWWIDGETS)
endif(WIN32 AND CMAKE_CONFIGURATION_TYPES)

#####################

set (CLP HelloWorld)

# If have the java compiler, try to compile the .java files
if( Java_JAVAC_EXECUTABLE )
# message (STATUS "Java run time = ${Java_JAVA_EXECUTABLE}, compiler = ${Java_JAVAC_EXECUTABLE}, jar = ${Java_JAR_EXECUTABLE}")

  # source files
  set( ${CLP}_JAR_SRCS               Examples/${CLP}App.java)

  # class path
  if( WIN32 )
    set( ${CLP}_CLASSPATH
      ".\;../Examples\;${CMAKE_CURRENT_BINARY_DIR}" )
  else( WIN32 )
    set( ${CLP}_CLASSPATH
      ".:../Examples:${CMAKE_CURRENT_BINARY_DIR}" )
  endif( WIN32 )

# message (STATUS "Adding custom java compile target: ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${${CLP}_CLASSPATH} ${${CLP}_JAR_SRCS}")
  add_custom_target( ${CLP}.jar.classes ALL
     ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${${CLP}_CLASSPATH} ${${CLP}_JAR_SRCS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )

# message (STATUS "Adding a custom target to jar it up into the plug ins directory ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/${build_type}")
  add_custom_target( ${CLP}.jar ALL
      ${Java_JAR_EXECUTABLE} cvf  ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/${build_type}/${CLP}.jar Examples
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

  add_dependencies( ${CLP}.jar ${CLP}.jar.classes )

  # install the jar file
  install (
     FILES ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/${build_type}/${CLP}.jar
     DESTINATION ${Slicer_INSTALL_CLIMODULES_BIN_DIR}
     )

  #####
  # install the tcl file that runs the jar file like an executable CLI
  #####

  if (Slicer_SOURCE_DIR)
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
      ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/${build_type}/${file}
      COPYONLY
      )
  endforeach(file)
#  install(
#    FILES ${TARGETS}
#    DESTINATION ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/${build_type}
#   )
  install (
     FILES ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/${build_type}/${CLP}.tcl
     DESTINATION ${Slicer_INSTALL_CLIMODULES_BIN_DIR}
     )
  endif (Slicer_SOURCE_DIR)

else( Java_JAVAC_EXECUTABLE)
   message (STATUS "Unable to compile java code, skipping installation of ${CLP}.jar and .tcl")
endif( Java_JAVAC_EXECUTABLE )

##############

# skip SPECTRE
if (0)

set (CLP SPECTRE)

#####
# install the tcl file that runs the jar file like an executable CLI
#####

if (Slicer_SOURCE_DIR)
  # install each target in the production area (where it would appear in an
  # installation) and install each target in the developer area (for running
  # from a build)
  set(TARGETS
     ${CLP}.tcl
#     SPECTRE_2010-04-01.jar
     SPECTRElite_2010-05-15.jar
    )

foreach(file ${TARGETS})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/${build_type}/${file}
    COPYONLY
    )
endforeach(file)
install(
  FILES ${TARGETS}
  DESTINATION ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/${build_type}
  )
endif (Slicer_SOURCE_DIR)


endif (0)
