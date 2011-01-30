#-----------------------------------------------------------------------------
set(proj python)
set(python_base ${CMAKE_CURRENT_BINARY_DIR}/${proj})
set(python_build ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
set(python_URL http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Python-2.6.6.tgz)

if(WIN32)

  set(python_sln ${CMAKE_BINARY_DIR}/${proj}-build/PCbuild/pcbuild.sln)
  string(REPLACE "/" "\\" python_sln ${python_sln})

  get_filename_component(python_base ${python_sln} PATH)
  get_filename_component(python_home ${python_base} PATH)

  # point the tkinter build file to the slicer tcl-build
  set(python_PATCH_COMMAND)
  if(Slicer_USE_PYTHONQT_WITH_TCL)
    set(python_tkinter ${python_base}/pyproject.vsprops)
    string(REPLACE "/" "\\" python_tkinter ${python_tkinter})

    set(script ${CMAKE_CURRENT_SOURCE_DIR}/CMake/StringFindReplace.cmake)
    set(out ${python_tkinter})
    set(in ${python_tkinter})

    set(python_PATCH_COMMAND 
      ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=tcltk\" -Dreplace=tcl-build\" -P ${script})
  endif()

  set(python_SOURCE_DIR ${python_build})
  configure_file(SuperBuild/python_patch_step_pythonrun.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step_pythonrun.cmake
    @ONLY)
  set(python_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step_pythonrun.cmake)

  ExternalProject_Add(${proj}
    URL ${python_URL}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR python-build
    UPDATE_COMMAND ${python_CONFIGURE_COMMAND}
    PATCH_COMMAND ${python_PATCH_COMMAND}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project select
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    DEPENDS 
      ${python_DEPENDENCIES}
  )

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    # this must match the version of tcl we are building for slicer.
    ExternalProject_Add_Step(${proj} Patch_tcltk_version
      COMMAND ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=85 -Dreplace=84 -P ${script}
      DEPENDEES configure
      DEPENDERS build
      )
  endif()
  
  # Convenient helper macro
  macro(build_python_target target depend)
    ExternalProject_Add_Step(${proj} Build_${target}
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project ${target}
      DEPENDEES ${depend}
      )
  endmacro(build_python_target)

  build_python_target(make_versioninfo build)
  build_python_target(make_buildinfo Build_make_versioninfo)
  build_python_target(kill_python Build_make_buildinfo)
  build_python_target(w9xpopen Build_kill_python)
  build_python_target(pythoncore Build_w9xpopen)
  build_python_target(_socket Build_pythoncore)

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    build_python_target(_tkinter Build__socket)
    build_python_target(_testcapi Build__tkinter)
  else()
    build_python_target(_testcapi Build__socket)
  endif()

  build_python_target(_msi Build__testcapi)
  build_python_target(_elementtree Build__msi)
  build_python_target(_ctypes_test Build__elementtree)
  build_python_target(_ctypes Build__ctypes_test)
  build_python_target(winsound Build__ctypes)
  build_python_target(pyexpat Build_winsound)
  build_python_target(pythonw Build_pyexpat)
  build_python_target(_multiprocessing Build_pythonw)
  
  ExternalProject_Add_Step(${proj} Build_python
    COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project python
    DEPENDEES Build__multiprocessing
    DEPENDERS install
    )

  ExternalProject_Add_Step(${proj} CopyPythonLib
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PCbuild/python26.lib ${CMAKE_BINARY_DIR}/python-build/Lib/python26.lib
    DEPENDEES install
    )
  ExternalProject_Add_Step(${proj} Copy_socketPyd
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PCbuild/_socket.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/_socket.pyd
    DEPENDEES install
    )
  ExternalProject_Add_Step(${proj} Copy_ctypesPyd
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PCbuild/_ctypes.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/_ctypes.pyd
    DEPENDEES install
    )

  ExternalProject_Add_Step(${proj} CopyPythonDll
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PCbuild/python26.dll ${CMAKE_BINARY_DIR}/Slicer-build/bin/${CMAKE_CFG_INTDIR}/python26.dll
    DEPENDEES install
    )

  ExternalProject_Add_Step(${proj} CopyPyconfigHeader
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PC/pyconfig.h ${CMAKE_BINARY_DIR}/python-build/Include/pyconfig.h
    DEPENDEES install
    )

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    ExternalProject_Add_Step(${proj} Copy_tkinterPyd
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PCbuild/_tkinter.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/_tkinter.pyd
      DEPENDEES install
      )
  endif()
    
elseif(UNIX)
  set(python_SOURCE_DIR python)
  set(python_BUILD_IN_SOURCE 1)
  
  configure_file(SuperBuild/python_patch_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step.cmake
    @ONLY)
    
  configure_file(SuperBuild/python_configure_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_configure_step.cmake
    @ONLY)
  
  configure_file(SuperBuild/python_make_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_make_step.cmake
    @ONLY)
    
  configure_file(SuperBuild/python_install_step.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_install_step.cmake
    @ONLY)

  set(python_PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step.cmake)
  set(python_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_configure_step.cmake)
  set(python_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_make_step.cmake)
  set(python_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_install_step.cmake)
  
  ExternalProject_Add(${proj}
    URL ${python_URL}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR ${python_SOURCE_DIR}
    BUILD_IN_SOURCE ${python_BUILD_IN_SOURCE}
    PATCH_COMMAND ${python_PATCH_COMMAND}
    CONFIGURE_COMMAND ${python_CONFIGURE_COMMAND}
    BUILD_COMMAND ${python_BUILD_COMMAND}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ${python_INSTALL_COMMAND}
    DEPENDS 
      ${python_DEPENDENCIES}
    )
    
endif()

#-----------------------------------------------------------------------------
# Set slicer_PYTHON_INCLUDE and slicer_PYTHON_LIBRARY variables
#

set(slicer_PYTHON_INCLUDE)
set(slicer_PYTHON_LIBRARY)
set(slicer_PYTHON_EXECUTABLE)

if(WIN32)
  set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/Python-build/Include)
  set(slicer_PYTHON_LIBRARY ${CMAKE_BINARY_DIR}/Python-build/PCbuild/python26.lib)
  set(slicer_PYTHON_EXECUTABLE ${CMAKE_BINARY_DIR}/Python-build/PCbuild/python.exe)
elseif(APPLE)
  set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/python-build/include/python2.6)
  set(slicer_PYTHON_LIBRARY ${CMAKE_BINARY_DIR}/python-build/lib/libpython2.6.dylib)
  set(slicer_PYTHON_EXECUTABLE ${CMAKE_BINARY_DIR}/python-build/bin/python)
else()
  set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/python-build/include/python2.6)
  set(slicer_PYTHON_LIBRARY ${CMAKE_BINARY_DIR}/python-build/lib/libpython2.6.so)
  set(slicer_PYTHON_EXECUTABLE ${CMAKE_BINARY_DIR}/python-build/bin/python)
endif()


