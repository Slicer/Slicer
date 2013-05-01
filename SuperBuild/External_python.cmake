
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(python_DEPENDENCIES CTKAPPLAUNCHER)
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    list(APPEND python_DEPENDENCIES tcl)
  else()
    list(APPEND python_DEPENDENCIES tcl tk)
  endif()
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(python)
set(proj python)

#message(STATUS "${__indent}Adding project ${proj}")

#
# WARNING - If you consider updating the Python version, make sure the patch
#           step associated with both window and unix are still valid !
#

if(NOT DEFINED python_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  set(python_SOURCE_DIR "${CMAKE_BINARY_DIR}/Python-2.7.3")

  ExternalProject_Add(python-source
    URL "http://www.python.org/ftp/python/2.7.3/Python-2.7.3.tgz"
    URL_MD5 "2cf641732ac23b18d139be077bd906cd"
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR ${python_SOURCE_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  set(python_TCL_EXTERNAL_PROJECT_ARGS)
  if(Slicer_USE_PYTHONQT_WITH_TCL)
    if(WIN32)
      set(tcl_library ${CMAKE_CURRENT_BINARY_DIR}/tcl-build/lib/tcl${TCL_TK_VERSION}.lib)
      set(tk_library ${CMAKE_CURRENT_BINARY_DIR}/tcl-build/lib/tk${TCL_TK_VERSION}.lib)
    else()
      set(tcl_library ${CMAKE_CURRENT_BINARY_DIR}/tcl-build/lib/libtcl${TCL_TK_VERSION_DOT}${CMAKE_SHARED_LIBRARY_SUFFIX})
      set(tk_library ${CMAKE_CURRENT_BINARY_DIR}/tcl-build/lib/libtk${TCL_TK_VERSION_DOT}${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    message(STATUS "TCL_LIBRARY:${tcl_library}")
    message(STATUS "TK_LIBRARY:${tk_library}")
    list(APPEND python_TCL_EXTERNAL_PROJECT_ARGS
      -DENABLE_TKINTER:BOOL=ON
      -DTCL_LIBRARY:FILEPATH=${tcl_library}
      -DTCL_INCLUDE_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/tcl-build/include
      -DTK_LIBRARY:FILEPATH=${tk_library}
      -DTK_INCLUDE_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/tcl-build/include
      )
  else()
    list(APPEND python_TCL_EXTERNAL_PROJECT_ARGS
      -DENABLE_TKINTER:BOOL=OFF
      )
  endif()

  # Force Python build to "Release".
  set(SAVED_CMAKE_CFG_INTDIR ${CMAKE_CFG_INTDIR})
  set(CMAKE_CFG_INTDIR "Release")

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/davidsansome/python-cmake-buildsystem.git"
    GIT_TAG "60ee8280184b5c2ace915777193f457df57f605a"
    "${${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288}"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      #-DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags} # Not used
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/${proj}-install
      #-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      #-DBUILD_TESTING:BOOL=OFF
      -DBUILD_SHARED:BOOL=ON
      -DBUILD_STATIC:BOOL=OFF
      -DUSE_SYSTEM_LIBRARIES:BOOL=OFF
      ${python_TCL_EXTERNAL_PROJECT_ARGS}
    DEPENDS
      python-source ${python_DEPENDENCIES}
    )
  set(python_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  set(CMAKE_CFG_INTDIR ${SAVED_CMAKE_CFG_INTDIR}) # Restore CMAKE_CFG_INTDIR

  if(UNIX)
    set(python_IMPORT_SUFFIX so)
    if(APPLE)
      set(python_IMPORT_SUFFIX dylib)
    endif()
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/lib)
    set(slicer_PYTHON_INCLUDE ${python_DIR}/include/python2.7)
    set(slicer_PYTHON_LIBRARY ${python_DIR}/lib/libpython2.7.${python_IMPORT_SUFFIX})
    set(slicer_PYTHON_EXECUTABLE ${python_DIR}/bin/customPython)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python)
  elseif(WIN32)
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/bin)
    set(slicer_PYTHON_INCLUDE ${python_DIR}/include)
    set(slicer_PYTHON_LIBRARY ${python_DIR}/libs/python27.lib)
    set(slicer_PYTHON_EXECUTABLE ${python_DIR}/bin/python.exe)
    set(slicer_PYTHON_REAL_EXECUTABLE ${slicer_PYTHON_EXECUTABLE})
  else()
    message(FATAL_ERROR "Unknown system !")
  endif()
else()
  # The project is provided using python_DIR, nevertheless since other project may depend on python,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${python_DEPENDENCIES}")
endif()



message(STATUS "slicer_PYTHON_INCLUDE:${slicer_PYTHON_INCLUDE}")
message(STATUS "slicer_PYTHON_LIBRARY:${slicer_PYTHON_LIBRARY}")
message(STATUS "slicer_PYTHON_EXECUTABLE:${slicer_PYTHON_EXECUTABLE}")

if(UNIX)
  configure_file(
    SuperBuild/python_customPython_configure.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
    @ONLY)
  ExternalProject_Add_Step(${proj} python_customPython_configure
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
    DEPENDEES install
    )
endif()

#-----------------------------------------------------------------------------
# Since fixup_bundle expects the library to be writable, let's add an extra step
# to make sure it's the case.
if(APPLE)
  ExternalProject_Add_Step(${proj} python_install_chmod_library
    COMMAND chmod u+xw ${slicer_PYTHON_LIBRARY}
    DEPENDEES install
    )
endif()
