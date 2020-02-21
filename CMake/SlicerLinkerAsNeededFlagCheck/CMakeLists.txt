project(SlicerLinkerAsNeededFlagCheck)
add_library(A SHARED A.cxx)
add_library(B SHARED B.cxx)
# No need to use ctk_add_executable_utf8 (since no special characters are involved in this test)
# and this is a check that is done very early, even before CTK CMake functions are imported,
# therefore we use plain add_executable here.
add_executable(C C.cxx)
target_link_libraries(C B A)
