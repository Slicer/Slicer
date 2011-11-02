set(CTEST_PROJECT_NAME "Slicer")
set(CTEST_NIGHTLY_START_TIME "3:00:00 UTC")

set(CTEST_DROP_METHOD "http")
# See this issue for more details: http://public.kitware.com/Bug/view.php?id=12555
#set(CTEST_DROP_SITE "slicer.cdash.org")
#set(CTEST_DROP_LOCATION "/submit.php?project=Slicer4")
set(CTEST_DROP_SITE "www.cdash.org")
set(CTEST_DROP_LOCATION "/slicer4/submit.php?project=Slicer4")
set(CTEST_DROP_SITE_CDASH TRUE)

