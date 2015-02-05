#message(WARNING "warning: Module ${MODULE_NAME} doesn't have any test !")

#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(CLP ${MODULE_NAME})
if(NOT DEFINED TEMP)
  set(TEMP ${PROJECT_BINARY_DIR}/Testing/Temporary)
endif()

#-----------------------------------------------------------------------------
add_executable(Generate${CLP}TestData Generate${CLP}TestData.cxx)
target_link_libraries(Generate${CLP}TestData
  ${ITK_LIBRARIES}
  ITKFactoryRegistration
  )
set_target_properties(Generate${CLP}TestData PROPERTIES LABELS ${CLP})
set_target_properties(Generate${CLP}TestData PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(frequency 10)

set(testname ${CLP}TestDataFixed)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}Fixed.mha
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataRigidArgs)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}RigidArgs.mha -p 2 4 6 -o 0.02 0.04 0.06
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataRigid)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}Rigid.mha -t ${INPUT}/RigidUNC24.tfm
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataAffine)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}Affine.mha -t ${INPUT}/AffineUNC24.tfm
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataBSpline)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}BSpline.mha -t ${INPUT}/BSplineUNC24.tfm
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
