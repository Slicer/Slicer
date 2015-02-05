
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

foreach(filenum RANGE 1 5)
  configure_file(${TEST_DATA}/ModelMakerTest.mrml
      ${TEMP}/ModelMakerTest${filenum}.mrml
      COPYONLY)
endforeach(filenum)

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --labels 2
    --modelSceneFile ${TEMP}/ModelMakerTest1.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllOneLabelTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest2.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllThreeLabelsTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest3.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllThreeLabelsPadTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest4.mrml\#vtkMRMLModelHierarchyNode1
    --pad
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}StartEndTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --start 1 --end 5
    --modelSceneFile ${TEMP}/ModelMakerTest5.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}LabelsTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --labels 3,1,5
    --modelSceneFile ${TEMP}/ModelMakerTest6.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})



set(testname ${CLP}GenerateAllThreeLabelsHierarchyTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --generateAll
    --debug
    --modelSceneFile ${TEMP}/ModelMakerTest7.mrml\#vtkMRMLModelHierarchyNode1
    --modelHierarchyFile ${MRML_TEST_DATA}/helixMask3Labels.mrml
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
