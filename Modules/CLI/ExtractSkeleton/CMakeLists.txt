
#-----------------------------------------------------------------------------
set(MODULE_NAME ExtractSkeleton)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKIOImageBase
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
# Disable MSVC 8 warnings
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif()

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  #LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS
    SkelGraph.h
    SkelGraph.cxx
    tilg_iso_3D.h
    tilg_iso_3D.cxx
    coordTypes.h
    misc.h
    misc.cxx
  TARGET_LIBRARIES ${ITK_LIBRARIES}
    MRMLCore
    vtkSlicerMarkupsModuleMRML
  INCLUDE_DIRECTORIES
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkSlicerMarkupsModuleMRML_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
