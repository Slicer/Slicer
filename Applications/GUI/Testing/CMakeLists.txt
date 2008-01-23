PROJECT( GUITesting )
#####################

FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

FIND_PACKAGE(vtkITK REQUIRED)

LINK_DIRECTORIES(${ModuleDescriptionParser_BINARY_DIR})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


SET (CLP EventBrokerTest)

SET ( ${CLP}_SOURCE ${CLP}.cxx )
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
LINK_DIRECTORIES(${vtkITK_LIB_DIR})
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    vtkITK vtkImaging vtkGraphics vtkIO
    ITKIO
    ModuleDescriptionParser
    MRML
)

INCLUDE_DIRECTORIES(
        ${vtkITK_SOURCE_DIR} 
        ${vtkITK_BINARY_DIR} 
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR}
)


SET(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")
ADD_TEST(EventBrokerTest 
  ${Slicer3_BINARY_DIR}/Slicer3 --launch EventBrokerTest --logFileName ${TEMP}/EventLog.dot --graphFile ${TEMP}/EventBroker.dot)
