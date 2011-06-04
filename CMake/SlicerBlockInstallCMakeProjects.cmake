
# -------------------------------------------------------------------------
# Install Slicer
# -------------------------------------------------------------------------
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${Slicer_BINARY_DIR};Slicer;ALL;/")

# -------------------------------------------------------------------------
# Install VTK
# -------------------------------------------------------------------------
if(EXISTS "${VTK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install ITK
# -------------------------------------------------------------------------
if(EXISTS "${ITK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install Teem
# -------------------------------------------------------------------------
if(EXISTS "${Teem_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Teem_DIR};teem;ALL;/")
endif()

# -------------------------------------------------------------------------
# Install BatchMake
# -------------------------------------------------------------------------
if(EXISTS "${BatchMake_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${BatchMake_DIR};BatchMake;Runtime;/")
endif()

# -------------------------------------------------------------------------
# Install OpenIGTLink
# -------------------------------------------------------------------------
if(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${OpenIGTLink_DIR};igtl;RuntimeLibraries;/")
endif()

# -------------------------------------------------------------------------
# Install CTK
# -------------------------------------------------------------------------
if(EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;Runtime;/")
endif()

