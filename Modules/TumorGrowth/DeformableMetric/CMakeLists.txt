PROJECT(DetectingTumorGrowth)

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
    MESSAGE(FATAL_ERROR
        "ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")

ADD_EXECUTABLE(DetectGrowth DetectGrowth.cxx)

TARGET_LINK_LIBRARIES(DetectGrowth ITKCommon ITKIO)

ADD_EXECUTABLE(applyDeformationITK applyDeformationITK.cxx)

TARGET_LINK_LIBRARIES(applyDeformationITK ITKNumerics ITKIO)

ADD_EXECUTABLE(DetectGrowthSegmentation DetectGrowthSegmentation.cxx)

TARGET_LINK_LIBRARIES(DetectGrowthSegmentation ITKCommon ITKIO)



                           
