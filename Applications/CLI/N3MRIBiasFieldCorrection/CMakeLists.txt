project( N3MRIBiasFieldCorrection )

find_package(GenerateCLP REQUIRED)

if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories(${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR})

#####################

set(CLP N3MRIBiasFieldCorrection)

set(${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml
  ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP}
  ITKIO
  ITKCommon
  ITKAlgorithms)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})

set_target_properties(${CLP}Lib PROPERTIES COMPILE_FLAGS
  "-Dmain=ModuleEntryPoint")

slicer3_set_plugins_output_path(${CLP}Lib)

target_link_libraries(${CLP}Lib ITKIO ITKCommon ITKAlgorithms)

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in
  # an installation) and install each target in the developer area (for
  # running from a build)

  set(TARGETS N3MRIBiasFieldCorrection)

  slicer3_install_plugins(${TARGETS})

endif(Slicer3_SOURCE_DIR)
