
#-----------------------------------------------------------------------------

set(proj blt)

set(blt_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/blt")
set(blt_BUILD_IN_SOURCE 0)
set(blt_CONFIGURE "")
set(blt_BUILD "")
set(blt_INSTALL "")
set(blt_PATCH "")

if(APPLE)
  set(blt_BUILD_IN_SOURCE 1)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/blt_download_tcl84_patch.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/blt_download_tcl84_patch.cmake
    @ONLY)

  set(blt_PATCH patch -p2 -t -N < ${tcl_base}/blt/bltpatch)
  set(blt_CONFIGURE sh configure --with-tcl=${tcl_base}/tcl/unix --with-tk=${tcl_build} --prefix=${tcl_build} --enable-shared --x-includes=/usr/X11R6/include --x-libraries=/usr/X11R6/lib --with-cflags=-fno-common)
  set(blt_BUILD make)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/blt_install_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/blt_install_step.cmake
    @ONLY)

  set(blt_INSTALL ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/blt_install_step.cmake)
else()
  set(blt_BUILD_IN_SOURCE 1)
  set(blt_CONFIGURE sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(blt_BUILD make)
  set(blt_INSTALL make install)
endif()

if(NOT WIN32)
  ExternalProject_Add(${proj}
    DEPENDS ${blt_DEPENDENCIES}
    SVN_REPOSITORY ${blt_SVN}
    SOURCE_DIR tcl/blt
    BUILD_IN_SOURCE ${blt_BUILD_IN_SOURCE}
    PATCH_COMMAND ${blt_PATCH}
    CONFIGURE_COMMAND ${blt_CONFIGURE}
    BUILD_COMMAND ${blt_BUILD}
    INSTALL_COMMAND ${blt_INSTALL}
  )
endif()

if(APPLE)
  ExternalProject_Add_Step(${proj} Download_blt_patch
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/blt_download_tcl84_patch.cmake
    WORKING_DIRECTORY ${tcl_base}/blt
    DEPENDEES update
    DEPENDERS patch
  )
endif()

