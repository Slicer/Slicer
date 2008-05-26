# Generate the LoadableModuleConfig.cmake file in the build tree 
# and configure one the installation tree.

# Settings specific to build trees
#
#
set(LoadableModule_INCLUDE_DIRS_CONFIG 
  ${LoadableModule_BINARY_DIR} 
  ${LoadableModule_SOURCE_DIR}
  )
set(LoadableModule_LIBRARY_DIRS_CONFIG 
  ${LoadableModule_BINARY_DIR}
  )
set(LoadableModule_USE_FILE_CONFIG 
  ${LoadableModule_BINARY_DIR}/UseLoadableModule.cmake
  )
set(ITK_DIR_CONFIG ${ITK_DIR})


# Configure LoadableModuleConfig.cmake for the build tree.
#
configure_file(
  ${LoadableModule_SOURCE_DIR}/LoadableModuleConfig.cmake.in
  ${LoadableModule_BINARY_DIR}/LoadableModuleConfig.cmake 
  @ONLY IMMEDIATE
  )

# Settings specific for installation trees
#
#

# Configure LoadableModuleConfig.cmake for the install tree.
#
configure_file(
  ${LoadableModule_SOURCE_DIR}/LoadableModuleInstallConfig.cmake.in
  ${LoadableModule_BINARY_DIR}/install/LoadableModuleConfig.cmake 
  @ONLY IMMEDIATE
  )
