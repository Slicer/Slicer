project(Tractography)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------

SET(Tractography_libs
  Logic
  Seeding
  Settings
  Display
  FiducialSeeding
  Editor
  )
FOREACH(lib ${Tractography_libs})
  ADD_SUBDIRECTORY(${lib})
ENDFOREACH(lib)

# --------------------------------------------------------------------------
if(Slicer_BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(Slicer_BUILD_TESTING)

# Install testing data

file(GLOB_RECURSE MRMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.mrml")
file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
file(GLOB_RECURSE VTKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.vtk")

foreach(file 
    ${MRMLFILES}
    ${TCLFILES}
    ${VTKFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file) 

install(DIRECTORY
  Testing
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeLists.txt" EXCLUDE
  )
