

#
# Included from a dashboard script, this cmake file will drive the configure and build
# steps of Slicer
#

# The following variable are expected to be define in the top-level script:
set(expected_variables
  ADDITIONAL_CMAKECACHE_OPTION
  CTEST_NOTES_FILES
  CTEST_SITE
  CTEST_DASHBOARD_ROOT
  CTEST_CMAKE_COMMAND
  CTEST_CMAKE_GENERATOR
  WITH_MEMCHECK
  WITH_COVERAGE
  WITH_DOCUMENTATION
  CTEST_BUILD_CONFIGURATION
  CTEST_TEST_TIMEOUT
  CTEST_BUILD_FLAGS
  TEST_TO_EXCLUDE_REGEX
  CTEST_PROJECT_NAME
  CTEST_SOURCE_DIRECTORY
  CTEST_BINARY_DIRECTORY
  CTEST_BUILD_NAME
  SCRIPT_MODE
  CTEST_COVERAGE_COMMAND
  CTEST_MEMORYCHECK_COMMAND
  CTEST_SVN_COMMAND
  CTEST_GIT_COMMAND
  QT_QMAKE_EXECUTABLE
  )

if(WITH_DOCUMENTATION)
  list(APPEND expected_variables DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)
endif()
if(NOT DEFINED CTEST_PARALLEL_LEVEL)
  set(CTEST_PARALLEL_LEVEL 8)
endif()
if(NOT DEFINED WITH_EXTENSIONS)
  set(WITH_EXTENSIONS OFF)
endif()

if(NOT DEFINED MIDAS_PACKAGE_URL)
  set(MIDAS_PACKAGE_URL "http://tarsonis.kitwarein.com/Midas3")
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

# Make sure command 'ctest_upload' is available if WITH_PACKAGES is True
if(WITH_PACKAGES)
  if(NOT COMMAND ctest_upload)
    message(FATAL_ERROR "Failed to enable option WITH_PACKAGES ! CMake ${CMAKE_VERSION} doesn't support 'ctest_upload' command.")
  endif()
endif()

set(repository http://svn.slicer.org/Slicer4/trunk)

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
set(track ${model})
if(WITH_PACKAGES)
  set(track "${track}-Packages")
endif()
set(track ${CTEST_TRACK_PREFIX}${track}${CTEST_TRACK_SUFFIX})

# For more details, see http://www.kitware.com/blog/home/post/11
set(CTEST_USE_LAUNCHERS 0)
if(CTEST_CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(CTEST_USE_LAUNCHERS 1)
endif()
#if(NOT ${CTEST_CMAKE_GENERATOR} MATCHES "Visual Studio")
#  set(CTEST_USE_LAUNCHERS 1)
#endif()

if(empty_binary_directory)
  message("Directory ${CTEST_BINARY_DIRECTORY} cleaned !")
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
endif()

if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  set(CTEST_CHECKOUT_COMMAND "${CTEST_SVN_COMMAND} checkout ${repository} ${CTEST_SOURCE_DIRECTORY}")
endif()

set(CTEST_UPDATE_COMMAND "${CTEST_SVN_COMMAND}")

set(CTEST_SOURCE_DIRECTORY "${CTEST_SOURCE_DIRECTORY}")

#-----------------------------------------------------------------------------
# Macro allowing to set a variable to its default value only if not already defined
macro(setIfNotDefined var defaultvalue)
  if(NOT DEFINED ${var})
    set(${var} "${defaultvalue}")
  endif()
endmacro()

#-----------------------------------------------------------------------------
# The following variable can be used while testing the driver scripts
#-----------------------------------------------------------------------------
setIfNotDefined(run_ctest_submit TRUE)
setIfNotDefined(run_ctest_with_update TRUE)
setIfNotDefined(run_ctest_with_configure TRUE)
setIfNotDefined(run_ctest_with_build TRUE)
setIfNotDefined(run_ctest_with_test TRUE)
setIfNotDefined(run_ctest_with_coverage TRUE)
setIfNotDefined(run_ctest_with_memcheck TRUE)
setIfNotDefined(run_ctest_with_packages TRUE)
setIfNotDefined(run_ctest_with_upload TRUE)
setIfNotDefined(run_ctest_with_notes TRUE)

#
# run_ctest macro
#
macro(run_ctest)
  ctest_start(${model} TRACK ${track})

  if(run_ctest_with_update)
    ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE FILES_UPDATED)
  endif()

  # force a build if this is the first run and the build dir is empty
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    message("First time build - Initialize CMakeCache.txt")
    set(force_build TRUE)

    if(WITH_EXTENSIONS)
      set(ADDITIONAL_CMAKECACHE_OPTION
        "${ADDITIONAL_CMAKECACHE_OPTION} CTEST_MODEL:STRING=${model}")
    endif()

    #-----------------------------------------------------------------------------
    # Write initial cache.
    #-----------------------------------------------------------------------------
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
QT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
GIT_EXECUTABLE:FILEPATH=${CTEST_GIT_COMMAND}
Subversion_SVN_EXECUTABLE:FILEPATH=${CTEST_SVN_COMMAND}
WITH_COVERAGE:BOOL=${WITH_COVERAGE}
DOCUMENTATION_TARGET_IN_ALL:BOOL=${WITH_DOCUMENTATION}
DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
Slicer_BUILD_EXTENSIONS:BOOL=${WITH_EXTENSIONS}
Slicer_UPLOAD_EXTENSIONS:BOOL=${WITH_EXTENSIONS}
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

      set_property(GLOBAL PROPERTY SubProject ${label})
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
    # Create packages / installers ...
    #-----------------------------------------------------------------------------
    if(WITH_PACKAGES AND (run_ctest_with_packages OR run_ctest_with_upload))
      message("----------- [ WITH_PACKAGES and UPLOAD ] -----------")

      if(build_errors GREATER "0")
        message("Build Errors Detected: ${build_errors}. Aborting package generation")
      else()

        if(MY_BITNESS EQUAL 32)
          set(dashboard_Architecture "i386")
        else()
          set(dashboard_Architecture "amd64")
        endif()

        if(WIN32)
          set(dashboard_OperatingSystem "win")
        elseif(APPLE)
          set(dashboard_OperatingSystem "macosx")
        elseif(UNIX)
          set(dashboard_OperatingSystem "linux")
        endif()

        #-----------------------------------------------------------------------------
        # Build and upload Slicer packages
        #-----------------------------------------------------------------------------

        # Update CMake module path so that in addition to our custom 'FindGit' module,
        # our custom macros/functions can also be included.
        set(CMAKE_MODULE_PATH ${CTEST_SOURCE_DIRECTORY}/CMake ${CMAKE_MODULE_PATH})

        include(CTestPackage)
        include(MIDASAPIUploadPackage)
        include(MIDASCTestUploadURL)
        include(SlicerMacroExtractRepositoryInfo)

        SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer SOURCE_DIR ${CTEST_SOURCE_DIRECTORY})

        set(packages)
        if(run_ctest_with_packages)
          message("Packaging ...")
          ctest_package(
            BINARY_DIR ${slicer_build_dir}
            CONFIG ${CTEST_BUILD_CONFIGURATION}
            RETURN_VAR packages)
        else()
          set(packages ${CMAKE_CURRENT_LIST_FILE})
        endif()
        if(run_ctest_with_upload)
          message("Uploading ...")
          foreach(p ${packages})
            get_filename_component(package_name "${p}" NAME)
            set(midas_upload_status "fail")
            if(DEFINED MIDAS_PACKAGE_URL
               AND DEFINED MIDAS_PACKAGE_EMAIL
               AND DEFINED MIDAS_PACKAGE_API_KEY)
              message("Uploading [${package_name}] on [${MIDAS_PACKAGE_URL}]")
              midas_api_upload_package(
                SERVER_URL ${MIDAS_PACKAGE_URL}
                SERVER_EMAIL ${MIDAS_PACKAGE_EMAIL}
                SERVER_APIKEY ${MIDAS_PACKAGE_API_KEY}
                SUBMISSION_TYPE ${SCRIPT_MODE}
                SOURCE_REVISION ${Slicer_WC_REVISION}
                SOURCE_CHECKOUTDATE ${Slicer_WC_LAST_CHANGED_DATE}
                OPERATING_SYSTEM ${dashboard_OperatingSystem}
                ARCHITECTURE ${dashboard_Architecture}
                PACKAGE_FILEPATH ${p}
                PACKAGE_TYPE "installer"
                RESULT_VARNAME midas_upload_status
                )
            endif()
            if(NOT midas_upload_status STREQUAL "ok")
              message("Uploading [${package_name}] on CDash") # On failure, upload the package to CDash instead
              ctest_upload(FILES ${p})
            else()
              message("Uploading URL on CDash")  # On success, upload a link to CDash
              midas_ctest_upload_url(
                API_URL ${MIDAS_PACKAGE_URL}
                FILEPATH ${p}
                )
            endif()
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

  endif()
endmacro()

if(SCRIPT_MODE STREQUAL "continuous")
  while(${CTEST_ELAPSED_TIME} LESS 68400)
    set(START_TIME ${CTEST_ELAPSED_TIME})
    run_ctest()
    # Loop no faster than once every 5 minutes
    message("Wait for 5 minutes ...")
    ctest_sleep(${START_TIME} 300 ${CTEST_ELAPSED_TIME})
  endwhile()
else()
  run_ctest()
endif()

