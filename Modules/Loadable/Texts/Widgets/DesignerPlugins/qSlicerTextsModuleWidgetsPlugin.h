/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef qSlicerTextsModuleWidgetsPlugin_h
#define qSlicerTextsModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// Texts includes
#include "qMRMLTextWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_MODULE_TEXTS_WIDGETS_PLUGINS_EXPORT qSlicerTextModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  /// Return designer plugins created by the module
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
  {
    QList<QDesignerCustomWidgetInterface*> plugins;
    plugins << new qMRMLTextWidgetPlugin;

    return plugins;
  }
};

#endif
