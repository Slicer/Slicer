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
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR}/BRAINSFit_Common)
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR}/LargestForegroundFilledMaskImageFilter)
INCLUDE_DIRECTORIES(${BRAINSFit_SOURCE_DIR}/FindCenterOfBrainFilter)

ENABLE_TESTING()
include(Dart)

SET(FindCenterOfBrain_SRC
FindCenterOfBrainCLP.cxx)

generateclp(FindCenterOfBrain_SRC FindCenterOfBrain.xml)

ADD_EXECUTABLE(FindCenterOfBrain ${FindCenterOfBrain_SRC})
TARGET_LINK_LIBRARIES(FindCenterOfBrain
  ${ITK_LIBRARIES}
)

IF(NOT BRAINS_BUILD)
  ADD_EXECUTABLE(ImageCompare ImageCompare.cxx)
  TARGET_LINK_LIBRARIES(ImageCompare  ${ITK_LIBRARIES})
ENDIF(NOT BRAINS_BUILD)

set(FINDCENTER_PROG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FindCenterOfBrain)

ADD_TEST(RudimentaryFindCenterOfBrainTest tclsh ${CMAKE_CURRENT_SOURCE_DIR}}/TestData/FindCenterOfBrainFilter/Test/TestFCOB.tcl
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  ${BRAINSFit_SOURCE_DIR}/TestData
  ${BRAINSFit_BINARY_DIR}/Testing
  ${BRAINSFit_SOURCE_DIR}/TestData/ANON0006_20_T1_dbg_splayed.nii.gz
)
