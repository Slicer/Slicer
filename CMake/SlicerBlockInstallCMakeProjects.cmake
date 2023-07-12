include(${Slicer_CMAKE_DIR}/SlicerCheckModuleEnabled.cmake)  # For slicer_is_loadable_builtin_module_enabled

# -------------------------------------------------------------------------
# Install VTK
# -------------------------------------------------------------------------
if(NOT "${VTK_DIR}" STREQUAL "" AND EXISTS "${VTK_DIR}/CMakeCache.txt")
  if(${VTK_VERSION} VERSION_GREATER_EQUAL "8.90")
    set(_runtime_component "runtime")
  else()
    set(_runtime_component "RuntimeLibraries")
  endif()
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;${_runtime_component};/")
  if(${VTK_VERSION} VERSION_GREATER_EQUAL "8.90" AND Slicer_USE_PYTHONQT)
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;python;/")
  endif()
endif()

# -------------------------------------------------------------------------
# Install ITK
# -------------------------------------------------------------------------
if(NOT "${ITK_DIR}" STREQUAL "" AND EXISTS "${ITK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;RuntimeLibraries;/")
  # GDCM
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;Libraries;/")
  # HDF5 - hdf5
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;libraries;/")
  # HDF5 - hdf5_cpp
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;cpplibraries;/")
  # HDF5 - hdf5_hl
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;hllibraries;/")
  # HDF5 until ITK4. final, then it can be removed
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;Unspecified;/")
endif()

# -------------------------------------------------------------------------
# Install SimpleITK
#
#-------------------------------------------------------------------------
if(NOT "${SimpleITK_DIR}" STREQUAL "" AND EXISTS "${SimpleITK_DIR}/CMakeCache.txt" AND ${Slicer_USE_SimpleITK_SHARED})
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${SimpleITK_DIR};SimpleITK;Runtime;/")
endif()

# -------------------------------------------------------------------------
# Install JsonCpp
# -------------------------------------------------------------------------

# JsonCpp is required to build VolumeRendering module
slicer_is_loadable_builtin_module_enabled("VolumeRendering" _build_volume_rendering_module)

if((Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT OR _build_volume_rendering_module)
  AND NOT "${JsonCpp_DIR}" STREQUAL "" AND EXISTS "${JsonCpp_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${JsonCpp_DIR};JsonCpp;Unspecified;/")
endif()

# -------------------------------------------------------------------------
# Install SlicerExecutionModel
# -------------------------------------------------------------------------
if(Slicer_BUILD_CLI_SUPPORT
  AND NOT "${SlicerExecutionModel_DIR}" STREQUAL "" AND EXISTS "${SlicerExecutionModel_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${SlicerExecutionModel_DIR};SlicerExecutionModel;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install Teem
# -------------------------------------------------------------------------
if(NOT "${Teem_DIR}" STREQUAL "" AND EXISTS "${Teem_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Teem_DIR};teem;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install CTK
# -------------------------------------------------------------------------
if(NOT "${CTK_DIR}" STREQUAL "" AND EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;RuntimeLibraries;/")
endif()

