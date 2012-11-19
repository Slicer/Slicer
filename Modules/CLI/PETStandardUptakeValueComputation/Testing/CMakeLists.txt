
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx ../itkDCMTKFileReader.cxx )
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK
  ModuleDescriptionParser
  SlicerBaseCLI
  MRMLCore
  ${DCMTK_LIBRARIES}
  ${ITK_LIBRARIES}
  )
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ${CLP}Test
  --xml
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
