cmake_minimum_required(VERSION 3.20.6)

foreach(required_var Slicer_SOURCE_DIR TEST_BINARY_DIR)
  if(NOT DEFINED ${required_var})
    message(FATAL_ERROR "${required_var} is required")
  endif()
endforeach()

set(source_dir "${TEST_BINARY_DIR}/Source")
set(build_dir "${TEST_BINARY_DIR}/Build")
set(install_dir "${TEST_BINARY_DIR}/Install")
set(qt_dir "${TEST_BINARY_DIR}/Qt")
set(cellar_dir "${TEST_BINARY_DIR}/Cellar")

file(REMOVE_RECURSE "${TEST_BINARY_DIR}")
file(MAKE_DIRECTORY
  "${source_dir}"
  "${qt_dir}/bin"
  "${qt_dir}/lib/QtCore.framework/Versions/A"
  "${qt_dir}/share/qt/plugins/imageformats"
  "${cellar_dir}/qttools/bin"
  "${cellar_dir}/qtbase/plugins/imageformats"
  )

file(WRITE "${qt_dir}/lib/QtCore.framework/Versions/A/QtCore" "QtCore")
find_program(clang_executable clang REQUIRED)
file(WRITE "${TEST_BINARY_DIR}/tool.c" "int main(void) { return 0; }\n")
execute_process(
  COMMAND "${clang_executable}"
    -Wl,-headerpad_max_install_names
    "${TEST_BINARY_DIR}/tool.c"
    -o "${cellar_dir}/qttools/bin/lconvert"
  RESULT_VARIABLE compile_result
  )
if(compile_result)
  message(FATAL_ERROR "Failed to compile the test Qt tool: ${compile_result}")
endif()
foreach(tool lconvert lrelease lupdate)
  if(NOT tool STREQUAL "lconvert")
    file(COPY_FILE
      "${cellar_dir}/qttools/bin/lconvert"
      "${cellar_dir}/qttools/bin/${tool}")
  endif()
  file(CREATE_LINK
    "${cellar_dir}/qttools/bin/${tool}"
    "${qt_dir}/bin/${tool}"
    SYMBOLIC)
endforeach()

set(plugin_name "libqtest.dylib")
file(WRITE "${cellar_dir}/qtbase/plugins/imageformats/${plugin_name}" "Qt plugin")
file(CREATE_LINK
  "${cellar_dir}/qtbase/plugins/imageformats/${plugin_name}"
  "${qt_dir}/share/qt/plugins/imageformats/${plugin_name}"
  SYMBOLIC)

set(test_project [=[
cmake_minimum_required(VERSION 3.20.6)
project(SlicerQtInstallSymlinkTest NONE)

function(slicerStripInstalledLibrary)
endfunction()

add_library(Qt6::Core SHARED IMPORTED GLOBAL)
set_property(TARGET Qt6::Core PROPERTY
  IMPORTED_LOCATION_RELEASE "@qt_dir@/lib/QtCore.framework/Versions/A/QtCore")

set(CTK_QT_VERSION 6)
set(Slicer_BUILD_I18N_SUPPORT ON)
set(Slicer_INSTALL_BIN_DIR bin)
include("@Slicer_SOURCE_DIR@/CMake/SlicerBlockInstallQtTools.cmake")

set(QT_PLUGINS_DIR "@qt_dir@/share/qt/plugins")
set(Slicer_INSTALL_QtPlugins_DIR lib/QtPlugins)
set(SlicerBlockInstallQtPlugins_subdirectories imageformats)
include("@Slicer_SOURCE_DIR@/CMake/SlicerBlockInstallQtPlugins.cmake")
]=])
string(CONFIGURE "${test_project}" test_project @ONLY)
file(WRITE "${source_dir}/CMakeLists.txt" "${test_project}")

execute_process(
  COMMAND "${CMAKE_COMMAND}"
    -S "${source_dir}"
    -B "${build_dir}"
    "-DCMAKE_INSTALL_PREFIX=${install_dir}"
  RESULT_VARIABLE configure_result
  )
if(configure_result)
  message(FATAL_ERROR "Test project configuration failed: ${configure_result}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${build_dir}"
  RESULT_VARIABLE install_result
  )
if(install_result)
  message(FATAL_ERROR "Test project installation failed: ${install_result}")
endif()

set(installed_files
  "${install_dir}/bin/lconvert"
  "${install_dir}/bin/lrelease"
  "${install_dir}/bin/lupdate"
  "${install_dir}/lib/QtPlugins/imageformats/${plugin_name}"
  )
foreach(installed_file IN LISTS installed_files)
  if(NOT EXISTS "${installed_file}")
    message(FATAL_ERROR "Expected installed file is missing: ${installed_file}")
  endif()
  if(IS_SYMLINK "${installed_file}")
    message(FATAL_ERROR "Installed file must not be a symlink: ${installed_file}")
  endif()
endforeach()

message("SUCCESS")
