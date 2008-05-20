project(SlicerIO)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# look for the Tcl library
if (TCL_LIBRARY)
else(TCL_LIBRARY)
  message(FATAL_ERROR "Could not find tcl")
endif (TCL_LIBRARY)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name slicerio)
add_library(${lib_name} STATIC 
  utilities.c 
  slicerio.c
)
target_link_libraries (${lib_name} 
  ${TCL_LIBRARY}
)

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build executables

add_executable(testSlicerIO testSlicerIO.c)
target_link_libraries(testSlicerIO ${lib_name} ${TCL_LIBRARY})

add_executable(scat scat.c)
target_link_libraries(scat ${lib_name} ${TCL_LIBRARY})

# This seems to be required for a static build against a static Tcl/Tk

if(NOT WIN32 AND VTK_TCL_TK_STATIC)
  target_link_libraries(scat m dl)
  target_link_libraries(testSlicerIO m dl)
endif(NOT WIN32 AND VTK_TCL_TK_STATIC)

