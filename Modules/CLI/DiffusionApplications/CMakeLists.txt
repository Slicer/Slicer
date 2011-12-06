project( DiffusionFiltering )
cmake_minimum_required(VERSION 2.8.2)

include_directories(${DiffusionFiltering_SOURCE_DIR}/DiffusionApplicationsCommon)

set(filters
  jointLMMSE
  dwiNoiseFilter
  dwiUNLM
  #StochasticTractographyFilter
  ResampleDTI
  )
foreach(filter ${filters})
  add_subdirectory(${filter})
endforeach(filter)
