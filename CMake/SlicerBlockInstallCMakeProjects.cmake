
# -------------------------------------------------------------------------
# Install Slicer
# -------------------------------------------------------------------------
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${Slicer_BINARY_DIR};Slicer;ALL;/")

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
if(NOT "${BatchMake_DIR}" STREQUAL "" AND EXISTS "${BatchMake_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${BatchMake_DIR};BatchMake;Runtime;/")
endif()

# -------------------------------------------------------------------------
# Install OpenIGTLink
# -------------------------------------------------------------------------
if(NOT "${OpenIGTLink_DIR}" STREQUAL "" AND EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${OpenIGTLink_DIR};igtl;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install CTK
# -------------------------------------------------------------------------
if(NOT "${CTK_DIR}" STREQUAL "" AND EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;Runtime;/")
endif()

