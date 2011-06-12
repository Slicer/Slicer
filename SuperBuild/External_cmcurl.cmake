#-----------------------------------------------------------------------------
# Get and build SLICERLIBCURL (slicerlibcurl)

set(proj cmcurl)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/cmcurl"
  SOURCE_DIR cmcurl
  BINARY_DIR cmcurl-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    #-DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags} # Unused
    -DSLICERLIBCURL_TESTING:BOOL=OFF
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DBUILD_SHARED_LIBS:BOOL=ON
  INSTALL_COMMAND ""
  DEPENDS 
    ${cmcurl_DEPENDENCIES}
)

set(SLICERLIBCURL_DIR ${CMAKE_BINARY_DIR}/cmcurl-build)
