
set(filters
  DWIJointRicianLMMSEFilter
  DWIRicianLMMSEFilter
  DWIUnbiasedNonLocalMeansFilter
  ResampleDTIVolume
  )
foreach(filter ${filters})
  add_subdirectory(${filter})
endforeach()
