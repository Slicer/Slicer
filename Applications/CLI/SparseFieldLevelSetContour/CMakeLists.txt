project( SparseFieldLevelSetContour )

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

link_directories(${ModuleDescriptionParser_BINARY_DIR} ${vtkTeem_LIB_DIR})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )

#####################

set (CLP SparseFieldLevelSetContour)

set ( SparseFieldLevelSetContour_SOURCE SparseFieldLevelSetContour.cxx Utils.cxx MeshOps.cxx LSops.cxx  MeshOps.h Utils.h LSops.h
MeshEnergy.cpp MeshEnergy.h MeanCurvatureEnergy.cpp MeanCurvatureEnergy.h ModuleEntry.h
ModuleEntry.cxx vtkComputeLocalGeometry.h vtkComputeLocalGeometry.cxx vtkInitClosedPath.h
vtkInitClosedPath.cxx vtkLevelSetMeshEvolver.h vtkLevelSetMeshEvolver.cxx)

generateclp(SparseFieldLevelSetContour_SOURCE SparseFieldLevelSetContour.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${SparseFieldLevelSetContour_SOURCE})
slicer3_set_plugins_output_path(${CLP}Lib)
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ModuleDescriptionParser vtkTeem MRML SlicerBaseCLI)

add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Lib)

include_directories(
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR}
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR})



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
