# -------------------------------------------------------------------------
# Find and install DCMTK private dictionary
# -------------------------------------------------------------------------

find_package(DCMTK REQUIRED)
if(NOT EXISTS ${DCM_STANDARD_DICT_PATH})
  message(FATAL_ERROR "error: '${DCM_STANDARD_DICT_PATH}' does not exist !")
endif()
if(NOT EXISTS ${DCM_PRIVATE_DICT_PATH})
  message(FATAL_ERROR "error: '${DCM_PRIVATE_DICT_PATH}' does not exist !")
endif()
install(FILES ${DCM_STANDARD_DICT_PATH} ${DCM_PRIVATE_DICT_PATH} DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Runtime)
