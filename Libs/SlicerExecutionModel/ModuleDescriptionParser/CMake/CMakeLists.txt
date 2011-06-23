project(TestBFD)

if(BUILD_SHARED_LIBS)
  add_library(TestBFD SHARED TestBFD.cxx)
else(BUILD_SHARED_LIBS)
  add_library(TestBFD TestBFD.cxx)
endif(BUILD_SHARED_LIBS)
target_link_libraries(TestBFD bfd iberty)
