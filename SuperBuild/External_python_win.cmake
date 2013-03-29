
if(NOT External_python_win_PROJECT_INCLUDED)
  message(FATAL_ERROR "This file shouldn't be included directly !")
endif()

#-----------------------------------------------------------------------------
# General purpose variables
#-----------------------------------------------------------------------------

# Path to solution file
set(python_sln ${CMAKE_BINARY_DIR}/${proj}-build/PCbuild/pcbuild.sln)
string(REPLACE "/" "\\" python_sln ${python_sln})

# Compute python home
get_filename_component(python_base ${python_sln} PATH)
get_filename_component(python_home ${python_base} PATH)


#-----------------------------------------------------------------------------
# 32-bit or 64-bit
#-----------------------------------------------------------------------------

if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
  set(python_build_type "Release")
  set(python_platform "x64")
  set(python_configuration "${python_build_type}|${python_platform}")
  set(PythonPCBuildDir ${CMAKE_BINARY_DIR}/python-build/PCbuild/amd64)
else()
  set(python_build_type "Release")
  set(python_platform "Win32")
  set(python_configuration "${python_build_type}|${python_platform}")
  set(PythonPCBuildDir ${CMAKE_BINARY_DIR}/python-build/PCbuild)
endif()

#-----------------------------------------------------------------------------
# Prepare patch command
#-----------------------------------------------------------------------------

# Point the tkinter build file to the slicer tcl-build
set(python_PATCH_COMMAND "")
if(Slicer_USE_PYTHONQT_WITH_TCL)
  set(python_tkinter ${python_base}/pyproject.vsprops)
  # The following if statement is specific to VS2010
  if("${MSVC_VERSION}" VERSION_GREATER "1599")
   set(python_tkinter ${python_base}/pyproject.props)
  endif()
  string(REPLACE "/" "\\" python_tkinter ${python_tkinter})

  set(script ${CMAKE_CURRENT_SOURCE_DIR}/CMake/SlicerBlockStringFindReplace.cmake)
  set(out ${python_tkinter})
  set(in ${python_tkinter})

  file(TO_CMAKE_PATH "${in}" in)
  file(TO_CMAKE_PATH "${out}" out)
  file(TO_CMAKE_PATH "${script}" script)
  set(python_SOURCE_DIR ${python_build})
  set(HAVE_PYRUN_CLOSEFILE 1)
  set(HAVE_PYRUN_OPENFILE 1)
  configure_file(SuperBuild/python_patch_step_win.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step.cmake
    @ONLY)
  set(python_PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/python_patch_step.cmake)
endif()

#-----------------------------------------------------------------------------
# Convenient helper macro
#-----------------------------------------------------------------------------

macro(set_ep_build_command_args target)
  set(ep_build_command_args /build ${python_configuration} /project ${target})
endmacro()

macro(build_python_target target depend)
  #message("build_python_target [${target}] depends on [${depend}]")
  set_ep_build_command_args(${target})
  ExternalProject_Add_Step(${proj} Build_${target}
    COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} ${ep_build_command_args}
    DEPENDEES ${depend}
    )
endmacro()

#-----------------------------------------------------------------------------
# Specify build steps
#-----------------------------------------------------------------------------

set_ep_build_command_args(select)
ExternalProject_Add(${proj}
  URL ${python_URL}
  URL_MD5 ${python_MD5}
  "${${PROJECT_NAME}_EP_DISABLED_UPDATE}"
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCE_DIR python-build
  PATCH_COMMAND ${python_PATCH_COMMAND}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} ${ep_build_command_args}
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ""
  DEPENDS
    ${python_DEPENDENCIES}
  )

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

set_ep_build_command_args(python)
ExternalProject_Add_Step(${proj} Build_python
  COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} ${ep_build_command_args}
  DEPENDEES Build__multiprocessing
  DEPENDERS install
  )

ExternalProject_Add_Step(${proj} CopyPythonLib
  COMMAND ${CMAKE_COMMAND} -E copy ${PythonPCBuildDir}/python${PYVER_SHORT}.lib ${CMAKE_BINARY_DIR}/python-build/Lib/python${PYVER_SHORT}.lib
  DEPENDEES install
  )
ExternalProject_Add_Step(${proj} Copy_socketPyd
  COMMAND ${CMAKE_COMMAND} -E copy ${PythonPCBuildDir}/_socket.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/_socket.pyd
  DEPENDEES install
  )
ExternalProject_Add_Step(${proj} Copy_ctypesPyd
  COMMAND ${CMAKE_COMMAND} -E copy ${PythonPCBuildDir}/_ctypes.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/_ctypes.pyd
  DEPENDEES install
  )
ExternalProject_Add_Step(${proj} Copy_pyexpatPyd
  COMMAND ${CMAKE_COMMAND} -E copy ${PythonPCBuildDir}/pyexpat.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/pyexpat.pyd
  DEPENDEES install
  )

ExternalProject_Add_Step(${proj} CopyPythonDll
  COMMAND ${CMAKE_COMMAND} -E copy ${PythonPCBuildDir}/python${PYVER_SHORT}.dll ${CMAKE_BINARY_DIR}/python-build/bin/${CMAKE_CFG_INTDIR}/python${PYVER_SHORT}.dll
  DEPENDEES install
  )

ExternalProject_Add_Step(${proj} CopyPyconfigHeader
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PC/pyconfig.h ${CMAKE_BINARY_DIR}/python-build/Include/pyconfig.h
  DEPENDEES install
  )

if(Slicer_USE_PYTHONQT_WITH_TCL)
  ExternalProject_Add_Step(${proj} Copy_tkinterPyd
    COMMAND ${CMAKE_COMMAND} -E copy ${PythonPCBuildDir}/_tkinter.pyd ${CMAKE_BINARY_DIR}/python-build/Lib/_tkinter.pyd
    DEPENDEES install
    )
endif()

#-----------------------------------------------------------------------------
# Set slicer_PYTHON_INCLUDE and slicer_PYTHON_LIBRARY variables
#
set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/Python-build/Include)
set(slicer_PYTHON_LIBRARY ${PythonPCBuildDir}/python${PYVER_SHORT}.lib)
set(slicer_PYTHON_EXECUTABLE ${PythonPCBuildDir}/python.exe)

