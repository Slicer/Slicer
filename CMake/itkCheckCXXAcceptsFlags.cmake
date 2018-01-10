#
# - Test CXX compiler for a flag
# Check if the CXX compiler accepts a flag
#
# itkCHECK_CXX_ACCEPTS_FLAGS(FLAGS VAR)
# - macro which checks if the code compiles with the given flags
#  FLAGS - cxx flags to try
#  VAR   - variable to store whether compiler accepts the FLAGS (TRUE or FALSE)
#
macro(itkCHECK_CXX_ACCEPTS_FLAGS FLAGS VAR)
    if(NOT DEFINED ${VAR})
    set(_SOURCE "int main() { return 0;}\n")
    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx"
      "${_SOURCE}")

    message(STATUS "Checking to see if CXX compiler accepts flag ${FLAGS}")
    try_compile(${VAR}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${FLAGS}
      OUTPUT_VARIABLE OUTPUT)
    if(${VAR})
      set(${VAR} TRUE CACHE INTERNAL "CXX compiler accepts flag ${FLAGS}")
    else()
      set(${VAR} FALSE CACHE INTERNAL "CXX compiler accepts flag ${FLAGS}")
    endif()

   set(_UNKNOWN_FLAG_MSGS
     "ignoring unknown option"
     "unrecognized option"
     "Incorrect command line option"
     "warning D9002" # MSVC, language independent
   )
   foreach(MSG ${_UNKNOWN_FLAG_MSGS})
     string(REGEX MATCH "${MSG}" _FOUNDIT "${OUTPUT}")
     if("${_FOUNDIT}" MATCHES "${MSG}")
       set(${VAR} FALSE CACHE INTERNAL "CXX compiler accepts flag ${FLAGS}")
     endif()
   endforeach()

   if(${VAR})
     message(STATUS "Checking to see if CXX compiler accepts flag ${FLAGS} - Yes")
     file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
       "Determining if the CXX compiler accepts the flag ${FLAGS} passed with "
       "the following output:\n${OUTPUT}\n"
       "Source file was:\n${_SOURCE}\n")
   else()
     message(STATUS "Checking to see if CXX compiler accepts flag ${FLAGS} - No")
     file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
       "Determining if the CXX compiler accepts the flag ${FLAGS} passed with "
       "the following output:\n${OUTPUT}\n"
       "Source file was:\n${_SOURCE}\n")
   endif()

   endif()
endmacro()
