################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
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

macro(slicerMacroBuildScriptedModule)
  set(options
    WITH_GENERIC_TESTS
    WITH_SUBDIR
    VERBOSE
    )
  set(oneValueArgs
    NAME
    )
  set(multiValueArgs
    SCRIPTS
    RESOURCES
    )
  cmake_parse_arguments(MY_SLICER
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  message(STATUS "Configuring Scripted module: ${MY_SLICER_NAME}")

  # --------------------------------------------------------------------------
  # Print information helpful for debugging checks
  # --------------------------------------------------------------------------
  if(MY_SLICER_VERBOSE)
    list(APPEND ALL_OPTIONS ${options} ${oneValueArgs} ${multiValueArgs})
    foreach(curr_opt ${ALL_OPTIONS})
      message(STATUS "${curr_opt} = ${MY_SLICER_${curr_opt}}")
    endforeach()
  endif()

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(MY_SLICER_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerMacroBuildScriptedModule(): \"${MY_SLICER_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT DEFINED MY_SLICER_NAME)
    message(FATAL_ERROR "NAME is mandatory")
  endif()

  set(expected_existing_vars SCRIPTS RESOURCES)
  foreach(var ${expected_existing_vars})
    foreach(value ${MY_SLICER_${var}})
      if(NOT IS_ABSOLUTE ${value})
        set(value_absolute ${CMAKE_CURRENT_SOURCE_DIR}/${value})
      else()
        set(value_absolute ${value})
      endif()
      if(NOT EXISTS ${value_absolute} AND NOT EXISTS ${value_absolute}.py)
        if(NOT IS_ABSOLUTE ${value})
          set(value_absolute ${CMAKE_CURRENT_BINARY_DIR}/${value})
        endif()
        get_source_file_property(is_generated ${value_absolute} GENERATED)
        if(NOT is_generated)
          message(FATAL_ERROR
            "slicerMacroBuildScriptedModule(${var}) given nonexistent"
            " file or directory '${value}'")
        endif()
      endif()
    endforeach()
  endforeach()

  if(NOT Slicer_USE_PYTHONQT)
      message(FATAL_ERROR
        "Attempting to build the Python scripted module '${MY_SLICER_NAME}'"
        " when Slicer_USE_PYTHONQT is OFF")
  endif()

  set(_no_install_subdir_option NO_INSTALL_SUBDIR)
  set(_destination_subdir "")
  if(MY_SLICER_WITH_SUBDIR)
    get_filename_component(_destination_subdir ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(_destination_subdir "/${_destination_subdir}")
    set(_no_install_subdir_option "")
  endif()

  # --------------------------------------------------------------------------
  # Copy and/or compile scripts and associated resources
  # --------------------------------------------------------------------------
  ctkMacroCompilePythonScript(
    TARGET_NAME ${MY_SLICER_NAME}
    SCRIPTS "${MY_SLICER_SCRIPTS}"
    RESOURCES "${MY_SLICER_RESOURCES}"
    DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}${_destination_subdir}
    INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
    ${_no_install_subdir_option}
    )

  # --------------------------------------------------------------------------
  # Translations
  # --------------------------------------------------------------------------
  if("${CTK_COMPILE_PYTHON_SCRIPTS_GLOBAL_TARGET_NAME}" STREQUAL "")
    SlicerFunctionAddPythonScriptTrFilesTargets(${MY_SLICER_NAME})
  endif()

  # --------------------------------------------------------------------------
  # Tests
  # --------------------------------------------------------------------------
  if(BUILD_TESTING AND MY_SLICER_WITH_GENERIC_TESTS)
    set(_generic_unitest_scripts)
    SlicerMacroConfigureGenericPythonModuleTests("${MY_SLICER_NAME}" _generic_unitest_scripts)

    foreach(script_name ${_generic_unitest_scripts})
      slicer_add_python_unittest(
        SCRIPT ${script_name}
        SLICER_ARGS --no-main-window --disable-cli-modules
                    --additional-module-path ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
        TESTNAME_PREFIX nomainwindow_
        )
    endforeach()
  endif()

endmacro()

function(SlicerFunctionAddPythonScriptTrFilesTargets target)

  set(rewrite_script "${Slicer_CMAKE_DIR}/RewriteTr.py")
  if(NOT EXISTS ${rewrite_script})
    message(FATAL_ERROR "Rewrite script does not exist [${rewrite_script}]")
  endif()

  set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Translations/")

  #get_property(Slicer_LANGUAGES GLOBAL PROPERTY Slicer_LANGUAGES)

  get_property(_CTK_${target}_PYTHON_SCRIPTS GLOBAL PROPERTY _CTK_${target}_PYTHON_SCRIPTS)

  set(rewritten_srcs)
  foreach(entry IN LISTS _CTK_${target}_PYTHON_SCRIPTS)
    string(REPLACE "|" ";" tuple "${entry}")
    list(GET tuple 0 src)
    list(GET tuple 1 tgt_file)
    list(GET tuple 2 dest_dir)

    set(rewritten_src_file "${tgt_file}.tr")
    set(rewritten_src "${dest_dir}/${rewritten_src_file}")

    add_custom_command(DEPENDS ${src}
                       COMMAND ${PYTHON_EXECUTABLE}
                         ${rewrite_script} -i ${src} -o ${rewritten_src}
                       OUTPUT ${rewritten_src}
                       COMMENT "Generating .py.tr file into binary directory: ${rewritten_src_file}")

    list(APPEND rewritten_srcs ${rewritten_src})
  endforeach()

  include(SlicerMacroTranslation)
  SlicerMacroTranslation(
    SRCS ${rewritten_srcs}
    TS_DIR ${TS_DIR}
    TS_BASEFILENAME ${target}
    TS_LANGUAGES ${Slicer_LANGUAGES}
    QM_OUTPUT_DIR_VAR QM_OUTPUT_DIR
    QM_OUTPUT_FILES_VAR QM_OUTPUT_FILES
    )

  # store the paths where the qm files are located
  set_property(GLOBAL APPEND PROPERTY Slicer_QM_OUTPUT_DIRS ${QM_OUTPUT_DIR})

  # store the qm files associated with scripted modules
  set_property(GLOBAL APPEND PROPERTY QM_SCRIPTED_MODULE_FILES ${QM_OUTPUT_FILES})

  set(target_name Add${target}PythonScriptTrFiles)
  if(NOT TARGET ${target_name})
    add_custom_target(${target_name} DEPENDS ${rewritten_srcs} ${ARGN})
  endif()
endfunction()
