/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

#include "vtkSlicerVersionConfigure.h"

// vtkAddon includes
#include <vtkAddonTestingMacros.h>

//-----------------------------------------------------------------------------
int vtkSlicerVersionConfigureTest1(int /*argc*/, char * /*argv*/ [])
{
  // From vtkSlicerVersionConfigure
  CHECK_STRING_DIFFERENT(Slicer_VERSION, "");
  CHECK_STRING_DIFFERENT(Slicer_VERSION_FULL, "");
  CHECK_STRING_DIFFERENT(Slicer_BUILDDATE, "");
  CHECK_STRING_DIFFERENT(Slicer_WC_URL, "");
  CHECK_STRING_DIFFERENT(Slicer_WC_REVISION, "");
  CHECK_STRING_DIFFERENT(Slicer_REVISION, "");

  // From vtkSlicerVersionConfigureInternal
  CHECK_STRING(Slicer_OS_LINUX_NAME, "linux");
  CHECK_STRING(Slicer_OS_MAC_NAME, "macosx");
  CHECK_STRING(Slicer_OS_WIN_NAME, "win");
  CHECK_STRING_DIFFERENT(Slicer_ARCHITECTURE, "");
  CHECK_STRING_DIFFERENT(Slicer_OS, "");

  return EXIT_SUCCESS;
}
