#
# Slicer3 specific FindPackage and Include call if outside of Slicer3's source-tree
#

  if(NOT Slicer3_SOURCE_DIR)
    # we are not in the Slicer3 Source Dir, we have to find Slicer3
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  endif(NOT Slicer3_SOURCE_DIR)
  
  SET(EM_SHARE_DIR ${Slicer3_INSTALL_MODULES_SHARE_DIR})
  SET(EM_INSTALL_SHARE_DIR ${Slicer3_INSTALL_MODULES_SHARE_DIR})
