################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#SLicerMacroTranslation

#Parameters :
#
#   SRCS ..................: All sources witch need to be translated
#
#   UI_SRCS ...............: All ui_sources witch need to be translated
#
#   TS_DIR.................: Path to the TS files
#
#   TS_BASEFILENAME........: Name of the librairi
#
#   TS_LANGUAGES...........: Variable with all the languages
#

function(SlicerMacroTranslation)
  set(options)
  set(oneValueArgs TS_DIR TS_BASEFILENAME)
  set(multiValueArgs SRCS UI_SRCS TS_LANGUAGES)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # ---------------------------------------------------------------------------------
  # Sanity Checks
  # ---------------------------------------------------------------------------------
  set(expected_nonempty_vars SRCS TS_DIR TS_BASEFILENAME )
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error : ${var} Cmake varianle is empty !")
    endif()
  endforeach()

  # ---------------------------------------------------------------------------------
  # Set FILES_TO_TRANSLATE
  # ---------------------------------------------------------------------------------
  set(FILES_TO_TRANSLATE )
  foreach(file IN LISTS MY_SRCS MY_UI_SRCS)
    if(NOT IS_ABSOLUTE ${file})
      set(file ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    endif()
    list(APPEND FILES_TO_TRANSLATE ${file})
  endforeach()

  # ---------------------------------------------------------------------------------
  # Set TS_FILES
  # ---------------------------------------------------------------------------------
  set(TS_FILES)
  foreach(language ${MY_TS_LANGUAGES})
    set(ts_file "${MY_TS_DIR}/${MY_TS_BASEFILENAME}_${language}.ts")

    if(NOT Slicer_UPDATE_TRANSLATION AND NOT EXISTS ${ts_file})
      continue()
    endif()

    list(APPEND TS_FILES ${ts_file})
  endforeach()

  # ---------------------------------------------------------------------------------
  # Set properties used in SlicerFunctionAddGenerateSlicerTranslationQMFilesTarget
  # ---------------------------------------------------------------------------------
  set_property(GLOBAL APPEND PROPERTY Slicer_TS_BASEFILENAMES ${MY_TS_BASEFILENAME})
  set_property(GLOBAL PROPERTY Slicer_${MY_TS_BASEFILENAME}_FILES_TO_TRANSLATE "${FILES_TO_TRANSLATE}")
  set_property(GLOBAL PROPERTY Slicer_${MY_TS_BASEFILENAME}_TS_FILES "${TS_FILES}")
  set_property(GLOBAL PROPERTY Slicer_${MY_TS_BASEFILENAME}_QM_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

endfunction()


function(SlicerFunctionAddGenerateSlicerTranslationQMFilesTarget)
  set(options)
  set(oneValueArgs )
  set(multiValueArgs QM_OUTPUT_DIRS_VAR)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # ---------------------------------------------------------------------------------
  # Sanity Checks
  # ---------------------------------------------------------------------------------
  set(expected_nonempty_vars QM_OUTPUT_DIRS_VAR)
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: CMake variable ${var} is empty !")
    endif()
  endforeach()


  # ---------------------------------------------------------------------------------
  set(qm_output_dirs)

  # ---------------------------------------------------------------------------------
  # UPDATE or ADD translation
  # ---------------------------------------------------------------------------------
  get_property(ts_basefilenames GLOBAL PROPERTY Slicer_TS_BASEFILENAMES)
  foreach(ts_basefilename IN LISTS ts_basefilenames)
    get_property(files_to_translate GLOBAL PROPERTY Slicer_${ts_basefilename}_FILES_TO_TRANSLATE)
    get_property(ts_files GLOBAL PROPERTY Slicer_${ts_basefilename}_TS_FILES)
    get_property(qm_output_dir GLOBAL PROPERTY Slicer_${ts_basefilename}_QM_OUTPUT_DIR)

    # Keep track of cpp source files and python scripts independently
    set(srcs_to_translate)
    set(scripts_to_translate)
    foreach(file IN LISTS files_to_translate)
      get_filename_component(file_ext ${file} EXT)
      if("${file_ext}" MATCHES "py")
        list(APPEND scripts_to_translate ${file})
      else()
        list(APPEND srcs_to_translate ${file})
      endif()
    endforeach()

    # Add custom commands for rewriting python scripts
    set(rewritten_scripts_to_translate_for_context)
    if(Slicer_USE_PYTHONQT)
      foreach(script IN LISTS scripts_to_translate)
        set(rewritten_src_file "${script}.tr")
        set(rewritten_src "${qm_output_dir}/${rewritten_src_file}")

        # TODO: Add custom command for generating .py.tr files into binary directory.
        #       See https://github.com/Slicer/Slicer/pull/6050

        # add_custom_command(DEPENDS ${script}
        #                    COMMAND ... -i ${script} -o ${rewritten_src}
        #                    OUTPUT ${rewritten_src}
        #                    COMMENT "Generating .py.tr file into binary directory: ${rewritten_src_file}")

        # list(APPEND rewritten_scripts_to_translate_for_context ${rewritten_src})
      endforeach()
    endif()

    set_source_files_properties(${ts_files} PROPERTIES OUTPUT_LOCATION ${qm_output_dir})

    if(Slicer_UPDATE_TRANSLATION)
      # Add custom command for creating or updating "*_<lang>.ts" translation files, and for
      # generating the associated "*_<lang>.qm" files.
      QT5_CREATE_TRANSLATION(
        QM_OUTPUT_FILES ${srcs_to_translate} ${rewritten_scripts_to_translate_for_context} ${ts_files}
        OPTIONS -source-language en_US -no-ui-lines
        )
    else()
      # Find existing TS files and only add translation if at least one translation file exist to avoid error
      # (Case may exist if Slicer_UPDATE_TRANSLATION is disabled and translation files were never
      # generated for the input language)
      set(EXISTING_TS_FILES)
      foreach(TS_FILE ${TS_FILES})
        if(EXISTS ${TS_FILE})
          list(APPEND EXISTING_TS_FILES ${TS_FILE})
        endif()
      endforeach()

      if(EXISTING_TS_FILES)
        # Add custom command for generating the associated "*_<lang>.qm" files.
        QT5_ADD_TRANSLATION(QM_OUTPUT_FILES ${EXISTING_TS_FILES})
      endif()
    endif()

    list(APPEND qm_output_dirs ${qm_output_dir})
  endforeach()

  # ---------------------------------------------------------------------------------
  # Targets
  # ---------------------------------------------------------------------------------

  add_custom_target(GenerateSlicerTranslationQMFiles ALL DEPENDS
    ${QM_OUTPUT_FILES}
    )

  # ---------------------------------------------------------------------------------
  # Install language
  # ---------------------------------------------------------------------------------
  install(
    FILES ${QM_OUTPUT_FILES}
    DESTINATION ${Slicer_INSTALL_QM_DIR}
    COMPONENT Runtime
    )

  # ---------------------------------------------------------------------------------
  # Output variables
  # ---------------------------------------------------------------------------------
  set(${MY_QM_OUTPUT_DIRS_VAR} ${qm_output_dirs} PARENT_SCOPE)

endfunction()
