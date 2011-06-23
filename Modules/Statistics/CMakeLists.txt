project(Statistics)

add_subdirectory(LabelStatistics)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
