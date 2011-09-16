#
# Slicer4 specific FindPackage and Include call if outside of Slicer4's source-tree
#

  if(NOT Slicer_SOURCE_DIR)
  
    # we are outside the Slicer4 source-tree but we compile for Slicer4
    
    # in this case we assume Slicer4 is built as an external extension
  
    # extension configuration!
    set(EXTENSION_NAME EMSegment)
  
    set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
    set(EXTENSION_CATEGORY "Segmentation")
    set(EXTENSION_STATUS "Beta")
    set(EXTENSION_DESCRIPTION "This is an example of Qt loadable module built as an extension")
    set(EXTENSION_DEPENDS CLIExtensionTemplate)  
  
    # find slicer
    FIND_PACKAGE(Slicer REQUIRED)
    
    set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} C Flags")
    set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} CXX Flags")    
    
    # include slicer
    INCLUDE(${Slicer_USE_FILE})
    
    set(EXTENSION_LICENSE_FILE ${Slicer_LICENSE_FILE})
    set(EXTENSION_README_FILE ${Slicer_README_FILE})
    
    # required for CDash uploading of the extension
    include(SlicerEnableExtensionTesting)
    
  endif()

  SET(EM_SHARE_DIR ${Slicer_QTLOADABLEMODULES_SHARE_DIR})
  SET(EM_INSTALL_SHARE_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR})  
