add_library(A SHARED A.cxx)
add_library(B SHARED B.cxx)
add_executable(C C.cxx)
target_link_libraries(C B A)
