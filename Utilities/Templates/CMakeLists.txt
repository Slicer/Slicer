install(DIRECTORY Extensions Modules
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Wizard/Templates
  COMPONENT Runtime
  USE_SOURCE_PERMISSIONS
  )

include(SlicerExtensionTemplatesGenerator)

# Test LoadableCustomMarkups module.
# TODO: it would be nicer to instantiate the template so that it has a more
# descriptive name than the default "TemplateKey".
add_subdirectory(Modules/LoadableCustomMarkups)
