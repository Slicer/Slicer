

#
# Included from a dashboard script, this cmake file will drive the configure and build
# steps of Slicer
#

#
# SCRIPT_MODE:
#
#   Experimental:
#     - run_ctest() macro will be called *ONE* time
#     - binary directory will *NOT* be cleaned
#   Continuous:
#     - run_ctest() macro will be called EVERY 5 minutes ...
#     - binary directory will *NOT* be cleaned
#     - configure/build will be executed *ONLY* if the repository has been updated
#   Nightly:
#     - run_ctest() macro will be called *ONE* time
#     - binary directory *WILL BE* cleaned
#

# The following variables are expected to be defined in the top-level script:
set(expected_variables
  SCRIPT_MODE
  CTEST_BUILD_CONFIGURATION
  WITH_MEMCHECK
  WITH_COVERAGE
  WITH_DOCUMENTATION
  ADDITIONAL_CMAKECACHE_OPTION
  CTEST_CMAKE_GENERATOR
  CTEST_BUILD_FLAGS
  TEST_TO_EXCLUDE_REGEX
  )

# Update list of expected variables based on build options.
if(WITH_COVERAGE)
  list(APPEND expected_variables CTEST_COVERAGE_COMMAND)
endif()
if(WITH_MEMCHECK)
  list(APPEND expected_variables CTEST_MEMORYCHECK_COMMAND)
endif()

#-----------------------------------------------------------------------------
# Defaults
#-----------------------------------------------------------------------------
if(DEFINED Slicer_RELEASE_TYPE)
  if("${Slicer_RELEASE_TYPE}" MATCHES "^E.*")
    set(Slicer_RELEASE_TYPE "Experimental")
  elseif("${Slicer_RELEASE_TYPE}" MATCHES "^P.*")
    set(Slicer_RELEASE_TYPE "Preview")
  elseif("${Slicer_RELEASE_TYPE}" MATCHES "^S.*")
    set(Slicer_RELEASE_TYPE "Stable")
  else()
    message(FATAL_ERROR "Invalid value for Slicer_RELEASE_TYPE [${Slicer_RELEASE_TYPE}]")
  endif()
else()
  set(Slicer_RELEASE_TYPE "Experimental")
endif()

if(WIN32)
  # By default, build a console application so that console output can be capture
  # by the launcher; but make the launcher a non-console application application
  # when creating packages to avoid popping up a console window when running Slicer.
  if(NOT DEFINED Slicer_BUILD_WIN32_CONSOLE)
    set(Slicer_BUILD_WIN32_CONSOLE ON)
  endif()
  list(APPEND expected_variables Slicer_BUILD_WIN32_CONSOLE)
  if(NOT DEFINED Slicer_BUILD_WIN32_CONSOLE_LAUNCHER)
    if(WITH_PACKAGES)
      set(Slicer_BUILD_WIN32_CONSOLE_LAUNCHER OFF)
    else()
      set(Slicer_BUILD_WIN32_CONSOLE_LAUNCHER ON)
    endif()
  endif()
  list(APPEND expected_variables Slicer_BUILD_WIN32_CONSOLE_LAUNCHER)
endif()

if(NOT DEFINED Slicer_USE_VTK_DEBUG_LEAKS)
  list(APPEND expected_variables Slicer_RELEASE_TYPE)
  set(Slicer_USE_VTK_DEBUG_LEAKS ON)
  if("${Slicer_RELEASE_TYPE}" STREQUAL "Stable")
    set(Slicer_USE_VTK_DEBUG_LEAKS OFF)
  endif()
endif()
list(APPEND expected_variables Slicer_USE_VTK_DEBUG_LEAKS)

if(NOT DEFINED BITNESS)
  set(BITNESS "64")
endif()
list(APPEND expected_variables BITNESS)

#-----------------------------------------------------------------------------
# Set CTEST_BUILD_NAME
#-----------------------------------------------------------------------------
if(NOT DEFINED CTEST_BUILD_NAME)
  list(APPEND expected_variables
    OPERATING_SYSTEM
    COMPILER
    QT_VERSION
    Slicer_USE_PYTHONQT
    Slicer_BUILD_CLI
    Slicer_USE_VTK_DEBUG_LEAKS
    BUILD_NAME_SUFFIX
    )
  set(name "${OPERATING_SYSTEM}-${COMPILER}-${BITNESS}bits-Qt${QT_VERSION}")
  if(NOT Slicer_USE_PYTHONQT)
    set(name "${name}-NoPython")
  endif()
  if(NOT Slicer_BUILD_CLI)
    set(name "${name}-NoCLI")
  endif()
  if(WIN32)
    if(NOT Slicer_BUILD_WIN32_CONSOLE_LAUNCHER)
      set(name "${name}-NoConsole")
    endif()
  endif()
  if(NOT Slicer_USE_VTK_DEBUG_LEAKS)
    set(name "${name}-NoVTKDebugLeaks")
  endif()
  if(NOT "${BUILD_NAME_SUFFIX}" STREQUAL "")
    set(name "${name}-${BUILD_NAME_SUFFIX}")
  endif()
  set(CTEST_BUILD_NAME "${name}-${CTEST_BUILD_CONFIGURATION}")
endif()
list(APPEND expected_variables CTEST_BUILD_NAME)

#-----------------------------------------------------------------------------
# Set CTEST_SITE
#-----------------------------------------------------------------------------
if(NOT DEFINED CTEST_SITE)
  list(APPEND expected_variables
    HOSTNAME
    ORGANIZATION
    )
  string(TOLOWER ${ORGANIZATION} ORGANIZATION_LC)
  set(CTEST_SITE "${HOSTNAME}.${ORGANIZATION_LC}")
endif()
list(APPEND expected_variables CTEST_SITE)

#-----------------------------------------------------------------------------
# Build and source directories
#-----------------------------------------------------------------------------
if(NOT DEFINED CTEST_SOURCE_DIRECTORY OR NOT DEFINED CTEST_BINARY_DIRECTORY)
  list(APPEND expected_variables
    DASHBOARDS_DIR
    Slicer_DASHBOARD_SUBDIR
    Slicer_DIRECTORY_BASENAME
    Slicer_DIRECTORY_IDENTIFIER
    )
  if(NOT DEFINED CTEST_SOURCE_DIRECTORY)
    set(CTEST_SOURCE_DIRECTORY "${DASHBOARDS_DIR}/${Slicer_DASHBOARD_SUBDIR}/${Slicer_DIRECTORY_BASENAME}-${Slicer_DIRECTORY_IDENTIFIER}")
  endif()
  if(NOT DEFINED CTEST_BINARY_DIRECTORY)
    set(CTEST_BINARY_DIRECTORY "${DASHBOARDS_DIR}/${Slicer_DASHBOARD_SUBDIR}/${Slicer_DIRECTORY_BASENAME}-${Slicer_DIRECTORY_IDENTIFIER}-build")
  endif()
endif()
list(APPEND expected_variables CTEST_SOURCE_DIRECTORY)
list(APPEND expected_variables CTEST_BINARY_DIRECTORY)

file(WRITE "${CTEST_BINARY_DIRECTORY} - ${CTEST_BUILD_NAME}-${CTEST_BUILD_CONFIGURATION}-${SCRIPT_MODE}.txt" "Generated by ${CTEST_SCRIPT_NAME}")

#-----------------------------------------------------------------------------
# Sanity check
#-----------------------------------------------------------------------------
foreach(var ${expected_variables})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} should be defined in top-level script !")
  endif()
endforeach()

# Collect variables to display
set(variables ${expected_variables})

#-----------------------------------------------------------------------------
# Handle Qt configuration
#-----------------------------------------------------------------------------
if(NOT DEFINED Qt5_DIR)
  message(FATAL_ERROR "Qt5_DIR should be defined in top-level script")
endif()
if(DEFINED QT_QMAKE_EXECUTABLE)
  message(FATAL_ERROR "Setting QT_QMAKE_EXECUTABLE in top-level script is specific to Qt4 and is not supported anymore.")
endif()
if(DEFINED Qt5_DIR)
  set(QT_CACHE_ENTRY "Qt5_DIR:PATH=${Qt5_DIR}")
  list(APPEND variables Qt5_DIR)
endif()

#-----------------------------------------------------------------------------
if(NOT DEFINED CTEST_CONFIGURATION_TYPE AND DEFINED CTEST_BUILD_CONFIGURATION)
  set(CTEST_CONFIGURATION_TYPE ${CTEST_BUILD_CONFIGURATION})
endif()
list(APPEND variables CTEST_CONFIGURATION_TYPE)

#-----------------------------------------------------------------------------
# Append top-level script as a CDash note
list(APPEND CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

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
setIfNotDefined(CTEST_TEST_TIMEOUT 900) # 15mins
setIfNotDefined(CTEST_PARALLEL_LEVEL 8)
setIfNotDefined(CTEST_CONTINUOUS_DURATION 46800) # Lasts 13 hours (Assuming it starts at 9am, it will end around 10pm)
setIfNotDefined(SLICER_PACKAGE_MANAGER_URL "https://slicer-packages.kitware.com")
if(WITH_DOCUMENTATION)
  setIfNotDefined(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY "$ENV{HOME}/Projects/Doxygen")
endif()

#-----------------------------------------------------------------------------
# The following variable can be used to disable specific steps
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
# CDash Project Name
#-----------------------------------------------------------------------------
if(NOT DEFINED CDASH_PROJECT_NAME)
  set(CDASH_PROJECT_NAME  "SlicerPreview")
  if("${Slicer_RELEASE_TYPE}" STREQUAL "Stable")
    set(CDASH_PROJECT_NAME  "SlicerStable")
  endif()
endif()
list(APPEND variables CDASH_PROJECT_NAME)

#-----------------------------------------------------------------------------
if(NOT DEFINED GIT_REPOSITORY)
  set(GIT_REPOSITORY "https://github.com/Slicer/Slicer")
endif()
if(NOT DEFINED GIT_TAG)
  set(GIT_TAG "master")
endif()
list(APPEND variables GIT_REPOSITORY GIT_TAG)

#-----------------------------------------------------------------------------
# Required executables
#-----------------------------------------------------------------------------
if(NOT DEFINED CTEST_GIT_COMMAND)
  find_program(CTEST_GIT_COMMAND NAMES git)
endif()
if(NOT EXISTS "${CTEST_GIT_COMMAND}")
  message(FATAL_ERROR "CTEST_GIT_COMMAND is set to a non-existent path [${CTEST_GIT_COMMAND}]")
endif()
message(STATUS "CTEST_GIT_COMMAND: ${CTEST_GIT_COMMAND}")

#-----------------------------------------------------------------------------
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

#-----------------------------------------------------------------------------
# Given a variable name, this function will display the text
#   "-- <varname> ................: ${<varname>}"
# and will ensure that the message is consistently padded.
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

#-----------------------------------------------------------------------------
# Environment
#-----------------------------------------------------------------------------

# This will ensure compiler paths specified using the cache variables are used
# consistently.
if(DEFINED CMAKE_C_COMPILER)
  set(ENV{CC} "/dev/null")
  message(STATUS "Setting ENV{CC} to $ENV{CC}")
endif()
if(DEFINED CMAKE_CXX_COMPILER)
  set(ENV{CXX} "/dev/null")
  message(STATUS "Setting ENV{CXX} to $ENV{CXX}")
endif()

if(UNIX)
  if("$ENV{DISPLAY}" STREQUAL "")
    set(ENV{DISPLAY} ":0")
  endif()
  message(STATUS "Setting ENV{DISPLAY} to $ENV{DISPLAY}")
endif()

# Cache for External data downloads
if("$ENV{ExternalData_OBJECT_STORES}" STREQUAL "")
  set(ENV{ExternalData_OBJECT_STORES} "$ENV{HOME}/.ExternalData")
endif()
message(STATUS "Setting ENV{ExternalData_OBJECT_STORES} to $ENV{ExternalData_OBJECT_STORES}")

#-----------------------------------------------------------------------------
# Cleanup previous dashboard
#-----------------------------------------------------------------------------
if(empty_binary_directory AND NOT run_ctest_with_disable_clean)
  set(msg "Removing binary directory [${CTEST_BINARY_DIRECTORY}]")
  message(STATUS "${msg}")
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
  message(STATUS "${msg} - done")
endif()

#-----------------------------------------------------------------------------
# Source code checkout and update commands
#-----------------------------------------------------------------------------
if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone -b ${GIT_TAG} ${GIT_REPOSITORY} ${CTEST_SOURCE_DIRECTORY}")
endif()
set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")

#-----------------------------------------------------------------------------
# run_ctest macro
#-----------------------------------------------------------------------------
macro(run_ctest)
  ctest_start(${model} TRACK ${track})

  set(build_in_progress_file ${CTEST_BINARY_DIRECTORY}/Slicer-build/BUILD_IN_PROGRESS)
  file(WRITE ${build_in_progress_file} "Generated by ${CMAKE_CURRENT_LIST_FILE}\n")

  if(run_ctest_with_update)
    ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE FILES_UPDATED)
  endif()

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message(STATUS "First time build - Initialize CMakeCache.txt")
    set(force_build TRUE)

    set(OPTIONAL_CACHE_CONTENT)

    if(DEFINED CMAKE_C_COMPILER)
      set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
CMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}")
    endif()

    if(DEFINED CMAKE_CXX_COMPILER)
      set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
CMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}")
    endif()

    if(DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
      set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
CMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()

    if(DEFINED Slicer_BUILD_WIN32_CONSOLE)
      set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
Slicer_BUILD_WIN32_CONSOLE:BOOL=${Slicer_BUILD_WIN32_CONSOLE}")
    endif()

    if(DEFINED Slicer_BUILD_WIN32_CONSOLE_LAUNCHER)
    set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
Slicer_BUILD_WIN32_CONSOLE_LAUNCHER:BOOL=${Slicer_BUILD_WIN32_CONSOLE_LAUNCHER}")
  endif()

    if(DEFINED Slicer_USE_PYTHONQT)
      set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
Slicer_USE_PYTHONQT:BOOL=${Slicer_USE_PYTHONQT}")
    endif()

    if(DEFINED Slicer_BUILD_CLI)
      set(OPTIONAL_CACHE_CONTENT "${OPTIONAL_CACHE_CONTENT}
Slicer_BUILD_CLI:BOOL=${Slicer_BUILD_CLI}")
    endif()

    #-----------------------------------------------------------------------------
    # Write initial cache.
    #-----------------------------------------------------------------------------
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
${QT_CACHE_ENTRY}
CDASH_PROJECT_NAME:STRING=${CDASH_PROJECT_NAME}
GIT_EXECUTABLE:FILEPATH=${CTEST_GIT_COMMAND}
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
DOCUMENTATION_TARGET_IN_ALL:BOOL=${WITH_DOCUMENTATION}
DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
Slicer_RELEASE_TYPE:STRING=${Slicer_RELEASE_TYPE}
Slicer_USE_VTK_DEBUG_LEAKS:BOOL=${Slicer_USE_VTK_DEBUG_LEAKS}
${OPTIONAL_CACHE_CONTENT}
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
    # Package and upload
    #-----------------------------------------------------------------------------
    if(WITH_PACKAGES AND (run_ctest_with_packages OR run_ctest_with_upload))
      message(STATUS "----------- [ WITH_PACKAGES and UPLOAD ] -----------")

      if(build_errors GREATER "0")
        message("Build Errors Detected: ${build_errors}. Aborting package generation")
      else()

        # Update CMake module path so that our custom macros/functions can be included.
        set(CMAKE_MODULE_PATH ${CTEST_SOURCE_DIRECTORY}/CMake ${CMAKE_MODULE_PATH})

        include(SlicerCTestUploadURL)

        message(STATUS "Packaging and uploading Slicer to packages server ...")
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
          message(STATUS "Uploading Slicer package URL ...")

          file(STRINGS ${slicer_build_dir}/PACKAGES.txt package_list)

          foreach(p ${package_list})
            get_filename_component(package_name "${p}" NAME)
            message(STATUS "Uploading URL to [${package_name}] on CDash")
            slicer_ctest_upload_url(
              ALGO "SHA512"
              DOWNLOAD_URL_TEMPLATE "${SLICER_PACKAGE_MANAGER_URL}/api/v1/file/hashsum/%(algo)/%(hash)/download"
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
    # Test
    #-----------------------------------------------------------------------------
    if(run_ctest_with_test)
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
    if(WITH_COVERAGE AND CTEST_COVERAGE_COMMAND AND run_ctest_with_coverage)
      # HACK Unfortunately ctest_coverage ignores the BUILD argument, try to force it...
      if(EXISTS ${slicer_build_dir}/CMakeFiles/TargetDirectories.txt)
        file(READ ${slicer_build_dir}/CMakeFiles/TargetDirectories.txt slicer_build_coverage_dirs)
        file(APPEND "${CTEST_BINARY_DIRECTORY}/CMakeFiles/TargetDirectories.txt" "${slicer_build_coverage_dirs}")
      endif()
      ctest_coverage(BUILD "${slicer_build_dir}")
      if(run_ctest_submit)
        ctest_submit(PARTS Coverage)
      endif()
    endif()

    #-----------------------------------------------------------------------------
    # Global dynamic analysis ...
    #-----------------------------------------------------------------------------
    if(WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND AND run_ctest_with_memcheck)
        ctest_memcheck(BUILD "${slicer_build_dir}")
        if(run_ctest_submit)
          ctest_submit(PARTS MemCheck)
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

#-----------------------------------------------------------------------------
# Dashboard execution
#-----------------------------------------------------------------------------

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

