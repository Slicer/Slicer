# -------------------------------------------------------------------------
# Find and install DCMTK private dictionary
# -------------------------------------------------------------------------

find_package(DCMTK REQUIRED)
if(NOT EXISTS ${DCM_DICT_PATH})
  message(FATAL_ERROR "error: '${DCM_DICT_PATH}' does not exist !")
endif()
install(FILES ${DCM_DICT_PATH} DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Runtime)
