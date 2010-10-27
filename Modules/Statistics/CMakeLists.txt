project(Statistics)

ADD_SUBDIRECTORY(LabelStatistics)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
