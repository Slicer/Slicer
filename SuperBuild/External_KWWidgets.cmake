
#-----------------------------------------------------------------------------
# Get and build kwwidgets

set(proj KWWidgets)
set(kwwidgets_tag Slicer-3-6)

ExternalProject_Add(${proj}
  DEPENDS ${KWWidgets_DEPENDENCIES}
  CVS_REPOSITORY ":pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets"
  CVS_MODULE "KWWidgets"
  CVS_TAG -r ${kwwidgets_tag}
  UPDATE_COMMAND ""
  SOURCE_DIR KWWidgets
  BINARY_DIR KWWidgets-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DVTK_DIR:PATH=${VTK_DIR}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_SKIP_RPATH:BOOL=ON
    -DBUILD_EXAMPLES:BOOL=OFF
    -DKWWidgets_BUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DKWWidgets_BUILD_TESTING:BOOL=OFF
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  INSTALL_COMMAND ""
)
