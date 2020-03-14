################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin and Johan Andruejol, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################


macro(slicerMacroBuildAppLibrary)
  set(options
    WRAP_PYTHONQT
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE
    FOLDER
    APPLICATION_NAME
    DESCRIPTION_SUMMARY
    DESCRIPTION_FILE
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    RESOURCES
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    )
  cmake_parse_arguments(SLICERAPPLIB
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(SLICERAPPLIB_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SlicerMacroBuildAppLibrary(): \"${SLICERAPPLIB_UNPARSED_ARGUMENTS}\"")
  endif()

  set(expected_defined_vars NAME EXPORT_DIRECTIVE DESCRIPTION_SUMMARY)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED SLICERAPPLIB_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  set(expected_existing_vars DESCRIPTION_FILE)
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${SLICERAPPLIB_${var}}")
      message(FATAL_ERROR "error: Variable ${var} set to ${SLICERAPPLIB_${var}} corresponds to an nonexistent file. ")
    endif()
  endforeach()

  if(NOT DEFINED Slicer_INSTALL_NO_DEVELOPMENT)
    message(SEND_ERROR "Slicer_INSTALL_NO_DEVELOPMENT is mandatory")
  endif()

  if(NOT DEFINED SLICERAPPLIB_APPLICATION_NAME)
    set(SLICERAPPLIB_APPLICATION_NAME ${SLICERAPPLIB_NAME})
  endif()

  message(STATUS "--------------------------------------------------")
  message(STATUS "Configuring ${SLICERAPPLIB_APPLICATION_NAME} application library: ${SLICERAPPLIB_NAME}")
  message(STATUS "--------------------------------------------------")

  macro(_set_applib_property varname)
    set_property(GLOBAL PROPERTY ${SLICERAPPLIB_APPLICATION_NAME}_${varname} ${SLICERAPPLIB_${varname}})
    message(STATUS "Setting ${SLICERAPPLIB_APPLICATION_NAME} ${varname} to '${SLICERAPPLIB_${varname}}'")
  endmacro()

  _set_applib_property(DESCRIPTION_SUMMARY)
  _set_applib_property(DESCRIPTION_FILE)

  # --------------------------------------------------------------------------
  # Define library name
  # --------------------------------------------------------------------------
  set(lib_name ${SLICERAPPLIB_NAME})

  # --------------------------------------------------------------------------
  # Folder
  # --------------------------------------------------------------------------
  if(NOT DEFINED SLICERAPPLIB_FOLDER)
    set(SLICERAPPLIB_FOLDER "App-${SLICERAPPLIB_APPLICATION_NAME}")
  endif()

  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------

  set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer_Base_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${MRMLLogic_INCLUDE_DIRS}
    ${qMRMLWidgets_INCLUDE_DIRS}
    ${qSlicerModulesCore_SOURCE_DIR}
    ${qSlicerModulesCore_BINARY_DIR}
    ${ITKFactoryRegistration_INCLUDE_DIRS}
    ${SLICERAPPLIB_INCLUDE_DIRECTORIES}
    )

  include_directories(${include_dirs})

  #-----------------------------------------------------------------------------
  # Update Slicer_Base_INCLUDE_DIRS
  #-----------------------------------------------------------------------------

  # NA

  #-----------------------------------------------------------------------------
  # Configure
  # --------------------------------------------------------------------------
  set(MY_LIBRARY_EXPORT_DIRECTIVE ${SLICERAPPLIB_EXPORT_DIRECTIVE})
  set(MY_EXPORT_HEADER_PREFIX ${SLICERAPPLIB_NAME})
  set(MY_LIBNAME ${lib_name})

  configure_file(
    ${Slicer_SOURCE_DIR}/CMake/qSlicerExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  set(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

  #-----------------------------------------------------------------------------
  # Sources
  # --------------------------------------------------------------------------
    set(_moc_options OPTIONS -DSlicer_HAVE_QT5)
    QT5_WRAP_CPP(SLICERAPPLIB_MOC_OUTPUT ${SLICERAPPLIB_MOC_SRCS} ${_moc_options})
    QT5_WRAP_UI(SLICERAPPLIB_UI_CXX ${SLICERAPPLIB_UI_SRCS})
    if(DEFINED SLICERAPPLIB_RESOURCES)
      QT5_ADD_RESOURCES(SLICERAPPLIB_QRC_SRCS ${SLICERAPPLIB_RESOURCES})
    endif()

  set_source_files_properties(
    ${SLICERAPPLIB_UI_CXX}
    ${SLICERAPPLIB_MOC_OUTPUT}
    ${SLICERAPPLIB_QRC_SRCS}
    WRAP_EXCLUDE
    )

  # --------------------------------------------------------------------------
  # Source groups
  # --------------------------------------------------------------------------
  source_group("Resources" FILES
    ${SLICERAPPLIB_UI_SRCS}
    ${Slicer_SOURCE_DIR}/Resources/qSlicer.qrc
    ${SLICERAPPLIB_RESOURCES}
  )

  source_group("Generated" FILES
    ${SLICERAPPLIB_UI_CXX}
    ${SLICERAPPLIB_MOC_OUTPUT}
    ${SLICERAPPLIB_QRC_SRCS}
    ${dynamicHeaders}
  )

  # --------------------------------------------------------------------------
  # Translation
  # --------------------------------------------------------------------------
  if(Slicer_BUILD_I18N_SUPPORT)
    set(TS_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Translations/"
    )
    get_property(Slicer_LANGUAGES GLOBAL PROPERTY Slicer_LANGUAGES)

    include(SlicerMacroTranslation)
    SlicerMacroTranslation(
      SRCS ${SLICERAPPLIB_SRCS}
      UI_SRCS ${SLICERAPPLIB_UI_SRCS}
      TS_DIR ${TS_DIR}
      TS_BASEFILENAME ${SLICERAPPLIB_NAME}
      TS_LANGUAGES ${Slicer_LANGUAGES}
      QM_OUTPUT_DIR_VAR QM_OUTPUT_DIR
      QM_OUTPUT_FILES_VAR QM_OUTPUT_FILES
      )

    set_property(GLOBAL APPEND PROPERTY Slicer_QM_OUTPUT_DIRS ${QM_OUTPUT_DIR})
  else()
    set(QM_OUTPUT_FILES )
  endif()

  # --------------------------------------------------------------------------
  # Build the library
  # --------------------------------------------------------------------------
  add_library(${lib_name}
    ${SLICERAPPLIB_SRCS}
    ${SLICERAPPLIB_MOC_OUTPUT}
    ${SLICERAPPLIB_UI_CXX}
    ${SLICERAPPLIB_QRC_SRCS}
    ${QM_OUTPUT_FILES}
    )
  set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})

  # Apply user-defined properties to the library target.
  if(Slicer_LIBRARY_PROPERTIES)
    set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  endif()

  target_link_libraries(${lib_name}
    qSlicerBaseQTApp
    ${SLICERAPPLIB_TARGET_LIBRARIES}
    )

  # Folder
  set_target_properties(${lib_name} PROPERTIES FOLDER ${SLICERAPPLIB_FOLDER})

  #-----------------------------------------------------------------------------
  # Install library
  #-----------------------------------------------------------------------------
  install(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

  # --------------------------------------------------------------------------
  # Install headers
  # --------------------------------------------------------------------------
  if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
    # Install headers
    file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    install(FILES
      ${headers}
      ${dynamicHeaders}
      DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${lib_name} COMPONENT Development
      )
  endif()

  # --------------------------------------------------------------------------
  # PythonQt wrapping
  # --------------------------------------------------------------------------
  if(Slicer_USE_PYTHONQT AND SLICERAPPLIB_WRAP_PYTHONQT)
    ctkMacroBuildLibWrapper(
      NAMESPACE "osa" # Use "osa" instead of "org.slicer.app" to avoid build error on windows
      TARGET ${lib_name}
      SRCS "${SLICERAPPLIB_SRCS}"
      INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
      INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
      )
    set_target_properties(${lib_name}PythonQt PROPERTIES FOLDER ${SLICERAPPLIB_FOLDER})
  endif()

  # --------------------------------------------------------------------------
  # Export target
  # --------------------------------------------------------------------------
  set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${SLICERAPPLIB_NAME})

endmacro()


#
# slicerMacroBuildApplication
#

macro(slicerMacroBuildApplication)
  set(options
    CONFIGURE_LAUNCHER
    )
  set(oneValueArgs
    NAME
    FOLDER
    APPLICATION_NAME

    DEFAULT_SETTINGS_FILE
    LAUNCHER_SPLASHSCREEN_FILE
    APPLE_ICON_FILE
    WIN_ICON_FILE
    LICENSE_FILE

    TARGET_NAME_VAR
    )
  set(multiValueArgs
    SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    )
  cmake_parse_arguments(SLICERAPP
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(SLICERAPP_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerMacroBuildApplication(): \"${SLICERAPP_UNPARSED_ARGUMENTS}\"")
  endif()

  set(expected_defined_vars
    NAME
    LAUNCHER_SPLASHSCREEN_FILE
    APPLE_ICON_FILE
    WIN_ICON_FILE
    LICENSE_FILE
    )
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED SLICERAPP_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  # Set defaults
  if(NOT DEFINED SLICERAPP_APPLICATION_NAME)
    string(REGEX REPLACE "(.+)App" "\\1" SLICERAPP_APPLICATION_NAME ${SLICERAPP_NAME})
  endif()

  message(STATUS "--------------------------------------------------")
  message(STATUS "Configuring ${SLICERAPP_APPLICATION_NAME} application: ${SLICERAPP_NAME}")
  message(STATUS "--------------------------------------------------")

  macro(_set_app_property varname)
    set_property(GLOBAL PROPERTY ${SLICERAPP_APPLICATION_NAME}_${varname} ${SLICERAPP_${varname}})
    message(STATUS "Setting ${SLICERAPP_APPLICATION_NAME} ${varname} to '${SLICERAPP_${varname}}'")
  endmacro()

  _set_app_property("APPLICATION_NAME")

  macro(_set_path_var varname)
    if(NOT IS_ABSOLUTE ${SLICERAPP_${varname}})
      set(SLICERAPP_${varname} ${CMAKE_CURRENT_SOURCE_DIR}/${SLICERAPP_${varname}})
    endif()
    if(NOT EXISTS "${SLICERAPP_${varname}}")
      message(FATAL_ERROR "error: Variable ${varname} set to ${SLICERAPP_${varname}} corresponds to an nonexistent file. ")
    endif()
    _set_app_property(${varname})
  endmacro()

  _set_path_var(LAUNCHER_SPLASHSCREEN_FILE)
  _set_path_var(APPLE_ICON_FILE)
  _set_path_var(WIN_ICON_FILE)
  _set_path_var(LICENSE_FILE)
  if(DEFINED SLICERAPP_DEFAULT_SETTINGS_FILE)
    _set_path_var(DEFAULT_SETTINGS_FILE)
  endif()

  # --------------------------------------------------------------------------
  # Folder
  # --------------------------------------------------------------------------
  if(NOT DEFINED SLICERAPP_FOLDER)
    set(SLICERAPP_FOLDER "App-${SLICERAPP_APPLICATION_NAME}")
  endif()

  # --------------------------------------------------------------------------
  # Configure Application Bundle Resources (Mac Only)
  # --------------------------------------------------------------------------

  if(APPLE)
    set(apple_bundle_sources ${SLICERAPP_APPLE_ICON_FILE})
    set_source_files_properties(
      "${apple_bundle_sources}"
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
      )
    get_filename_component(apple_icon_filename ${SLICERAPP_APPLE_ICON_FILE} NAME)
    set(MACOSX_BUNDLE_ICON_FILE ${apple_icon_filename})
    message(STATUS "Setting MACOSX_BUNDLE_ICON_FILE to '${MACOSX_BUNDLE_ICON_FILE}'")
  endif()

  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------

  set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SLICERAPP_INCLUDE_DIRECTORIES}
    )

  include_directories(${include_dirs})

  # --------------------------------------------------------------------------
  # Build the executable
  # --------------------------------------------------------------------------
  set(Slicer_HAS_CONSOLE_IO_SUPPORT TRUE)
  if(WIN32)
    set(Slicer_HAS_CONSOLE_IO_SUPPORT ${Slicer_BUILD_WIN32_CONSOLE})
  endif()

  set(SLICERAPP_EXE_OPTIONS)
  if(WIN32)
    if(NOT Slicer_HAS_CONSOLE_IO_SUPPORT)
      set(SLICERAPP_EXE_OPTIONS WIN32)
    endif()
  endif()

  if(APPLE)
    set(SLICERAPP_EXE_OPTIONS MACOSX_BUNDLE)
  endif()

  set(slicerapp_target ${SLICERAPP_NAME})
  if(DEFINED SLICERAPP_TARGET_NAME_VAR)
    set(${SLICERAPP_TARGET_NAME_VAR} ${slicerapp_target})
  endif()

  set(executable_name ${SLICERAPP_APPLICATION_NAME})
  if(NOT APPLE)
    set(executable_name ${executable_name}App-real)
  endif()
  message(STATUS "Setting ${SLICERAPP_APPLICATION_NAME} executable name to '${executable_name}${CMAKE_EXECUTABLE_SUFFIX}'")

  ctk_add_executable_utf8(${slicerapp_target}
    ${SLICERAPP_EXE_OPTIONS}
    Main.cxx
    ${apple_bundle_sources}
    )
  set_target_properties(${slicerapp_target} PROPERTIES
    LABELS ${SLICERAPP_NAME}
    OUTPUT_NAME ${executable_name}
    )

  if(APPLE)
    set(link_flags "-Wl,-rpath,@loader_path/../")
    set_target_properties(${slicerapp_target}
      PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "${SLICERAPP_APPLICATION_NAME} ${Slicer_MAIN_PROJECT_VERSION_FULL}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${Slicer_MAIN_PROJECT_VERSION_FULL}"
        MACOSX_BUNDLE_INFO_PLIST "${Slicer_CMAKE_DIR}/MacOSXBundleInfo.plist.in"
        LINK_FLAGS ${link_flags}
      )
    if("${Slicer_RELEASE_TYPE}" STREQUAL "Stable")
      set_target_properties(${slicerapp_target} PROPERTIES
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}.${Slicer_VERSION_PATCH}"
        )
    endif()
  endif()

  get_target_property(_slicerapp_output_dir ${slicerapp_target} RUNTIME_OUTPUT_DIRECTORY)
  set(_slicerapp_build_subdir "")

  if(APPLE)
    get_target_property(_is_bundle ${slicerapp_target} MACOSX_BUNDLE)
    if(_is_bundle)
      set(_slicerapp_build_subdir "${executable_name}.app/Contents/MacOS/")
    endif()
  endif()

  set(SLICERAPP_EXECUTABLE "${_slicerapp_output_dir}/${_slicerapp_build_subdir}${executable_name}${CMAKE_EXECUTABLE_SUFFIX}")
  _set_app_property("EXECUTABLE")

  if(WIN32)
    if(Slicer_USE_PYTHONQT)
      # HACK - See http://www.na-mic.org/Bug/view.php?id=1180
      get_filename_component(_python_library_name_we ${PYTHON_LIBRARY} NAME_WE)
      add_custom_command(
        TARGET ${slicerapp_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PYTHON_LIBRARY_PATH}/${_python_library_name_we}.dll
                ${_slicerapp_output_dir}/${CMAKE_CFG_INTDIR}
        COMMENT "Copy '${_python_library_name_we}.dll' along side '${slicerapp_target}' executable. See Slicer issue #1180"
        )
    endif()
  endif()

  if(DEFINED SLICERAPP_TARGET_LIBRARIES)
    target_link_libraries(${slicerapp_target}
      ${SLICERAPP_TARGET_LIBRARIES}
      )
  endif()

  # Folder
  set_target_properties(${slicerapp_target} PROPERTIES FOLDER ${SLICERAPP_FOLDER})

  # --------------------------------------------------------------------------
  # Install
  # --------------------------------------------------------------------------
  if(NOT APPLE)
    set(SLICERAPP_INSTALL_DESTINATION_ARGS RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR})
  else()
    set(SLICERAPP_INSTALL_DESTINATION_ARGS BUNDLE DESTINATION ".")
  endif()

  install(TARGETS ${slicerapp_target}
    ${SLICERAPP_INSTALL_DESTINATION_ARGS}
    COMPONENT Runtime)

  if(DEFINED SLICERAPP_DEFAULT_SETTINGS_FILE)
    get_filename_component(default_settings_filename ${SLICERAPP_DEFAULT_SETTINGS_FILE} NAME)
    set(dest_default_settings_filename ${default_settings_filename})
    if(NOT ${default_settings_filename} MATCHES "^${SLICERAPP_APPLICATION_NAME}")
      set(dest_default_settings_filename ${SLICERAPP_APPLICATION_NAME}${default_settings_filename})
    endif()
    set(default_settings_build_dir ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR})
    message(STATUS "Copying '${default_settings_filename}' to '${default_settings_build_dir}/${dest_default_settings_filename}'")
    configure_file(
      ${SLICERAPP_DEFAULT_SETTINGS_FILE}
      ${default_settings_build_dir}/${dest_default_settings_filename}
      COPYONLY
      )
    install(FILES
      ${SLICERAPP_DEFAULT_SETTINGS_FILE}
      DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Runtime
      RENAME ${dest_default_settings_filename}
      )
  endif()

  # --------------------------------------------------------------------------
  # Configure Launcher
  # --------------------------------------------------------------------------
  if(SLICERAPP_CONFIGURE_LAUNCHER)
    if(Slicer_USE_CTKAPPLAUNCHER)

      find_package(CTKAppLauncher REQUIRED)

      # Define list of extra 'application to launch' to associate with the launcher
      # within the build tree
      set(extraApplicationToLaunchListForBuildTree)

      if(NOT QT_DESIGNER_EXECUTABLE)
        # Since Qt only provides a CMake module to find the designer library, we work
        # around this limitation by finding the designer executable.
        find_program(QT_DESIGNER_EXECUTABLE designer Designer HINTS "${QT_BINARY_DIR}" NO_DEFAULT_PATH)
      endif()

      if(EXISTS ${QT_DESIGNER_EXECUTABLE})
        ctkAppLauncherAppendExtraAppToLaunchToList(
          LONG_ARG designer
          HELP "Start Qt designer using Slicer plugins"
          PATH ${QT_DESIGNER_EXECUTABLE}
          OUTPUTVAR extraApplicationToLaunchListForBuildTree
          )
      endif()
      set(executables)
      if(UNIX)
        list(APPEND executables gnome-terminal xterm)
      elseif(WIN32)
        list(APPEND executables VisualStudio VisualStudioProject cmd)
        set(VisualStudio_EXECUTABLE ${CMAKE_VS_DEVENV_COMMAND})
        set(VisualStudio_HELP "Open Visual Studio with Slicer's DLL paths set up")
        set(VisualStudioProject_EXECUTABLE ${CMAKE_VS_DEVENV_COMMAND})
        set(VisualStudioProject_ARGUMENTS ${Slicer_BINARY_DIR}/Slicer.sln)
        set(VisualStudioProject_HELP "Open Visual Studio Slicer project with Slicer's DLL paths set up")
        set(cmd_ARGUMENTS "/c start cmd")
      endif()
      foreach(executable ${executables})
        find_program(${executable}_EXECUTABLE ${executable})
        if(${executable}_EXECUTABLE)
          message(STATUS "Enabling ${SLICERAPP_APPLICATION_NAME} build tree launcher option: --${executable}")
          if(NOT DEFINED ${executable}_HELP)
            set(${executable}_HELP "Start ${executable}")
          endif()
          ctkAppLauncherAppendExtraAppToLaunchToList(
            LONG_ARG ${executable}
            HELP ${${executable}_HELP}
            PATH ${${executable}_EXECUTABLE}
            ARGUMENTS ${${executable}_ARGUMENTS}
            OUTPUTVAR extraApplicationToLaunchListForBuildTree
            )
        endif()
      endforeach()

      # Define list of extra 'application to launch' to associate with the launcher
      # within the install tree
      set(executables)
      if(WIN32)
        list(APPEND executables cmd)
        set(cmd_ARGUMENTS "/c start cmd")
      endif()
      foreach(executable ${executables})
        find_program(${executable}_EXECUTABLE ${executable})
        if(${executable}_EXECUTABLE)
          message(STATUS "Enabling ${SLICERAPP_APPLICATION_NAME} install tree launcher option: --${executable}")
          ctkAppLauncherAppendExtraAppToLaunchToList(
            LONG_ARG ${executable}
            HELP "Start ${executable}"
            PATH ${${executable}_EXECUTABLE}
            ARGUMENTS ${${executable}_ARGUMENTS}
            OUTPUTVAR extraApplicationToLaunchListForInstallTree
            )
        endif()
      endforeach()

      if(EXISTS ${QT_DESIGNER_EXECUTABLE} AND NOT APPLE)
        ctkAppLauncherAppendExtraAppToLaunchToList(
          LONG_ARG designer
          HELP "Start Qt designer using Slicer plugins"
          PATH "<APPLAUNCHER_SETTINGS_DIR>/../bin/designer-real${CMAKE_EXECUTABLE_SUFFIX}"
          OUTPUTVAR extraApplicationToLaunchListForInstallTree
          )
      endif()

      include(SlicerBlockCTKAppLauncherSettings)

      ctkAppLauncherConfigureForTarget(
        # Executable target associated with the launcher
        TARGET ${slicerapp_target}
        # Location of the launcher settings in the install tree
        APPLICATION_INSTALL_SUBDIR ${Slicer_BIN_DIR}
        # Info allowing to retrieve the Slicer extension settings
        APPLICATION_NAME ${SLICERAPP_APPLICATION_NAME}
        APPLICATION_REVISION ${Slicer_REVISION}
        ORGANIZATION_DOMAIN ${Slicer_ORGANIZATION_DOMAIN}
        ORGANIZATION_NAME ${Slicer_ORGANIZATION_NAME}
        USER_ADDITIONAL_SETTINGS_FILEBASENAME ${SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME}
        # Splash screen
        SPLASH_IMAGE_PATH ${SLICERAPP_LAUNCHER_SPLASHSCREEN_FILE}
        SPLASH_IMAGE_INSTALL_SUBDIR ${Slicer_BIN_DIR}
        SPLASHSCREEN_HIDE_DELAY_MS 3000
        # Slicer arguments triggering display of launcher help
        HELP_SHORT_ARG "-h"
        HELP_LONG_ARG "--help"
        # Slicer arguments that should NOT be associated with the spash screeb
        NOSPLASH_ARGS "--no-splash,--help,--version,--home,--program-path,--no-main-window,--settings-path,--temporary-path"
        # Extra application associated with the launcher
        EXTRA_APPLICATION_TO_LAUNCH_BUILD ${extraApplicationToLaunchListForBuildTree}
        EXTRA_APPLICATION_TO_LAUNCH_INSTALLED ${extraApplicationToLaunchListForInstallTree}
        # Location of the launcher settings in the build tree
        DESTINATION_DIR ${Slicer_BINARY_DIR}
        # Launcher settings specific to build tree
        LIBRARY_PATHS_BUILD "${SLICER_LIBRARY_PATHS_BUILD}"
        PATHS_BUILD "${SLICER_PATHS_BUILD}"
        ENVVARS_BUILD "${SLICER_ENVVARS_BUILD}"
        # Launcher settings specific to install tree
        LIBRARY_PATHS_INSTALLED "${SLICER_LIBRARY_PATHS_INSTALLED}"
        PATHS_INSTALLED "${SLICER_PATHS_INSTALLED}"
        ENVVARS_INSTALLED "${SLICER_ENVVARS_INSTALLED}"
        # The ADDITIONAL_PATH_ENVVARS_(BUILD_INSTALLED) variables contains names of
        # environment variables expected to be associated with a list of paths.
        # Examples of such variables are PYTHONPATH, QT_PLUGIN_PATH, ...
        # For each "ADDITIONAL_PATH_ENVVARS", the "ctkAppLauncherConfigure" macro
        # will look for variables named <ADDITIONAL_PATH_ENVVARS_PREFIX>_<ADDITIONAL_PATH_ENVVAR>_(BUILD|INSTALLED)
        # listing paths.
        # For example: SLICER_PYTHONPATH_BUILD, SLICER_PYTHONPATH_INSTALLED
        ADDITIONAL_PATH_ENVVARS_PREFIX SLICER_
        ADDITIONAL_PATH_ENVVARS_BUILD "${SLICER_ADDITIONAL_PATH_ENVVARS_BUILD}"
        ADDITIONAL_PATH_ENVVARS_INSTALLED "${SLICER_ADDITIONAL_PATH_ENVVARS_INSTALLED}"
        )

      # Folder
      set_target_properties(${SLICERAPP_APPLICATION_NAME}ConfigureLauncher PROPERTIES FOLDER ${SLICERAPP_FOLDER})

      if(NOT APPLE)
        if(Slicer_HAS_CONSOLE_IO_SUPPORT)
          install(
            PROGRAMS "${Slicer_BINARY_DIR}/${SLICERAPP_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
            DESTINATION "."
            COMPONENT Runtime
            )
        else()
          # Create command to update launcher icon
          add_custom_command(
            DEPENDS
              ${CTKAppLauncher_DIR}/bin/CTKAppLauncherW${CMAKE_EXECUTABLE_SUFFIX}
            OUTPUT
              ${Slicer_BINARY_DIR}/CMakeFiles/${SLICERAPP_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
            COMMAND ${CMAKE_COMMAND} -E copy
              ${CTKAppLauncher_DIR}/bin/CTKAppLauncherW${CMAKE_EXECUTABLE_SUFFIX}
              ${Slicer_BINARY_DIR}/CMakeFiles/${SLICERAPP_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
            COMMAND
              ${CTKResEdit_EXECUTABLE}
                --update-resource-ico ${Slicer_BINARY_DIR}/CMakeFiles/${SLICERAPP_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
                IDI_ICON1 ${SLICERAPP_WIN_ICON_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT ""
            )
          add_custom_target(${SLICERAPP_APPLICATION_NAME}UpdateLauncherWIcon ALL
            DEPENDS
              ${Slicer_BINARY_DIR}/CMakeFiles/${SLICERAPP_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}
            )

          # Folder
          set_target_properties(${SLICERAPP_APPLICATION_NAME}UpdateLauncherWIcon PROPERTIES FOLDER ${SLICERAPP_FOLDER})
          install(
            PROGRAMS "${Slicer_BINARY_DIR}/CMakeFiles/${SLICERAPP_APPLICATION_NAME}W${CMAKE_EXECUTABLE_SUFFIX}"
            DESTINATION "."
            RENAME "${SLICERAPP_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
            COMPONENT Runtime
            )
        endif()

        install(
          FILES ${SLICERAPP_LAUNCHER_SPLASHSCREEN_FILE}
          DESTINATION ${Slicer_INSTALL_BIN_DIR}
          COMPONENT Runtime
          )
      endif()

      #
      # On MacOSX, the installed launcher settings are *only* read directly by the
      # qSlicerCoreApplication using the LauncherLib.
      #
      # On Linux and Windows, the installed launcher settings are first read by the
      # installed launcher, and then read using the LauncherLib.
      #
      install(
        FILES "${Slicer_BINARY_DIR}/${SLICERAPP_APPLICATION_NAME}LauncherSettingsToInstall.ini"
        DESTINATION ${Slicer_INSTALL_BIN_DIR}
        RENAME ${SLICERAPP_APPLICATION_NAME}LauncherSettings.ini
        COMPONENT Runtime
        )

      if(WIN32)
        # Create target to update launcher icon
        add_custom_target(${SLICERAPP_APPLICATION_NAME}UpdateLauncherIcon ALL
          COMMAND
            ${CTKResEdit_EXECUTABLE}
              --update-resource-ico ${Slicer_BINARY_DIR}/${SLICERAPP_APPLICATION_NAME}${CMAKE_EXECUTABLE_SUFFIX}
              IDI_ICON1 ${SLICERAPP_WIN_ICON_FILE}
          )
        add_dependencies(${SLICERAPP_APPLICATION_NAME}UpdateLauncherIcon ${SLICERAPP_APPLICATION_NAME}ConfigureLauncher)

        # Folder
        set_target_properties(${SLICERAPP_APPLICATION_NAME}UpdateLauncherIcon PROPERTIES FOLDER ${SLICERAPP_FOLDER})
      endif()

    endif()
  endif()

endmacro()
