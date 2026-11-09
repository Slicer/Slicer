/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __qSlicerVectorFieldRenderingModuleWidgetsPlugin_h
#define __qSlicerVectorFieldRenderingModuleWidgetsPlugin_h

// Qt includes
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

// VectorFieldRendering includes
#include "qMRMLColorLegendGroupBoxPlugin.h"
#include "qMRMLVectorFieldDisplayWidgetPlugin.h"

// \class Group the plugins in one library
class Q_SLICER_QTMODULES_VECTORFIELDRENDERING_WIDGETS_PLUGINS_EXPORT qSlicerVectorFieldRenderingModuleWidgetsPlugin
  : public QObject
  , public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const override
  {
    QList<QDesignerCustomWidgetInterface*> plugins;
    plugins << new qMRMLColorLegendGroupBoxPlugin << new qMRMLVectorFieldDisplayWidgetPlugin;
    return plugins;
  }
};

#endif
