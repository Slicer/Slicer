/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __qMRMLTransformInfoWidgetPlugin_h
#define __qMRMLTransformInfoWidgetPlugin_h

#include "qSlicerTransformsModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_TRANSFORMS_WIDGETS_PLUGINS_EXPORT
qMRMLTransformInfoWidgetPlugin
  : public QObject, public qSlicerTransformsModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLTransformInfoWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
