# See http://www.na-mic.org/Bug/view.php?id=3334
# message(WARNING "warning: Module ${MODULE_NAME} doesn't have any test !")

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
