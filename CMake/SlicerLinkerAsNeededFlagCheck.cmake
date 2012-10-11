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
#  This file was originally developed by Sankhesh Jhaveri, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# Slicer Linker --as-needed flag check
#
# Check if the linker will resolve symbols of underlinked libraries
#
# This script sets a boolean flag Slicer_LINKER_NO_AS_NEEDED_FLAG_REQUIRED
# to either true or false.


message(STATUS "Checking if --no-as-needed linker flag is required")
set(Slicer_LINKER_NO_AS_NEEDED_FLAG_REQUIRED false)
set(LINK_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake/SlicerLinkerAsNeededFlagCheck)
set(LINK_TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerLinkerAsNeededFlagCheck)
try_compile(Slicer_LINKER_LINKS_UNDERLINKED_LIBS
  ${LINK_TEST_BINARY_DIR}
  ${LINK_TEST_SOURCE_DIR}
  LINK_TEST
  )
if(Slicer_LINKER_LINKS_UNDERLINKED_LIBS)
  message(STATUS "Checking if --no-as-needed linker flag is required - no")
else()
  try_compile(Slicer_LINKER_NO_AS_NEEDED_LINKS_UNDERLINKED_LIBS
    ${LINK_TEST_BINARY_DIR}
    ${LINK_TEST_SOURCE_DIR}
    LINK_TEST_FLAGS
    CMAKE_FLAGS -DCMAKE_EXE_LINKER_FLAGS=-Wl,--no-as-needed
    )
  if(Slicer_LINKER_NO_AS_NEEDED_LINKS_UNDERLINKED_LIBS)
    message(STATUS "Checking if --no-as-needed linker flag is required - yes")
    set(Slicer_LINKER_NO_AS_NEEDED_FLAG_REQUIRED true)
  else()
    message(STATUS "Checking if --no-as-needed linker flag is required - failed.")
    message(WARNING "Could not compile test code."
      "Linker could fail trying to resolve symbols for underlinked libraries."
      "See issue 2321 (http://na-mic.org/Mantis/view.php?id=2321) for more details.")
  endif()
endif()
