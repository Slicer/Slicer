#-----------------------------------------------------------------------------
# Get and build VTK
#

set(proj VTK)

set(vtk_source ${CMAKE_BINARY_DIR}/VTK)
set(vtk_WRAP_TCL OFF)
set(vtk_WRAP_PYTHON OFF)

if(Slicer3_USE_KWWIDGETS)
  list(APPEND vtk_DEPENDENCIES tcl)
  set(vtk_WRAP_TCL ON)
endif(Slicer3_USE_KWWIDGETS)

if (Slicer3_USE_PYTHONQT)
  set(vtk_WRAP_PYTHON ON)
  list(APPEND vtk_DEPENDENCIES python)
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
  DEPENDS ${vtk_DEPENDENCIES}
  SOURCE_DIR ${vtk_source}
  BINARY_DIR VTK-build
  GIT_REPOSITORY "git://github.com/pieper/SlicerVTK.git"
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
    -DVTK_USE_RPATH:BOOL=ON
    ${vtk_TCL_ARGS}
    -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
    ${vtk_PYTHON_ARGS}
    ${vtk_QT_ARGS}
    ${vtk_MAC_ARGS}
  INSTALL_COMMAND ""
)
