PROJECT(VoxelMeshModule)

FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

IF (Slicer3_SOURCE_DIR)
  SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
ENDIF (Slicer3_SOURCE_DIR)

 
INCLUDE_DIRECTORIES( ${MimxMeshing_SOURCE_DIR}/Filter 
                     ${MimxMeshing_BINARY_DIR}/Filter 
                     ${MimxMeshing_SOURCE_DIR}/IO 
                     ${MimxMeshing_BINARY_DIR}/IO
                     ${VoxelMeshModule_BINARY_DIR}
                     ${VoxelMeshModule_SOURCE_DIR}
                     ${CLI_SOURCE_DIR}
                     ${CLI_BINARY_DIR} ) 
                     
SET (VoxelMeshModule_SOURCE 
     VoxelMesh.cxx 
     ${CLI_SOURCE_DIR}/vtkPluginFilterWatcher.cxx)

GENERATECLP( VoxelMeshModule_SOURCE VoxelMesh.xml ${VoxelMeshModule_SOURCE_DIR}/Resources/mimxLogo.h )
ADD_EXECUTABLE ( VoxelMeshModule ${VoxelMeshModule_SOURCE} )
TARGET_LINK_LIBRARIES ( VoxelMeshModule 
  mimxFilter
  MimxMeshIO
  ITKCommon 
  ITKBasicFilters 
  ITKIO 
  vtkIO 
  vtkHybrid 
  vtkFiltering 
  vtkCommon )

ADD_LIBRARY(VoxelMeshModuleLib SHARED ${VoxelMeshModule_SOURCE})
SET_TARGET_PROPERTIES (VoxelMeshModuleLib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (VoxelMeshModuleLib ITKIO 
  mimxFilter
  MimxMeshIO
  ITKCommon 
  ITKBasicFilters 
  ITKIO 
  vtkIO 
  vtkHybrid 
  vtkFiltering 
  vtkCommon)



INSTALL(TARGETS VoxelMeshModule
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)

