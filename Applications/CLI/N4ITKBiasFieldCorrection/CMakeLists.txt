SET(CLP N4ITKBiasFieldCorrection)
SET(${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES(${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
slicer3_set_plugins_output_path(${CLP}Lib)
TARGET_LINK_LIBRARIES(${CLP}Lib ITKIO ITKStatistics ITKNumerics)

ADD_EXECUTABLE(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
TARGET_LINK_LIBRARIES(${CLP} ${CLP}Lib)

slicer3_set_plugins_output_path(${CLP})
slicer3_set_plugins_output_path(${CLP}Lib)

IF(Slicer_SOURCE_DIR)
  slicer3_install_plugins(${CLP} ${CLP}Lib)
ENDIF(Slicer_SOURCE_DIR)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
