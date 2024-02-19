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
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerMarkupsModuleWidgetsPlugin_h
#define __qSlicerMarkupsModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// Markups includes
#include "qMRMLMarkupsDisplayNodeWidgetPlugin.h"
#include "qMRMLMarkupsFiducialProjectionPropertyWidgetPlugin.h"
#include "qMRMLMarkupsInteractionHandleWidgetPlugin.h"
#include "qMRMLMarkupsROIWidgetPlugin.h"
#include "qSlicerMarkupsPlaceWidgetPlugin.h"
#include "qSlicerSimpleMarkupsWidgetPlugin.h"
#include "qSlicerMarkupsMeasurementsWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_MODULE_MARKUPS_WIDGETS_PLUGINS_EXPORT qSlicerMarkupsModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLMarkupsDisplayNodeWidgetPlugin;
    plugins << new qMRMLMarkupsFiducialProjectionPropertyWidgetPlugin;
    plugins << new qMRMLMarkupsInteractionHandleWidgetPlugin;
    plugins << new qMRMLMarkupsROIWidgetPlugin;
    plugins << new qSlicerMarkupsPlaceWidgetPlugin;
    plugins << new qSlicerSimpleMarkupsWidgetPlugin;
    plugins << new qSlicerMarkupsMeasurementsWidgetPlugin;

    return plugins;
    }
};

#endif
