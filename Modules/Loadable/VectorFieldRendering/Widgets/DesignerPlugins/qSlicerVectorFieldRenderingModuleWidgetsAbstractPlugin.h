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

#ifndef __qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin_h
#define __qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin_h

#include <QtGlobal>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

#include "qSlicerVectorFieldRenderingModuleWidgetsPluginsExport.h"

class Q_SLICER_QTMODULES_VECTORFIELDRENDERING_WIDGETS_PLUGINS_EXPORT qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin : public QDesignerCustomWidgetInterface
{
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
  Q_INTERFACES(QDesignerCustomWidgetInterface);

public:
  qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin();
  // Don't reimplement this method.
  QString group() const override;
  // You can reimplement these methods
  QIcon icon() const override;
  QString toolTip() const override;
  QString whatsThis() const override;
};

#endif
