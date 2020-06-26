# Generate the SlicerConfig.cmake file in the build tree and configure one
# for the installation tree.  This file tells external project how to use
# Slicer.  This provides a mechanism for third party developers to build
# modules against a Slicer installation.
#
#

# The configuration process is very different for a build tree and an
# installation. The resulting directory structures are vastly
# different. So, the two configured files not only have different
# settings, they have a different structure.

# Settings that are the same for build trees and installation trees
#
#

# Settings specific to build trees
#
#

set(Slicer_USE_FILE_CONFIG ${Slicer_USE_FILE})

# Compilation settings
set(Slicer_CMAKE_CXX_COMPILER_CONFIG ${CMAKE_CXX_COMPILER})
set(Slicer_CMAKE_C_COMPILER_CONFIG   ${CMAKE_C_COMPILER})
set(Slicer_CMAKE_CXX_STANDARD_CONFIG          ${CMAKE_CXX_STANDARD})
set(Slicer_CMAKE_CXX_STANDARD_REQUIRED_CONFIG ${CMAKE_CXX_STANDARD_REQUIRED})
set(Slicer_CMAKE_CXX_EXTENSIONS_CONFIG        ${CMAKE_CXX_EXTENSIONS})

# Launcher command
set(Slicer_LAUNCHER_EXECUTABLE_CONFIG ${Slicer_LAUNCHER_EXECUTABLE})
set(Slicer_LAUNCH_COMMAND_CONFIG ${Slicer_LAUNCH_COMMAND})

# License and Readme file
set(Slicer_LICENSE_FILE_CONFIG ${Slicer_SOURCE_DIR}/License.txt)
set(Slicer_README_FILE_CONFIG ${Slicer_SOURCE_DIR}/README.txt)

# Test templates directory
set(Slicer_CXX_MODULE_TEST_TEMPLATES_DIR_CONFIG ${Slicer_CXX_MODULE_TEST_TEMPLATES_DIR})
set(Slicer_PYTHON_MODULE_TEST_TEMPLATES_DIR_CONFIG ${Slicer_PYTHON_MODULE_TEST_TEMPLATES_DIR})

# Path to extension CPack script(s)
set(Slicer_EXTENSION_CPACK_CONFIG ${Slicer_EXTENSION_CPACK})
set(Slicer_EXTENSION_CPACK_BUNDLE_FIXUP_CONFIG ${Slicer_SOURCE_DIR}/CMake/SlicerExtensionCPackBundleFixup.cmake.in)

set(Slicer_GUI_LIBRARY_CONFIG ${Slicer_GUI_LIBRARY})
set(Slicer_CORE_LIBRARY_CONFIG ${Slicer_CORE_LIBRARY})

get_property(_module_targets GLOBAL PROPERTY SLICER_MODULE_TARGETS)
if(_module_targets)
  foreach(target ${_module_targets})
    set(Slicer_INCLUDE_MODULE_DIRS_CONFIG
"${Slicer_INCLUDE_MODULE_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
  endforeach()
endif()

get_property(_module_logic_targets GLOBAL PROPERTY SLICER_MODULE_LOGIC_TARGETS)
if(_module_logic_targets)
  foreach(target ${_module_logic_targets})
    set(Slicer_INCLUDE_MODULE_LOGIC_DIRS_CONFIG
"${Slicer_INCLUDE_MODULE_LOGIC_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
    list(APPEND Slicer_ModuleLogic_INCLUDE_DIRS_CONFIG "\${${target}_INCLUDE_DIRS}")
  endforeach()
endif()

get_property(_module_mrml_targets GLOBAL PROPERTY SLICER_MODULE_MRML_TARGETS)
if(_module_mrml_targets)
  foreach(target ${_module_mrml_targets})
    set(Slicer_INCLUDE_MODULE_MRML_DIRS_CONFIG
"${Slicer_INCLUDE_MODULE_MRML_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
    list(APPEND Slicer_ModuleMRML_INCLUDE_DIRS_CONFIG "\${${target}_INCLUDE_DIRS}")
  endforeach()
endif()

get_property(_module_widget_targets GLOBAL PROPERTY SLICER_MODULE_WIDGET_TARGETS)
if(_module_widget_targets)
  foreach(target ${_module_widget_targets})
    set(Slicer_INCLUDE_MODULE_WIDGET_DIRS_CONFIG
"${Slicer_INCLUDE_MODULE_WIDGET_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
    list(APPEND Slicer_ModuleWidgets_INCLUDE_DIRS_CONFIG "\${${target}_INCLUDE_DIRS}")
  endforeach()
endif()

get_property(_wrap_hierarchy_targets GLOBAL PROPERTY SLICER_WRAP_HIERARCHY_TARGETS)
if(_wrap_hierarchy_targets)
  foreach(target ${_wrap_hierarchy_targets})
    set(Slicer_WRAP_HIERARCHY_FILES_CONFIG
"${Slicer_WRAP_HIERARCHY_FILES_CONFIG}
set(${target}_WRAP_HIERARCHY_FILE
  \"${${target}_WRAP_HIERARCHY_FILE}\")"
)
  endforeach()
endif()

set(Slicer_Libs_INCLUDE_DIRS_CONFIG ${Slicer_Libs_INCLUDE_DIRS})
set(Slicer_Base_INCLUDE_DIRS_CONFIG ${Slicer_Base_INCLUDE_DIRS})

set(ITKFactoryRegistration_INCLUDE_DIRS_CONFIG ${ITKFactoryRegistration_INCLUDE_DIRS})
set(MRMLCore_INCLUDE_DIRS_CONFIG ${MRMLCore_INCLUDE_DIRS})
set(MRMLLogic_INCLUDE_DIRS_CONFIG ${MRMLLogic_INCLUDE_DIRS})
set(MRMLCLI_INCLUDE_DIRS_CONFIG ${MRMLCLI_INCLUDE_DIRS})
set(qMRMLWidgets_INCLUDE_DIRS_CONFIG ${qMRMLWidgets_INCLUDE_DIRS})
set(RemoteIO_INCLUDE_DIRS_CONFIG ${RemoteIO_INCLUDE_DIRS})
set(vtkTeem_INCLUDE_DIRS_CONFIG ${vtkTeem_INCLUDE_DIRS})
set(vtkAddon_INCLUDE_DIRS_CONFIG ${vtkAddon_INCLUDE_DIRS})
set(vtkITK_INCLUDE_DIRS_CONFIG ${vtkITK_INCLUDE_DIRS})
set(vtkSegmentationCore_INCLUDE_DIRS_CONFIG ${vtkSegmentationCore_INCLUDE_DIRS})

# Note: For sake of simplification, the macro 'slicer_config_set_ep' is not invoked conditionally, if
# the configured 'value' parameter is an empty string, the macro 'slicer_config_set_ep' is a no-op.

# Slicer external projects variables
set(Slicer_SUPERBUILD_EP_VARS_CONFIG)
foreach(varname ${Slicer_EP_LABEL_FIND_PACKAGE} QtTesting_DIR BRAINSCommonLib_DIR)
  set(Slicer_SUPERBUILD_EP_VARS_CONFIG
   "${Slicer_SUPERBUILD_EP_VARS_CONFIG}
slicer_config_set_ep(
  ${varname}
  \"${${varname}}\"
  CACHE STRING \"Path to project build directory or file\" FORCE)
")
endforeach()

# Slicer external project component variables
set(Slicer_EP_COMPONENT_VARS_CONFIG
  "set(Slicer_VTK_COMPONENTS \"${Slicer_VTK_COMPONENTS}\")")

# List all required external project
set(Slicer_EXTERNAL_PROJECTS_CONFIG CTK CTKAppLauncherLib ITK CURL Teem VTK RapidJSON)
set(Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG CURL CTKAppLauncherLib RapidJSON)
if(Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG CTKAppLauncher)
  list(APPEND Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG CTKAppLauncher)
endif()
if(Slicer_USE_QtTesting)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG QtTesting)
  list(APPEND Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG QtTesting)
endif()
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG SlicerExecutionModel)
endif()
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG qRestAPI)
endif()
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG DCMTK)
  list(APPEND Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG DCMTK)
endif()
if(Slicer_USE_PYTHONQT)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG PythonLibs PythonInterp)
  list(APPEND Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG PythonLibs PythonInterp)
endif()
if(Slicer_USE_SimpleITK)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG SWIG)
  list(APPEND Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG SWIG)
endif()

# Configure Slicer_USE_SYSTEM_* variables
set(Slicer_EP_USE_SYSTEM_VARS_CONFIG "")
foreach(varname ${Slicer_EP_LABEL_USE_SYSTEM})
  set(Slicer_EP_USE_SYSTEM_VARS_CONFIG
    "${Slicer_EP_USE_SYSTEM_VARS_CONFIG}
set(Slicer_USE_SYSTEM_${varname} \"${Slicer_USE_SYSTEM_${varname}}\")"
    )
endforeach()

if(Slicer_BUILD_CLI_SUPPORT)
  set(SlicerExecutionModel_CLI_LIBRARY_WRAPPER_CXX_CONFIG ${SlicerExecutionModel_CLI_LIBRARY_WRAPPER_CXX})
  set(SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES_CONFIG ${SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES})
  set(SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES_CONFIG ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
endif()

# Export Targets file.
set(Slicer_TARGETS_FILE "${Slicer_BINARY_DIR}/SlicerTargets.cmake")

# Configure SlicerConfig.cmake for the build tree.
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerConfig.cmake.in
  ${Slicer_BINARY_DIR}/SlicerConfig.cmake @ONLY)
