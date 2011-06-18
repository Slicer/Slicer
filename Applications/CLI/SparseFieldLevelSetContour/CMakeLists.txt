project( SparseFieldLevelSetContour )

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkTeem MRML SlicerBaseCLI
  ADDITIONAL_SRCS
    SparseFieldLevelSetContour.cxx 
    Utils.cxx 
    MeshOps.cxx 
    LSops.cxx  
    MeshOps.h 
    Utils.h 
    LSops.h
    MeshEnergy.cpp 
    MeshEnergy.h 
    MeanCurvatureEnergy.cpp
    MeanCurvatureEnergy.h 
    ModuleEntry.h
    ModuleEntry.cxx 
    vtkComputeLocalGeometry.h 
    vtkComputeLocalGeometry.cxx 
    vtkInitClosedPath.h
    vtkInitClosedPath.cxx 
    vtkLevelSetMeshEvolver.h 
    vtkLevelSetMeshEvolver.cxx
  LINK_DIRECTORIES 
    ${MRML_LIB_DIR} 
    ${vtkTeem_LIB_DIR}
    ${ModuleDescriptionParser_BINARY_DIR}
  INCLUDE_DIRECTORIES 
    ${MRML_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  )
