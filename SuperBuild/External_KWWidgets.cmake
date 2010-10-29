
#-----------------------------------------------------------------------------
# Get and build kwwidgets

set(proj KWWidgets)
set(kwwidgets_tag Slicer-3-6)

ExternalProject_Add(${proj}
  CVS_REPOSITORY ":pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets"
  CVS_MODULE "KWWidgets"
  CVS_TAG -r ${kwwidgets_tag}
  UPDATE_COMMAND ""
  SOURCE_DIR KWWidgets
  BINARY_DIR KWWidgets-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_args}
    -DKWWidgets_BUILD_TESTING:BOOL=OFF
    -DBUILD_EXAMPLES:BOOL=OFF
    -DKWWidgets_BUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_SKIP_RPATH:BOOL=ON
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DVTK_DIR:PATH=${VTK_DIR}
  INSTALL_COMMAND ""
  DEPENDS
    ${KWWidgets_DEPENDENCIES}
)

set(KWWidgets_DIR ${CMAKE_BINARY_DIR}/KWWidgets-build)
