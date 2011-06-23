## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   enable_testing()
##   include(Dart)
site_name(SITE)
set(CTEST_PROJECT_NAME "BRAINSFit")
set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "testing.psychiatry.uiowa.edu")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=BRAINSFit")
set(CTEST_DROP_SITE_CDASH TRUE)
