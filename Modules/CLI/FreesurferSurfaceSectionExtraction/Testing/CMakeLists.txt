
#-----------------------------------------------------------------------------
#set(CLP ${MODULE_NAME})

#ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
#add_dependencies(${CLP}Test ${CLP})
#target_link_libraries(${CLP}Test ${ITK_LIBRARIES} ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
#set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
#set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

#set(testname ${CLP}Test)
#ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
#  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
#  FreesurferSurfaceSectionExtractionTest --help
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
