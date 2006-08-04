PROJECT(Editor)

#
# install the tcl package for the Module
# - make the module packages build directory if needed
# - make this module's package directory
# - copy over the tcl package files to the build directory
#

SET (Package_DIR ${Slicer3_BINARY_DIR}/Modules/Packages)
IF (NOT EXISTS ${Package_DIR})
  FILE(MAKE_DIRECTORY ${Package_DIR})
ENDIF (NOT EXISTS ${Package_DIR})

SET(Editor_DIR ${Slicer3_BINARY_DIR}/Modules/Packages/Editor)

FILE(MAKE_DIRECTORY ${Editor_DIR})
CONFIGURE_FILE( pkgIndex.tcl ${Editor_DIR}/pkgIndex.tcl COPYONLY)
CONFIGURE_FILE( Editor.tcl ${Editor_DIR}/Editor.tcl COPYONLY)
CONFIGURE_FILE( EditorNode.tcl ${Editor_DIR}/EditorNode.tcl COPYONLY)
CONFIGURE_FILE( EditorLogic.tcl ${Editor_DIR}/EditorLogic.tcl COPYONLY)
CONFIGURE_FILE( EditorGUI.tcl ${Editor_DIR}/EditorGUI.tcl COPYONLY)
