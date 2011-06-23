
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(cmcurl_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(cmcurl)
set(proj cmcurl)

#message(STATUS "${__indent}Adding project ${proj}")
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

