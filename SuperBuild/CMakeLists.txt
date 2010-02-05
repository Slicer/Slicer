

project(Slicer3)

enable_language(C)
enable_language(CXX)

cmake_minimum_required(VERSION 2.8)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

#-----------------------------------------------------------------------------
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#
if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../CMake)

# Attempt to detect if we have a Fortran Compiler available.
include(CheckFortran)

if(CMAKE_Fortran_COMPILER)
  enable_language(Fortran)
endif(CMAKE_Fortran_COMPILER)

include(ExternalProject)

set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
set_property(DIRECTORY PROPERTY EP_BASE ${base})

set(prefix "${base}/Install")

OPTION(BUILD_SHARED_LIBS "Build with shared libraries.")
SET(CMAKE_BUILD_TYPE "Release")

# Compute -G arg for configuring external projects with the same CMake generator:
#
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()


# Set the default build type---this will affect all libraries and
# applications
#
set(build_type "")
if(CMAKE_BUILD_TYPE)
  set(build_type "${CMAKE_BUILD_TYPE}")
endif()

set(mac_args)
if(APPLE)
  # With Qt 4.4, VTK_USE_QVTK means we should use Carbon, not Cocoa:
  set(mac_args
    -DVTK_USE_CARBON:BOOL=ON
    -DVTK_USE_COCOA:BOOL=OFF
    )
endif()

set(parallelism_level)
set(build_dir ${CMAKE_CURRENT_BINARY_DIR})

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
  set(tcl_CONFIGURE_COMMAND ./unix/configure --prefix=${tcl_build})
  set(tcl_BUILD_COMMAND make ${parallelism_level})
  set(tcl_INSTALL_COMMAND make install)
endif()

ExternalProject_Add(${proj}
  SVN_REPOSITORY ${tcl_SVN_REPOSITORY}
  SOURCE_DIR ${tcl_SOURCE_DIR}
  BUILD_IN_SOURCE ${tcl_BUILD_IN_SOURCE}
  CONFIGURE_COMMAND ${tcl_CONFIGURE_COMMAND}
  BUILD_COMMAND ${tcl_BUILD_COMMAND}
  INSTALL_COMMAND ${tcl_INSTALL_COMMAND}
)

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
elseif(APPLE)
# uncmd ./configure --with-tcl=./tcl-build/lib --prefix=./tcl-build --disable-corefoundation --x-libraries=/usr/X11R6/lib --x-includes=/usr/X11R6/include --with-x
  set(tk_DEPENDS tcl)
  set(tk_SOURCE_DIR tcl/tk)
  set(tk_BUILD_IN_SOURCE 1)
  set(tk_CONFIGURE_COMMAND ./unix/configure --with-tcl=${tcl_build}/lib --prefix=${tcl_build})
  set(tk_BUILD_COMMAND make ${parallelism_level})
  set(tk_INSTALL_COMMAND make install)
else()
  set(tk_DEPENDS tcl)
  set(tk_SOURCE_DIR tcl/tk)
  set(tk_BUILD_IN_SOURCE 1)
  set(tk_CONFIGURE_COMMAND ./unix/configure --with-tcl=${tcl_build}/lib --prefix=${tcl_build})
  set(tk_BUILD_COMMAND make ${parallelism_level})
  set(tk_INSTALL_COMMAND make install)
endif()

ExternalProject_Add(${proj}
  DEPENDS tcl
  SVN_REPOSITORY ${tk_SVN_REPOSITORY}
  SOURCE_DIR ${tk_SOURCE_DIR}
  BUILD_IN_SOURCE ${tk_BUILD_IN_SOURCE}
  CONFIGURE_COMMAND ${tk_CONFIGURE_COMMAND}
  BUILD_COMMAND ${tk_BUILD_COMMAND}
  INSTALL_COMMAND ${tk_INSTALL_COMMAND}
)

set(proj itcl)

set(itcl_SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/incrTcl")
set(itcl_BUILD_IN_SOURCE 0)
set(itcl_CONFIGURE_COMMAND "")
set(itcl_BUILD_COMMAND "")
set(itcl_INSTALL_COMMAND "")

if(WIN32)
elseif(APPLE)
  set(itcl_BUILD_IN_SOURCE 1)
  set(itcl_CONFIGURE_COMMAND sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(itcl_BUILD_COMMAND make ${parallelism_level} all)
  set(itcl_INSTALL_COMMAND make install)
else()
  set(itcl_BUILD_IN_SOURCE 1)
  set(itcl_CONFIGURE_COMMAND sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(itcl_BUILD_COMMAND make ${parallelism_level} all)
  set(itcl_INSTALL_COMMAND make install)
endif()

#exec chmod +x ../incrTcl/configure

ExternalProject_Add(${proj}
  DEPENDS tk
  SVN_REPOSITORY ${itcl_SVN_REPOSITORY}
  SOURCE_DIR tcl/incrTcl
  BUILD_IN_SOURCE ${itcl_BUILD_IN_SOURCE}
  CONFIGURE_COMMAND ${itcl_CONFIGURE_COMMAND}
  BUILD_COMMAND ${itcl_BUILD_COMMAND}
  INSTALL_COMMAND ${itcl_INSTALL_COMMAND}
)

ExternalProject_Add_Step(${proj} CHMOD_incrTcl_configure
  COMMAND chmod +x ${tcl_base}/incrTcl/configure
  DEPENDEES update
  DEPENDERS configure
  )

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

set(proj blt)

set(blt_SVN "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/tcl/blt")
set(blt_BUILD_IN_SOURCE 0)
set(blt_CONFIGURE "")
set(blt_BUILD "")
set(blt_INSTALL "")

if(WIN32)
else()
  set(blt_BUILD_IN_SOURCE 1)
  set(blt_CONFIGURE sh configure --with-tcl=${tcl_build}/lib --with-tk=${tcl_build}/lib --prefix=${tcl_build})
  set(blt_BUILD make ${parallelism_level})
  set(blt_INSTAL make install)
endif()

ExternalProject_Add(${proj}
  DEPENDS tcl tk
  SVN_REPOSITORY ${blt_SVN}
  SOURCE_DIR tcl/blt
  BUILD_IN_SOURCE ${blt_BUILD_IN_SOURCE}
  CONFIGURE_COMMAND ${blt_CONFIGURE}
  BUILD_COMMAND ${blt_BUILD}
  INSTALL_COMMAND ${btl_INSTALL}
)

set(proj python)

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
  DEPENDS tcl
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
else(WIN32)

  set(python_SVN "http://svn.python.org/projects/python/branches/release26-maint")
  set(python_BUILD_IN_SOURCE 1)
  set(python_CONFIGURE sh configure --prefix=${build_dir}/python-build --with-tcl=${tcl_build} --enable-shared)
  set(python_BUILD make)
  set(python_INSTALL make install)

  ExternalProject_Add(${proj}
    DEPENDS tcl tk
    SVN_REPOSITORY ${python_SVN}
    SOURCE_DIR python
    BUILD_IN_SOURCE ${python_BUILD_IN_SOURCE}
    CONFIGURE_COMMAND ${python_CONFIGURE}
    BUILD_COMMAND ${python_BUILD}
    INSTALL_COMMAND ${python_INSTALL}
  )
endif(WIN32)

# Get and build netlib (blas and lapack)

if(CMAKE_Fortran_COMPILER)

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
  DEPENDS python
  SVN_REPOSITORY ${blas_SVN}
  SOURCE_DIR netlib/BLAS
#  BUILD_IN_SOURCE ${blas_BUILD_IN_SOURCE}
  CMAKE_GENERATOR ${gen}
#  CONFIGURE_COMMAND ${blas_CONFIGURE}
#  BUILD_COMMAND ${blas_BUILD}
  INSTALL_COMMAND ""
)

if(NOT WIN32)
  
endif(NOT WIN32)

#return()

set(proj lapack)

ExternalProject_Add(${proj}
  DEPENDS python
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/netlib/lapack-3.1.1"
  SOURCE_DIR netlib/lapack
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

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
  #message("LD_LIBRARY_PATH:=ENV{LD_LIBRARY_PATH}")
  #set(ENV{PYTHONHOME} ${python_home})
endif()

#} else {
#          # Jim's way - cygwin does mount c:/ as /c and doesn't use cygdrive
#          set devenvdir [file dirname $::MAKE]
#          set vcbindir $::COMPILER_PATH
#          set ::env(PATH) $devenvdir\;$vcbindir\;$::env(PATH)
#          set ::env(PATH) $::env(PATH)\;$::Slicer3_LIB/python-build/PCbuild
#        }
#        set ::env(INCLUDE) [file dirname $::COMPILER_PATH]/include
#        set ::env(INCLUDE) $::MSSDK_PATH/Include\;$::env(INCLUDE)
#        set ::env(INCLUDE) [file normalize $::Slicer3_LIB/python-build/Include]\;$::env(INCLUDE)
#        set ::env(LIB) $::MSSDK_PATH/Lib\;[file dirname $::COMPILER_PATH]/lib
#        set ::env(LIBPATH) $devenvdir

#make build command call cmake -P on a cmake script that sets LD_LIBRARY_PATH
ExternalProject_Add(${proj}
  DEPENDS blas lapack python
  SVN_REPOSITORY "http://svn.scipy.org/svn/numpy/branches/1.3.x"
  SOURCE_DIR python/numpy
  CONFIGURE_COMMAND ""
  BINARY_DIR ${CMAKE_BINARY_DIR}/python/numpy 
  BUILD_COMMAND ${slicer_PYTHON_INTERPRETER} ./setup.py install
  INSTALL_COMMAND ""
)

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

endif() # CMAKE_Fortran_COMPILER

# Get and build vtk

set(proj vtk)
set(vtk_tag VTK-5-4)
set(vtk_module VTK)
set(vtk_source ${CMAKE_BINARY_DIR}/VTK)

set(slicer_TCL_LIB)
set(slicer_TK_LIB)
set(slicer_TCLSH)

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

#set ::TCL_BIN_DIR $::Slicer3_LIB/tcl-build/bin
#set ::TCL_LIB_DIR $::Slicer3_LIB/tcl-build/lib
#set ::TCL_INCLUDE_DIR $::Slicer3_LIB/tcl-build/include
#set ::VTK_TCL_LIB $::TCL_LIB_DIR/tcl84.lib
#set ::VTK_TK_LIB $::TCL_LIB_DIR/tk84.lib
#set ::VTK_TCLSH $::TCL_BIN_DIR/tclsh84.exe

ExternalProject_Add(vtk
  DEPENDS python iwidgets
  CVS_REPOSITORY ":pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK"
  CVS_MODULE ${vtk_module}
  CVS_TAG -r ${vtk_tag}
  SOURCE_DIR ${vtk_source}
  BINARY_DIR VTK-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DBUILD_TESTING:BOOL=OFF
    -DVTK_WRAP_TCL:BOOL=ON
    -DVTK_USE_PARALLEL:BOOL=ON
    -DVTK_DEBUG_LEAKS:BOOL=ON
    -DTCL_INCLUDE_PATH:PATH=${CMAKE_BINARY_DIR}/tcl-build/include
    -DTK_INCLUDE_PATH:PATH=${CMAKE_BINARY_DIR}/tcl-build/include
    -DTCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DTK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DTCL_TCLSH:FILEPATH=${slicer_TCLSH}
    ${mac_args}
  INSTALL_COMMAND ""
)

# Get and build kwwidgets

set(proj KWWidgets)
set(kwwidgets_tag Slicer-3-4)

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

# Get and build itk

set(proj Insight)

ExternalProject_Add(${proj}
  CVS_REPOSITORY ":pserver:anonymous:insight@public.kitware.com:/cvsroot/Insight"
  CVS_MODULE "Insight"
  CVS_TAG -r ITK-3-16
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

# Get and build teem

set(proj teem)

#http://teem.svn.sourceforge.net/svnroot/teem/teem/tags/1.10.0
#set zlib "vtkzlib.lib"
#set png "vtkpng.lib"
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

# Get and build OpenIGTLink 

#set ::OpenIGTLink_TAG "http://svn.na-mic.org/NAMICSandBox/branches/OpenIGTLink-1-0"
#runcmd $::SVN co $::OpenIGTLink_TAG OpenIGTLink
#
#      file mkdir $Slicer3_LIB/OpenIGTLink-build
#      cd $Slicer3_LIB/OpenIGTLink-build

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
    -DCMAKE_BUILD_TYPE:STRING=Release
  INSTALL_COMMAND ""
)

# Get and build BatchMake

#set ::BatchMake_TAG "HEAD"
#runcmd $::CVS -d :pserver:anoncvs:@batchmake.org:/cvsroot/BatchMake co -r $::BatchMake_TAG BatchMake

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

# Get and build SLICERLIBCURL (slicerlibcurl)

#set ::SLICERLIBCURL_TAG "HEAD"

set(proj cmcurl)

#runcmd $::SVN co http://svn.slicer.org/Slicer3-lib-mirrors/trunk/cmcurl cmcurl

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

# build the slicer
# - first run cmake
# - create the Slicer3Version.txt file
# - then run plaftorm specific build command

set(proj Slicer3)

set(slicer_PYTHON_INCLUDE)
set(slicer_PYTHON_LIBRARY)
set(slicer_DEPENDENCIES tcl KWWidgets teem OpenIGTLink BatchMake python vtk)

if(CMAKE_Fortran_COMPILER)
  set(slicer_DEPENDENCIES tcl KWWidgets teem OpenIGTLink BatchMake python numpy blas lapack vtk)
endif(CMAKE_Fortran_COMPILER)

if(WIN32)
  set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/Python-build/Include)
  set(slicer_PYTHON_LIBRARY ${CMAKE_BINARY_DIR}/Python-build/PCbuild/python26.lib)
elseif(APPLE)
  set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/python-build/include/python2.6)
  set(slicer_PYTHON_LIBRARY ${CMAKE_BINARY_DIR}/python-build/lib/libpython2.6.dylib)
else()
  set(slicer_PYTHON_INCLUDE ${CMAKE_BINARY_DIR}/python-build/include/python2.6)
  set(slicer_PYTHON_LIBRARY ${CMAKE_BINARY_DIR}/python-build/lib/libpython2.6.so)
endif()

ExternalProject_Add(${proj}
  DEPENDS ${slicer_DEPENDENCIES}
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk"
  SOURCE_DIR Slicer3
  BINARY_DIR Slicer3-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
#    -DCMAKE_INSTALL_PREFIX:PATH=${prefix
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DVTK_DEBUG_LEAKS:BOOL=ON
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_TESTING:BOOL=ON
    -DITK_DIR:FILEPATH=${CMAKE_BINARY_DIR}/Insight-build
    -DKWWidgets_DIR:FILEPATH=${CMAKE_BINARY_DIR}/KWWidgets-build
    -DTeem_DIR:FILEPATH=${CMAKE_BINARY_DIR}/teem-build
    -DOpenIGTLink_DIR:FILEPATH=${CMAKE_BINARY_DIR}/OpenIGTLink-build
    -DBatchMake_DIR:FILEPATH=${CMAKE_BINARY_DIR}/BatchMake-build
    -DSlicer3_USE_BatchMake=ON
    -DINCR_TCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DINCR_TK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DSlicer3_USE_PYTHON:BOOL=ON
    -DSlicer3_USE_SYSTEM_PYTHON:BOOL=OFF
    -DSlicer3_USE_NUMPY:BOOL=OFF
    -DSlicer3_USE_OPENIGTLINK:BOOL=ON
    -DPYTHON_INCLUDE_PATH:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    -DSLICERLIBCURL_DIR:FILEPATH=${CMAKE_BINARY_DIR}/cmcurl-build 
     ${mac_args}
  INSTALL_COMMAND ""
)
