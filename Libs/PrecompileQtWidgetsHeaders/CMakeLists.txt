project(PrecompileQtWidgetsHeaders)

# No way found to make it work with add_library, this error appears in the project which tries to reuse the precompiled headers:
# cmake_pch.hxx.gch: not used because `PrecompileQtCoreHeaders_EXPORTS' not defined [-Winvalid-pch]
add_executable(${PROJECT_NAME} PrecompileQtWidgetsHeaders.cxx)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

get_property(qt_widgets_lib_file_path TARGET "Qt5::Widgets" PROPERTY LOCATION_RELEASE)
get_filename_component(qt_widgets_lib_dir ${qt_widgets_lib_file_path} PATH)
set(qt_widgets_include_dir "${qt_widgets_lib_dir}/../include/QtWidgets")

# Precompile only low level most common headers because header precompilation takes time
set(precompiled_headers
  "${qt_widgets_include_dir}/QAction"
  "${qt_widgets_include_dir}/QApplication"
  "${qt_widgets_include_dir}/QDialog"
  "${qt_widgets_include_dir}/QLabel"
  "${qt_widgets_include_dir}/QMessageBox"
  "${qt_widgets_include_dir}/QVBoxLayout"
  "${qt_widgets_include_dir}/QWidget"
)

target_precompile_headers(${PROJECT_NAME} PUBLIC ${precompiled_headers})
