# SlicerExecutionModel
find_package(SlicerExecutionModel NO_MODULE REQUIRED GenerateCLP)
include(${SlicerExecutionModel_USE_FILE})

macro(slicerMacroBuildCLI)
  set(options
    EXECUTABLE_ONLY
    NO_INSTALL VERBOSE
  )
  set(oneValueArgs  NAME LOGO_HEADER
  )
  set(multiValueArgs
    ADDITIONAL_SRCS
    TARGET_LIBRARIES
    LINK_DIRECTORIES
    INCLUDE_DIRECTORIES
  )
  cmake_parse_arguments(MY_SLICER
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(${MY_SLICER_EXECUTABLE_ONLY})
    set(PASS_EXECUTABLE_ONLY EXECUTABLE_ONLY)
  endif()
  if(${MY_SLICER_NO_INSTALL})
    set(PASS_NO_INSTALL NO_INSTALL)
  endif()
  if(${MY_SLICER_VERBOSE})
    set(PASS_VERBOSE VERBOSE)
  endif()

  message(WARNING "Macro 'slicerMacroBuildCLI' is *DEPRECATED* - Use 'SEMMacroBuildCLI' instead.")

  SEMMacroBuildCLI(
    ${PASS_EXECUTABLE_ONLY}
    ${PASS_NO_INSTALL}
    ${PASS_VERBOSE}
    ADDITIONAL_SRCS                 ${MY_SLICER_ADDITIONAL_SRCS}
    TARGET_LIBRARIES                ${MY_SLICER_TARGET_LIBRARIES}
    LINK_DIRECTORIES                ${MY_SLICER_LINK_DIRECTORIES}
    INCLUDE_DIRECTORIES             "${MY_SLICER_INCLUDE_DIRECTORIES}"
    NAME                            ${MY_SLICER_NAME}
    LOGO_HEADER                     ${MY_SLICER_LOGO_HEADER}
    RUNTIME_OUTPUT_DIRECTORY        "${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY        "${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY        "${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR}"
    INSTALL_RUNTIME_DESTINATION     ${Slicer_INSTALL_CLIMODULES_BIN_DIR}
    INSTALL_LIBRARY_DESTINATION     ${Slicer_INSTALL_CLIMODULES_LIB_DIR}
    INSTALL_ARCHIVE_DESTINATION     ${Slicer_INSTALL_CLIMODULES_LIB_DIR}
    )
endmacro()
