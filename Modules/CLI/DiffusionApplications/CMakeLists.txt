project( DiffusionFiltering )

include_directories(${DiffusionFiltering_SOURCE_DIR}/DiffusionApplicationsCommon)

set(filters
  DWIJointRicianLMMSEFilter
  DWIRicianLMMSEFilter
  DWIUnbiasedNonLocalMeansFilter
  ResampleDTIVolume
  )
foreach(filter ${filters})
  add_subdirectory(${filter})
endforeach()
