#message(WARNING "warning: Module ${MODULE_NAME} doesn't have any test !")

#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()
if(NOT DEFINED TEMP)
  set(TEMP ${PROJECT_BINARY_DIR}/Testing/Temporary)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(Generate${CLP}TestData Generate${CLP}TestData.cxx)
target_link_libraries(Generate${CLP}TestData
  ${ITK_LIBRARIES}
  ITKFactoryRegistration
  )
set_target_properties(Generate${CLP}TestData PROPERTIES LABELS ${CLP})
set_target_properties(Generate${CLP}TestData PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(frequency 10)

set(testname ${CLP}TestDataFixed)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}Fixed.mha
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataRigidArgs)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}RigidArgs.mha -p 2 4 6 -o 0.02 0.04 0.06
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataRigid)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}Rigid.mha -t ${INPUT}/RigidUNC24.tfm
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataAffine)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}Affine.mha -t ${INPUT}/AffineUNC24.tfm
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}TestDataBSpline)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:Generate${CLP}TestData>
  ${frequency} ${TEMP}/${CLP}BSpline.mha -t ${INPUT}/BSplineUNC24.tfm
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
