if(DEFINED Slicer_VERSION_TWEAK) # This is defined only for release
  set(Slicer_VERSION_TWEAK_OR_DEV "${Slicer_VERSION_TWEAK}")
else()
  set(Slicer_VERSION_TWEAK_OR_DEV "\"dev0\"")
endif()

# Yes, this file is configured in the SOURCE directory. And yes it is checked
# in. The contents of the generated file should only be changed when the Slicer
# version number is changed, and the updated __version__.py should be committed
# at the same time. This is mainly to ensure that __version__.py is kept in
# sync.
configure_file("__version__.py.in" "__version__.py" @ONLY)
