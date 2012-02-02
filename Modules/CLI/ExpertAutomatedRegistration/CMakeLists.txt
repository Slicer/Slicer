
set(CLP ExpertAutomatedRegistration)
slicerMacroBuildCLI(
  NAME ${CLP}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/ITKRegistrationHelper
  )

if(SUPPORT_2D_IMAGES)
  set_target_properties(${CLP}Lib
    PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DSUPPORT_2D_IMAGES" )
endif()

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
