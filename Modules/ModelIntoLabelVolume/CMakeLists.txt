project(ModelIntoLabelVolume)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

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
  add_test(ModelIntoLabelVolumeTest ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/ModelIntoLabelVolumeTest.tcl)
endif(BUILD_TESTING)

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
