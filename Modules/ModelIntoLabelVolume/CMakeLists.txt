project(ModelIntoLabelVolume)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
# Install support files

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.tcl")
foreach(file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}
  )


# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  add_test(ModelIntoLabelVolumeTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/ModelIntoLabelVolumeTest.tcl)
endif()

# --------------------------------------------------------------------------
# Install the testing data

file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")

foreach(file 
    ${TCLFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file) 

install(DIRECTORY
  Testing/TestData
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
