project( Realign )

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

link_directories(${ModuleDescriptionParser_BINARY_DIR} ${vtkTeem_LIB_DIR})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )

#####################

set (CLP Realign)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)

add_executable(${CLP} ${${CLP}_SOURCE} vtkPrincipalAxesAlign.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP}
  vtkImaging vtkGraphics vtkIO
  ModuleDescriptionParser vtkTeem
  MRML
  SlicerBaseCLI
  )

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE} vtkPrincipalAxesAlign.cxx )
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib vtkImaging vtkGraphics vtkIO ModuleDescriptionParser vtkTeem MRML SlicerBaseCLI)

include_directories(
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR}
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR})



if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    Realign
    RealignLib
    )
  slicer3_install_plugins(${TARGETS})

endif (Slicer3_SOURCE_DIR)
