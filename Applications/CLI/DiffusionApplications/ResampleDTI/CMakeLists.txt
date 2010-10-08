project(ResampleDTI)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})


set (CLP ResampleDTI )
if (Slicer_SOURCE_DIR)
   SET(LOGOFILE ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
endif (Slicer_SOURCE_DIR)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${LOGOFILE})

Set( dtiProcessFiles
dtiprocessFiles/deformationfieldio.h
dtiprocessFiles/deformationfieldio.cxx
dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE} ${dtiProcessFiles} )
if(Slicer_SOURCE_DIR)
  slicer3_set_plugins_output_path(${CLP}Lib)
endif(Slicer_SOURCE_DIR)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib
  ModuleDescriptionParser
  ITKIO
  ITKAlgorithms
  ITKBasicFilters
)

add_executable(${CLP} ResampleDTIApp.cxx)
if(Slicer_SOURCE_DIR)
  slicer3_set_plugins_output_path(${CLP})
endif(Slicer_SOURCE_DIR)
target_link_libraries (${CLP} ${CLP}Lib)

if (Slicer_SOURCE_DIR)
  # install each target in the production area (where it would appear in an
  # installation) and install each target in the developer area (for running
  # from a build)
  set(TARGETS
    ${CLP}
    ${CLP}Lib
    )
  slicer3_install_plugins(${TARGETS})

endif (Slicer_SOURCE_DIR)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
