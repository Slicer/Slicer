project( CLI )

#

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)
find_package(vtkITK REQUIRED)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )
link_directories(${SlicerBaseCLI_BINARY_DIR})

#####################
if(BUILD_TESTING)
  set (CLP CLIModule4Test)

  set ( ${CLP}_SOURCE ${CLP}.cxx)
  generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

  add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
  set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
  slicer3_set_plugins_output_path(${CLP}Lib)
  target_link_libraries (${CLP}Lib)

  add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
  slicer3_set_plugins_output_path(${CLP})
  target_link_libraries (${CLP} ${CLP}Lib)
endif()

#####################
set (CLP DiffusionTensorTest)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKIO)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
#set (CLP DiffusionWeightedTest)

#set ( ${CLP}_SOURCE ${CLP}.cxx)
#generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

#add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
#set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
#slicer3_set_plugins_output_path(${CLP}Lib)
#target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
#slicer3_set_plugins_output_path(${CLP})
#target_link_libraries (${CLP} ${CLP}Lib)


#####################
set (CLP DiffusionTensorEstimation)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
link_directories(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR} ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR})
target_link_libraries (${CLP} MRML vtkTeem SlicerBaseLogic)
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
set (CLP DiffusionTensorMathematics)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
link_directories(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR})
target_link_libraries (${CLP} MRML vtkTeem)
include_directories(
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
set (CLP OrientImage)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)


#####################
set (CLP LinearRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKNumerics)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP RigidRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKNumerics)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP AffineRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKNumerics)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP BSplineDeformableRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKStatistics ITKIO ITKBasicFilters ITKNumerics)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP TestGridTransformRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
target_link_libraries (${CLP}Lib ITKStatistics ITKIO ITKBasicFilters ITKNumerics)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP CheckerBoard)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml  ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP ResampleVolume)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP PolyDataToLabelmap)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO vtkIO vtkGraphics ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP GaussianBlurImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)


#####################
set (CLP ConfidenceConnected)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP ExecutionModelTour)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/BillsLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} MRML)

#####################


set (CLP ImageReadDicomWrite)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp (${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP CurvatureAnisotropicDiffusion)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP GradientAnisotropicDiffusion)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)
#####################
set (CLP MedianImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)
#####################
add_subdirectory(DicomToNrrdConverter)
#####################
# set (CLP DWIDicomLoad)

# set ( ${CLP}_SOURCE ${CLP}.cxx)
# generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
# add_executable(${CLP} ${${CLP}_SOURCE})
# slicer3_set_plugins_output_path(${CLP})
# target_link_libraries (${CLP} ITKIO ITKBasicFilters)

# add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
# slicer3_set_plugins_output_path(${CLP}Lib)
# set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
# target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP HistogramMatching)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKStatistics)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP OtsuThresholdImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP OtsuThresholdSegmentation)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP Subtract)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP Multiply)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP Add)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP Threshold)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP Mask)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP Cast)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP VotingBinaryHoleFillingImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################

set (CLP ModelMaker)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
# TODO: remove SlicerBaseLogic once pass in color node information
target_link_libraries (${CLP}
  vtkITK
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR})

#####################

set (CLP MultipleModelsExample)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
# TODO: remove SlicerBaseLogic once pass in color node information
target_link_libraries (${CLP}
  vtkITK
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR})


#####################
set (CLP GrayscaleModelMaker)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP}
  vtkITK
  ModuleDescriptionParser
  SlicerBaseCLI
  vtkHybrid
  )
include_directories(${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR})

#####################

set (CLP MergeModels)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

link_directories(${vtkITK_LIB_DIR})
include_directories(${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR})

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib vtkIO vtkGraphics)

add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib
  vtkIO
  vtkGraphics
  )

#####################
set (CLP GrayscaleFillHoleImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP GrayscaleGrindPeakImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP LabelMapSmoothing)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKAlgorithms ITKBasicFilters)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################
set (CLP ImageLabelCombine)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)


#####################
set (CLP ComputeSUVBodyWeight)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h ${Slicer_SOURCE_DIR}/Resources/CTSCLogo.h)

link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP}
  vtkITK
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )

include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR}
)


#####################
set (CLP ResampleVolume2)

set( TRANSFORM_SOURCE 
DiffusionApplications/ResampleDTI/itkWarpTransform3D.h
DiffusionApplications/ResampleDTI/itkWarpTransform3D.txx
DiffusionApplications/ResampleDTI/itkTransformDeformationFieldFilter.h
DiffusionApplications/ResampleDTI/itkTransformDeformationFieldFilter.txx
DiffusionApplications/ResampleDTI/dtiprocessFiles/deformationfieldio.h
DiffusionApplications/ResampleDTI/dtiprocessFiles/deformationfieldio.cxx
DiffusionApplications/ResampleDTI/dtiprocessFiles/dtitypes.h
DiffusionApplications/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
DiffusionApplications/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
)

set ( ${CLP}_SOURCE ${CLP}.cxx ${TRANSFORM_SOURCE} )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKAlgorithms)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

#####################

# culled from Slicer 3.6 release
#set (CLP ZeroCrossingBasedEdgeDetectionImageFilter)
#
#set ( ${CLP}_SOURCE ${CLP}.cxx)
#generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h)
#
#add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
#slicer3_set_plugins_output_path(${CLP}Lib)
#set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
#target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)
#
#add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
#slicer3_set_plugins_output_path(${CLP})
#target_link_libraries (${CLP} ${CLP}Lib)

#####################


# culled from Slicer 3.6 release
#set (CLP FreesurferSurfaceSectionExtraction)
#set ( ${CLP}_SOURCE ${CLP}.cxx)
#generateclp(${CLP}_SOURCE ${CLP}.xml)
#
#add_executable(${CLP} ${${CLP}_SOURCE})
#slicer3_set_plugins_output_path(${CLP})
#target_link_libraries (${CLP} ITKIO ITKBasicFilters)
#
#set( Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3 )

###################

SET(cli_modules
  MRIBiasFieldCorrection
  SparseFieldLevelSetContour
  Realign
  DiffusionApplications
  #EndoNavTracking
  ExtractSkeleton
  Java
  N4ITKBiasFieldCorrection
  RobustStatisticsSegmenter
  )

if ( ${ITK_VERSION_MAJOR} GREATER 3 )

  SET(cli_modules ${cli_modules} RegisterImagesModule)

else ( ${ITK_VERSION_MAJOR} GREATER 3 )

  if ( ${ITK_VERSION_MAJOR} MATCHES 3 AND ${ITK_VERSION_MINOR} GREATER 12 )

    SET(cli_modules ${cli_modules} RegisterImagesModule)

  else ( ${ITK_VERSION_MAJOR} MATCHES 3 AND ${ITK_VERSION_MINOR} GREATER 12 )

    if ( ${ITK_VERSION_MAJOR} MATCHES 3 AND ${ITK_VERSION_MINOR} GREATER 6 )
      if ( ITK_USE_OPTIMIZED_REGISTRATION_METHODS )
        SET(cli_modules ${cli_modules} RegisterImagesModule)
      else ( ITK_USE_OPTIMIZED_REGISTRATION_METHODS )
        message ("RegisterImagesModule requires that the ITK_USE_OPTIMIZED_REGISTRATION_METHODS option be turned on when building ITK")
      endif ( ITK_USE_OPTIMIZED_REGISTRATION_METHODS )
    else ( ${ITK_VERSION_MAJOR} MATCHES 3 AND ${ITK_VERSION_MINOR} GREATER 6 )
      message ("RegisterImagesModule requires ITK version 3.8 or higher")
    endif ( ${ITK_VERSION_MAJOR} MATCHES 3 AND ${ITK_VERSION_MINOR} GREATER 6 )

  endif ( ${ITK_VERSION_MAJOR} MATCHES 3 AND ${ITK_VERSION_MINOR} GREATER 12 )

endif ( ${ITK_VERSION_MAJOR} GREATER 3 )

# culled from Slicer 3.6 release
#SET(cli_modules ${cli_modules}
  # UtahBSpline
  # DiffeomorphicDemonsRegistration
#  )

if(Slicer_USE_MIDAS)
  SET(cli_modules ${cli_modules} MIDASApplications)
endif(Slicer_USE_MIDAS)

if(Slicer_USE_BatchMake)
  SET(cli_modules ${cli_modules} BatchMakeApplications)
endif(Slicer_USE_BatchMake)

#SET(cli_modules ${cli_modules} Utah)

FOREACH(cli_module ${cli_modules})
  ADD_SUBDIRECTORY(${cli_module})
ENDFOREACH(cli_module)

if (Slicer_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS
    ResampleVolume
    ResampleVolumeLib
    ResampleVolume2
    ResampleVolume2Lib
    ExecutionModelTour
    ImageReadDicomWrite
    ImageReadDicomWriteLib
    Add
    AddLib
    Threshold
    ThresholdLib
    Mask
    MaskLib
    Cast
    CastLib
    ConfidenceConnected
    ConfidenceConnectedLib
    #CurvatureAnisotropicDiffusion
    #CurvatureAnisotropicDiffusionLib
    GradientAnisotropicDiffusion
    GradientAnisotropicDiffusionLib
    MedianImageFilter
    MedianImageFilterLib
    Multiply
    MultiplyLib
    OtsuThresholdImageFilter
    OtsuThresholdImageFilterLib
    OtsuThresholdSegmentation
    OtsuThresholdSegmentationLib
    Subtract
    SubtractLib
    VotingBinaryHoleFillingImageFilter
    VotingBinaryHoleFillingImageFilterLib
    ModelMaker
    GrayscaleModelMaker
    GrayscaleFillHoleImageFilter
    GrayscaleFillHoleImageFilterLib
    GrayscaleGrindPeakImageFilter
    GrayscaleGrindPeakImageFilterLib
    HistogramMatching
    HistogramMatchingLib
    RigidRegistration
    RigidRegistrationLib
    LinearRegistration
    LinearRegistrationLib
    AffineRegistration
    AffineRegistrationLib
    BSplineDeformableRegistration
    BSplineDeformableRegistrationLib
    ComputeSUVBodyWeight
    LabelMapSmoothing
    LabelMapSmoothingLib
    ImageLabelCombine
    OrientImage
    OrientImageLib
    CheckerBoardLib
    GaussianBlurImageFilter
    GaussianBlurImageFilterLib
#    DWIDicomLoad
#    DWIDicomLoadLib
    #FreesurferSurfaceSectionExtraction
    #ZeroCrossingBasedEdgeDetectionImageFilter
    #ZeroCrossingBasedEdgeDetectionImageFilterLib
    DiffusionTensorMathematics
    DiffusionTensorEstimation
    )

  slicer3_install_plugins(${TARGETS})

endif (Slicer_SOURCE_DIR)

#

option(BUILD_BRAINSTOOLS "Build the BRAINS subset of registraiton and segmentation tools" ON)
if(BUILD_BRAINSTOOLS)
  add_subdirectory(BRAINSTools)
endif(BUILD_BRAINSTOOLS)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
