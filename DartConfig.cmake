INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CTestConfig.cmake)

# Dashboard is opened for submissions for a 24 hour period starting at
# the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
SET (NIGHTLY_START_TIME "${CTEST_NIGHTLY_START_TIME}")


# Dart server to submit results (used by client)
SET(DROP_METHOD xmlrpc)
SET (DROP_SITE "${CTEST_DROP_SITE}")
SET (DROP_LOCATION "${CTEST_DROP_LOCATION}")
SET (TRIGGER_SITE "${CTEST_TRIGGER_SITE}")
SET (COMPRESS_SUBMISSION ON)
