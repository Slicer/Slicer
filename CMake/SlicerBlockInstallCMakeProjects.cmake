
# -------------------------------------------------------------------------
# Install VTK
# -------------------------------------------------------------------------
if(NOT "${VTK_DIR}" STREQUAL "" AND EXISTS "${VTK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install ITK
# -------------------------------------------------------------------------
if(NOT "${ITK_DIR}" STREQUAL "" AND EXISTS "${ITK_DIR}/CMakeCache.txt")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;RuntimeLibraries;/")
  if(${ITK_VERSION_MAJOR} STREQUAL "4")
    # GDCM
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;Libraries;/")
    # HDF5
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;libraries;/")
    # HDF5 until ITK4. final, then it can be removed
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;Unspecified;/")
  endif()
endif()

# -------------------------------------------------------------------------
# Install SimpleITK
#
#-------------------------------------------------------------------------
if(NOT "${SimpleITK_DIR}" STREQUAL "" AND EXISTS "${SimpleITK_DIR}/CMakeCache.txt" AND ${Slicer_USE_SimpleITK_SHARED})
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${SimpleITK_DIR};SimpleITK;Unspecified;/")
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
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Teem_DIR};teem;ALL;/")
endif()

# -------------------------------------------------------------------------
# Install BatchMake
# -------------------------------------------------------------------------
if(Slicer_USE_BatchMake
  AND NOT "${BatchMake_DIR}" STREQUAL "" AND EXISTS "${BatchMake_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${BatchMake_DIR};BatchMake;Runtime;/")
endif()

# -------------------------------------------------------------------------
# Install OpenIGTLink
# -------------------------------------------------------------------------
if(Slicer_USE_OpenIGTLink
  AND NOT "${OpenIGTLink_DIR}" STREQUAL "" AND EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${OpenIGTLink_DIR};igtl;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install CTK
# -------------------------------------------------------------------------
if(NOT "${CTK_DIR}" STREQUAL "" AND EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;RuntimeLibraries;/")
endif()

