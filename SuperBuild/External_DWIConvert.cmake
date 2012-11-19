
set(DWIConvert_DEPENDENCIES ITKv4 DCMTK SlicerExecutionModel)

SlicerMacroCheckExternalProjectDependency(DWIConvert)

set(proj DWIConvert)

set(DWIConvert_REPOSITORY
  ${git_protocol}://github.com/Chaircrusher/NewDicomToNrrdConverter.git)

set(DWIConvert_GIT_TAG d8f0cbc604051aaabe30e6b32fbd7e8bdc4f1e88 )
set(DWIConvert_DCMTK_ARGS -DDCMTK_DIR:PATH=${DCMTK_DIR} )

ExternalProject_Add(${proj}
  GIT_REPOSITORY ${DWIConvert_REPOSITORY}
  GIT_TAG ${DWIConvert_GIT_TAG}
  ${slicer_external_update}
  SOURCE_DIR ${proj}
  BINARY_DIR ${proj}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DBUILD_SHARED_LIBS:BOOL=ON
  -DUSE_SYSTEM_DCMTK:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
  -DITK_DIR=${ITK_DIR}
  ${DWIConvert_DCMTK_ARGS}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
  ${DWIConvert_DEPENDENCIES}
  )

ExternalProject_Add_Step(${proj} InstallSlicerCMakeLists
  COMMENT "Install simple CMakeList.txt for DWIConvert"
  DEPENDEES download
  DEPENDERS configure
  COMMAND ${CMAKE_COMMAND}
  -E copy ${CMAKE_CURRENT_LIST_DIR}/DWIConvert.CMakeLists.txt
  <SOURCE_DIR>/CMakeLists.txt
  )

set(DWIConvert_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
