#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

# Module name (usually identique to the directory name)
SET(EXTENSION_NAME EditorExtensionTemplate)

IF(NOT Slicer_SOURCE_DIR)

  SET(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
  SET(EXTENSION_CATEGORY "Examples")
  SET(EXTENSION_STATUS "Beta")
  SET(EXTENSION_DESCRIPTION "This is an example of scripted loadable editor effect built as an extension")
  set(EXTENSION_DEPENDS NA) # Specified as a space separated list or 'NA' if any

  FIND_PACKAGE(Slicer REQUIRED)
  
  # Additional C/CXX flags - Should be defined before including Slicer_USE_FILE
  set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} C Flags")
  set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} CXX Flags")
  
  INCLUDE(${Slicer_USE_FILE})
  
  SET(EXTENSION_LICENSE_FILE ${Slicer_LICENSE_FILE})
  SET(EXTENSION_README_FILE ${Slicer_README_FILE})

  INCLUDE(SlicerEnableExtensionTesting)

ENDIF()

SET(KIT_PYTHON_SCRIPTS
  EditorExtensionTemplate.py
  )
  
SET(KIT_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME EditorExtensionTemplate
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_ROOT}
  )

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

IF(NOT Slicer_SOURCE_DIR)
  INCLUDE(SlicerFunctionGenerateExtensionDescription)
  slicerFunctionGenerateExtensionDescription(
    EXTENSION_NAME ${EXTENSION_NAME}
    EXTENSION_CATEGORY ${EXTENSION_CATEGORY}
    EXTENSION_STATUS ${EXTENSION_STATUS}
    EXTENSION_HOMEPAGE ${EXTENSION_HOMEPAGE}
    EXTENSION_DESCRIPTION ${EXTENSION_DESCRIPTION}
    EXTENSION_DEPENDS ${EXTENSION_DEPENDS}
    DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SLICER_WC_REVISION ${Slicer_WC_REVISION}
    SLICER_WC_ROOT ${Slicer_WC_ROOT}
    )
  INCLUDE(${Slicer_EXTENSION_CPACK})
ENDIF()
