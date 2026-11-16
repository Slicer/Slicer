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

#ifndef __qMRMLVectorFieldDisplayWidgetPlugin_h
#define __qMRMLVectorFieldDisplayWidgetPlugin_h

#include "qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin.h"

class Q_SLICER_QTMODULES_VECTORFIELDRENDERING_WIDGETS_PLUGINS_EXPORT qMRMLVectorFieldDisplayWidgetPlugin
  : public QObject
  , public qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLVectorFieldDisplayWidgetPlugin(QObject* newParent = nullptr);

  QWidget* createWidget(QWidget* newParent) override;
  QString domXml() const override;
  QIcon icon() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;
};

#endif
