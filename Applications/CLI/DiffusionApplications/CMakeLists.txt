PROJECT( DiffusionFiltering )

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
