project(Statistics)

ADD_SUBDIRECTORY(LabelStatistics)

if(Slicer3_BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(Slicer3_BUILD_TESTING)
