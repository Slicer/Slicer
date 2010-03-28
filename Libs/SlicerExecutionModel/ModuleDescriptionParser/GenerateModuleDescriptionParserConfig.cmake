# Generate the ModuleDescriptionParserConfig.cmake file in the build tree 
# and configure one the installation tree.

# Settings specific to build trees
#
#
set(ModuleDescriptionParser_INCLUDE_DIRS_CONFIG 
  ${ModuleDescriptionParser_BINARY_DIR} 
  ${ModuleDescriptionParser_SOURCE_DIR}
  )
set(ModuleDescriptionParser_LIBRARY_DIRS_CONFIG 
  ${ModuleDescriptionParser_BINARY_DIR}
  )
set(ModuleDescriptionParser_USE_FILE_CONFIG 
  ${ModuleDescriptionParser_BINARY_DIR}/UseModuleDescriptionParser.cmake
  )
set(ITK_DIR_CONFIG ${ITK_DIR})


# Configure ModuleDescriptionParserConfig.cmake for the build tree.
#
configure_file(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserConfig.cmake.in
  ${ModuleDescriptionParser_BINARY_DIR}/ModuleDescriptionParserConfig.cmake 
  @ONLY IMMEDIATE
  )

# Settings specific for installation trees
#
#

# Configure ModuleDescriptionParserConfig.cmake for the install tree.
#
configure_file(
  ${ModuleDescriptionParser_SOURCE_DIR}/ModuleDescriptionParserInstallConfig.cmake.in
  ${ModuleDescriptionParser_BINARY_DIR}/install/ModuleDescriptionParserConfig.cmake 
  @ONLY IMMEDIATE
  )
