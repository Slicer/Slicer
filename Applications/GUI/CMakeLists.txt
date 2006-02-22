PROJECT( GUI )

SET (SRC
    Slicer3.cxx
)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )


ADD_EXECUTABLE (Slicer3 ${SRC})
TARGET_LINK_LIBRARIES( 
    Slicer3 SlicerBaseLogic SlicerBaseGUI 
    MRML vtkITK 
    ITKCommon ITKIO
    ${KWWidgets_LIBRARIES} )

IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
