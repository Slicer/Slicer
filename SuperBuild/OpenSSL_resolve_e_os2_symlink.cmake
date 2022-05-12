
cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)

foreach(varname IN ITEMS
  OPENSSL_SOURCE_DIR
  )
  if("${${varname}}" STREQUAL "")
    message(FATAL_ERROR "${varname} is empty")
  endif()
endforeach()

set(include_dir "${OPENSSL_SOURCE_DIR}/include/openssl")

if(NOT IS_SYMLINK "${include_dir}/e_os2.h")
  return()
endif()

# Remove symlink
file(REMOVE ${include_dir}/e_os2.h)

# Copy real file into include directory
file(COPY ${OPENSSL_SOURCE_DIR}/e_os2.h DESTINATION ${include_dir})

# Remove original file
file(REMOVE ${OPENSSL_SOURCE_DIR}/e_os2.h)
