
function(slicer_vtk_module_build)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    ""
    "SOURCE_DIR"
    "MODULES"
  )

  if(NOT arg_SOURCE_DIR)
    set(arg_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
  endif()

  if(NOT arg_MODULES)
    message(FATAL_ERROR "Missing or empty argument MODULES.")
  endif()

  # Module tests are not built using VTK modules entry points,
  # because the legacy Slicer test infrastructure is not compatible with the VTK module build.
  # Instead they are simply included in modules CMakeLists.txt as it was done before.
  set(enable_tests OFF)
  set(base_install_dir "slicer")
  set(lib_install_dir "slicer")
  set(python_lib_install_dir ".")
  if(NOT DEFINED arg_INSTALL_HEADERS)
    set(arg_INSTALL_HEADERS OFF)
  endif()

  # Variables defined in this are used by VTK module system
  include(GNUInstallDirs)

  # There are two sets because we output the libs and python wrapped at different locations:
  # - Base and Libs are placed in bin (+ site-packages) and lib
  # - Loadable modules are placed in qt-loadable-modules (+ site-packages)

  vtk_module_find_modules(libs_module_files
    "${arg_SOURCE_DIR}/Base"
    "${arg_SOURCE_DIR}/Libs"
  )
  vtk_module_scan(
    MODULE_FILES          ${libs_module_files}
    PROVIDES_MODULES      libs_modules
    REQUIRES_MODULES      required_modules
    REQUEST_MODULES       ${arg_MODULES}
    ENABLE_TESTS          "${enable_tests}"
  )

  vtk_module_find_modules(loadable_module_files
    "${arg_SOURCE_DIR}/Modules"
    "${arg_SOURCE_DIR}/Utilities/Templates"
  )
  vtk_module_scan(
    MODULE_FILES          ${loadable_module_files}
    PROVIDES_MODULES      loadable_modules
    REQUIRES_MODULES      required_modules
    REQUEST_MODULES       ${arg_MODULES}
    ENABLE_TESTS          "${enable_tests}"
  )

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_LIB_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_LIB_DIR}")
  vtk_module_build(
    MODULES               ${libs_modules}
    PACKAGE               SlicerLibs
    INSTALL_EXPORT        SlicerLibsTargets
    INSTALL_HEADERS       "${arg_INSTALL_HEADERS}"
    HEADERS_DESTINATION   "${lib_install_dir}/include"
    CMAKE_DESTINATION     "${lib_install_dir}/lib/cmake/Slicer"
    LICENSE_DESTINATION   "${lib_install_dir}"
    HIERARCHY_DESTINATION "${lib_install_dir}/hierarchy"
  )

  if(Slicer_USE_PYTHONQT)
    vtk_module_wrap_python(
      MODULES              ${libs_modules} ${base_modules}
      MODULE_DESTINATION   "${Slicer_SITE_PACKAGES_DIR}"
      PYTHON_PACKAGE       "slicer"
      SOABI                ""
      BUILD_STATIC         OFF
      BUILD_PYI_FILES      ON
      INSTALL_HEADERS      "${arg_INSTALL_HEADERS}"
    )
  endif()

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
  vtk_module_build(
    MODULES               ${loadable_modules}
    PACKAGE               SlicerLoadable
    INSTALL_EXPORT        SlicerLoadableTargets
    INSTALL_HEADERS       "${arg_INSTALL_HEADERS}"
    HEADERS_DESTINATION   "${base_install_dir}/include"
    CMAKE_DESTINATION     "${base_install_dir}/lib/cmake/Slicer"
    LICENSE_DESTINATION   "${base_install_dir}"
    HIERARCHY_DESTINATION "${base_install_dir}/hierarchy"
  )

  if(Slicer_USE_PYTHONQT)
    vtk_module_wrap_python(
      MODULES             ${loadable_modules}
      PYTHON_PACKAGE      "slicer"
      MODULE_DESTINATION  "${Slicer_QTLOADABLEMODULES_SITE_PACKAGES_DIR}"
      SOABI               ""
      BUILD_STATIC        OFF
      BUILD_PYI_FILES     OFF
      INSTALL_HEADERS     "${arg_INSTALL_HEADERS}"
    )
  endif()

  # There are 3 names for each module
  # - The library name on disk, e.g. MRMLCore.[so/dll/dylib], this used to be the name of the CMake target.
  # - The new VTK-like module name, e.g. Slicer::MRMLCore.
  # - The real target name, e.g. MRMLCore, this is how VTK module system names the targets.
  # For Slicer::MRMLCore this works, as the real target name is the auto-determined one
  # But for Slicer::ModuleMRML, that used to be called vtkSlicerModuleNameMRML, now has:
  # "vtkSlicerModuleNameMRML.[so/dll/sylib]" as output name, but "ModuleMRML" as target name.
  # This logic will map the output library name to be an alias on the module.
  set(aliases_code_to_export)
  foreach(module IN LISTS libs_modules loadable_modules)
    vtk_module_set_properties("${module}" INTERFACE_vtk_module_name "${module}")
    vtk_module_get_property("${module}"
      PROPERTY INTERFACE_vtk_module_library_name
      VARIABLE lib_name
    )
    # Create an alias to provide compatibility with the previous target name
    if(NOT TARGET "${lib_name}")
      _vtk_module_real_target(real_target "${module}")
      add_library("${lib_name}" ALIAS "${real_target}")
      string(APPEND aliases_code_to_export "add_library("${lib_name}" ALIAS "${real_target}")\n")
    endif()
    # Provide legacy CMake variables removed in the transition.
    _slicer_define_compatibility_variable_for_module("${module}" "${lib_name}")
  endforeach()

  # For each modules, creates a compatibility python module in the legacy location
  # Slicer basic scripts are still in bin/python/slicer
  # Libs and base:
  # - before: "<root>/lib/Slicer-X.Y/NamePython.[pyd|so|dylib]" (in build, they were in bin on Windows for some reasons)
  # - now: "<root>/lib/site-packages/slicer/Name.[pyd|so|dylib]"
  # Loadables:
  # - before: "<root>/lib/Slicer-X.Y/qt-loadable-modules/NamePython.[pyd|so|dylib]"
  # - after: "<root>/lib/Slicer-X.Y/qt-loadable-modules/site-packages/slicer/Name.[pyd|so|dylib]"
  if(Slicer_USE_PYTHONQT)
    foreach(module IN LISTS libs_modules)
      _slicer_add_compatibility_module("${module}" "${Slicer_SITE_PACKAGES_DIR}")
    endforeach()
    foreach(module IN LISTS loadable_modules)
      _slicer_add_compatibility_module("${module}" "${Slicer_QTLOADABLEMODULES_LIB_DIR}/site-packages")
    endforeach()
  endif()

  # Export the targets so extensions can find them
  foreach(set IN ITEMS SlicerLibsTargets SlicerLoadableTargets)
    export(EXPORT "${set}")
  endforeach()
    # This file is a manually generated export set, because we can not export aliases
  file(WRITE "${CMAKE_BINARY_DIR}/SlicerAliasesTargets.cmake" "${aliases_code_to_export}")
endfunction()

function(_slicer_deprecated_variable_callback variable access value current_list_file stack)
  if(access STREQUAL READ_ACCESS)
    message(DEPRECATION "Variable \"${variable}\" is deprecated and should no longer be used.")
  elseif(access STREQUAL UNKNOWN_READ_ACCESS)
    message(SEND_ERROR "Variable \"${variable}\" has been removed and must no longer be used.")
  endif()
endfunction()

macro(_slicer_define_compatibility_variable name value)
  if(NOT "${value}" STREQUAL "")
    set("${name}" "${value}" PARENT_SCOPE)
  endif()
  variable_watch("${name}" _slicer_deprecated_variable_callback)
endmacro()

macro(_slicer_define_compatibility_variable_for_module module_name legacy_name)
  set_property(GLOBAL APPEND PROPERTY SLICER_MODULE_MRML_TARGETS ${legacy_name})
  _slicer_define_compatibility_variable("${legacy_name}_SOURCE_DIR" "")
  _slicer_define_compatibility_variable("${legacy_name}_BINARY_DIR" "")
  _slicer_define_compatibility_variable("${legacy_name}_INCLUDE_DIRS" "$<TARGET_PROPERTY:${module_name},INTERFACE_INCLUDE_DIRECTORIES>")
endmacro()

function(_slicer_add_compatibility_module module stem)
  vtk_module_get_property("${module}"
    PROPERTY INTERFACE_vtk_module_library_name
    VARIABLE lib_name
  )
  file(WRITE "${CMAKE_BINARY_DIR}/${stem}/${lib_name}Python.py"
    "from slicer.${lib_name} import *"
  )
  install(
    FILES "${CMAKE_BINARY_DIR}/${stem}/${lib_name}Python.py"
    DESTINATION "${Slicer_INSTALL_ROOT}${stem}"
  )
endfunction()
