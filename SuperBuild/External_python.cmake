
superbuild_include_once()

# Set dependency list
set(python_DEPENDENCIES zlib)
if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_python)
  list(APPEND python_DEPENDENCIES CTKAPPLAUNCHER)
endif()
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    list(APPEND python_DEPENDENCIES tcl)
  else()
    list(APPEND python_DEPENDENCIES tcl tk)
  endif()
endif()
if(PYTHON_ENABLE_SSL)
  list(APPEND python_DEPENDENCIES OpenSSL)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(python)
set(proj python)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(PYTHON_INCLUDE_DIR CACHE)
  unset(PYTHON_LIBRARY CACHE)
  unset(PYTHON_EXECUTABLE CACHE)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)
  set(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
  set(PYTHON_LIBRARY ${PYTHON_LIBRARIES})
  set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
endif()

if((NOT DEFINED PYTHON_INCLUDE_DIRS
   OR NOT DEFINED PYTHON_LIBRARIES
   OR NOT DEFINED PYTHON_EXECUTABLE) AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
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

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  # Set CMake OSX variable to pass down the external project
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DENABLE_TKINTER:BOOL=ON
      -DTCL_LIBRARY:FILEPATH=${TCL_LIBRARY}
      -DTCL_INCLUDE_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/tcl-build/include
      -DTK_LIBRARY:FILEPATH=${TK_LIBRARY}
      -DTK_INCLUDE_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/tcl-build/include
      )
  else()
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DENABLE_TKINTER:BOOL=OFF
      )
  endif()

  if(PYTHON_ENABLE_SSL)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DENABLE_SSL:BOOL=ON
      -DOPENSSL_INCLUDE_DIR:PATH=${OPENSSL_INCLUDE_DIR}
      -DOPENSSL_LIBRARIES:STRING=${OPENSSL_LIBRARIES}
      )
  else()
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DENABLE_SSL:BOOL=OFF
      )
  endif()

  # Force Python build to "Release".
  set(SAVED_CMAKE_CFG_INTDIR ${CMAKE_CFG_INTDIR})
  set(CMAKE_CFG_INTDIR "Release")

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/davidsansome/python-cmake-buildsystem.git"
    GIT_TAG "892c95b5024a52ebd47a12292ff74ec7d713db24"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    LIST_SEPARATOR ${ep_list_separator}
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      #-DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags} # Not used
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/${proj}-install
      #-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      #-DBUILD_TESTING:BOOL=OFF
      -DBUILD_SHARED:BOOL=ON
      -DBUILD_STATIC:BOOL=OFF
      -DUSE_SYSTEM_LIBRARIES:BOOL=OFF
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    DEPENDS
      python-source ${python_DEPENDENCIES}
    )
  set(python_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  if(UNIX)
    set(python_IMPORT_SUFFIX so)
    if(APPLE)
      set(python_IMPORT_SUFFIX dylib)
    endif()
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/lib)
    set(PYTHON_INCLUDE_DIR ${python_DIR}/include/python2.7)
    set(PYTHON_LIBRARY ${python_DIR}/lib/libpython2.7.${python_IMPORT_SUFFIX})
    set(PYTHON_EXECUTABLE ${python_DIR}/bin/customPython)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python)
  elseif(WIN32)
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/bin)
    set(PYTHON_INCLUDE_DIR ${python_DIR}/include)
    set(PYTHON_LIBRARY ${python_DIR}/libs/python27.lib)
    set(PYTHON_EXECUTABLE ${python_DIR}/bin/customPython.exe)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python.exe)
  else()
    message(FATAL_ERROR "Unknown system !")
  endif()

  if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_python)

    # Configure python launcher
    configure_file(
      SuperBuild/python_customPython_configure.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
      @ONLY)
    set(python_customPython_configure_args)

    if(PYTHON_ENABLE_SSL)
      set(python_customPython_configure_args -DOPENSSL_EXPORT_LIBRARY_DIR:PATH=${OPENSSL_EXPORT_LIBRARY_DIR})
    endif()

    ExternalProject_Add_Step(${proj} python_customPython_configure
      COMMAND ${CMAKE_COMMAND} ${python_customPython_configure_args}
        -P ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
      DEPENDEES install
      )

  endif()

  # Since fixup_bundle expects the library to be writable, let's add an extra step
  # to make sure it's the case.
  if(APPLE)
    ExternalProject_Add_Step(${proj} python_install_chmod_library
      COMMAND chmod u+xw ${PYTHON_LIBRARY}
      DEPENDEES install
      )
  endif()

  set(CMAKE_CFG_INTDIR ${SAVED_CMAKE_CFG_INTDIR}) # Restore CMAKE_CFG_INTDIR

else()
  # The project is provided using python_DIR, nevertheless since other project may depend on python,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${python_DEPENDENCIES}")
endif()

message(STATUS "${__${proj}_superbuild_message} - PYTHON_INCLUDE_DIR:${PYTHON_INCLUDE_DIR}")
message(STATUS "${__${proj}_superbuild_message} - PYTHON_LIBRARY:${PYTHON_LIBRARY}")
message(STATUS "${__${proj}_superbuild_message} - PYTHON_EXECUTABLE:${PYTHON_EXECUTABLE}")
