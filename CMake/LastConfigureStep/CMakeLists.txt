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

# The commands in this directory are intended to be executed as
# the end of the whole configuration process, as a "last step".
# This directory is typically the last SUBDIRS in the main CMakeLists.txt.
# It enable the above commands to use variables that might have been configured
# in previous SUBDIRS. This is especially important when it comes to
# the CONFIGURE_FILE command, since in IMMEDIATE mode that command will
# use the current values of CMake variables instead of waiting until the
# end of CMakeLists processing, i.e. instead of waiting until some variables
# are configured in SUBDIRS.

#-----------------------------------------------------------------------------
get_property(Slicer_TARGETS GLOBAL PROPERTY Slicer_TARGETS)
export(TARGETS ${Slicer_TARGETS} APPEND FILE ${Slicer_BINARY_DIR}/SlicerTargets.cmake)

#-----------------------------------------------------------------------------
# Create the SlicerConfig.cmake file containing the Slicer configuration.
# Since it might generate configuration file depending
include(${Slicer_SOURCE_DIR}/CMake/SlicerGenerateSlicerConfig.cmake)

#-----------------------------------------------------------------------------
include(SlicerPackageAndUploadTarget)

#-----------------------------------------------------------------------------
include(${Slicer_SOURCE_DIR}/CMake/SlicerCPack.cmake)
