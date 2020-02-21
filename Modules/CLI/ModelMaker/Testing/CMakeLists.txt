
#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

foreach(filenum RANGE 1 5)
  configure_file(${INPUT}/ModelMakerTest.mrml
      ${TEMP}/ModelMakerTest${filenum}.mrml
      COPYONLY)
endforeach()

set(testname ${CLP}Test)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --labels 2
    --modelSceneFile ${TEMP}/ModelMakerTest1.mrml\#vtkMRMLModelHierarchyNode1
    DATA{${INPUT}/helix-roi-lable2.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllOneLabelTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest2.mrml\#vtkMRMLModelHierarchyNode1
    DATA{${INPUT}/helix-roi-lable2.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllThreeLabelsTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest3.mrml\#vtkMRMLModelHierarchyNode1
    DATA{${INPUT}/helixMask3Labels.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllThreeLabelsPadTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest4.mrml\#vtkMRMLModelHierarchyNode1
    --pad
    DATA{${INPUT}/helixMask3Labels.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}StartEndTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --start 1 --end 5
    --modelSceneFile ${TEMP}/ModelMakerTest5.mrml\#vtkMRMLModelHierarchyNode1
    DATA{${INPUT}/helixMask3Labels.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}LabelsTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --labels 3,1,5
    --modelSceneFile ${TEMP}/ModelMakerTest6.mrml\#vtkMRMLModelHierarchyNode1
    DATA{${INPUT}/helixMask3Labels.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})



set(testname ${CLP}GenerateAllThreeLabelsHierarchyTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --debug
    --modelSceneFile ${TEMP}/ModelMakerTest7.mrml\#vtkMRMLModelHierarchyNode1
    --modelHierarchyFile ${INPUT}/helixMask3Labels.mrml
    DATA{${INPUT}/helixMask3Labels.nrrd}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
