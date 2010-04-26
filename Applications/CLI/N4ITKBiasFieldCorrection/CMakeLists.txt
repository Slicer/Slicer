#cmake_minimum_required(VERSION 2.4)

#PROJECT( N4ITKBiasFieldCorrection )

#------------------Setup slicer builds.
#IF(BUILD_AGAINST_SLICER3)
#  FIND_PACKAGE(Slicer3 REQUIRED)
#  IF (Slicer3_FOUND)
#    INCLUDE(${Slicer3_USE_FILE})
#  ELSE (Slicer3_FOUND)
#    MESSAGE(FATAL_ERROR "Cannot build without a Slicer3 build tree or a Slicer3 installation. Please set Slicer3_DIR.")
#  ENDIF (Slicer3_FOUND)
#  slicer3_set_default_install_prefix_for_external_projects()
#ELSE(BUILD_AGAINST_SLICER)
# Set up ITK
#  FIND_PACKAGE(ITK)
#  IF(ITK_FOUND)
#    INCLUDE(${ITK_USE_FILE})
#  ELSE(ITK_FOUND)
#    MESSAGE(FATAL_ERROR
#      "Cannot build without ITK.  Please set ITK_DIR.")
#  ENDIF(ITK_FOUND)
#ENDIF(BUILD_AGAINST_SLICER3)

#FIND_PACKAGE(GenerateCLP NO_MODULE REQUIRED)
#F(GenerateCLP_DIR)
# INCLUDE(${GenerateCLP_USE_FILE})
#ELSE(GenerateCLP_DIR)
#  MESSAGE(FATAL_ERROR "Can't build without GenerateCLP. Please set GenerateCLP_DIR")
#ENDIF(GenerateCLP_DIR)

GENERATECLP(N4ITKBiasFieldCorrection N4ITKBiasFieldCorrection.xml)
ADD_EXECUTABLE(N4ITKBiasFieldCorrection N4ITKBiasFieldCorrection.cxx)
TARGET_LINK_LIBRARIES(N4ITKBiasFieldCorrection ITKIO ITKStatistics ITKNumerics)
slicer3_set_plugins_output_path(N4ITKBiasFieldCorrection)
slicer3_install_plugins(N4ITKBiasFieldCorrection)

