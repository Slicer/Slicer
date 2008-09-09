
ADD_EXECUTABLE(VolumeRenderingCudaTest
  main.cxx
  ntkTimer.cxx
  ntkTimer.h
)

#ADD_EXECUTABLE(ConvertToUnsignedCharArray
#  ConvertToUnsignedChar.cxx
#  )


#TARGET_LINK_LIBRARIES(VolumeRenderingCudaTest
#)

#ADD_TEST(VolumeRenderingCudaTest
#  ${EXECUTABLE_OUTPUT_PATH}/VolumeRenderingCudaTest ${CMAKE_CURRENT_SOURCE_DIR}/heart256.raw)
