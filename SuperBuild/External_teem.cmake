
#-----------------------------------------------------------------------------
# Get and build teem

set(proj teem)

set(zlib "vtkzlib.lib")
set(png "vtkpng.lib")

if(WIN32)
  set(teem_ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/${CMAKE_CFG_INTDIR}/vtkzlib.lib)
  set(teem_PNG_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/${CMAKE_CFG_INTDIR}/vtkpng.lib)
elseif(APPLE)
  set(teem_ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkzlib.dylib)
  set(teem_PNG_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkpng.dylib)
else()
  set(teem_ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkzlib.so)
  set(teem_PNG_LIBRARY ${CMAKE_BINARY_DIR}/VTK-build/bin/libvtkpng.so)
endif()

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://teem.svn.sourceforge.net/svnroot/teem/teem/tags/1.10.0"
  UPDATE_COMMAND ""
  DEPENDS ${teem_DEPENDENCIES}
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
