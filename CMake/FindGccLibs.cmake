# FindGccLibs.cmake

# Get the GCC version
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Build the path to libgcc.a based on the GCC version
set(GCC_LIB_PATH "/usr/lib/gcc/x86_64-pc-linux-gnu/${GCC_VERSION}")

# Check if libgcc.a exists at the expected path
find_library(
    LIBGCC_STATIC
    NAMES gcc
    PATHS ${GCC_LIB_PATH}
    NO_DEFAULT_PATH
)

if (NOT LIBGCC_STATIC)
    message(FATAL_ERROR "libgcc.a not found in ${GCC_LIB_PATH}")
endif()

# Set the GccLibs_LIBRARIES variable
set(GccLibs_LIBRARIES ${LIBGCC_STATIC} CACHE INTERNAL "GCC Libraries")
