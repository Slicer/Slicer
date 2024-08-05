project(PrecompileQtCoreHeaders)

# No way found to make it work with add_library, this error appears in the project which tries to reuse the precompiled headers:
# cmake_pch.hxx.gch: not used because `PrecompileQtCoreHeaders_EXPORTS' not defined [-Winvalid-pch]
add_executable(${PROJECT_NAME} PrecompileQtCoreHeaders.cxx)
find_package(Qt5 COMPONENTS Core REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt5::Core)

get_property(qt_core_lib_file_path TARGET "Qt5::Core" PROPERTY LOCATION_RELEASE)
get_filename_component(qt_core_lib_dir ${qt_core_lib_file_path} PATH)
set(qt_core_include_dir "${qt_core_lib_dir}/../include/QtCore")

# Precompile only low level most common headers because header precompilation takes time
set(precompiled_headers
  "${qt_core_include_dir}/QDateTime"
  "${qt_core_include_dir}/QDebug"
  "${qt_core_include_dir}/QDir"
  "${qt_core_include_dir}/QFile"
  "${qt_core_include_dir}/QFileInfo"
  "${qt_core_include_dir}/QHash"
  "${qt_core_include_dir}/QList"
  "${qt_core_include_dir}/QMap"
  "${qt_core_include_dir}/QMetaType"
  "${qt_core_include_dir}/QObject"
  "${qt_core_include_dir}/QPointer"
  "${qt_core_include_dir}/QSettings"
  "${qt_core_include_dir}/QSharedPointer"
  "${qt_core_include_dir}/QString"
  "${qt_core_include_dir}/QStringList"
  "${qt_core_include_dir}/QTimer"
  "${qt_core_include_dir}/QUrl"
  "${qt_core_include_dir}/QVariant"
  )

target_precompile_headers(${PROJECT_NAME} PUBLIC ${precompiled_headers})
