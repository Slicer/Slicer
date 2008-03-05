# Generate the LoadableModuleConfig.cmake file in the build tree 
# and configure one the installation tree.

# Settings specific to build trees
#
#
SET(LoadableModule_INCLUDE_DIRS_CONFIG ${LoadableModule_BINARY_DIR} ${LoadableModule_SOURCE_DIR})
SET(LoadableModule_LIBRARY_DIRS_CONFIG ${LoadableModule_BINARY_DIR})
SET(LoadableModule_USE_FILE_CONFIG ${LoadableModule_BINARY_DIR}/UseLoadableModule.cmake)
SET(ITK_DIR_CONFIG ${ITK_DIR})


# Configure LoadableModuleConfig.cmake for the install tree.
CONFIGURE_FILE(${LoadableModule_SOURCE_DIR}/LoadableModuleConfig.cmake.in
               ${LoadableModule_BINARY_DIR}/LoadableModuleConfig.cmake @ONLY IMMEDIATE)



# Settings specific for installation trees
#
#
SET(LoadableModule_INCLUDE_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/include/LoadableModule)
SET(LoadableModule_LIBRARY_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/LoadableModule)
SET(LoadableModule_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/LoadableModule/UseLoadableModule.cmake)
SET(ITK_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/InsightToolkit)


# Configure LoadableModuleConfig.cmake for the install tree.
CONFIGURE_FILE(${LoadableModule_SOURCE_DIR}/LoadableModuleConfig.cmake.in
               ${LoadableModule_BINARY_DIR}/install/LoadableModuleConfig.cmake @ONLY IMMEDIATE)
