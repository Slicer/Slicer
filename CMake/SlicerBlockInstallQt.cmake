# -------------------------------------------------------------------------
# Find and install Qt
# -------------------------------------------------------------------------
set(QT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})

  list(APPEND QT_LIBRARIES
    "Qt${CTK_QT_VERSION}::Gui"
    )

  # WebEngine Dependencies
  if("Qt${CTK_QT_VERSION}::WebEngine" IN_LIST QT_LIBRARIES)
    find_package(Qt${CTK_QT_VERSION} REQUIRED
      COMPONENTS
        Qml
        Quick
        QuickWidgets
      OPTIONAL_COMPONENTS
        Positioning  # Soft build-time dependency. See https://bugreports.qt.io/browse/QTBUG-57418
      )
    list(APPEND QT_LIBRARIES
      "Qt${CTK_QT_VERSION}::Qml"
      "Qt${CTK_QT_VERSION}::Quick"
      "Qt${CTK_QT_VERSION}::QuickWidgets"
      "Qt${CTK_QT_VERSION}::WebEngineCore"
      )
    # QmlModels moved into its own library in Qt >= 5.14
    # Since windeployqt implicitly copies "Qt${CTK_QT_VERSION}QmlModels.dll" when
    # "-qml" is specified, exclude it on Windows.
    if(TARGET "Qt${CTK_QT_VERSION}::QmlModels" AND NOT WIN32)
      list(APPEND QT_LIBRARIES "Qt${CTK_QT_VERSION}::QmlModels")
    endif()
    if(TARGET "Qt${CTK_QT_VERSION}::Positioning")
      list(APPEND QT_LIBRARIES "Qt${CTK_QT_VERSION}::Positioning")
    endif()
  endif()

  # Get root directory
  get_property(_filepath TARGET "Qt${CTK_QT_VERSION}::Core" PROPERTY LOCATION_RELEASE)
  get_filename_component(_dir ${_filepath} PATH)
  set(qt_root_dir "${_dir}/..")

  # Qt designer plugin (needed on all platforms)
  if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
    if(CTK_QT_VERSION VERSION_EQUAL "5")
      # Qt5 provides a CMake module for DesignerComponents
      find_package(Qt${CTK_QT_VERSION} REQUIRED COMPONENTS
        Designer
        DesignerComponents
        )
      list(APPEND QT_LIBRARIES
        "Qt${CTK_QT_VERSION}::Designer"
        "Qt${CTK_QT_VERSION}::DesignerComponents"
        )
    else()
      # Qt6 doesn't provide a CMake module for DesignerComponents
      find_package(Qt${CTK_QT_VERSION} REQUIRED COMPONENTS
        Designer
        )
      list(APPEND QT_LIBRARIES
        "Qt${CTK_QT_VERSION}::Designer"
        )
    endif()
  endif()

  if(UNIX)

    find_package(Qt${CTK_QT_VERSION} REQUIRED COMPONENTS
      DBus
      )
    list(APPEND QT_LIBRARIES
      "Qt${CTK_QT_VERSION}::DBus"
      )
    if(CTK_QT_VERSION VERSION_EQUAL "5")
      find_package(Qt${CTK_QT_VERSION} REQUIRED COMPONENTS
        X11Extras
        )
      list(APPEND QT_LIBRARIES
        "Qt${CTK_QT_VERSION}::X11Extras"
        )
    endif()

    # XcbQpa
    slicerInstallLibrary(FILE ${qt_root_dir}/lib/libQt${CTK_QT_VERSION}XcbQpa.so
      DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime
      STRIP
      )

    # ICU libraries
    foreach(iculib IN ITEMS data i18n io le lx test tu uc)
      slicerInstallLibrary(FILE ${qt_root_dir}/lib/libicu${iculib}.so
        DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime
        STRIP
        )
    endforeach()

    # WebEngine Dependencies
    if("Qt${CTK_QT_VERSION}::WebEngine" IN_LIST QT_LIBRARIES)
      install(PROGRAMS ${qt_root_dir}/libexec/QtWebEngineProcess
        DESTINATION ${Slicer_INSTALL_ROOT}/libexec COMPONENT Runtime
        )
      slicerStripInstalledLibrary(
        FILES "${Slicer_INSTALL_ROOT}/libexec/QtWebEngineProcess"
        COMPONENT Runtime)
      # XXX Workaround for QTBUG-66346 fixed in Qt >= 5.11 (See https://github.com/Slicer/Slicer/pull/944)
      set(qt_conf_contents "[Paths]\nPrefix = ..\nTranslations = share/QtTranslations")
      install(
        CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/libexec/qt.conf\" \"${qt_conf_contents}\")"
        COMPONENT Runtime)
    endif()

    if(TARGET Qt5::Core)
      set(_qt_version "${Qt5_VERSION_MAJOR}.${Qt5_VERSION_MINOR}.${Qt5_VERSION_PATCH}")
    elseif(TARGET Qt6::Core)
      set(_qt_version "${Qt6_VERSION_MAJOR}.${Qt6_VERSION_MINOR}.${Qt6_VERSION_PATCH}")
    else()
      message(FATAL_ERROR "Failed to locate target Qt5::Core or Qt6::Core")
    endif()

    foreach(target ${QT_LIBRARIES})
      get_target_property(type ${target} TYPE)
      if(NOT type STREQUAL "SHARED_LIBRARY")
        continue()
      endif()
      get_property(location TARGET ${target} PROPERTY LOCATION_RELEASE)
      if(UNIX)
        # Install .so and versioned .so.x.y
        get_filename_component(QT_LIB_DIR_tmp ${location} PATH)
        get_filename_component(QT_LIB_NAME_tmp ${location} NAME)
        string(REPLACE ".${_qt_version}" "" QT_LIB_NAME_tmp ${QT_LIB_NAME_tmp})
        install(DIRECTORY ${QT_LIB_DIR_tmp}/
          DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime
          FILES_MATCHING PATTERN "${QT_LIB_NAME_tmp}*"
          PATTERN "${QT_LIB_NAME_tmp}*.debug" EXCLUDE)
        slicerStripInstalledLibrary(
          FILES "${QT_INSTALL_LIB_DIR}/${QT_LIB_NAME_tmp}"
          COMPONENT Runtime)
      endif()
    endforeach()

    # Install resources directory
    set(resources_dir "${qt_root_dir}/resources")
    install(DIRECTORY ${resources_dir}
      DESTINATION ${Slicer_INSTALL_ROOT} COMPONENT Runtime
      )

    # Install webengine translations
    if("Qt${CTK_QT_VERSION}::WebEngine" IN_LIST QT_LIBRARIES)
      set(translations_dir "${qt_root_dir}/translations/qtwebengine_locales")
      install(DIRECTORY ${translations_dir}
        DESTINATION ${Slicer_INSTALL_ROOT}/share/QtTranslations/ COMPONENT Runtime
        )
    endif()

    # Configure and install qt.conf
    set(qt_conf_contents "[Paths]\nPrefix = ..\nPlugins = ${Slicer_INSTALL_QtPlugins_DIR}\nTranslations = share/QtTranslations")
    install(
      CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/bin/qt.conf\" \"${qt_conf_contents}\")"
      COMPONENT Runtime
      )

  elseif(WIN32)
    # Get location of windeployqt.exe based on uic.exe location
    get_target_property(uic_location "Qt${CTK_QT_VERSION}::uic" IMPORTED_LOCATION)
    get_filename_component(_dir ${uic_location} DIRECTORY)
    set(windeployqt "${_dir}/windeployqt.exe")
    if(NOT EXISTS ${windeployqt})
      message(FATAL_ERROR "Failed to locate windeployqt executable: [${windeployqt}]")
    endif()
    set(_args "--release --no-compiler-runtime --no-translations --no-plugins")

    foreach(target IN LISTS QT_LIBRARIES)
      get_target_property(type ${target} TYPE)
      if(NOT type STREQUAL "SHARED_LIBRARY")
        continue()
      endif()
      string(REPLACE "Qt${CTK_QT_VERSION}::" "" module ${target})
      string(TOLOWER ${module} module_lc)
      set(_args "${_args} -${module_lc}")
    endforeach()

    # Qt designer components
    if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
      if(CTK_QT_VERSION VERSION_EQUAL "6")
        set(_args "${_args} -designercomponentsInternal")
      elseif(CTK_QT_VERSION VERSION_EQUAL "5")
        set(_args "${_args} -designercomponents")
      endif()
    endif()

    set(executable "${Slicer_MAIN_PROJECT_APPLICATION_NAME}App-real.exe")

    # Setting the "WindowsSdkDir" env. variable before building the PACKAGE target ensures
    # that the "windeployqt" tool can lookup the path of a recent version of the "d3dcompiler_47.dll"
    # library in the directory "%WindowsSdkDir%/Redist/D3D/x64"

    if(CMAKE_WINDOWS_KITS_10_DIR)
      # See https://cmake.org/cmake/help/latest/variable/CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION.html
      # and https://cmake.org/cmake/help/latest/module/InstallRequiredSystemLibraries.html
      set(windows_kits_dir "${CMAKE_WINDOWS_KITS_10_DIR}")
    elseif(ENV{WindowsSdkDir})
      # If building from an environments established by vcvarsall.bat or similar scripts.
      set(windows_kits_dir "$ENV{WindowsSdkDir}")
    else()
      # Default to registry value
      # Copied from CMake/Modules/InstallRequiredSystemLibraries.cmake
      get_filename_component(windows_kits_dir
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
    endif()
    if(NOT windows_kits_dir)
      message(WARNING "Failed to lookup Windows SDK directory required to package Qt libraries")
    endif()
    install(
      CODE "
        set(windows_kits_dir \"${windows_kits_dir}\")
        set(ENV{WindowsSdkDir} \${windows_kits_dir})
        if(\"\$ENV{WindowsSdkDir}\" STREQUAL \"\")
          message(FATAL_ERROR \"Setting WindowsSdkDir env. variable is required to ensure windeployqt can install the most recent version of d3dcompiler_47.dll\")
        endif()
        set(ENV{PATH} \"${QT_BINARY_DIR};\$ENV{PATH}\")
        execute_process(COMMAND \"${windeployqt}\" ${_args} \"\${CMAKE_INSTALL_PREFIX}/bin/${executable}\")
      "
      COMPONENT Runtime
      )
  endif()

  # Qt designer
  if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
    install(PROGRAMS ${qt_root_dir}/bin/designer${CMAKE_EXECUTABLE_SUFFIX}
      DESTINATION ${Slicer_INSTALL_ROOT}/bin COMPONENT Runtime
      RENAME designer-real${CMAKE_EXECUTABLE_SUFFIX}
      )
    slicerStripInstalledLibrary(
      FILES "${Slicer_INSTALL_ROOT}/bin/designer-real"
      COMPONENT Runtime)
  endif()
