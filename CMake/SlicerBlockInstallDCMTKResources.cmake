# -------------------------------------------------------------------------
# Find and install DCMTK private dictionary
# -------------------------------------------------------------------------

find_package(DCMTK REQUIRED)
message('Installing private.dic from ${DCMTK_SOURCE_DIR}')
install(FILES ${DCMTK_SOURCE_DIR}/dcmdata/data/private.dic
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
