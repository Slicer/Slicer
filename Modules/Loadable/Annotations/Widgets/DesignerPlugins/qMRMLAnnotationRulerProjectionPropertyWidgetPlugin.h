/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital, based on a template developed by Jean-Christophe Fillion-Robin,
  Kitware Inc. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886

==============================================================================*/

#ifndef __qMRMLAnnotationRulerProjectionPropertyWidgetPlugin_h
#define __qMRMLAnnotationRulerProjectionPropertyWidgetPlugin_h

#include "qSlicerAnnotationModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_PLUGINS_EXPORT
qMRMLAnnotationRulerProjectionPropertyWidgetPlugin
  : public QObject, public qSlicerAnnotationModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLAnnotationRulerProjectionPropertyWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString domXml() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;

};

#endif
