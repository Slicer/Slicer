################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
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

if(NOT DEFINED proj)
  message(FATAL_ERROR "'proj' variable is NOT defined !")
endif()
if(NOT DEFINED ${proj}_DEPENDENCIES)
  message(FATAL_ERROR "'${proj}_DEPENDENCIES' variable is NOT defined !")
endif()
foreach(dep ${${proj}_DEPENDENCIES})
  if(NOT ${dep}_EXTERNAL_PROJECT_INCLUDED)
    message(FATAL_ERROR "External project ${proj} depends on ${dep}. Make sure External_${dep}.cmake is included before External_${proj}.cmake !")
  endif()
endforeach()
