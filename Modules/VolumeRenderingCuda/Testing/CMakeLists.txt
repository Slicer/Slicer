
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
#  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/VolumeRenderingCudaTest ${CMAKE_CURRENT_SOURCE_DIR}/heart256.raw)
