
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP BlobDetection)

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/BlobDetectionTest.nrrd
            ${TEMP}/BlobDetectionTest.nrrd
  ModuleEntryPoint
   --scale 0.617
   --steps 0
   --detectBrightObjects true
   --alpha 0.5
   --beta 0.5
   --gamma 5.0
   ${BASELINE}/BlobDetection.nrrd
   ${TEMP}/BlobDetectionTest.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
