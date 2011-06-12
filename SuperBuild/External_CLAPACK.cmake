
set(proj CLAPACK)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)
set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)

set(CLAPACK_version 3.2.1)
#set(CLAPACK_file "http://www.netlib.org/clapack/clapack-${clapack_version}-CMAKE.tgz")
# Since the netlib.org server has been down several time, especially when the nightly dashboard 
# started, we added a copy of the archive to slicer3 lib mirrors.
set(CLAPACK_file http://svn.slicer.org/Slicer3-lib-mirrors/trunk/clapack-${CLAPACK_version}-CMAKE.tgz)
set(CLAPACK_MD5 4fd18eb33f3ff8c5d65a7d43913d661b)

# The CLAPACK external project for Titan

set(CLAPACK_source "${CMAKE_CURRENT_BINARY_DIR}/CLAPACK")
set(CLAPACK_binary "${CMAKE_CURRENT_BINARY_DIR}/CLAPACK-build")

# Turn off the warnings for CLAPACK on windows
string(REPLACE "/W3" "/W0" CMAKE_CXX_FLAGS_CLAPACK "${ep_common_cxx_flags}")
string(REPLACE "/W4" "/W0" CMAKE_CXX_FLAGS_CLAPACK "${CMAKE_CXX_FLAGS_CLAPACK}")
string(REPLACE "/W3" "/W0" CMAKE_C_FLAGS_CLAPACK "${ep_common_c_flags}")
string(REPLACE "/W4" "/W0" CMAKE_C_FLAGS_CLAPACK "${CMAKE_C_FLAGS_CLAPACK}")

#
# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
# See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
#
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set(CMAKE_C_FLAGS_CLAPACK "-fPIC ${CMAKE_C_FLAGS_CLAPACK}")
endif()
  
ExternalProject_Add(${proj}
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCE_DIR ${CLAPACK_source}
  BINARY_DIR ${CLAPACK_binary}
  URL ${CLAPACK_file}
  URL_MD5 ${CLAPACK_MD5}
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS_CLAPACK}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS_CLAPACK}
    -DBUILD_SHARED_LIBS:BOOL=OFF
  INSTALL_COMMAND ""
  DEPENDS 
    ${CLAPACK_DEPENDENCIES}
  )
set(CLAPACK_DIR "${CLAPACK_binary}" CACHE PATH 
  "CLAPACK binary directory" FORCE)
mark_as_advanced(CLAPACK_DIR)
