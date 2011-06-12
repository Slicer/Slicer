
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
IF(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  RETURN()
ENDIF()
SET(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(incrTcl_DEPENDENCIES tcl tk)

set(proj incrTcl)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)

#message(STATUS "Adding project '${proj}'")
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

set(incrTcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/incrTcl")
set(incrTcl_BUILD_IN_SOURCE 0)
set(incrTcl_CONFIGURE_COMMAND "")
set(incrTcl_BUILD_COMMAND "")
set(incrTcl_INSTALL_COMMAND "")
set(incrTcl_PATCH_COMMAND "")

if(APPLE)
  set(incrTcl_BUILD_IN_SOURCE 1)

  set(incrTcl_configure ${tcl_base}/incrTcl/itcl/configure)
  set(incrTcl_configure_find "*.c | *.o | *.obj) \;\;")
  set(incrTcl_configure_replace "*.c | *.o | *.obj | *.dSYM | *.gnoc ) \;\;")

  set(script ${CMAKE_CURRENT_SOURCE_DIR}/CMake/StringFindReplace.cmake)
  set(in ${incrTcl_configure})
  set(out ${incrTcl_configure})

  set(incrTcl_PATCH_COMMAND ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=${incrTcl_configure_find} -Dreplace=${incrTcl_configure_replace} -P ${script})

  set(incrTcl_CONFIGURE_COMMAND ./configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(incrTcl_BUILD_COMMAND make)
  set(incrTcl_INSTALL_COMMAND make install)
  
else()
  set(incrTcl_BUILD_IN_SOURCE 1)
  set(incrTcl_CONFIGURE_COMMAND sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(incrTcl_BUILD_COMMAND make ${parallelism_level} all)
  set(incrTcl_INSTALL_COMMAND make install)
endif()

if(NOT WIN32)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY ${incrTcl_SVN_REPOSITORY}
    SOURCE_DIR tcl/incrTcl
    BUILD_IN_SOURCE ${incrTcl_BUILD_IN_SOURCE}
    PATCH_COMMAND ${incrTcl_PATCH_COMMAND}
    CONFIGURE_COMMAND ${incrTcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${incrTcl_BUILD_COMMAND}
    INSTALL_COMMAND ${incrTcl_INSTALL_COMMAND}
    DEPENDS 
      ${incrTcl_DEPENDENCIES}
  )

  ExternalProject_Add_Step(${proj} CHMOD_incrTcl_configure
    COMMAND chmod +x ${tcl_base}/incrTcl/configure
    DEPENDEES update
    DEPENDERS configure
  )
endif()

