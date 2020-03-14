
  set(Slicer_QT_VERSION_MAJOR "${Qt5_VERSION_MAJOR}")
  set(Slicer_QT_VERSION_MINOR "${Qt5_VERSION_MINOR}")

#-----------------------------------------------------------------------------
# Sanity checks
set(expected_defined_vars
  CMAKE_GENERATOR
  Slicer_QT_VERSION_MAJOR
  Slicer_QT_VERSION_MINOR
  Slicer_EXTENSIONS_TRACK_QUALIFIER
  Slicer_REVISION
  CTEST_DROP_SITE
  )
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

set(expected_existing_vars
  GIT_EXECUTABLE
  Slicer_CMAKE_DIR
  Slicer_DIR
  Slicer_EXTENSION_DESCRIPTION_DIR
  Slicer_LOCAL_EXTENSIONS_DIR
  Subversion_SVN_EXECUTABLE
  )
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Convert to absolute path
foreach(varname
  Slicer_CMAKE_DIR
  Slicer_DIR
  Slicer_EXTENSION_DESCRIPTION_DIR
  Slicer_LOCAL_EXTENSIONS_DIR
  )
  if(NOT IS_ABSOLUTE ${${varname}})
    set(${varname} "${CMAKE_CURRENT_BINARY_DIR}/${${varname}}")
  endif()
endforeach()

include(SlicerFunctionExtractExtensionDescription)
include(SlicerBlockUploadExtensionPrerequisites) # Common to all extensions

#-----------------------------------------------------------------------------
# Collect extension description file (*.s4ext)
#-----------------------------------------------------------------------------
file(GLOB s4extfiles "${Slicer_EXTENSION_DESCRIPTION_DIR}/*.s4ext")

# Get the dependency information of each extension
set(EXTENSION_LIST)
foreach(file ${s4extfiles})
  message(STATUS "Extension:${file}")

  # Extract extension description info
  slicerFunctionExtractExtensionDescription(EXTENSION_FILE ${file} VAR_PREFIX EXTENSION)

  # Extract file basename
  get_filename_component(EXTENSION_NAME ${file} NAME_WE)
  if("${EXTENSION_NAME}" STREQUAL "")
    message(WARNING "Failed to extract extension name associated with file: ${file}")
  else()
    list(APPEND EXTENSION_LIST ${EXTENSION_NAME})
    string(REGEX REPLACE "^NA$" "" EXTENSION_EXT_DEPENDS "${EXTENSION_EXT_DEPENDS}")
    set(EXTENSION_${EXTENSION_NAME}_DEPENDS ${EXTENSION_EXT_DEPENDS})
    set(${EXTENSION_NAME}_BUILD_SUBDIRECTORY ${EXTENSION_FILE_BUILD_SUBDIRECTORY})
  endif()
endforeach()

# Sort extensions
include(TopologicalSort)
topological_sort(EXTENSION_LIST "EXTENSION_" "_DEPENDS")

foreach(EXTENSION_NAME ${EXTENSION_LIST})
  # Set extension description filename using EXTENSION_NAME
  set(file ${Slicer_EXTENSION_DESCRIPTION_DIR}/${EXTENSION_NAME}.s4ext)

  # Extract extension description info
  slicerFunctionExtractExtensionDescription(EXTENSION_FILE ${file} VAR_PREFIX EXTENSION)
  set(EXTENSION_CATEGORY ${EXTENSION_EXT_CATEGORY})
  set(EXTENSION_STATUS ${EXTENSION_EXT_STATUS})
  set(EXTENSION_ICONURL ${EXTENSION_EXT_ICONURL})
  set(EXTENSION_CONTRIBUTORS ${EXTENSION_EXT_CONTRIBUTORS})
  set(EXTENSION_DESCRIPTION ${EXTENSION_EXT_DESCRIPTION})
  set(EXTENSION_HOMEPAGE ${EXTENSION_EXT_HOMEPAGE})
  set(EXTENSION_SCREENSHOTURLS ${EXTENSION_EXT_SCREENSHOTURLS})
  set(EXTENSION_ENABLED ${EXTENSION_EXT_ENABLED})
  set(EXTENSION_DEPENDS ${EXTENSION_EXT_DEPENDS})

  # Ensure extensions depending on this extension can lookup the corresponding
  # _DIR and _BUILD_SUBDIRECTORY variables.
  set(${EXTENSION_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}-build)
  set(${EXTENSION_NAME}_BUILD_SUBDIRECTORY ${EXTENSION_EXT_BUILD_SUBDIRECTORY})

  message(STATUS "Configuring extension: ${EXTENSION_NAME}")
  if("${EXTENSION_EXT_SCM}" STREQUAL "" AND "${EXTENSION_EXT_SCMURL}" STREQUAL "")
    message(WARNING "Failed to extract extension information associated to file: ${file}")
    continue()
  endif()

  # Set convenience variable
  set(proj ${EXTENSION_NAME})

  # Set external project DEPENDS parameter
  set(EP_ARG_EXTENSION_DEPENDS)
  if(Slicer_SOURCE_DIR)
    set(EXTENSIONEP_ARG_EXTENSION_DEPENDS DEPENDS Slicer ${EXTENSION_DEPENDS})
  else()
    if(NOT "${EXTENSION_DEPENDS}" STREQUAL "")
      set(EP_ARG_EXTENSION_DEPENDS DEPENDS ${EXTENSION_DEPENDS})
    endif()
  endif()

  #-----------------------------------------------------------------------------
  # Configure extension source download wrapper script
  #-----------------------------------------------------------------------------
  set(ext_ep_options_repository)
  set(ext_revision ${EXTENSION_EXT_SCMREVISION})
  if("${EXTENSION_EXT_SCM}" STREQUAL "git")
    set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME})
    if("${ext_revision}" STREQUAL "")
      set(ext_revision "origin/master")
    endif()
    set(ext_ep_options_repository
      GIT_REPOSITORY ${EXTENSION_EXT_SCMURL} GIT_TAG ${ext_revision})
  elseif("${EXTENSION_EXT_SCM}" STREQUAL "svn")
    set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME})
    if("${ext_revision}" STREQUAL "")
      set(ext_revision "HEAD")
    endif()
    set(ext_ep_options_repository
      SVN_REPOSITORY ${EXTENSION_EXT_SCMURL} SVN_REVISION -r ${ext_revision})
    if(NOT ${EXTENSION_EXT_SVNUSERNAME} STREQUAL "")
       list(APPEND ext_ep_options_repository
         SVN_USERNAME "${EXTENSION_EXT_SVNUSERNAME}"
         SVN_PASSWORD "${EXTENSION_EXT_SVNPASSWORD}"
         SVN_TRUST_CERT 1
         )
    endif()
  elseif("${EXTENSION_EXT_SCM}" STREQUAL "local")
    set(EXTENSION_SOURCE_DIR ${EXTENSION_EXT_SCMURL})
    if(NOT IS_ABSOLUTE ${EXTENSION_SOURCE_DIR})
      set(EXTENSION_SOURCE_DIR ${Slicer_LOCAL_EXTENSIONS_DIR}/${EXTENSION_SOURCE_DIR})
    endif()
    set(ext_ep_download_command DOWNLOAD_COMMAND "")
  else()
    message(WARNING "Unknown type of SCM [${EXTENSION_EXT_SCM}] associated with extension named ${EXTENSION_NAME} - See file ${file}")
    continue()
  endif()
  if(NOT "${ext_ep_options_repository}" STREQUAL "")
    #
    # The following wrapper script is required to avoid the overall extension
    # build from failing if only one extension source tree failed to be
    # downloaded.
    #
    # Add a "download" project allowing to retrieve the download command.
    ExternalProject_Add(${proj}-download
      ${ext_ep_options_repository}
      SOURCE_DIR ${EXTENSION_SOURCE_DIR}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      BUILD_IN_SOURCE 1
      INSTALL_COMMAND ""
      EXCLUDE_FROM_ALL 1
      )
    get_property(command TARGET ${proj}-download PROPERTY _EP_download_COMMAND)
    set(download_extension_wrapper_script
      ${CMAKE_CURRENT_BINARY_DIR}/download_${proj}_wrapper_script.cmake)
    #message(STATUS "Configuring extension download wrapper script: ${download_extension_wrapper_script}")
    file(WRITE ${download_extension_wrapper_script} "
      execute_process(
        COMMAND ${command}
        WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
        RESULT_VARIABLE result
        ERROR_VARIABLE error
        )
      # Sanitize error string to prevent false positive by Visual Studio
      set(sanitized_error \"\${error}\")
      string(REPLACE \"error:\" \"error \" sanitized_error \"\${sanitized_error}\")
      message(STATUS \"download_${proj}_wrapper_script: Ignoring result \${result}\")
      if(NOT result EQUAL 0)
        message(STATUS \"Generating '${EXTENSION_SOURCE_DIR}/CMakeLists.txt'\")
        file(MAKE_DIRECTORY \"${EXTENSION_SOURCE_DIR}\")
        file(WRITE \"${EXTENSION_SOURCE_DIR}/CMakeLists.txt\"
          \"cmake_minimum_required(VERSION 3.13.4)
          project(${proj} NONE)
          message(FATAL_ERROR \\\"Failed to download extension using ${ext_ep_options_repository}\\n\${sanitized_error}\\\")
          \"
          )
      endif()
      ")
    set(ext_ep_download_command
      DOWNLOAD_COMMAND ${CMAKE_COMMAND} -P ${download_extension_wrapper_script}
      )
  endif()

  #-----------------------------------------------------------------------------
  # Configure extension build wrapper script
  #-----------------------------------------------------------------------------
  set(EXTENSION_SUPERBUILD_BINARY_DIR ${${EXTENSION_NAME}_BINARY_DIR})
  set(EXTENSION_BUILD_SUBDIRECTORY ${${EXTENSION_NAME}_BUILD_SUBDIRECTORY})
  if(NOT DEFINED CTEST_MODEL)
    set(CTEST_MODEL "Experimental")
  endif()
  include(SlicerBlockUploadExtension)
  if(Slicer_UPLOAD_EXTENSIONS)
    set(wrapper_command ${EXTENSION_UPLOAD_WRAPPER_COMMAND})
  else()
    set(wrapper_command ${EXTENSION_TEST_WRAPPER_COMMAND})
  endif()

  #
  # The following wrapper script is required to avoid the overall extension
  # build from failing if only one test of an extension being depended on fails.
  # See #4247
  #
  # Note that as soon as CMake >= 3.6.7 is released, it should be possible
  # to remove the wrapper script and simply specify CAPTURE_CMAKE_ERROR
  # ctest_test option.
  #
  # See https://cmake.org/cmake/help/v3.7/command/ctest_test.html
  #
  set(build_extension_wrapper_script
    ${CMAKE_CURRENT_BINARY_DIR}/build_${proj}_wrapper_script.cmake)
  set(build_output_file "${CMAKE_CURRENT_BINARY_DIR}/build_${proj}output.txt")
  set(build_error_file "${CMAKE_CURRENT_BINARY_DIR}/build_${proj}_error.txt")
  #message(STATUS "Configuring extension upload wrapper script: ${build_extension_wrapper_script}")
  file(WRITE ${build_extension_wrapper_script} "
    execute_process(
      COMMAND ${wrapper_command}
      WORKING_DIR \"${EXTENSION_SUPERBUILD_BINARY_DIR}\"
      OUTPUT_FILE \"${build_output_file}\"
      ERROR_FILE \"${build_error_file}\"
      RESULT_VARIABLE result
      )
    message(STATUS \"build_${proj}_wrapper_script: Ignoring result \${result}\")
    message(STATUS \"build_${proj}_output_file: ${build_output_file}\")
    message(STATUS \"build_${proj}_error_file: ${build_error_file}\")
    ")

  # Add extension external project
  #message("ext_ep_download_command:${ext_ep_download_command}")
  ExternalProject_Add(${proj}
    ${ext_ep_download_command}
    SOURCE_DIR ${EXTENSION_SOURCE_DIR}
    BINARY_DIR ${EXTENSION_SUPERBUILD_BINARY_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} -DCTEST_BUILD_CONFIGURATION=${CTEST_BUILD_CONFIGURATION} -P ${build_extension_wrapper_script}
    INSTALL_COMMAND ""
    ${EP_ARG_EXTENSION_DEPENDS}
    )
  # This custom external project step forces the build and later
  # steps to run whenever a top level build is done...
  ExternalProject_Add_Step(${proj} forcebuild
    DEPENDEES configure
    DEPENDERS build
    ALWAYS 1
    )

endforeach()
