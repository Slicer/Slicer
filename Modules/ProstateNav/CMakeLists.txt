PROJECT(ProstateNav)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# ProstateNav_REVISION

set(ProstateNav_REVISION "")

# Case where Slicer is checkout using SVN
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.svn)
  FIND_PACKAGE(Subversion)
  if (Subversion_FOUND)
    Subversion_WC_INFO(${CMAKE_HOME_DIRECTORY} ProstateNav)
    set(ProstateNav_REVISION "${ProstateNav_WC_REVISION}")
  endif (Subversion_FOUND)
endif()

# TODO Make sure Slicer3/CMake/FindGit.cmake is in CMAKE_MODULE_PATH
# Case where Slicer is checkout using Git-SVN
#IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
#  FIND_PACKAGE(Git)
#  IF(Git_FOUND)
#    Git_WC_INFO(${CMAKE_HOME_DIRECTORY} ProstateNav)
#    set(ProstateNav_REVISION "${ProstateNav_WC_REVISION}")
#  endif()
#endif()

# --------------------------------------------------------------------------
# Include dirs


#
# Include dirs
#


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateTemplate
  ${CMAKE_CURRENT_SOURCE_DIR}/TransRectalProstateRobot
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${OpenIGTLinkIF_SOURCE_DIR}
  ${OpenIGTLinkIF_BINARY_DIR}
  ${OpenIGTLink_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )


#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkProstateNavConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkProstateNavConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateTemplate/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/TransRectalProstateRobot/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Wizard/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/newmat/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkProstateNavConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Sources

set(ProstateNav_SRCS 
  vtkMRMLProstateNavManagerNode.cxx
  vtkProstateNavLogic.cxx
  vtkProstateNavGUI.cxx
  vtkSlicerSecondaryViewerWindow.cxx 
  vtkMRMLRobotNode.cxx
  vtkMRMLRobotDisplayNode.cxx
  vtkProstateNavTargetDescriptor.cxx  
  vtkRobotToImageRegistration.cxx
  vtkZFrameRobotToImageRegistration.cxx
  vtkZFrameRobotToImageRegistration2.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkIGTLToMRMLCoordinate.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkIGTLToMRMLBrpRobotCommand.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkMRMLBrpRobotCommandNode.cxx  
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/vtkMRMLTransPerinealProstateRobotNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateTemplate/vtkMRMLTransPerinealProstateTemplateNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransRectalProstateRobot/vtkMRMLTransRectalProstateRobotNode.cxx  
  ${CMAKE_CURRENT_SOURCE_DIR}/TransRectalProstateRobot/vtkTransRectalFiducialCalibrationAlgo.cxx  
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavStepSetUp.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavStepSetUpTemplate.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavCalibrationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavFiducialCalibrationStep.cxx   
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavTargetingStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavManualControlStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavStepVerification.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavStepTargetingTemplate.cxx
  )

set(ProstateNavIGTL_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlCoordinateMessage.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlHeaderMessage.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlMoveToMessage.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/TransPerinealProstateRobot/igtlSetZFrameMessage.cxx
)

set(ProstateNavNewmat_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat9.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmatrm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/nm_misc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/bandmat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/solution.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/cholesky.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/hholder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmatex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat3.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/sort.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/evalue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/jacobi.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat4.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/submat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/example.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat5.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/svd.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/myexcept.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat6.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmatnl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/fft.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat7.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newfft.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/newmat8.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/newmat/ZLinAlg.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(ProstateNav ProstateNav_TCL_SRCS "${ProstateNav_SRCS}" "")


#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(ProstateNav_SRCS ProstateNav.txt)


# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ProstateNav)

add_library(${lib_name}
  ${ProstateNav_SRCS} 
  ${ProstateNavIGTL_SRCS}
  ${ProstateNavNewmat_SRCS} 
  ${ProstateNav_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

#link_directories(
#  ${Slicer_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}
#)


target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  OpenIGTLinkIF
  vtkSlicerVolumesModuleLogic
  Volumes
  )


slicer3_install_modules(${lib_name})

