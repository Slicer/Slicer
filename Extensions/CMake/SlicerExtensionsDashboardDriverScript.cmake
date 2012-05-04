

#
# Included from a dashboard script, this cmake file will drive the configure and build
# steps of Slicer
#

# Helper function used to generate the CMake script that will perform the initial clone
# of a git repository. (Copied from ExternalProject CMake module)
function(_write_gitclone_script script_filename source_dir git_EXECUTABLE git_repository git_tag src_name work_dir)
  file(WRITE ${script_filename}
"if(\"${git_tag}\" STREQUAL \"\")
  message(FATAL_ERROR \"Tag for git checkout should not be empty.\")
endif()

execute_process(
  COMMAND \${CMAKE_COMMAND} -E remove_directory \"${source_dir}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to remove directory: '${source_dir}'\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" clone \"${git_repository}\" \"${src_name}\"
  WORKING_DIRECTORY \"${work_dir}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to clone repository: '${git_repository}'\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" checkout ${git_tag}
  WORKING_DIRECTORY \"${work_dir}/${src_name}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to checkout tag: '${git_tag}'\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" submodule init
  WORKING_DIRECTORY \"${work_dir}/${src_name}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to init submodules in: '${work_dir}/${src_name}'\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" submodule update --recursive
  WORKING_DIRECTORY \"${work_dir}/${src_name}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to update submodules in: '${work_dir}/${src_name}'\")
endif()

"
)
endfunction()

# Helper function used to generate the CMake script that will update a git clone given a tag.
# (Inspired from ExternalProject CMake module)
function(_update_gitclone_script script_filename source_dir git_EXECUTABLE git_repository git_tag src_name work_dir)
  file(WRITE ${script_filename}
"if(\"${git_tag}\" STREQUAL \"\")
  message(FATAL_ERROR \"Tag for git checkout should not be empty.\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" fetch
  WORKING_DIRECTORY \"${work_dir}/${src_name}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to fetch repository: '${git_repository}'\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" checkout ${git_tag}
  WORKING_DIRECTORY \"${work_dir}/${src_name}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to checkout tag: '${git_tag}'\")
endif()

execute_process(
  COMMAND \"${git_EXECUTABLE}\" submodule update --recursive
  WORKING_DIRECTORY \"${work_dir}/${src_name}\"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR \"Failed to update submodules in: '${work_dir}/${src_name}'\")
endif()

"
)
endfunction()

# Helper function used to extract slicer revision within a header file. The revision is
# expected to be defined using the format '#define Slicer_WC_REVISION "abcd123456"'
function(_get_slicer_revision headerfile varname)
  set(_macroname Slicer_WC_REVISION)
  set(_revision_regex "^#define[ \t]+${_macroname}[ \t]+[\"]*([0-9A-Za-z\\.]+)[\"][ \t]*$")
  file(STRINGS "${headerfile}" _revision_string
       LIMIT_COUNT 1 REGEX ${_revision_regex})
  set(dollar "$")
  string(REGEX REPLACE ${_revision_regex} "\\1" _revision "${_revision_string}")

  set(${varname} ${_revision} PARENT_SCOPE)
endfunction()


# Macro allowing to set a variable to its default value only if not already defined
macro(setIfNotDefined var defaultvalue)
  if(NOT DEFINED ${var})
    set(${var} "${defaultvalue}")
  endif()
endmacro()

# The following variable are expected to be define in the top-level script:
set(expected_variables
  ADDITIONAL_CMAKECACHE_OPTION
  CTEST_NOTES_FILES
  CTEST_SITE
  CTEST_DASHBOARD_ROOT
  CTEST_CMAKE_GENERATOR
  WITH_MEMCHECK
  WITH_COVERAGE
  WITH_DOCUMENTATION
  CTEST_BUILD_CONFIGURATION
  CTEST_TEST_TIMEOUT
  CTEST_BUILD_FLAGS
  CTEST_PROJECT_NAME
  EXTENSIONS_BUILDSYSTEM_SOURCE_DIRECTORY
  EXTENSIONS_BUILDSYSTEM_TESTING
  EXTENSIONS_INDEX_GIT_REPOSITORY
  EXTENSIONS_INDEX_GIT_TAG
  CTEST_BINARY_DIRECTORY
  CTEST_BUILD_NAME
  SCRIPT_MODE
  CTEST_COVERAGE_COMMAND
  CTEST_MEMORYCHECK_COMMAND
  CTEST_SVN_COMMAND
  CTEST_GIT_COMMAND
  MIDAS_PACKAGE_URL
  MIDAS_PACKAGE_EMAIL
  MIDAS_PACKAGE_API_KEY
  Slicer_DIR
  )

if(WITH_DOCUMENTATION)
  list(APPEND expected_variables DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)
endif()
if(NOT DEFINED CTEST_PARALLEL_LEVEL)
  set(CTEST_PARALLEL_LEVEL 8)
endif()

if(EXISTS "${CTEST_LOG_FILE}")
  list(APPEND CTEST_NOTES_FILES ${CTEST_LOG_FILE})
endif()

foreach(var ${expected_variables})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} should be defined in top-level script !")
  endif()
endforeach()

if(NOT DEFINED CTEST_CONFIGURATION_TYPE AND DEFINED CTEST_BUILD_CONFIGURATION)
  set(CTEST_CONFIGURATION_TYPE ${CTEST_BUILD_CONFIGURATION})
endif()

set(CTEST_COMMAND ${CMAKE_CTEST_COMMAND})
set(CTEST_SOURCE_DIRECTORY ${EXTENSIONS_BUILDSYSTEM_SOURCE_DIRECTORY})
message(CTEST_SOURCE_DIRECTORY:${CTEST_SOURCE_DIRECTORY})
if(NOT EXTENSIONS_BUILDSYSTEM_TESTING)
  set(Slicer_EXTENSION_DESCRIPTION_DIR "${CTEST_BINARY_DIRECTORY}/ExtensionsIndex")
endif()

# Since having '/' in the build name confuses CDash (Separate entries are added for both
# update and rest of the build), let's convert them into '-'.
string(REGEX REPLACE "[/]" "-" CTEST_BUILD_NAME ${CTEST_BUILD_NAME})

# Should binary directory be cleaned?
set(empty_binary_directory FALSE)

# Attempt to build and test also if 'ctest_update' returned an error
set(force_build FALSE)

# Set model and track options
set(model "")
if(SCRIPT_MODE STREQUAL "experimental")
  set(empty_binary_directory FALSE)
  set(force_build TRUE)
  set(model Experimental)
elseif(SCRIPT_MODE STREQUAL "continuous")
  set(empty_binary_directory TRUE)
  set(force_build FALSE)
  set(model Continuous)
elseif(SCRIPT_MODE STREQUAL "nightly")
  set(empty_binary_directory TRUE)
  set(force_build TRUE)
  set(model Nightly)
else()
  message(FATAL_ERROR "Unknown script mode: '${SCRIPT_MODE}'. Script mode should be either 'experimental', 'continuous' or 'nightly'")
endif()
set(track_qualifier_cleaned "${EXTENSIONS_TRACK_QUALIFIER}-")
# Track associated with 'master' should default to either 'Continuous', 'Nightly' or 'Experimental'
if(track_qualifier_cleaned STREQUAL "master-")
  set(track_qualifier_cleaned "")
endif()
set(track Extensions-${track_qualifier_cleaned}${model})
set(track ${CTEST_TRACK_PREFIX}${track}${CTEST_TRACK_SUFFIX})


# Display build info
message("CTEST_SITE ................: ${CTEST_SITE}")
message("CTEST_BUILD_NAME ..........: ${CTEST_BUILD_NAME}")
message("SCRIPT_MODE ...............: ${SCRIPT_MODE}")
message("CTEST_BUILD_CONFIGURATION .: ${CTEST_BUILD_CONFIGURATION}")
message("WITH_KWSTYLE ..............: ${WITH_KWSTYLE}")
message("WITH_COVERAGE: ............: ${WITH_COVERAGE}")
message("WITH_MEMCHECK .............: ${WITH_MEMCHECK}")
message("WITH_PACKAGES .............: ${WITH_PACKAGES}")
message("WITH_DOCUMENTATION ........: ${WITH_DOCUMENTATION}")
message("EXTENSIONS_TRACK_QUALIFIER : ${EXTENSIONS_TRACK_QUALIFIER}")
message("DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY: ${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}")
message("EXTENSIONS_BUILDSYSTEM_SOURCE_DIRECTORY: ${EXTENSIONS_BUILDSYSTEM_SOURCE_DIRECTORY}")
message("Slicer_DIR ................: ${Slicer_DIR}")

# For more details, see http://www.kitware.com/blog/home/post/11
set(CTEST_USE_LAUNCHERS 0)
if(CTEST_CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(CTEST_USE_LAUNCHERS 1)
endif()

if(empty_binary_directory)
  message("Directory ${CTEST_BINARY_DIRECTORY} cleaned !")
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
endif()

if(NOT EXTENSIONS_BUILDSYSTEM_TESTING)
  # Compute 'work_dir' and 'src_name' variable used by both _write_gitclone_script and _update_gitclone_script
  get_filename_component(src_dir ${Slicer_EXTENSION_DESCRIPTION_DIR} REALPATH)
  get_filename_component(work_dir ${src_dir} PATH)
  get_filename_component(src_name ${src_dir} NAME)

  string(REGEX REPLACE "[/]" "-" git_tag_cleaned ${EXTENSIONS_INDEX_GIT_TAG})

  if(NOT EXISTS "${Slicer_EXTENSION_DESCRIPTION_DIR}/.git")
    _write_gitclone_script(
      ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}-${git_tag_cleaned}-${SCRIPT_MODE}-gitclone.cmake # script_filename
      ${Slicer_EXTENSION_DESCRIPTION_DIR} # source_dir
      ${CTEST_GIT_COMMAND} # git_EXECUTABLE
      ${EXTENSIONS_INDEX_GIT_REPOSITORY} # git_repository
      ${EXTENSIONS_INDEX_GIT_TAG} # git_tag
      ${src_name} # src_name
      ${work_dir} # work_dir
    )
    # Note: The following command should be specified as a string.
    string(REPLACE " " "\\ " CMAKE_COMMAND_ESCAPED ${CMAKE_COMMAND})
    set(CTEST_CHECKOUT_COMMAND "${CMAKE_COMMAND_ESCAPED} -P ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}-${git_tag_cleaned}-${SCRIPT_MODE}-gitclone.cmake")
  endif()

  _update_gitclone_script(
    ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}-${git_tag_cleaned}-${SCRIPT_MODE}-gitupdate.cmake # script_filename
    ${Slicer_EXTENSION_DESCRIPTION_DIR} # source_dir
    ${CTEST_GIT_COMMAND} # git_EXECUTABLE
    ${EXTENSIONS_INDEX_GIT_REPOSITORY} # git_repository
    ${EXTENSIONS_INDEX_GIT_TAG} # git_tag
    ${src_name} # src_name
    ${work_dir} # work_dir
    )

  # Note: The following command should be specified as a list.
  set(CTEST_GIT_UPDATE_CUSTOM ${CMAKE_COMMAND} -P ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}-${git_tag_cleaned}-${SCRIPT_MODE}-gitupdate.cmake)

  # Retrieve revision associated with Slicer source tree
  _get_slicer_revision("${Slicer_DIR}/vtkSlicerVersionConfigure.h" Slicer_WC_REVISION)
  message("Slicer_WC_REVISION:${Slicer_WC_REVISION}")
  set(Slicer_PREVIOUS_WC_REVISION ${Slicer_WC_REVISION})

endif()

#-----------------------------------------------------------------------------
# The following variable can be used while testing the driver scripts
#-----------------------------------------------------------------------------
setIfNotDefined(run_ctest_submit TRUE)
setIfNotDefined(run_ctest_with_update TRUE)
setIfNotDefined(run_ctest_with_configure TRUE)
setIfNotDefined(run_ctest_with_build TRUE)
setIfNotDefined(run_ctest_with_notes TRUE)

#
# run_ctest macro
#
macro(run_ctest)
  ctest_start(${model} TRACK ${track})

  if(NOT EXTENSIONS_BUILDSYSTEM_TESTING AND run_ctest_with_update)
    ctest_update(SOURCE "${Slicer_EXTENSION_DESCRIPTION_DIR}" RETURN_VALUE FILES_UPDATED)
  endif()
  set(CTEST_CHECKOUT_COMMAND) # checkout on first iteration only

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message("First time build - Initialize CMakeCache.txt")
    set(force_build TRUE)

    set(Slicer_EXTENSION_DESCRIPTION_DIR_CMAKECACHE)
    if(NOT EXTENSIONS_BUILDSYSTEM_TESTING)
      set(Slicer_EXTENSION_DESCRIPTION_DIR_CMAKECACHE "Slicer_EXTENSION_DESCRIPTION_DIR:PATH=${Slicer_EXTENSION_DESCRIPTION_DIR}")
    endif()

    #-----------------------------------------------------------------------------
    # Write initial cache.
    #-----------------------------------------------------------------------------
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CTEST_MODEL:STRING=${model}
CTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
Slicer_EXTENSIONS_TRACK_QUALIFIER:STRING=${EXTENSIONS_TRACK_QUALIFIER}
GIT_EXECUTABLE:FILEPATH=${CTEST_GIT_COMMAND}
Subversion_SVN_EXECUTABLE:FILEPATH=${CTEST_SVN_COMMAND}
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
DOCUMENTATION_TARGET_IN_ALL:BOOL=${WITH_DOCUMENTATION}
DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
MIDAS_PACKAGE_URL:STRING=${MIDAS_PACKAGE_URL}
MIDAS_PACKAGE_EMAIL:STRING=${MIDAS_PACKAGE_EMAIL}
MIDAS_PACKAGE_API_KEY:STRING=${MIDAS_PACKAGE_API_KEY}
Slicer_DIR:PATH=${Slicer_DIR}
Slicer_UPLOAD_EXTENSIONS:BOOL=TRUE
${Slicer_EXTENSION_DESCRIPTION_DIR_CMAKECACHE}
${ADDITIONAL_CMAKECACHE_OPTION}
")
  endif()

  _get_slicer_revision("${Slicer_DIR}/vtkSlicerVersionConfigure.h" Slicer_WC_REVISION)
  set(slicer_source_updated FALSE)
  if(NOT "${Slicer_PREVIOUS_WC_REVISION}" STREQUAL "${Slicer_WC_REVISION}")
    set(slicer_source_updated TRUE)
  endif()

  message("FILES_UPDATED ................: ${FILES_UPDATED}")
  message("force_build ..................: ${force_build}")
  message("Slicer_PREVIOUS_WC_REVISION ..: ${Slicer_PREVIOUS_WC_REVISION}")
  message("Slicer_WC_REVISION ...........: ${Slicer_WC_REVISION}")

  if(FILES_UPDATED GREATER 0 OR force_build OR slicer_source_updated)

    set(Slicer_PREVIOUS_WC_REVISION ${Slicer_WC_REVISION})
    set(force_build FALSE)

    #-----------------------------------------------------------------------------
    # Update
    #-----------------------------------------------------------------------------
    if(run_ctest_with_update AND run_ctest_submit)
      ctest_submit(PARTS Update)
    endif()

    #-----------------------------------------------------------------------------
    # Configure
    #-----------------------------------------------------------------------------
    if(run_ctest_with_configure)
      message("----------- [ Configure ${CTEST_PROJECT_NAME} ] -----------")

      #set(label Slicer)

      set_property(GLOBAL PROPERTY SubProject ${label})
      set_property(GLOBAL PROPERTY Label ${label})

      ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" SOURCE "${EXTENSIONS_BUILDSYSTEM_SOURCE_DIRECTORY}")
      ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
      if(run_ctest_submit)
        ctest_submit(PARTS Configure)
      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Build
    #-----------------------------------------------------------------------------
    set(build_errors)
    if(run_ctest_with_build)
      message("----------- [ Build ${CTEST_PROJECT_NAME} ] -----------")
      ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" NUMBER_ERRORS build_errors APPEND)
      if(run_ctest_submit)
        ctest_submit(PARTS Build)
      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Note should be at the end
    #-----------------------------------------------------------------------------
    if(run_ctest_with_notes AND run_ctest_submit)
      ctest_submit(PARTS Notes)
    endif()

  endif()
endmacro()

if(SCRIPT_MODE STREQUAL "continuous")
  while(${CTEST_ELAPSED_TIME} LESS 46800) # Lasts 13 hours (Assuming it starts at 9am, it will end around 10pm)
    set(START_TIME ${CTEST_ELAPSED_TIME})
    run_ctest()
    set(interval 300)
    # Loop no faster than once every <interval> seconds
    message("Wait for ${interval} seconds ...")
    ctest_sleep(${START_TIME} ${interval} ${CTEST_ELAPSED_TIME})
  endwhile()
else()
  run_ctest()
endif()
