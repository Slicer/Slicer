PROJECT( Realign )

FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

LINK_DIRECTORIES(${ModuleDescriptionParser_BINARY_DIR} ${vtkTeem_LIB_DIR})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

IF (Slicer3_SOURCE_DIR)
  SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
ENDIF (Slicer3_SOURCE_DIR)

INCLUDE_DIRECTORIES( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )

#####################

SET (CLP Realign)

SET ( ${CLP}_SOURCE ${CLP}.cxx )
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)

ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE} vtkPrincipalAxesAlign.cxx)
TARGET_LINK_LIBRARIES (${CLP}
     vtkImaging vtkGraphics vtkIO
    ModuleDescriptionParser vtkTeem
    MRML
    SlicerBaseCLI
)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE} vtkPrincipalAxesAlign.cxx )
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib vtkImaging vtkGraphics vtkIO ModuleDescriptionParser vtkTeem MRML SlicerBaseCLI)

INCLUDE_DIRECTORIES(
        ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR}
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR})



IF (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  SET(TARGETS        
        Realign
        RealignLib
  )

  FOREACH(target ${TARGETS})
    INSTALL(TARGETS ${target}
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
