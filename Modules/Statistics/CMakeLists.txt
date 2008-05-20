project(Statistics)

subdirs(
  LabelStatistics        
  )

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)
