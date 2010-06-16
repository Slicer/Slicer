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
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DBUILD_TESTING:BOOL=OFF
  INSTALL_COMMAND ""
  DEPENDS 
    ${cmcurl_DEPENDENCIES}
)
