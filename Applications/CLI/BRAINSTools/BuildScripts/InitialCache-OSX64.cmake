## These setting are for building Macintosh versions of the software
## The compiler flags are set to allow 64 bit building against the
## Qt libraries, and have been confirmed to work on 10.6.3 version of
## MacOSX

## -arch arch
## Compile for the specified target architecture arch. The allowable values are i386, x86_64, ppc
## and ppc64. Multiple options work, and direct the compiler to produce "universal" binaries
## including object code for each architecture specified with -arch. This option only works if
## assembler and libraries are available for each architecture specified. (APPLE ONLY)



set (CMAKE_CXX_COMPILER "/usr/bin/g++-4.2" CACHE STRING "The g++ compiler")
set (CMAKE_C_COMPILER "/usr/bin/gcc-4.2" CACHE STRING "The gcc compiler")
set (ARCH "Mac64" CACHE STRING "")
set (ABI "Release" CACHE STRING "")
set (MAKECOMMAND "make -j8" CACHE STRING "" )
set (CMAKE_MAKE_PROGRAM "/usr/bin/make" CACHE FILEPATH "" )
set (CVSCOMMAND "/usr/bin/cvs" CACHE FILEPATH "" )
set (CVS_UPDATE_OPTIONS "-d -A -P -C" CACHE STRING "" )
set (BUILDNAME "${ARCH}-${ABI}" CACHE STRING "" )
set (SITE "uiowa" CACHE STRING "" )
set (CMAKE_SKIP_RPATH "ON" CACHE BOOL "" )
set (CMAKE_BUILD_TYPE "${ABI}" CACHE STRING "" )
set (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2 -msse -mmmx -msse2 -m64 -arch x86_64 -Wall -W -Wshadow -Wcast-qual -Wwrite-strings" CACHE STRING "" )
set (CMAKE_CXX_FLAGS_DEBUG "-UNDEBUG -g -m64 -arch x86_64 -Wall -W -Wshadow -Wcast-qual -Wwrite-strings" CACHE STRING "" )
set (CMAKE_CXX_FLAGS "-UNDEBUG -g -m64 -arch x86_64 -Wall -W -Wshadow -Wcast-qual -Wwrite-strings" CACHE STRING "" )
set (CMAKE_C_FLAGS_RELEASE "-pipe -DNDEBUG -O2 -msse -mmmx -msse2 -m64 -arch x86_64 -Wall -W -Wshadow -Wcast-qual -Wwrite-strings" CACHE STRING "" )
set (CMAKE_C_FLAGS_DEBUG "-UNDEBUG -g -m64 -arch x86_64 -Wall -W -Wshadow -Wcast-qual -Wwrite-strings" CACHE STRING "" )
set (CMAKE_C_FLAGS "-UNDEBUG -g -m64 -arch x86_64 -Wall -W -Wshadow -Wcast-qual -Wwrite-strings" CACHE STRING "" )
set (COVERAGE_COMMAND "/usr/bin/gcov" CACHE FILEPATH "" )
set (DART_TESTING_TIMEOUT "600" CACHE STRING "" )
set (MEMORYCHECK_COMMAND "/usr/bin/valgrind" CACHE PATH "" )
set (MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --quiet --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=100 --verbose --demangle=yes --suppressions=${CTEST_SOURCE_DIRECTORY}/CMake/InsightValgrind.supp" CACHE STRING "" )

