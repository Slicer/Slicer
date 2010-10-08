project(Statistics)

ADD_SUBDIRECTORY(LabelStatistics)

if(Slicer_BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(Slicer_BUILD_TESTING)
