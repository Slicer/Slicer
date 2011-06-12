
#-----------------------------------------------------------------------------
set(proj itcl)
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

set(itcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/incrTcl")
set(itcl_BUILD_IN_SOURCE 0)
set(itcl_CONFIGURE_COMMAND "")
set(itcl_BUILD_COMMAND "")
set(itcl_INSTALL_COMMAND "")
set(itcl_PATCH_COMMAND "")

if(APPLE)
  set(itcl_BUILD_IN_SOURCE 1)

  set(itcl_configure ${tcl_base}/incrTcl/itcl/configure)
  set(itcl_configure_find "*.c | *.o | *.obj) \;\;")
  set(itcl_configure_replace "*.c | *.o | *.obj | *.dSYM | *.gnoc ) \;\;")

  set(script ${CMAKE_CURRENT_SOURCE_DIR}/CMake/StringFindReplace.cmake)
  set(in ${itcl_configure})
  set(out ${itcl_configure})

  set(itcl_PATCH_COMMAND ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=${itcl_configure_find} -Dreplace=${itcl_configure_replace} -P ${script})

  set(itcl_CONFIGURE_COMMAND ./configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(itcl_BUILD_COMMAND make)
  set(itcl_INSTALL_COMMAND make install)
  
else()
  set(itcl_BUILD_IN_SOURCE 1)
  set(itcl_CONFIGURE_COMMAND sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(itcl_BUILD_COMMAND make ${parallelism_level} all)
  set(itcl_INSTALL_COMMAND make install)
endif()

if(NOT WIN32)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY ${itcl_SVN_REPOSITORY}
    SOURCE_DIR tcl/incrTcl
    BUILD_IN_SOURCE ${itcl_BUILD_IN_SOURCE}
    PATCH_COMMAND ${itcl_PATCH_COMMAND}
    CONFIGURE_COMMAND ${itcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${itcl_BUILD_COMMAND}
    INSTALL_COMMAND ${itcl_INSTALL_COMMAND}
    DEPENDS 
      ${incrTcl_DEPENDENCIES}
  )

  ExternalProject_Add_Step(${proj} CHMOD_incrTcl_configure
    COMMAND chmod +x ${tcl_base}/incrTcl/configure
    DEPENDEES update
    DEPENDERS configure
  )
endif()
