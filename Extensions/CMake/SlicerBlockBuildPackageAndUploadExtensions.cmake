
# Sanity checks
set(expected_defined_vars Slicer_DIR Slicer_EXTENSION_DESCRIPTION_DIR Slicer_LOCAL_EXTENSIONS_DIR Slicer_CMAKE_DIR Slicer_WC_REVISION QT_VERSION_MAJOR QT_VERSION_MINOR CMAKE_GENERATOR)
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

INCLUDE(SlicerFunctionExtractExtensionDescription)

IF(Slicer_UPLOAD_EXTENSIONS)
  INCLUDE(SlicerBlockUploadExtensionPrerequisites) # Common to all extensions
ENDIF()

#-----------------------------------------------------------------------------
# Collect extension description file (*.s4ext)
#-----------------------------------------------------------------------------
FILE(GLOB_RECURSE s4extfiles "${Slicer_EXTENSION_DESCRIPTION_DIR}/*.s4ext")

FOREACH(file ${s4extfiles})
  # Extract extension description info
  slicerFunctionExtractExtensionDescription(EXTENSION_FILE ${file} VAR_PREFIX EXTENSION)
  
  #foreach(v SCM SCMURL DEPENDS HOMEPAGE CATEGORY STATUS DESCRIPTION)
  #  message(${v}:${EXTENSION_SEXT_${v}})
  #endforeach()
  
  # Extract file basename
  get_filename_component(EXTENSION_NAME ${file} NAME_WE)
  IF("${EXTENSION_NAME}" STREQUAL "")
    MESSAGE(WARNING "Failed to extract extension name associated with file: ${file}")
  ELSE()
    MESSAGE(STATUS "Configuring extension: ${EXTENSION_NAME} (${file})")
    IF("${EXTENSION_SEXT_SCM}" STREQUAL "" AND "${EXTENSION_SEXT_SCMURL}" STREQUAL "")
      MESSAGE(WARNING "Failed to extract extension information associated file: ${file}")
    ELSE()

      SET(sext_add_project True)
      SET(sext_ep_options_repository)
      SET(sext_ep_option_scm_executable)
      IF(${EXTENSION_SEXT_SCM} STREQUAL "git")
        find_package(Git REQUIRED)
        SET(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME})
        SET(sext_ep_options_repository
          "GIT_REPOSITORY ${EXTENSION_SEXT_SCMURL} GIT_TAG \"origin/master\"")
        SET(sext_ep_option_scm_executable
           -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE})
      ELSEIF(${EXTENSION_SEXT_SCM} STREQUAL "svn")
        find_package(Subversion REQUIRED)
        SET(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME})
        SET(sext_ep_options_repository
          "SVN_REPOSITORY ${EXTENSION_SEXT_SCMURL} SVN_REVISION -r \"HEAD\"")
        SET(sext_ep_option_scm_executable
           -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE})
      ELSEIF(${EXTENSION_SEXT_SCM} STREQUAL "local")
        SET(EXTENSION_SOURCE_DIR ${EXTENSION_SEXT_SCMURL})
        IF(NOT EXISTS ${EXTENSION_SOURCE_DIR})
          SET(EXTENSION_SOURCE_DIR ${Slicer_LOCAL_EXTENSIONS_DIR}/${EXTENSION_SOURCE_DIR})
        ENDIF()
      ELSE()
        SET(sext_add_project False)
        MESSAGE(WARNING "Unknown type of SCM [${EXTENSION_SEXT_SCM}] associated with extension named ${EXTENSION_NAME} - See file ${file}")
      ENDIF()
      IF(sext_add_project)
        # Set external project DEPENDS parameter
        SET(EXTENSION_DEPENDS)
        IF(Slicer_SOURCE_DIR)
          SET(EXTENSION_DEPENDS DEPENDS Slicer)
        ENDIF()
        IF(Slicer_UPLOAD_EXTENSIONS)
          #-----------------------------------------------------------------------------
          # Slicer_UPLOAD_EXTENSIONS: TRUE
          #-----------------------------------------------------------------------------
          SET(EXTENSION_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}-build)
          INCLUDE(SlicerBlockUploadExtension)
          # Add extension external project
          set(proj ${EXTENSION_NAME})
          ExternalProject_Add(${proj}
            DOWNLOAD_COMMAND ""
            ${sext_ep_options_repository}
            SOURCE_DIR ${EXTENSION_SOURCE_DIR}
            BINARY_DIR ${EXTENSION_NAME}-build
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${EXTENSION_UPLOAD_COMMAND}
            INSTALL_COMMAND ""
            ${EXTENSION_DEPENDS}
            )
          # This custom external project step forces the build and later
          # steps to run whenever a top level build is done...
          ExternalProject_Add_Step(${proj} forcebuild
            DEPENDEES configure
            DEPENDERS build
            ALWAYS 1
            )
          IF(Slicer_SOURCE_DIR)
            # Add convenient external project allowing to build the extension
            # independently of Slicer
            ExternalProject_Add(${proj}-rebuild
              DOWNLOAD_COMMAND ""
              ${sext_ep_options_repository}
              SOURCE_DIR ${EXTENSION_SOURCE_DIR}
              BINARY_DIR ${EXTENSION_NAME}-build
              CONFIGURE_COMMAND ""
              BUILD_COMMAND ${EXTENSION_UPLOAD_COMMAND}
              INSTALL_COMMAND ""
              )
            # This custom external project step forces the build and later
            # steps to run whenever a top level build is done...
            ExternalProject_Add_Step(${proj}-rebuild forcebuild
              DEPENDEES configure
              DEPENDERS build
              ALWAYS 1
              )
            SET_PROPERTY(TARGET ${proj}-rebuild PROPERTY EXCLUDE_FROM_ALL TRUE)
          ENDIF()
        ELSE()
          #-----------------------------------------------------------------------------
          # Slicer_UPLOAD_EXTENSIONS: FALSE
          #-----------------------------------------------------------------------------
          # Add extension external project
          set(proj ${EXTENSION_NAME})
          ExternalProject_Add(${proj}
            DOWNLOAD_COMMAND ""
            ${sext_ep_options_repository}
            INSTALL_COMMAND ""
            SOURCE_DIR ${EXTENSION_SOURCE_DIR}
            BINARY_DIR ${EXTENSION_NAME}-build
            CMAKE_GENERATOR ${Slicer_EXTENSION_CMAKE_GENERATOR}
            CMAKE_ARGS
              -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
              -DBUILD_TESTING:BOOL=OFF
              -DSlicer_DIR:PATH=${Slicer_BINARY_DIR}/Slicer-build
              ${sext_ep_option_scm_executable}
            ${EXTENSION_DEPENDS}
            )
          # This custom external project step forces the build and later
          # steps to run whenever a top level build is done...
          ExternalProject_Add_Step(${proj} forcebuild
            DEPENDEES configure
            DEPENDERS build
            ALWAYS 1
            )
          IF(Slicer_SOURCE_DIR)
            # Add convenient external project allowing to build the extension
            # independently of Slicer
            ExternalProject_Add(${proj}-rebuild
              DOWNLOAD_COMMAND ""
              ${sext_ep_options_repository}
              INSTALL_COMMAND ""
              SOURCE_DIR ${EXTENSION_SOURCE_DIR}
              BINARY_DIR ${EXTENSION_NAME}-build
              CMAKE_GENERATOR ${Slicer_EXTENSION_CMAKE_GENERATOR}
              CMAKE_ARGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DBUILD_TESTING:BOOL=OFF
                -DSlicer_DIR:PATH=${Slicer_DIR}
                ${sext_ep_option_scm_executable}
              )
            # This custom external project step forces the build and later
            # steps to run whenever a top level build is done...
            ExternalProject_Add_Step(${proj}-rebuild forcebuild
              DEPENDEES configure
              DEPENDERS build
              ALWAYS 1
              )
            SET_PROPERTY(TARGET ${proj}-rebuild PROPERTY EXCLUDE_FROM_ALL TRUE)
          ENDIF()
        ENDIF()
      ENDIF()
    ENDIF()
  ENDIF()
ENDFOREACH()
