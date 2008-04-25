project(SlicerIO)

# look for the Tcl library
if (TCL_LIBRARY)
else(TCL_LIBRARY)
  message(FATAL_ERROR "Could not find tcl")
endif (TCL_LIBRARY)

add_library(slicerio STATIC utilities.c slicerio.c)

if(BUILD_SHARED_LIBS)
  install(TARGETS slicerio
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries (slicerio ${TCL_LIBRARY})

add_executable(testSlicerIO testSlicerIO.c)
target_link_libraries(testSlicerIO slicerio ${TCL_LIBRARY})

add_executable(scat scat.c)
target_link_libraries(scat slicerio ${TCL_LIBRARY})

# This seems to be required for a static build against a static Tcl/Tk

if(NOT WIN32 AND VTK_TCL_TK_STATIC)
  target_link_libraries(scat m dl)
  target_link_libraries(testSlicerIO m dl)
endif(NOT WIN32 AND VTK_TCL_TK_STATIC)

