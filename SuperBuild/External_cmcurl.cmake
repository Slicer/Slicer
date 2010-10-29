#-----------------------------------------------------------------------------
# Get and build SLICERLIBCURL (slicerlibcurl)

set(proj cmcurl)

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/cmcurl"
  SOURCE_DIR cmcurl
  BINARY_DIR cmcurl-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_args}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DBUILD_SHARED_LIBS:BOOL=ON
  INSTALL_COMMAND ""
  DEPENDS 
    ${cmcurl_DEPENDENCIES}
)

set(SLICERLIBCURL_DIR ${CMAKE_BINARY_DIR}/cmcurl-build)
