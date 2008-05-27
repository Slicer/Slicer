project(ResampleDTI) 

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories( ${CLI_SOURCE_DIR} )

set (CLP ResampleDTI)
set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP}
  ITKIO
  ITKAlgorithms 
  ITKBasicFilters 
  ITKCommon
  ModuleDescriptionParser
  # MRML
  )

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    ResampleDTI
    )
  slicer3_install_plugins(${TARGETS})

endif (Slicer3_SOURCE_DIR)
