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

#ifndef __qSlicerPresetComboBoxPlugin_h
#define __qSlicerPresetComboBoxPlugin_h

#include "qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_PLUGINS_EXPORT qSlicerPresetComboBoxPlugin
  : public QObject
  , public qSlicerVolumeRenderingModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerPresetComboBoxPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
