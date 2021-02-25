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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qMRMLMarkupsInteractionHandleWidgetPlugin_h
#define __qMRMLMarkupsInteractionHandleWidgetPlugin_h

#include "qSlicerMarkupsModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_MARKUPS_WIDGETS_PLUGINS_EXPORT qMRMLMarkupsInteractionHandleWidgetPlugin
    : public QObject, public qSlicerMarkupsModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLMarkupsInteractionHandleWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
