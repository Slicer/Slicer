project(VoxelMeshModule)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_modules_install_prefix()
endif(NOT Slicer3_SOURCE_DIR)

slicer3_set_default_plugins_output_path()

# --------------------------------------------------------------------------
# Include dirs

include_directories( 
  ${MimxMeshing_SOURCE_DIR}/Filter 
  ${MimxMeshing_BINARY_DIR}/Filter 
  ${MimxMeshing_SOURCE_DIR}/IO 
  ${MimxMeshing_BINARY_DIR}/IO
  ${VoxelMeshModule_BINARY_DIR}
  ${VoxelMeshModule_SOURCE_DIR}
  ${SlicerBaseCLI_BINARY_DIR}
  ${SlicerBaseCLI_SOURCE_DIR} 
) 

# --------------------------------------------------------------------------
# Sources

set(VoxelMeshModule_SOURCE VoxelMesh.cxx)

generateclp(
  VoxelMeshModule_SOURCE 
  VoxelMesh.xml 
  ${VoxelMeshModule_SOURCE_DIR}/Resources/mimxLogo.h 
  )

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(VoxelMeshModule ${VoxelMeshModule_SOURCE})
target_link_libraries(VoxelMeshModule 
  mimxFilter
  MimxMeshIO
  SlicerBaseCLI
  ITKCommon 
  ITKBasicFilters 
  ITKIO 
  vtkIO 
  vtkHybrid 
  vtkFiltering 
  vtkCommon
)

install(TARGETS VoxelMeshModule
  RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
  )

add_library(VoxelMeshModuleLib SHARED 
  ${VoxelMeshModule_SOURCE}
)
set_target_properties(VoxelMeshModuleLib 
  PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries(VoxelMeshModuleLib 
  ITKIO 
  mimxFilter
  MimxMeshIO
  SlicerBaseCLI
  ITKCommon 
  ITKBasicFilters 
  ITKIO 
  vtkIO 
  vtkHybrid 
  vtkFiltering 
  vtkCommon
)
