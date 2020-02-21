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

macro(SlicerMacroConfigureModuleCxxTestDriver)
  set(options
    WITH_VTK_DEBUG_LEAKS_CHECK
    WITH_VTK_ERROR_OUTPUT_CHECK
    )
  set(oneValueArgs
    NAME
    TESTS_TO_RUN_VAR
    FOLDER
    )
  set(multiValueArgs
    SOURCES
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    )
  cmake_parse_arguments(SLICER_TEST_DRIVER
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  set(expected_defined_vars NAME)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED SLICER_TEST_DRIVER_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  if(SLICER_TEST_DRIVER_SOURCES)

    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "")
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "")

    set(EXTRA_INCLUDE "vtkWin32OutputWindow.h")

    if(SLICER_TEST_DRIVER_WITH_VTK_ERROR_OUTPUT_CHECK)
      set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN
        "${CMAKE_TESTDRIVER_BEFORE_TESTMAIN}\nTESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);")
      set(CMAKE_TESTDRIVER_AFTER_TESTMAIN
        "${CMAKE_TESTDRIVER_AFTER_TESTMAIN}\nTESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);")
      set(EXTRA_INCLUDE "${EXTRA_INCLUDE}\"\n\#include \"vtkTestingOutputWindow.h")
    else()
    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN
      "${CMAKE_TESTDRIVER_BEFORE_TESTMAIN}\n// Direct VTK messages to standard output
      #ifdef WIN32
        vtkWin32OutputWindow* outputWindow =
          vtkWin32OutputWindow::SafeDownCast(vtkOutputWindow::GetInstance());
        if (outputWindow)
          {
          outputWindow->SendToStdErrOn();
          }
      #endif")
    endif()

    if(SLICER_TEST_DRIVER_WITH_VTK_DEBUG_LEAKS_CHECK)
      set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN
        "${CMAKE_TESTDRIVER_BEFORE_TESTMAIN}\nDEBUG_LEAKS_ENABLE_EXIT_ERROR();")
      set(EXTRA_INCLUDE "${EXTRA_INCLUDE}\"\n\#include \"vtkMRMLDebugLeaksMacro.h")
    endif()

    if(SLICER_TEST_DRIVER_INCLUDE_DIRECTORIES)
      include_directories(${SLICER_TEST_DRIVER_INCLUDE_DIRECTORIES})
    endif()

    create_test_sourcelist(Tests ${SLICER_TEST_DRIVER_NAME}CxxTests.cxx
      ${SLICER_TEST_DRIVER_SOURCES}
      EXTRA_INCLUDE ${EXTRA_INCLUDE}
      )

    set(TestsToRun ${Tests})
    list(REMOVE_ITEM TestsToRun ${SLICER_TEST_DRIVER_NAME}CxxTests.cxx)

    if(SLICER_TEST_DRIVER_TESTS_TO_RUN_VAR)
      set(${SLICER_TEST_DRIVER_TESTS_TO_RUN_VAR} ${TestsToRun})
    endif()

    ctk_add_executable_utf8(${SLICER_TEST_DRIVER_NAME}CxxTests ${Tests})
    set_target_properties(${SLICER_TEST_DRIVER_NAME}CxxTests
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR}
      )
    target_link_libraries(${SLICER_TEST_DRIVER_NAME}CxxTests
      ${SLICER_TEST_DRIVER_NAME}
      ${SLICER_TEST_DRIVER_TARGET_LIBRARIES}
      )
    if(NOT DEFINED SLICER_TEST_DRIVER_FOLDER AND DEFINED MODULE_NAME)
      set(SLICER_TEST_DRIVER_FOLDER "Module-${MODULE_NAME}")
    endif()
    if(NOT "${SLICER_TEST_DRIVER_FOLDER}" STREQUAL "")
      set_target_properties(${SLICER_TEST_DRIVER_NAME}CxxTests
        PROPERTIES FOLDER ${SLICER_TEST_DRIVER_FOLDER})
    endif()
  endif()

endmacro()
