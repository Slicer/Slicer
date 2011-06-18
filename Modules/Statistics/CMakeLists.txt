project(Statistics)

ADD_SUBDIRECTORY(LabelStatistics)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()
