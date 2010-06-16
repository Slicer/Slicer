
#-----------------------------------------------------------------------------
set(proj iwidgets)

set(iwidgets_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/iwidgets")
set(iwidgets_BUILD_IN_SOURCE 0)
set(iwidgets_CONFIGURE "")
set(iwidgets_BUILD "")
set(iwidgets_INSTALL "")

if(NOT WIN32)
  set(iwidgets_BUILD_IN_SOURCE 1)
  set(iwidgets_CONFIGURE sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --with-itcl=${tcl_base}/incrTcl --prefix=${tcl_build})
  set(iwidgets_BUILD make all) # iwidgets doesn't build in parallel
  set(iwidgets_INSTALL make install)
  
  ExternalProject_Add(${proj}
    SVN_REPOSITORY ${iwidgets_SVN}
    SOURCE_DIR tcl/iwidgets
    BUILD_IN_SOURCE ${iwidgets_BUILD_IN_SOURCE}
    CONFIGURE_COMMAND ${iwidgets_CONFIGURE}
    BUILD_COMMAND ${iwidgets_BUILD}
    INSTALL_COMMAND ${iwidgets_INSTALL}
    DEPENDS 
      ${iwidgets_DEPENDENCIES}
  )
endif()
