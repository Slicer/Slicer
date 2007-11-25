# Generate the ModuleDescriptionParserConfig.cmake file in the build tree 
# and configure one the installation tree.

# Settings specific to build trees
#
#
SET(ModuleDescriptionParser_INCLUDE_DIRS_CONFIG ${ModuleDescriptionParser_BINARY_DIR} ${ModuleDescriptionParser_SOURCE_DIR})
SET(ModuleDescriptionParser_LIBRARY_DIRS_CONFIG ${ModuleDescriptionParser_BINARY_DIR})
SET(ModuleDescriptionParser_USE_FILE_CONFIG ${ModuleDescriptionParser_BINARY_DIR}/UseModuleDescriptionParser.cmake)
SET(ITK_DIR_CONFIG ${ITK_DIR})


# Configure ModuleDescriptionParserConfig.cmake for the install tree.
CONFIGURE_FILE(${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfig.cmake.in
               ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfig.cmake @ONLY IMMEDIATE)



# Settings specific for installation trees
#
#
SET(ModuleDescriptionParser_INCLUDE_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/include/ModuleDescriptionParser)
SET(ModuleDescriptionParser_LIBRARY_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/ModuleDescriptionParser)
SET(ModuleDescriptionParser_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/ModuleDescriptionParser/UseModuleDescriptionParser.cmake)
SET(ITK_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/InsightToolkit)


# Configure ModuleDescriptionParserConfig.cmake for the install tree.
CONFIGURE_FILE(${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfig.cmake.in
               ${ModuleDescriptionParser_BINARY_DIR}/install/ModuleDescriptionParserConfig.cmake @ONLY IMMEDIATE)
