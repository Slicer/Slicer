PROJECT( Realign )

# 
# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  vtkRealignConfigure.h.in
  Applications/CLI/Realign/vtkRealignConfigure.h
)

FIND_PACKAGE(GenerateCLP REQUIRED)
FIND_PACKAGE(vtkITK REQUIRED)

FIND_PACKAGE(ModuleDescriptionParser REQUIRED)
LINK_DIRECTORIES(${ModuleDescriptionParser_BINARY_DIR} ${vtkTeem_LIB_DIR})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

IF (Slicer3_SOURCE_DIR)
  SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
ENDIF (Slicer3_SOURCE_DIR)

INCLUDE_DIRECTORIES( ${CLI_SOURCE_DIR} )

#####################

SET (CLP Realign)

SET ( ${CLP}_SOURCE ${CLP}.cxx vtkPrincipalAxesAlign.cxx ${CLI_SOURCE_DIR}/vtkPluginFilterWatcher.cxx )
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
LINK_DIRECTORIES(${vtkITK_LIB_DIR})
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    vtkITK vtkImaging vtkGraphics vtkIO
    ITKIO
    ModuleDescriptionParser vtkTeem
    MRML
)
INCLUDE_DIRECTORIES(
        ${vtkITK_SOURCE_DIR} 
        ${vtkITK_BINARY_DIR}
        ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR}) 
#        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 



IF (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  SET(TARGETS        
        Realign
  )

  FOREACH(target ${TARGETS})
    INSTALL(TARGETS ${target}
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
