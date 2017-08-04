

#
# Included from a dashboard script, this cmake file will drive the configure and build
# steps of Slicer
#

# The following variable are expected to be define in the top-level script:
set(expected_variables
  CTEST_SITE
  CTEST_BUILD_NAME
  SCRIPT_MODE
  CTEST_BUILD_CONFIGURATION
  WITH_MEMCHECK
  WITH_COVERAGE
  WITH_DOCUMENTATION
  ADDITIONAL_CMAKECACHE_OPTION
  CTEST_NOTES_FILES
  CTEST_DASHBOARD_ROOT
  CTEST_CMAKE_GENERATOR
  CTEST_TEST_TIMEOUT
  CTEST_BUILD_FLAGS
  TEST_TO_EXCLUDE_REGEX
  CTEST_PROJECT_NAME
  CTEST_SOURCE_DIRECTORY
  CTEST_BINARY_DIRECTORY
  CTEST_COVERAGE_COMMAND
  CTEST_MEMORYCHECK_COMMAND
  CTEST_SVN_COMMAND
  CTEST_GIT_COMMAND
  QT_QMAKE_EXECUTABLE
  )

# Update list of expected variables based on build options.
if(WITH_DOCUMENTATION)
  list(APPEND expected_variables DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)
endif()

# List of all variables
set(variables ${expected_variables})

# Sanity check
foreach(var ${expected_variables})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} should be defined in top-level script !")
  endif()
endforeach()

if(NOT DEFINED CTEST_CONFIGURATION_TYPE AND DEFINED CTEST_BUILD_CONFIGURATION)
  set(CTEST_CONFIGURATION_TYPE ${CTEST_BUILD_CONFIGURATION})
endif()
list(APPEND variables CTEST_CONFIGURATION_TYPE)

if(EXISTS "${CTEST_LOG_FILE}")
  list(APPEND CTEST_NOTES_FILES ${CTEST_LOG_FILE})
endif()
list(APPEND variables CTEST_LOG_FILE)
list(APPEND variables CTEST_NOTES_FILES)

#-----------------------------------------------------------------------------
# Macro allowing to set a variable to its default value if not already defined.
# The default value is set with:
#  (1) if set, the value environment variable <var>.
#  (2) if set, the value of local variable variable <var>.
#  (3) if none of the above, the value passed as a parameter.
# Setting the optional parameter 'OBFUSCATE' will display 'OBFUSCATED' instead of the real value.
macro(setIfNotDefined var defaultvalue)
  set(_obfuscate FALSE)
  foreach(arg ${ARGN})
    if(arg STREQUAL "OBFUSCATE")
      set(_obfuscate TRUE)
    endif()
  endforeach()
  if(DEFINED ENV{${var}} AND NOT DEFINED ${var})
    set(_value "$ENV{${var}}")
    if(_obfuscate)
      set(_value "OBFUSCATED")
    endif()
    message(STATUS "Setting '${var}' variable with environment variable value '${_value}'")
    set(${var} $ENV{${var}})
  endif()
  if(NOT DEFINED ${var})
    set(_value "${defaultvalue}")
    if(_obfuscate)
      set(_value "OBFUSCATED")
    endif()
    message(STATUS "Setting '${var}' variable with default value '${_value}'")
    set(${var} "${defaultvalue}")
  endif()
  if(NOT _obfuscate)
    list(APPEND variables ${var})
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Set default values
#-----------------------------------------------------------------------------
setIfNotDefined(CTEST_PARALLEL_LEVEL 8)
setIfNotDefined(CTEST_CONTINUOUS_DURATION 46800) # Lasts 13 hours (Assuming it starts at 9am, it will end around 10pm)
setIfNotDefined(MIDAS_PACKAGE_URL "http://slicer.kitware.com/midas3")
setIfNotDefined(MIDAS_PACKAGE_EMAIL "MIDAS_PACKAGE_EMAIL-NOTDEFINED" OBFUSCATE)
setIfNotDefined(MIDAS_PACKAGE_API_KEY "MIDAS_PACKAGE_API_KEY-NOTDEFINED" OBFUSCATE)

#-----------------------------------------------------------------------------
# The following variable can be used while testing the driver scripts
#-----------------------------------------------------------------------------
setIfNotDefined(run_ctest_submit TRUE)
setIfNotDefined(run_ctest_with_disable_clean FALSE)
setIfNotDefined(run_ctest_with_update TRUE)
setIfNotDefined(run_ctest_with_configure TRUE)
setIfNotDefined(run_ctest_with_build TRUE)
setIfNotDefined(run_ctest_with_test TRUE)
setIfNotDefined(run_ctest_with_coverage TRUE)
setIfNotDefined(run_ctest_with_memcheck TRUE)
setIfNotDefined(run_ctest_with_packages TRUE)
setIfNotDefined(run_ctest_with_upload TRUE)
setIfNotDefined(run_ctest_with_notes TRUE)

#-----------------------------------------------------------------------------
if(NOT DEFINED GIT_REPOSITORY)
  if(NOT DEFINED SVN_REPOSITORY)
    set(SVN_REPOSITORY "http://svn.slicer.org/${CTEST_PROJECT_NAME}")
  endif()
  if(NOT DEFINED SVN_BRANCH)
    set(SVN_BRANCH "trunk")
  endif()
  set(SVN_URL ${SVN_REPOSITORY}/${SVN_BRANCH})
  set(svn_checkout_option "")
  if(NOT "${SVN_REVISION}" STREQUAL "")
    set(SVN_URL "${SVN_URL}@${SVN_REVISION}")
    set(run_ctest_with_update FALSE)
  endif()
  set(repository ${SVN_URL})
  list(APPEND variables SVN_REPOSITORY SVN_BRANCH SVN_REVISION SVN_URL)
else()
  set(repository ${GIT_REPOSITORY})
  set(git_branch_option "")
  if(NOT "${GIT_TAG}" STREQUAL "")
    set(git_branch_option "-b ${GIT_TAG}")
  endif()
  list(APPEND variables GIT_REPOSITORY GIT_TAG)
endif()

# Should binary directory be cleaned?
set(empty_binary_directory FALSE)

# Attempt to build and test also if 'ctest_update' returned an error
set(force_build FALSE)

# Ensure SCRIPT_MODE is lowercase
string(TOLOWER ${SCRIPT_MODE} SCRIPT_MODE)

# Set model and track options
set(model "")
if(SCRIPT_MODE STREQUAL "experimental")
  set(empty_binary_directory FALSE)
  set(force_build TRUE)
  set(model Experimental)
elseif(SCRIPT_MODE STREQUAL "continuous")
  if(${CTEST_CONTINUOUS_DURATION} GREATER 0)
    # Continuous tests are performed in a loop
    # until duration expires. Clean up the build
    # tree at the beginning.
    set(empty_binary_directory TRUE)
  else()
    # A single continuous test run is requested,
    # do not delete the build tree.
    # (useful when the nightly build tree is reused
    # as continuous build tree and repeated builds
    # are triggered by an external scheduler)
    set(empty_binary_directory FALSE)
  endif()
  set(force_build FALSE)
  set(model Continuous)
elseif(SCRIPT_MODE STREQUAL "nightly")
  set(empty_binary_directory TRUE)
  set(force_build TRUE)
  set(model Nightly)
else()
  message(FATAL_ERROR "Unknown script mode: '${SCRIPT_MODE}'. Script mode should be either 'experimental', 'continuous' or 'nightly'")
endif()
set(track ${model})
if(WITH_PACKAGES)
  set(track "${track}-Packages")
endif()
set(track ${CTEST_TRACK_PREFIX}${track}${CTEST_TRACK_SUFFIX})

# Used in SlicerPackageAndUploadTarget CMake module
set(ENV{CTEST_MODEL} ${model})

# For more details, see http://www.kitware.com/blog/home/post/11
set(CTEST_USE_LAUNCHERS 1)
if(NOT "${CTEST_CMAKE_GENERATOR}" MATCHES "Make")
  set(CTEST_USE_LAUNCHERS 0)
endif()
set(ENV{CTEST_USE_LAUNCHERS_DEFAULT} ${CTEST_USE_LAUNCHERS})

list(APPEND variables empty_binary_directory)
list(APPEND variables force_build)
list(APPEND variables model)
list(APPEND variables track)
list(APPEND variables CTEST_USE_LAUNCHERS)

# Given a variable name, this function will display the text
#   "-- <varname> ................: ${<varname>}"
# and will ensure that the message is consistenly padded.
#
# If the variable is not defined, it will display:
#   "-- <varname> ................: <NOT DEFINED>"
function(display_var varname)
  set(pretext_right_jusitfy_length 35)
  set(fill_char ".")

  set(value ${${varname}})
  if(NOT DEFINED ${varname})
    set(value "NOT DEFINED")
  endif()

  set(pretext "${varname}")
  string(LENGTH ${pretext} pretext_length)
  math(EXPR pad_length "${pretext_right_jusitfy_length} - ${pretext_length} - 1")
  if(pad_length GREATER 0)
    string(RANDOM LENGTH ${pad_length} ALPHABET ${fill_char} pretext_dots)
    set(text "${pretext} ${pretext_dots}: ${value}")
  elseif(pad_length EQUAL 0)
    set(text "${pretext} : ${value}")
  else()
    set(text "${pretext}: ${value}")
  endif()
  message(STATUS "${text}")
endfunction()

# Display variables
foreach(var ${variables})
  display_var(${var})
endforeach()

if(empty_binary_directory AND NOT run_ctest_with_disable_clean)
  message("Directory ${CTEST_BINARY_DIRECTORY} cleaned !")
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
endif()

if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  if(NOT DEFINED GIT_REPOSITORY)
    set(CTEST_CHECKOUT_COMMAND "${CTEST_SVN_COMMAND} checkout ${repository} ${CTEST_SOURCE_DIRECTORY}")
  else()
    set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone ${git_branch_option} ${repository} ${CTEST_SOURCE_DIRECTORY}")
  endif()
endif()

if(NOT DEFINED GIT_REPOSITORY)
  set(CTEST_UPDATE_COMMAND "${CTEST_SVN_COMMAND}")
else()
  set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")
endif()

set(CTEST_SOURCE_DIRECTORY "${CTEST_SOURCE_DIRECTORY}")

#
# run_ctest macro
#
macro(run_ctest)
  ctest_start(${model} TRACK ${track})

  set(build_in_progress_file ${CTEST_BINARY_DIRECTORY}/Slicer-build/BUILD_IN_PROGRESS)
  file(WRITE ${build_in_progress_file} "Generated by ${CMAKE_CURRENT_LIST_FILE}\n")

  if(run_ctest_with_update)
    ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE FILES_UPDATED)
  endif()

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message("First time build - Initialize CMakeCache.txt")
    set(force_build TRUE)

    #-----------------------------------------------------------------------------
    # Write initial cache.
    #-----------------------------------------------------------------------------
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
QT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
GIT_EXECUTABLE:FILEPATH=${CTEST_GIT_COMMAND}
Subversion_SVN_EXECUTABLE:FILEPATH=${CTEST_SVN_COMMAND}
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
DOCUMENTATION_TARGET_IN_ALL:BOOL=${WITH_DOCUMENTATION}
DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
${ADDITIONAL_CMAKECACHE_OPTION}
")
  endif()

  if(FILES_UPDATED GREATER 0 OR force_build)

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

      #set_property(GLOBAL PROPERTY SubProject ${label})
      set_property(GLOBAL PROPERTY Label ${label})

      ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
      ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
      if(run_ctest_submit)
        ctest_submit(PARTS Configure)
      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Build top level
    #-----------------------------------------------------------------------------
    set(build_errors)
    if(run_ctest_with_build)
      message("----------- [ Build ${CTEST_PROJECT_NAME} ] -----------")
      ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" NUMBER_ERRORS build_errors APPEND)
      if(run_ctest_submit)
        ctest_submit(PARTS Build)
      endif()
    endif()

    file(REMOVE ${build_in_progress_file})

    #-----------------------------------------------------------------------------
    # Inner build directory
    #-----------------------------------------------------------------------------
    set(slicer_build_dir "${CTEST_BINARY_DIRECTORY}/Slicer-build")

    #-----------------------------------------------------------------------------
    # Test
    #-----------------------------------------------------------------------------
    if(run_ctest_with_test)
      message("----------- [ Test ${CTEST_PROJECT_NAME} ] -----------")
      ctest_test(
        BUILD "${slicer_build_dir}"
        #INCLUDE_LABEL ${label}
        PARALLEL_LEVEL ${CTEST_PARALLEL_LEVEL}
        EXCLUDE ${TEST_TO_EXCLUDE_REGEX})
      # runs only tests that have a LABELS property matching "${label}"
      if(run_ctest_submit)
        ctest_submit(PARTS Test)
      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Global coverage ...
    #-----------------------------------------------------------------------------
    if(run_ctest_with_coverage)
      # HACK Unfortunately ctest_coverage ignores the BUILD argument, try to force it...
      file(READ ${slicer_build_dir}/CMakeFiles/TargetDirectories.txt slicer_build_coverage_dirs)
      file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${slicer_build_coverage_dirs}")

      if(WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
        message("----------- [ Global coverage ] -----------")
        ctest_coverage(BUILD "${slicer_build_dir}")
        if(run_ctest_submit)
          ctest_submit(PARTS Coverage)
        endif()
      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Global dynamic analysis ...
    #-----------------------------------------------------------------------------
    if(WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND AND run_ctest_with_memcheck)
        message("----------- [ Global memcheck ] -----------")
        ctest_memcheck(BUILD "${slicer_build_dir}")
        if(run_ctest_submit)
          ctest_submit(PARTS MemCheck)
        endif()
    endif()

    #-----------------------------------------------------------------------------
    # Package and upload
    #-----------------------------------------------------------------------------
    if(WITH_PACKAGES AND (run_ctest_with_packages OR run_ctest_with_upload))
      message("----------- [ WITH_PACKAGES and UPLOAD ] -----------")

      if(build_errors GREATER "0")
        message("Build Errors Detected: ${build_errors}. Aborting package generation")
      else()

        # Update CMake module path so that our custom macros/functions can be included.
        set(CMAKE_MODULE_PATH ${CTEST_SOURCE_DIRECTORY}/CMake ${CMAKE_MODULE_PATH})

        include(MIDASCTestUploadURL)

        message("Packaging and uploading Slicer to midas ...")
        set(package_list)
        if(run_ctest_with_packages)
          ctest_build(
            TARGET packageupload
            BUILD ${slicer_build_dir}
            APPEND
            )
          ctest_submit(PARTS Build)
        endif()

        if(run_ctest_with_upload)
          message("Uploading Slicer package URL ...")

          file(STRINGS ${slicer_build_dir}/PACKAGES.txt package_list)

          foreach(p ${package_list})
            get_filename_component(package_name "${p}" NAME)
            message("Uploading URL to [${package_name}] on CDash")
            midas_ctest_upload_url(
              API_URL ${MIDAS_PACKAGE_URL}
              FILEPATH ${p}
              )
            if(run_ctest_submit)
              ctest_submit(PARTS Upload)
            endif()
          endforeach()
        endif()

      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Note should be at the end
    #-----------------------------------------------------------------------------
    if(run_ctest_with_notes AND run_ctest_submit)
      ctest_submit(PARTS Notes)
    endif()

  else()
    file(REMOVE ${build_in_progress_file})
  endif()
endmacro()

if(SCRIPT_MODE STREQUAL "continuous" AND ${CTEST_CONTINUOUS_DURATION} GREATER 0)
  while(${CTEST_ELAPSED_TIME} LESS ${CTEST_CONTINUOUS_DURATION})
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

