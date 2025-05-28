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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by the Center for Intelligent Image-guided Interventions (CI3).

==============================================================================*/

#ifndef __qSlicerMarkupsMeasurementsWidgetPlugin_h
#define __qSlicerMarkupsMeasurementsWidgetPlugin_h

#include "qSlicerMarkupsModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_MARKUPS_WIDGETS_PLUGINS_EXPORT
qSlicerMarkupsMeasurementsWidgetPlugin
  : public QObject, public qSlicerMarkupsModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerMarkupsMeasurementsWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString domXml() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;

};

#endif
