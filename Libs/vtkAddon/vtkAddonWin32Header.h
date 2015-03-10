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

==============================================================================*/ 

///  vtkAddonWin32Header - manage Windows system differences
///
/// The vtkAddonWin32Header captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkAddonWin32Header_h
#define __vtkAddonWin32Header_h

#include <vtkAddonConfigure.h>

#if defined(WIN32) && !defined(VTKADDON_STATIC)
#if defined(vtkAddon_EXPORTS)
#define VTK_ADDON_EXPORT __declspec( dllexport )
#else
#define VTK_ADDON_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_ADDON_EXPORT
#endif

#endif
