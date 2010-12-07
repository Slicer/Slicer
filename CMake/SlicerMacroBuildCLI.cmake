
#
# Depends on:
#  Slicer/CMake/SlicerParseArgumentsMacro.cmake
#

MACRO(slicerMacroBuildCLI)
  SLICER_PARSE_ARGUMENTS(MY
    "NAME;ADDITIONAL_SRCS;LOGO_HEADER;TARGET_LIBRARIES;LINK_DIRECTORIES;INCLUDE_DIRECTORIES"
    ""
    ${ARGN}
    )
    
  # Sanity checks
  IF(NOT DEFINED MY_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF()
  IF(DEFINED MY_LOGO_HEADER AND NOT EXISTS ${MY_LOGO_HEADER})
    MESSAGE(SEND_ERROR "Specified LOGO_HEADER [${header}] doesn't exist")
  ELSE()
    SET(MY_LOGO_HEADER)
  ENDIF()
  IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MY_NAME}.xml)
    MESSAGE(SEND_ERROR "Xml file [${MY_NAME}.xml] doesn't exist !")
  ENDIF()
  
  SET(CLP ${MY_NAME})
  
  SET(${CLP}_SOURCE ${CLP}.cxx ${MY_ADDITIONAL_SRCS})
  generateclp(${CLP}_SOURCE ${CLP}.xml ${MY_LOGO_HEADER})
  
  IF(DEFINED MY_LINK_DIRECTORIES)
    LINK_DIRECTORIES(${MY_LINK_DIRECTORIES})
  ENDIF()
  
  IF(DEFINED MY_INCLUDE_DIRECTORIES)
    INCLUDE_DIRECTORIES(${MY_INCLUDE_DIRECTORIES})
  ENDIF()

  ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
  SET_TARGET_PROPERTIES(${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
  slicer3_set_plugins_output_path(${CLP}Lib)
  IF(DEFINED MY_TARGET_LIBRARIES)
    TARGET_LINK_LIBRARIES(${CLP}Lib ${MY_TARGET_LIBRARIES})
  ENDIF()
  
  # Set labels associated with the target.
  SET_TARGET_PROPERTIES(${CLP}Lib PROPERTIES LABELS ${CLP})

  ADD_EXECUTABLE(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
  slicer3_set_plugins_output_path(${CLP})
  TARGET_LINK_LIBRARIES(${CLP} ${CLP}Lib)
  
  # Set labels associated with the target.
  SET_TARGET_PROPERTIES(${CLP} PROPERTIES LABELS ${CLP})
  
  # Install each target in the production area (where it would appear in an installation) 
  # and install each target in the developer area (for running from a build)
  set(TARGETS ${CLP} ${CLP}Lib)
  slicer3_install_plugins(${TARGETS})

ENDMACRO()
