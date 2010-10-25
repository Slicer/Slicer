PROJECT(PatientToImageRegistration)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkPatientToImageRegistrationConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkPatientToImageRegistrationConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkPatientToImageRegistrationConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Sources
set(PatientToImageRegistration_SRCS 
  vtkPatientToImageRegistrationLogic.cxx
  vtkPatientToImageRegistrationGUI.cxx
)
SET (PatinetToImageRegistration_NO_WRAP_SRCS
  PivotCalibration.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(PatientToImageRegistration PatientToImageRegistration_TCL_SRCS "${PatientToImageRegistration_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(PatientToImageRegistration_SRCS PatientToImageRegistration.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name PatientToImageRegistration)
add_library(${lib_name}
  ${PatientToImageRegistration_SRCS} 
  ${PatientToImageRegistration_TCL_SRCS}
  ${PatinetToImageRegistration_NO_WRAP_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})


