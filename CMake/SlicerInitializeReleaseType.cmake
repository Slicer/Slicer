
# Default release type to use of none was specified
if(NOT DEFINED Slicer_DEFAULT_RELEASE_TYPE)
  set(Slicer_DEFAULT_RELEASE_TYPE "Experimental")
endif()

# Slicer Release type
set(Slicer_RELEASE_TYPE "${Slicer_DEFAULT_RELEASE_TYPE}" CACHE STRING "Type of Slicer release.")
mark_as_advanced(Slicer_RELEASE_TYPE)

# Set the possible values for cmake-gui
set_property(CACHE Slicer_RELEASE_TYPE PROPERTY STRINGS
  "Experimental"
  "Preview"
  "Stable"
  )

mark_as_superbuild(Slicer_RELEASE_TYPE)
