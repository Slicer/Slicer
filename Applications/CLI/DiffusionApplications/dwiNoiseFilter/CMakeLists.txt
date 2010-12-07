project( dwiNoiseFilter )

#
# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
#CONFIGURE_FILE(
#  vtkRealignConfigure.h.in
#  Applications/CLI/Realign/vtkRealignConfigure.h
#)

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})


link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )

#####################

set (CLP dwiNoiseFilter)
if (Slicer_SOURCE_DIR)
   SET(LOGOFILE ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
endif (Slicer_SOURCE_DIR)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${LOGOFILE})
add_executable(${CLP} ${${CLP}_SOURCE})
if(Slicer_SOURCE_DIR)
  slicer3_set_plugins_output_path(${CLP})
endif(Slicer_SOURCE_DIR)
target_link_libraries (${CLP}
  ITKIO
  ITKAlgorithms
  ITKBasicFilters
  ModuleDescriptionParser
  # MRML
  )


if (Slicer_SOURCE_DIR)
  # install each target in the production area (where it would appear in an
  # installation) and install each target in the developer area (for running
  # from a build)
  set(TARGETS
    dwiNoiseFilter
    )
if(Slicer_SOURCE_DIR)
  slicer3_install_plugins(${TARGETS})
endif(Slicer_SOURCE_DIR)

endif (Slicer_SOURCE_DIR)
