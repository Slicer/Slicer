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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was funded the Applied Cancer Research Unit program of Cancer Care Ontario
  with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerDataModuleWidgetsPlugin_h
#define __qSlicerDataModuleWidgetsPlugin_h

// Qt includes
#include <QDesignerCustomWidgetCollectionInterface>

// Data includes
#include "qMRMLNodeAttributeTableViewPlugin.h"
#include "qMRMLNodeAttributeTableWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_MODULE_DATA_WIDGETS_PLUGINS_EXPORT qSlicerDataModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLNodeAttributeTableViewPlugin;
    plugins << new qMRMLNodeAttributeTableWidgetPlugin;
    return plugins;
    }
};

#endif
