PROJECT( Slicer3 )

#
# Top level slicer3 configurations
#

SET (LIBRARY_OUTPUT_PATH ${Slicer3_BINARY_DIR}/lib/ 
        CACHE PATH 
        "Single output directory for building all libraries." 
        FORCE)
SET (EXECUTABLE_OUTPUT_PATH ${Slicer3_BINARY_DIR}/bin/ 
        CACHE PATH 
        "Single output directory for building all executables." 
        FORCE)

# Add the testing directories

OPTION(Slicer3_BUILD_TESTING "Test the project" ON)
IF(Slicer3_BUILD_TESTING)
    ENABLE_TESTING()
    INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)
ENDIF(Slicer3_BUILD_TESTING)


#
# Subdirectories
#

SUBDIRS(
    Libs
    Base
    Modules
    Applications
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)

