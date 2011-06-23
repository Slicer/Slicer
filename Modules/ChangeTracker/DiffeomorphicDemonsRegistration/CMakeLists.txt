project( DemonsRegistration )


find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )

#####################

set (CLP DemonsRegistration )

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ${ITK_LIBRARIES})

add_executable(${CLP} ${Slicer_SOURCE_DIR}/Applications/CLI/Templates/CommandLineSharedLibraryWrapper.cxx)
target_link_libraries (${CLP} ${CLP}Lib)

#the following line is an example of how to add a test to your project.
#Testname is the title for this particular test.  ExecutableToRun is the
#program which will be running this test.  It can either be a part of this
#project or an external executable.  After that list any args that are needed
#for this test.  Include as many tests as you like.  If your project doesn't have
#any tests you can comment out or delete the following line.
#if(WIN32)
# add_test(DemonsRegistrationTest1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/${CLP} --help)
#else(WIN32)
#  add_test(DemonsRegistrationTest1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CLP} --help)
#endif(WIN32)

# if(WIN32)
#  add_test(DemonsRegistrationTest2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/${CLP}  -f ${CMAKE_SOURCE_DIR}/RatLungSlice1.mha -m ${CMAKE_SOURCE_DIR}/RatLungSlice2.mha -v -s2 -g1 -i 20x20x20 )
# else(WIN32)
#   add_test(DemonsRegistrationTest2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CLP} -f ${CMAKE_SOURCE_DIR}/RatLungSlice1.mha -m ${CMAKE_SOURCE_DIR}/RatLungSlice2.mha -v -s2 -g1 -i 20x20x20)
# endif(WIN32)

# add_test(CompareImage ImageCompare ${CMAKE_SOURCE_DIR}/RatLungSlice2-Reg.mha output.mha)

#Once you're done modifying this template, you should rename it to "CMakeLists.txt"
