

project(Slicer3)

enable_language(C)
enable_language(CXX)

cmake_minimum_required(VERSION 2.8)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

#
#                         ----------------------------
#                         | Case 1 | Case 2 | Case 3 |
# ----------------------------------------------------
# | Slicer3_USE_KWWIDGETS |  ON    |  ON    |  OFF   |
# |---------------------------------------------------
# | Slicer3_USE_QT        |  OFF   |  ON    |  ON    |
# |--------------------------------------------------|
# | Slicer3_USE_PYTHON    | ON|OFF | ON|OFF |  OFF   |
# |--------------------------------------------------|
# | Slicer3_USE_PYTHONQT  |   OFF  |  OFF   | ON|OFF |
# |---------------------------------------------------
#
# Case 1: [Slicer3_USE_KWWIDGETS: ON, Slicer3_USE_QT: OFF, VTK_WRAP_TCL:ON]
#   a) KwwOnly-NoPython  
#   b) KwwOnly-Python    
# 
# Case 2: [Slicer3_USE_KWWIDGETS: ON, Slicer3_USE_QT: ON, VTK_WRAP_TCL:ON]
#   a) KwwAndQt-NoPython-NoPythonQt
#   b) KwwAndQt-Python
#   c) KwwAndQt-PythonQt          [VTK_WRAP_PYTHON: ON]  -> TODO
#   d) KwwAndQt-Python-PythonQt   [VTK_WRAP_PYTHON: ON]  -> TODO
#
# Case 3: [Slicer3_USE_KWWIDGETS: OFF, Slicer3_USE_QT: ON,  VTK_WRAP_TCL:OFF]
#   a) QtOnly-NoPythonQt
#   b) QtOnly-PythonQt            [VTK_WRAP_PYTHON: ON]
#
#
# Note:
#   Tcl, Tk, blt, incrTcl and iwidgets are build only if Slicer3_USE_KWWIDGETS is ON
#   Python is build if either Slicer3_USE_PYTHON or Slicer3_USE_PYTHONQT are ON
# 

#-----------------------------------------------------------------------------
# Update CMake module path
#
set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/CMake
  ${CMAKE_BINARY_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )
  
#-----------------------------------------------------------------------------
# Convenient macro allowing to download a file
#
MACRO(downloadFile url dest)
  FILE(DOWNLOAD ${url} ${dest} STATUS status)
  LIST(GET status 0 error_code)
  LIST(GET status 1 error_msg)
  IF(error_code)
    MESSAGE(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  ENDIF()
ENDMACRO()

#-----------------------------------------------------------------------------
# Prerequisites
#
set(git_repo http://github.com/pieper/CTK/raw/master/)

set(file CMake/ExternalProject.cmake)
downloadFile(${git_repo}/${file} ${CMAKE_BINARY_DIR}/${file})

#-----------------------------------------------------------------------------
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#
if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../CMake)

include(ExternalProject)

#-----------------------------------------------------------------------------
# Python
#
option(Slicer3_USE_PYTHON "Build Slicer3 with Python support" OFF)

if(Slicer3_USE_PYTHON)

  # Attempt to detect if we have a Fortran Compiler available.
  include(CheckFortran)

  if(CMAKE_Fortran_COMPILER)
    option(Slicer3_BUILD_NUMPY "Build Slicer with NumPy (builds Blas and Lapack)." ON)

    if(Slicer3_BUILD_NUMPY)
      option(Slicer3_BUILD_SCIPY "Build Slicer with SciPy (requires NumPy)" ON)
    endif()

  endif(CMAKE_Fortran_COMPILER)
endif() # Slicer3_USE_PYTHON

#-----------------------------------------------------------------------------
# KWWidgets on by default.

option(Slicer3_USE_KWWIDGETS "Build Slicer3 KWWidgets GUI" ON)

#-----------------------------------------------------------------------------
# QT
#

option(Slicer3_USE_QT  "Build Slicer3 Qt GUI" OFF)

if(Slicer3_USE_QT)
  find_package(Qt4 REQUIRED)
  # Enable modules
  SET(QT_USE_QTNETWORK ON)
  # Includes Qt headers
  INCLUDE(${QT_USE_FILE})

  option(Slicer3_USE_PYTHONQT "Integrate a python-QT interpreter into Slicer." OFF)
  if (Slicer3_USE_PYTHONQT)
  
  endif()
  
endif()

#-----------------------------------------------------------------------------
# Check options
#
if (NOT Slicer3_USE_KWWIDGETS AND NOT Slicer3_USE_QT)
  set(err "error: Slicer SuperBuild requires at least Slicer3_USE_KWWIDGETS or Slicer3_USE_QT to be ON.

Slicer3_USE_KWWIDGETS = ON
 -> Means Slicer will be built only with KWWidgets support.
 
Slicer3_USE_KWWIDGETS = ON and Slicer3_USE_QT = ON
 -> Means Slicer will be built with both KWWidgets and QT support.
 
Slicer3_USE_QT = ON
 -> Means Slicer will be build only with QT support.
")

  message(FATAL_ERROR "${err}")
endif()

if (Slicer3_USE_PYTHON AND Slicer3_USE_PYTHONQT)
  set(err "error: Slicer SuperBuild doesn't support Slicer3_USE_PYTHON and Slicer3_USE_PYTHONQT ON.")

  message(FATAL_ERROR "${err}")
endif()

#-----------------------------------------------------------------------------
# Setup CMake 
#

set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
#set_property(DIRECTORY PROPERTY EP_BASE ${base})

set(prefix "${base}/Install")

# Let's use shared libraries
set(BUILD_SHARED_LIBS ON)
#option(BUILD_SHARED_LIBS "Build with shared libraries." ON)

# By default, let's build as Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# let a dashboard override the default.
if(CTEST_BUILD_CONFIGURATION)
  set(CMAKE_BUILD_TYPE "${CTEST_BUILD_CONFIGURATION}")
endif()

# Compute -G arg for configuring external projects with the same CMake generator:
#
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

set(parallelism_level)
set(build_dir ${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
# VTK option for Mac when QT is enabled
#
set(mac_args)
if(APPLE)
  # With Qt 4.4, VTK_USE_QVTK means we should use Carbon, not Cocoa:
  set(mac_args
    -DVTK_USE_CARBON:BOOL=ON
    -DVTK_USE_COCOA:BOOL=OFF
    )
endif()

option(Slicer3_USE_VTK_CVSHEAD "Use VTK CVS head when building." OFF)
option(Slicer3_USE_VTK_DEBUG_LEAKS "Turn on VTKs Debug Leaks functionality in both VTK and Slicer." ON)

#-----------------------------------------------------------------------------
# TCL
set(tcl_build)
if(Slicer3_USE_KWWIDGETS)
  set(proj tcl)

  set(tcl_SVN_REPOSITORY)
  set(tcl_SOURCE_DIR "")
  set(tcl_BINARY_DIR "")
  set(tcl_BUILD_IN_SOURCE 0)
  set(tcl_CONFIGURE_COMMAND "")
  set(tcl_BUILD_COMMAND "")
  set(tcl_INSTALL_COMMAND "")
  set(tcl_base ${CMAKE_CURRENT_BINARY_DIR}/tcl)
  set(tcl_build ${CMAKE_CURRENT_BINARY_DIR}/tcl-build)


  if(WIN32)
    set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/Binaries/Windows/tcl-build")
    set(tcl_SOURCE_DIR tcl-build)
  else()
    set(tcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tcl")
    set(tcl_SOURCE_DIR tcl/tcl)
    set(tcl_BUILD_IN_SOURCE 1)

    # configure, make and make install all need to be executed in tcl/unix. External_Project
    # doesn't provide any way to set the working directory for each step so we do so by
    # configuring a script that has an execute_process command that has the correct working
    # directory   
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tcl_configure_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tcl_configure_step.cmake
      @ONLY)

    set(tcl_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tcl_configure_step.cmake)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tcl_make_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tcl_make_step.cmake
      @ONLY)

    set(tcl_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tcl_make_step.cmake)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tcl_install_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tcl_install_step.cmake
      @ONLY)

    set(tcl_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tcl_install_step.cmake)
  endif()

  ExternalProject_Add(${proj}
    SVN_REPOSITORY ${tcl_SVN_REPOSITORY}
    SOURCE_DIR ${tcl_SOURCE_DIR}
    BUILD_IN_SOURCE ${tcl_BUILD_IN_SOURCE}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ${tcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${tcl_BUILD_COMMAND}
    INSTALL_COMMAND ${tcl_INSTALL_COMMAND}
  )

endif(Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
if(Slicer3_USE_KWWIDGETS)
  set(proj tk)

  set(tk_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/tk")
  set(tk_SOURCE_DIR "")
  set(tk_BINARY_DIR "")
  set(tk_BUILD_IN_SOURCE 0)
  set(tk_CONFIGURE_COMMAND "")
  set(tk_BUILD_COMMAND "")
  set(tk_INSTALL_COMMAND "")

  if(WIN32)
    set(tk_SOURCE_DIR tcl/tk)
  else()
    set(tk_SOURCE_DIR tcl/tk)
    set(tk_BUILD_IN_SOURCE 1)

    # configure, make and make install all need to be executed in tk/unix. External_Project
    # doesn't provide any way to set the working directory for each step so we do so by
    # configuring a script that has an execute_process command that has the correct working
    # directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tk_configure_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tk_configure_step.cmake
      @ONLY)

    set(tk_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tk_configure_step.cmake)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tk_make_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tk_make_step.cmake
      @ONLY)

    set(tk_BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tk_make_step.cmake)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tk_install_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/tk_install_step.cmake
      @ONLY)

    set(tk_INSTALL_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/tk_install_step.cmake)
  endif()

  ExternalProject_Add(${proj}
    DEPENDS tcl
    SVN_REPOSITORY ${tk_SVN_REPOSITORY}
    SOURCE_DIR ${tk_SOURCE_DIR}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE ${tk_BUILD_IN_SOURCE}
    CONFIGURE_COMMAND ${tk_CONFIGURE_COMMAND}
    BUILD_COMMAND ${tk_BUILD_COMMAND}
    INSTALL_COMMAND ${tk_INSTALL_COMMAND}
  )

  if(NOT WIN32)
    ExternalProject_Add_Step(${proj} Install_default.h
      COMMAND ${CMAKE_COMMAND} -E copy ${tcl_base}/tk/generic/default.h ${tcl_build}/include
      DEPENDEES install
    )

    ExternalProject_Add_Step(${proj} Install_tkUnixDefault.h
      COMMAND ${CMAKE_COMMAND} -E copy ${tcl_base}/tk/unix/tkUnixDefault.h ${tcl_build}/include
      DEPENDEES install
    )
  endif(NOT WIN32)
endif(Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
if(Slicer3_USE_KWWIDGETS)
  set(proj itcl)

  set(itcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/incrTcl")
  set(itcl_BUILD_IN_SOURCE 0)
  set(itcl_CONFIGURE_COMMAND "")
  set(itcl_BUILD_COMMAND "")
  set(itcl_INSTALL_COMMAND "")
  set(itcl_PATCH_COMMAND "")

  if(WIN32)
  elseif(APPLE)
    set(itcl_BUILD_IN_SOURCE 1)

    set(itcl_configure ${tcl_base}/incrTcl/itcl/configure)
    set(itcl_configure_find "*.c | *.o | *.obj) \;\;")
    set(itcl_configure_replace "*.c | *.o | *.obj | *.dSYM | *.gnoc ) \;\;")

    set(script ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/StringFindReplace.cmake)
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

  ExternalProject_Add(${proj}
    DEPENDS tk
    SVN_REPOSITORY ${itcl_SVN_REPOSITORY}
    SOURCE_DIR tcl/incrTcl
    BUILD_IN_SOURCE ${itcl_BUILD_IN_SOURCE}
    PATCH_COMMAND ${itcl_PATCH_COMMAND}
    CONFIGURE_COMMAND ${itcl_CONFIGURE_COMMAND}
    BUILD_COMMAND ${itcl_BUILD_COMMAND}
    INSTALL_COMMAND ${itcl_INSTALL_COMMAND}
  )

  if(NOT WIN32)
    ExternalProject_Add_Step(${proj} CHMOD_incrTcl_configure
      COMMAND chmod +x ${tcl_base}/incrTcl/configure
      DEPENDEES update
      DEPENDERS configure
    )
  endif(NOT WIN32)

endif(Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
if(Slicer3_USE_KWWIDGETS)
  set(proj iwidgets)

  set(iwidgets_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/iwidgets")
  set(iwidgets_BUILD_IN_SOURCE 0)
  set(iwidgets_CONFIGURE "")
  set(iwidgets_BUILD "")
  set(iwidgets_INSTALL "")

  if(WIN32)
  else()
    set(iwidgets_BUILD_IN_SOURCE 1)
    set(iwidgets_CONFIGURE sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --with-itcl=${tcl_base}/incrTcl --prefix=${tcl_build})
    set(iwidgets_BUILD make all) # iwidgets doesn't build in parallel
    set(iwidgets_INSTALL make install)
  endif()

  ExternalProject_Add(${proj}
    DEPENDS tcl itcl
    SVN_REPOSITORY ${iwidgets_SVN}
    SOURCE_DIR tcl/iwidgets
    BUILD_IN_SOURCE ${iwidgets_BUILD_IN_SOURCE}
    CONFIGURE_COMMAND ${iwidgets_CONFIGURE}
    BUILD_COMMAND ${iwidgets_BUILD}
    INSTALL_COMMAND ${iwidgets_INSTALL}
  )
endif(Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
if(Slicer3_USE_KWWIDGETS)
  set(proj blt)

  set(blt_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/blt")
  set(blt_BUILD_IN_SOURCE 0)
  set(blt_CONFIGURE "")
  set(blt_BUILD "")
  set(blt_INSTALL "")
  set(blt_PATCH "")

  if(WIN32)
  elseif(APPLE)
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

  ExternalProject_Add(${proj}
    DEPENDS tcl tk
    SVN_REPOSITORY ${blt_SVN}
    SOURCE_DIR tcl/blt
    BUILD_IN_SOURCE ${blt_BUILD_IN_SOURCE}
    PATCH_COMMAND ${blt_PATCH}
    CONFIGURE_COMMAND ${blt_CONFIGURE}
    BUILD_COMMAND ${blt_BUILD}
    INSTALL_COMMAND ${blt_INSTALL}
  )

  if(APPLE)
    ExternalProject_Add_Step(${proj} Download_blt_patch
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/blt_download_tcl84_patch.cmake
      WORKING_DIRECTORY ${tcl_base}/blt
      DEPENDEES update
      DEPENDERS patch
    )
  endif(APPLE)

endif(Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
set(proj python)

set(python_DEPENDS )

if(Slicer3_USE_PYTHON)
  list(APPEND python_DEPENDS tcl tk)
endif(Slicer3_USE_PYTHON)

if(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)

  if(WIN32)
  
    set(python_sln ${CMAKE_BINARY_DIR}/${proj}-build/PCbuild/pcbuild.sln)
    string(REPLACE "/" "\\" python_sln ${python_sln})

    # point the tkinter build file to the slicer tcl-build 

    get_filename_component(python_base ${python_sln} PATH)
    get_filename_component(python_home ${python_base} PATH)

    set(python_tkinter ${python_base}/pyproject.vsprops)
    string(REPLACE "/" "\\" python_tkinter ${python_tkinter})

    set(script ${CMAKE_CURRENT_SOURCE_DIR}/StringFindReplace.cmake)
    set(out ${python_tkinter})
    set(in ${python_tkinter})

    ExternalProject_Add(${proj}
      DEPENDS ${python_DEPENDS}
      SVN_REPOSITORY "http://svn.python.org/projects/python/branches/release26-maint"
      SOURCE_DIR python-build
      UPDATE_COMMAND ""
      PATCH_COMMAND ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=tcltk\" -Dreplace=tcl-build\" -P ${script}
      CONFIGURE_COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /Upgrade
      BUILD_COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project select
      BUILD_IN_SOURCE 1
      INSTALL_COMMAND ""
    )

    # this must match the version of tcl we are building for slicer.
    ExternalProject_Add_Step(${proj} Patch_tcltk_version
      COMMAND ${CMAKE_COMMAND} -Din=${in} -Dout=${out} -Dfind=85 -Dreplace=84 -P ${script}
      DEPENDEES configure
      DEPENDERS build
      )

    ExternalProject_Add_Step(${proj} Build_make_versioninfo
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project make_versioninfo
      DEPENDEES configure
      )
      
    ExternalProject_Add_Step(${proj} Build_make_buildinfo
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project make_buildinfo
      DEPENDEES Build_make_versioninfo
      )
      
    ExternalProject_Add_Step(${proj} Build_kill_python
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project kill_python
      DEPENDEES Build_kill_python
      )
      
    ExternalProject_Add_Step(${proj} Build_w9xpopen
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project w9xpopen
      DEPENDEES Build_kill_python
      )
      
    ExternalProject_Add_Step(${proj} Build_pythoncore
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project pythoncore
      DEPENDEES Build_w9xpopen
      )
      
    ExternalProject_Add_Step(${proj} Build__socket
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _socket
      DEPENDEES Build_pythoncore
      )
      
    ExternalProject_Add_Step(${proj} Build__tkinter
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _tkinter
      DEPENDEES Build__socket
      )
      
    ExternalProject_Add_Step(${proj} Build__testcapi
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _testcapi
      DEPENDEES Build__tkinter
      )
      
    ExternalProject_Add_Step(${proj} Build__msi
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _msi
      DEPENDEES Build__testcapi
      )
      
    ExternalProject_Add_Step(${proj} Build__elementtree
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _elementtree
      DEPENDEES Build__msi
      )
      
    ExternalProject_Add_Step(${proj} Build__ctypes_test
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _ctypes_test
      DEPENDEES Build__elementtree
      )
      
    ExternalProject_Add_Step(${proj} Build__ctypes
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _ctypes
      DEPENDEES python_sln
      )
      
    ExternalProject_Add_Step(${proj} Build_winsound
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project winsound
      DEPENDEES Build__ctypes
      )
      
    ExternalProject_Add_Step(${proj} Build_pyexpat
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project pyexpat
      DEPENDEES Build_winsound
      )
      
    ExternalProject_Add_Step(${proj} Build_pythonw
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project pythonw
      DEPENDEES Build_pyexpat
      )
      
    ExternalProject_Add_Step(${proj} Build__multiprocessing
      COMMAND ${CMAKE_BUILD_TOOL} ${python_sln} /build Release /project _multiprocessing
      DEPENDEES Build_pythonw
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
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python-build/PCbuild/python26.dll ${CMAKE_BINARY_DIR}/Slicer3-build/bin/${CMAKE_BUILD_TYPE}/python26.dll
      DEPENDEES install
      )
  elseif(APPLE)
    set(python_SVN "http://svn.python.org/projects/python/branches/release26-maint")
    set(python_BUILD_IN_SOURCE 1)
    set(python_CONFIGURE sh configure --prefix=${build_dir}/python-build --with-tcl=${tcl_build} --enable-shared)
    set(python_BUILD make)
    set(python_INSTALL make install)

    ExternalProject_Add(${proj}
      DEPENDS ${python_DEPENDS}
      SVN_REPOSITORY ${python_SVN}
      SOURCE_DIR python
      BUILD_IN_SOURCE ${python_BUILD_IN_SOURCE}
      CONFIGURE_COMMAND ${python_CONFIGURE}
      BUILD_COMMAND ${python_BUILD}
      INSTALL_COMMAND ${python_INSTALL}
      )
#  if { $isDarwin } {
#            # Special Slicer hack to build and install the .dylib
#            file mkdir $::Slicer3_LIB/python-build/lib/
#            file delete -force $::Slicer3_LIB/python-build/lib/libpython2.6.dylib
#            set fid [open environhack.c w]
#            puts $fid "char **environ=0;"
#            close $fid
#            runcmd gcc -c -o environhack.o environhack.c
#            runcmd libtool -o $::Slicer3_LIB/python-build/lib/libpython2.6.dylib -dynamic  \
#                -all_load libpython2.6.a environhack.o -single_module \
#                -install_name $::Slicer3_LIB/python-build/lib/libpython2.6.dylib \
#                -compatibility_version 2.6 \
#                -current_version 2.6 -lSystem -lSystemStubs
#
# 
  else()
    set(python_SVN "http://svn.python.org/projects/python/branches/release26-maint")
    set(python_BUILD_IN_SOURCE 1)
    set(python_CONFIGURE sh configure --prefix=${build_dir}/python-build --with-tcl=${tcl_build} --enable-shared)
    set(python_BUILD make)
    set(python_INSTALL make install)

    ExternalProject_Add(${proj}
      DEPENDS ${python_DEPENDS}
      SVN_REPOSITORY ${python_SVN}
      SOURCE_DIR python
      BUILD_IN_SOURCE ${python_BUILD_IN_SOURCE}
      CONFIGURE_COMMAND ${python_CONFIGURE}
      BUILD_COMMAND ${python_BUILD}
      INSTALL_COMMAND ${python_INSTALL}
      )
  endif()
  
endif(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)

#-----------------------------------------------------------------------------
# Set vtk_PYTHON_ARGS, slicer_PYTHON_INCLUDE and slicer_PYTHON_LIBRARY variables
#

set(vtk_PYTHON_ARGS)
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

if(Slicer3_USE_PYTHON)
  set(vtk_PYTHON_ARGS
    -DPYTHON_INCLUDE_PATH:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    )
endif(Slicer3_USE_PYTHON)

#-----------------------------------------------------------------------------
# PythonQT
#

if(Slicer3_USE_PYTHONQT)
  set(proj PythonQt)

  find_program(Slicer3_PATCH_EXECUTABLE patch
         "C:/Program Files/GnuWin32/bin"
         "C:/Program Files (x86)/GnuWin32/bin")
  mark_as_advanced(Slicer3_PATCH_EXECUTABLE)
  if (NOT Slicer3_PATCH_EXECUTABLE)
    message(FATAL_ERROR "Building PythonQt requires a patch program.  Please set Slicer3_PATCH_EXECUTABLE!")
  endif()

  set(pythonqt_src_dir "${CMAKE_CURRENT_BINARY_DIR}/${proj}")
  set(pythonqt_patches_dir "${CMAKE_CURRENT_SOURCE_DIR}/../Utilities/Patches/PythonQt")
  set(pythonqt_patch_script "${CMAKE_CURRENT_BINARY_DIR}/Slicer3-build/CMake/Slicer3PatchPythonQt.cmake")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../CMake/Slicer3PatchPythonQt.cmake.in ${pythonqt_patch_script} @ONLY)

  ExternalProject_Add(${proj}
    DEPENDS python
    SVN_REPOSITORY "https://pythonqt.svn.sourceforge.net/svnroot/pythonqt/trunk"
    SOURCE_DIR PythonQt
    PATCH_COMMAND ${CMAKE_COMMAND} -P ${pythonqt_patch_script}
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      #-DCMAKE_INSTALL_PREFIX:PATH=${prefix}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      ${vtk_PYTHON_ARGS}
    INSTALL_COMMAND "")
      
endif(Slicer3_USE_PYTHONQT)

#-----------------------------------------------------------------------------
# Get and build netlib (blas and lapack)

if(CMAKE_Fortran_COMPILER AND Slicer3_BUILD_NUMPY)

set(proj blas)

set(blas_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/netlib/BLAS")
set(blas_BUILD_IN_SOURCE 0)
set(blas_CONFIGURE "pwd")
set(blas_BUILD "")

if(WIN32)
else()
  set(blas_BUILD_IN_SOURCE 1)
#  set(blas_CONFIGURE sh configure)
#  set(blas_BUILD ${CMAKE_Fortran_COMPILER} -O3 -fno-second-underscore -fPIC -m64 -c *.f)
#  set(blas_INSTAL make install)
endif()

ExternalProject_Add(${proj}
  SVN_REPOSITORY ${blas_SVN}
  SOURCE_DIR netlib/BLAS
  CMAKE_GENERATOR ${gen}
  INSTALL_COMMAND ""
)

set(proj lapack)

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/netlib/lapack-3.1.1"
  SOURCE_DIR netlib/lapack
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# Get and build numpy and scipy

#set ::NUMPY_TAG "http://svn.scipy.org/svn/numpy/branches/1.2.x"
#set ::env(PYTHONHOME) $::Slicer3_LIB/python-build
#set ::env(LD_LIBRARY_PATH) $::Slicer3_LIB/python-build/lib:$::env(LD_LIBRARY_PATH)

set(proj numpy)

set(slicer_PYTHON_INTERPRETER)

if(WIN32)
  set(slicer_PYTHON_INTERPRETER ${python_base}/python.exe)
else()
  set(slicer_PYTHON_INTERPRETER ${CMAKE_BINARY_DIR}/python-build/bin/python)
  set(ENV{LD_LIBRARY_PATH} "${CMAKE_BINARY_DIR}/python-build/lib")
endif()

ExternalProject_Add(${proj}
  DEPENDS blas lapack python
  SVN_REPOSITORY "http://svn.scipy.org/svn/numpy/branches/1.3.x"
  SOURCE_DIR python/numpy
  CONFIGURE_COMMAND ""
  BINARY_DIR ${CMAKE_BINARY_DIR}/python/numpy 
  BUILD_COMMAND ${slicer_PYTHON_INTERPRETER} ./setup.py install
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
#set ::SCIPY_TAG "http://svn.scipy.org/svn/scipy/branches/0.7.x"

set(proj scipy)

ExternalProject_Add(${proj}
  DEPENDS numpy
  SVN_REPOSITORY "http://svn.scipy.org/svn/scipy/branches/0.7.x"
  SOURCE_DIR python/scipy
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

endif() # CMAKE_Fortran_COMPILER AND Slicer3_BUILD_NUMPY

#-----------------------------------------------------------------------------
# Get and build VTK
#

set(proj VTK)

if(Slicer3_USE_VTK_CVSHEAD)
  set(vtk_tag -dAP)
else()
  set(vtk_tag -r VTK-5-6)
endif()

set(vtk_module VTK)
set(vtk_source ${CMAKE_BINARY_DIR}/VTK)

set(vtk_DEPENDS)
set(vtk_WRAP_TCL OFF)
set(vtk_WRAP_PYTHON OFF)

if(Slicer3_USE_KWWIDGETS)
  list(APPEND vtk_DEPENDS iwidgets)
  set(vtk_WRAP_TCL ON)
endif(Slicer3_USE_KWWIDGETS)

if (Slicer3_USE_PYTHONQT)
  set(vtk_WRAP_PYTHON ON)
  list(APPEND vtk_DEPENDS python)
endif(Slicer3_USE_PYTHONQT)

set(vtk_QT_ARGS)
if(Slicer3_USE_QT)
  set(vtk_QT_ARGS
    -DDESIRED_QT_VERSION:STRING=4
    -DVTK_USE_GUISUPPORT:BOOL=ON
    -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
    -DVTK_USE_QT:BOOL=ON
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    )
endif(Slicer3_USE_QT)

set(vtk_MAC_ARGS)
if(APPLE)
  set(vtk_MAC_ARGS
    -DCMAKE_SHARED_LINKER_FLAGS:STRING="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
    -DCMAKE_EXE_LINKER_FLAGS:STRING="-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
    -DOPENGL_INCLUDE_DIR:PATH=/usr/X11R6/include
    -DVTK_USE_CARBON:BOOL=OFF
    -DVTK_USE_COCOA:BOOL=OFF
    -DVTK_USE_X:BOOL=ON
    -DVTK_USE_RPATH:BOOL=ON
  )
endif()

set(slicer_TCL_LIB)
set(slicer_TK_LIB)
set(slicer_TCLSH)
set(vtk_TCL_ARGS)
if(vtk_WRAP_TCL)
  if(WIN32)
    set(slicer_TCL_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/tcl84.lib)
    set(slicer_TK_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/tk84.lib)
    set(slicer_TCLSH ${CMAKE_BINARY_DIR}/tcl-build/bin/tclsh.exe)
  elseif(APPLE)
    set(slicer_TCL_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtcl8.4.dylib)
    set(slicer_TK_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtk8.4.dylib)
    set(slicer_TCLSH ${CMAKE_BINARY_DIR}/tcl-build/bin/tclsh84)
  else()
    set(slicer_TCL_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtcl8.4.so)
    set(slicer_TK_LIB ${CMAKE_BINARY_DIR}/tcl-build/lib/libtk8.4.so)
    set(slicer_TCLSH ${CMAKE_BINARY_DIR}/tcl-build/bin/tclsh84)
  endif()
  set(vtk_TCL_ARGS
    -DTCL_INCLUDE_PATH:PATH=${CMAKE_BINARY_DIR}/tcl-build/include
    -DTK_INCLUDE_PATH:PATH=${CMAKE_BINARY_DIR}/tcl-build/include
    -DTCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DTK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DTCL_TCLSH:FILEPATH=${slicer_TCLSH}
    )
endif(vtk_WRAP_TCL)

ExternalProject_Add(vtk
  DEPENDS ${vtk_DEPENDS}
  SOURCE_DIR ${vtk_source}
  BINARY_DIR VTK-build
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/VTK/"
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DVTK_USE_PARALLEL:BOOL=ON
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer3_USE_VTK_DEBUG_LEAKS}
    -DVTK_WRAP_TCL:BOOL=${vtk_WRAP_TCL}
    ${vtk_TCL_ARGS}
    -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
    ${vtk_PYTHON_ARGS}
    ${vtk_QT_ARGS}
    ${vtk_MAC_ARGS}
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# Get and build CTK
set(ctk_source ${CMAKE_BINARY_DIR}/CTK)
if (Slicer3_USE_QT)
  ExternalProject_Add(CTK
    GIT_REPOSITORY "git://github.com/pieper/CTK.git"
    SOURCE_DIR ${ctk_source}
    BINARY_DIR CTK-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
      -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_TESTING:BOOL=OFF
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      -DVTK_DIR:PATH=${CMAKE_BINARY_DIR}/VTK-build
      -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
      -DCTK_LIB_Widgets:BOOL=ON
      -DCTK_LIB_Visualization/VTK/Widgets:BOOL=ON
      -DCTK_LIB_Scripting/Python/Widgets:BOOL=${Slicer3_USE_PYTHONQT}
    INSTALL_COMMAND ""
    DEPENDS 
      vtk
    )
endif()

#-----------------------------------------------------------------------------
# Get and build kwwidgets

if(Slicer3_USE_KWWIDGETS)
  set(proj KWWidgets)
  set(kwwidgets_tag Slicer-3-6)

  ExternalProject_Add(${proj}
    DEPENDS vtk
    CVS_REPOSITORY ":pserver:anoncvs@www.kwwidgets.org:/cvsroot/KWWidgets"
    CVS_MODULE "KWWidgets"
    CVS_TAG -r ${kwwidgets_tag}
    SOURCE_DIR KWWidgets
    BINARY_DIR KWWidgets-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
      -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
      -DVTK_DIR:PATH=${CMAKE_BINARY_DIR}/VTK-build
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DCMAKE_SKIP_RPATH:BOOL=ON
      -DBUILD_EXAMPLES:BOOL=OFF
      -DKWWidgets_BUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DKWWidgets_BUILD_TESTING:BOOL=OFF
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND ""
  )
endif(Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# Get and build itk

set(proj Insight)

ExternalProject_Add(${proj}
  CVS_REPOSITORY ":pserver:anonymous:insight@public.kitware.com:/cvsroot/Insight"
  CVS_MODULE "Insight"
  CVS_TAG -r ITK-3-18
  SOURCE_DIR Insight
  BINARY_DIR Insight-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DBUILD_TESTING:BOOL=OFF
    -DITK_USE_REVIEW:BOOL=ON
    -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
    -DITK_USE_PORTABLE_ROUND:BOOL=ON
    -DITK_USE_CENTERED_PIXEL_COORDINATES_CONSISTENTLY:BOOL=ON
    -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_SKIP_RPATH:BOOL=ON
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DITK_LEGACY_REMOVE:BOOL=ON
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# Get and build teem

set(proj teem)

set(zlib "vtkzlib.lib")
set(png "vtkpng.lib")

if(WIN32)
  set(teem_ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/${CMAKE_BUILD_TYPE}/vtkzlib.lib)
  set(teem_PNG_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/${CMAKE_BUILD_TYPE}/vtkpng.lib)
elseif(APPLE)
  set(teem_ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkzlib.dylib)
  set(teem_PNG_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkpng.dylib)
else()
  set(teem_ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkzlib.so)
  set(teem_PNG_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkpng.so)
endif()

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://teem.svn.sourceforge.net/svnroot/teem/teem/tags/1.10.0"
  DEPENDS vtk
  SOURCE_DIR teem
  BINARY_DIR teem-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DBUILD_TESTING:BOOL=ON
    -DTeem_PTHREAD:BOOL=OFF
    -DTeem_BZIP2:BOOL=OFF
    -DTeem_ZLIB:BOOL=ON
    -DTeem_PNG:BOOL=ON
    -DTeem_VTK_MANGLE:BOOL=ON
    -DTeem_VTK_TOOLKITS_IPATH:FILEPATH=${CMAKE_BINARY_DIR}/VTK-build
    -DZLIB_INCLUDE_DIR:PATH=${CMAKE_BINARY_DIR}/VTK/Utilities
    -DTeem_VTK_ZLIB_MANGLE_IPATH:PATH=${CMAKE_BINARY_DIR}/VTK/Utilities/vtkzlib
    -DTeem_ZLIB_DLLCONF_IPATH:PATH=${CMAKE_BINARY_DIR}/VTK-build/Utilities
    -DZLIB_LIBRARY:FILEPATH=${teem_ZLIB_LIBRARY}
    -DPNG_PNG_INCLUDE_DIR:PATH=${CMAKE_BINARY_DIR}/VTK/Utilities/vtkpng
    -DTeem_PNG_DLLCONF_IPATH:PATH=${CMAKE_BINARY_DIR}/VTK-build/Utilities
    -DPNG_LIBRARY:FILEPATH=${teem_PNG_LIBRARY}
    -DTeem_USE_LIB_INSTALL_SUBDIR:BOOL=ON
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# Get and build OpenIGTLink 

set(proj OpenIGTLink)

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.na-mic.org/NAMICSandBox/branches/OpenIGTLink-1-0"
  SOURCE_DIR OpenIGTLink
  BINARY_DIR OpenIGTLink-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_SKIP_RPATH:BOOL=ON
    -DOpenIGTLink_DIR:FILEPATH=${CMAKE_BINARY_DIR}/OpenIGTLink-build
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# Get and build BatchMake

set(proj BatchMake)

ExternalProject_Add(${proj}
  DEPENDS Insight
  CVS_REPOSITORY ":pserver:anoncvs:@batchmake.org:/cvsroot/BatchMake"
  CVS_MODULE "BatchMake"
  SOURCE_DIR BatchMake
  BINARY_DIR BatchMake-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_TESTING:BOOL=OFF
    -DUSE_FLTK:BOOL=OFF
    -DDASHBOARD_SUPPORT:BOOL=OFF
    -DGRID_SUPPORT:BOOL=ON
    -DUSE_SPLASHSCREEN:BOOL=OFF
    -DITK_DIR:FILEPATH=${CMAKE_BINARY_DIR}/Insight-build
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# Get and build SLICERLIBCURL (slicerlibcurl)

set(proj cmcurl)

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/cmcurl"
  SOURCE_DIR cmcurl
  BINARY_DIR cmcurl-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_TESTING:BOOL=OFF
  INSTALL_COMMAND ""
)

#-----------------------------------------------------------------------------
# build the slicer
# - first run cmake
# - create the Slicer3Version.txt file
# - then run plaftorm specific build command

set(proj Slicer3)

set(slicer_DEPENDS teem OpenIGTLink BatchMake vtk Insight)

# For now, tk and itcl are used only when Slicer3_USE_KWWIDGETS is ON
if(Slicer3_USE_KWWIDGETS)
  list(APPEND slicer_DEPENDS tcl tk itcl KWWidgets)
endif(Slicer3_USE_KWWIDGETS)

if(CMAKE_Fortran_COMPILER)
  list(APPEND slicer_DEPENDS numpy blas lapack)
endif(CMAKE_Fortran_COMPILER)

set(slicer_QT_ARGS)
if(Slicer3_USE_QT)
  set(slicer_QT_ARGS
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    )
endif(Slicer3_USE_QT)

if(Slicer3_USE_PYTHON)
  list(APPEND slicer_DEPENDS python)
endif(Slicer3_USE_PYTHON)

if (Slicer3_USE_PYTHONQT)
  list(APPEND slicer_DEPENDS PythonQt)
endif(Slicer3_USE_PYTHONQT)

set(slicer_KWWIDGETS_ARGS)
if(Slicer3_USE_KWWIDGETS)
  set(slicer_KWWIDGETS_ARGS
    -DKWWidgets_DIR:FILEPATH=${CMAKE_BINARY_DIR}/KWWidgets-build
    )
endif(Slicer3_USE_KWWIDGETS)

ExternalProject_Add(${proj}
  DEPENDS ${slicer_DEPENDS}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
  BINARY_DIR Slicer3-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
#    -DCMAKE_INSTALL_PREFIX:PATH=${prefix
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer3_USE_VTK_DEBUG_LEAKS}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=ON
    -DITK_DIR:PATH=${CMAKE_BINARY_DIR}/Insight-build
    -DTeem_DIR:PATH=${CMAKE_BINARY_DIR}/teem-build
    -DOpenIGTLink_DIR:PATH=${CMAKE_BINARY_DIR}/OpenIGTLink-build
    -DBatchMake_DIR:PATH=${CMAKE_BINARY_DIR}/BatchMake-build
    -DVTK_DIR:PATH=${CMAKE_BINARY_DIR}/VTK-build
    -DCTK_DIR:PATH=${CMAKE_BINARY_DIR}/CTK-build
    -DSlicer3_USE_BatchMake=ON
    -DINCR_TCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DINCR_TK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DSlicer3_TCL_DIR:PATH=${tcl_build}
    -DSlicer3_USE_OPENIGTLINK:BOOL=ON
    -DSLICERLIBCURL_DIR:PATH=${CMAKE_BINARY_DIR}/cmcurl-build
    -DSlicer3_USE_PYTHON:BOOL=${Slicer3_USE_PYTHON}
    -DSlicer3_USE_SYSTEM_PYTHON:BOOL=OFF
    -DSlicer3_USE_NUMPY:BOOL=${Slicer3_BUILD_NUMPY}
    ${vtk_PYTHON_ARGS}
    -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
    -DSlicer3_USE_QT:BOOL=${Slicer3_USE_QT}
    ${slicer_QT_ARGS}
    -DSlicer3_USE_KWWIDGETS:BOOL=${Slicer3_USE_KWWIDGETS}
    ${slicer_KWWIDGETS_ARGS}
    -DSlicer3_USE_VTK_CVSHEAD:BOOL=${Slicer3_USE_VTK_CVSHEAD}
  INSTALL_COMMAND ""
)
