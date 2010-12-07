project (DownloadSlicerTutorials)

# Disable MSVC 8 warnings
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(WIN32)

# Find VTK
find_package(VTK REQUIRED)
include_directories(${VTK_SOURCE_DIR})

# Find SLICERLIBCURL (slicercurl)
find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

# Find GenerateCLP
find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  #LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${SLICERLIBCURL_LIBRARIES} ITKIO MRMLIDIO itkzlib
  )

