PROJECT( DiffusionFiltering )
cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${DiffusionFiltering_SOURCE_DIR}/DiffusionApplicationsCommon)

SET(filters
  jointLMMSE
  dwiNoiseFilter
  dwiUNLM
  #StochasticTractographyFilter
  ResampleDTI
  )
FOREACH(filter ${filters})
  ADD_SUBDIRECTORY(${filter})
ENDFOREACH(filter)
