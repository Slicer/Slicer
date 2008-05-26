project( dwiNoiseFilter )

# 
# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
#CONFIGURE_FILE(
#  vtkRealignConfigure.h.in
#  Applications/CLI/Realign/vtkRealignConfigure.h
#)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

slicer3_set_default_plugins_output_path()

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )

#####################

set (CLP dwiNoiseFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
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
    dwiNoiseFilter
    )
  slicer3_install_plugins(${TARGETS})

endif (Slicer3_SOURCE_DIR)
