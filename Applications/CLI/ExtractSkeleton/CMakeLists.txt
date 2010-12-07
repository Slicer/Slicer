project (ExtractSkeleton)

# Disable MSVC 8 warnings
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(WIN32)

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  #LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS
    SkelGraph.h
    SkelGraph.cxx
    tilg_iso_3D.h
    tilg_iso_3D.cxx
    coordTypes.h
    misc.h
    misc.cxx
  TARGET_LIBRARIES ITKIO
  )
