#
# This is rather a problem: FindCenterOfBrainFilter is included in more than one
# NITRC based BRAINS Project, which means that BRAINSFit_SOURCE_DIR is not always
# defined.
#
# In practice, this is only a problem if programs other than BRAINSFit actually
# add FindCenterOfBrainFilter as a subdirectory, rather than just using it for the
# include files.
#
# But if a build fails because someone added it to a non-brainsfit application, I
# hope they look at this file and read this comment, and CUT THAT BUSINESS RIGHT OUT.
# Unless this is a subdirectory of BRAINSFit, there's NO USE adding it as a subdirectory
# in the CMakeLists.txt.  It will just end in tears.
#
include_directories(${BRAINSFit_SOURCE_DIR}/BRAINSFit_Common)
include_directories(${BRAINSFit_SOURCE_DIR}/LargestForegroundFilledMaskImageFilter)
include_directories(${BRAINSFit_SOURCE_DIR}/FindCenterOfBrainFilter)

enable_testing()
include(Dart)

set(FindCenterOfBrain_SRC
FindCenterOfBrainCLP.cxx)

generateclp(FindCenterOfBrain_SRC FindCenterOfBrain.xml)

add_executable(FindCenterOfBrain ${FindCenterOfBrain_SRC})
target_link_libraries(FindCenterOfBrain
  ${ITK_LIBRARIES}
)

if(NOT BRAINS_BUILD)
  add_executable(ImageCompare ImageCompare.cxx)
  target_link_libraries(ImageCompare ${ITK_LIBRARIES} )
endif(NOT BRAINS_BUILD)

set(FINDCENTER_PROG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FindCenterOfBrain)

add_test(RudimentaryFindCenterOfBrainTest tclsh ${CMAKE_CURRENT_SOURCE_DIR}}/TestData/FindCenterOfBrainFilter/Test/TestFCOB.tcl
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  ${BRAINSFit_SOURCE_DIR}/TestData
  ${BRAINSFit_BINARY_DIR}/Testing
  ${BRAINSFit_SOURCE_DIR}/TestData/ANON0006_20_T1_dbg_splayed.nii.gz
)
