set(CTEST_PROJECT_NAME "Slicer")
set(CTEST_NIGHTLY_START_TIME "3:00:00 UTC")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "www.cdash.org")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=Slicer4")
set(CTEST_DROP_SITE_CDASH TRUE)

# Since the instance of CDash available on www.cdash.org is 1.8.1, let's use 
# Slicer4 dedicated instance of CDash in case the command 'ctest_upload' is available.
if(WITH_PACKAGES AND COMMAND ctest_upload)
  set(CTEST_DROP_LOCATION "/slicer4/submit.php?project=Slicer4")
endif()
