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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerIconEnginePlugin_h
#define __qSlicerIconEnginePlugin_h

// Qt includes
#include <ctkIconEnginePlugin.h>

// Slicer includes
#include "qSlicerIconEnginePluginExport.h"

/// Customized ctkIconEnginePlugin to look for icons in specific directories.
/// By default it will look for icons in the Small/Medium/Large and XLarge
/// directories.
/// To be working, each icon resolution must be added into each
/// subdirectory (Resources/Icons/Small, Resources/Icons/Medium...) using the
/// same file name. All files must be referenced in the resource file (.qrc).
/// In the code, only refer to one icon resolution.
/// Example:
/// \code
/// QIcon icon(":Medium/AnIcon.png");
/// \endcode
/// When the icon is asked for a pixmap at a given resolution, automatically,
/// the icon engine picks the best icon file for the requested size.
/// \code
/// icon.pixmap(128, 128);
/// \endcode
/// will use the icon in the XLarge subdirectory instead of the icon in the
/// Medium subdirectory.
class Q_SLICER_BASE_QTGUI_ICON_ENGINE_EXPORT qSlicerIconEnginePlugin: public ctkIconEnginePlugin
{
  Q_OBJECT
public:
  qSlicerIconEnginePlugin(QObject* parent = nullptr);
};

#endif
