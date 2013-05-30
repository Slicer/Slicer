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
set(Slicer_CMAKE_CXX_FLAGS_CONFIG    ${ep_common_cxx_flags})
set(Slicer_CMAKE_C_COMPILER_CONFIG   ${CMAKE_C_COMPILER})
set(Slicer_CMAKE_C_FLAGS_CONFIG      ${ep_common_c_flags})

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

set(Slicer_Libs_INCLUDE_DIRS_CONFIG ${Slicer_Libs_INCLUDE_DIRS})
set(Slicer_Base_INCLUDE_DIRS_CONFIG ${Slicer_Base_INCLUDE_DIRS})
set(Slicer_ModuleLogic_INCLUDE_DIRS_CONFIG ${Slicer_ModuleLogic_INCLUDE_DIRS})
set(Slicer_ModuleMRML_INCLUDE_DIRS_CONFIG ${Slicer_ModuleMRML_INCLUDE_DIRS})
set(Slicer_ModuleWidgets_INCLUDE_DIRS_CONFIG ${Slicer_ModuleWidgets_INCLUDE_DIRS})

set(FreeSurfer_INCLUDE_DIRS_CONFIG ${FreeSurfer_INCLUDE_DIRS})
set(ITKFactoryRegistration_INCLUDE_DIRS_CONFIG ${ITKFactoryRegistration_INCLUDE_DIRS})
set(MRMLCore_INCLUDE_DIRS_CONFIG ${MRMLCore_INCLUDE_DIRS})
set(RemoteIO_INCLUDE_DIRS_CONFIG ${RemoteIO_INCLUDE_DIRS})
set(vtkTeem_INCLUDE_DIRS_CONFIG ${vtkTeem_INCLUDE_DIRS})
set(vtkITK_INCLUDE_DIRS_CONFIG ${vtkITK_INCLUDE_DIRS})

# Qt
set(QT_QMAKE_EXECUTABLE_CONFIG ${QT_QMAKE_EXECUTABLE})

# External project
if(Slicer_USE_BatchMake)
  set(BatchMake_DIR_CONFIG ${BatchMake_DIR})
endif()
set(CTK_DIR_CONFIG ${CTK_DIR})
if(Slicer_BUILD_DICOM_SUPPORT)
  set(DCMTK_DIR_CONFIG ${DCMTK_DIR})
endif()
if(Slicer_USE_QtTesting)
  set(QtTesting_DIR_CONFIG ${QtTesting_DIR})
endif()
set(ITK_DIR_CONFIG ${ITK_DIR})
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  set(qRestAPI_DIR_CONFIG ${qRestAPI_DIR})
endif()
if(Slicer_USE_OpenIGTLink)
  set(OpenIGTLink_DIR_CONFIG ${OpenIGTLink_DIR})
endif()
if(Slicer_USE_PYTHONQT)
  set(PYTHON_EXECUTABLE_CONFIG ${PYTHON_EXECUTABLE})
  set(PYTHON_INCLUDE_DIR_CONFIG ${PYTHON_INCLUDE_DIR})
  set(PYTHON_LIBRARY_CONFIG ${PYTHON_LIBRARY})
  if(WIN32)
    set(PYTHON_DEBUG_LIBRARY_CONFIG ${PYTHON_DEBUG_LIBRARY})
  endif()
endif()
if(Slicer_BUILD_CLI_SUPPORT)
  set(SlicerExecutionModel_DIR_CONFIG ${SlicerExecutionModel_DIR})
endif()
set(SLICERLIBCURL_DIR_CONFIG ${SLICERLIBCURL_DIR})
set(Teem_DIR_CONFIG ${Teem_DIR})
set(VTK_DIR_CONFIG ${VTK_DIR})

# List all required external project
set(Slicer_EXTERNAL_PROJECTS_CONFIG CTK ITK SLICERLIBCURL Teem VTK)
set(Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG)
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
if(Slicer_USE_BatchMake)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG BatchMake)
endif()
if(Slicer_USE_OpenIGTLink)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG OpenIGTLink)
endif()
if(Slicer_USE_PYTHONQT)
  list(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG PythonLibs PythonInterp)
  list(APPEND Slicer_EXTERNAL_PROJECTS_NO_USEFILE_CONFIG PythonLibs PythonInterp)
endif()

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

# Settings specific for installation trees
#
# (Note we configure from a different file than use for the build tree)

# Configure SlicerConfig.cmake for the install tree.
configure_file(
  ${Slicer_SOURCE_DIR}/CMake/SlicerInstallConfig.cmake.in
  ${Slicer_BINARY_DIR}/Utilities/SlicerConfig.cmake @ONLY)
