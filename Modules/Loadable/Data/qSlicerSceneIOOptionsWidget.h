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

#ifndef __qSlicerSceneIOOptionsWidget_h
#define __qSlicerSceneIOOptionsWidget_h

// SlicerQt includes
#include "qSlicerIOOptionsWidget.h"

// Cameras includes
#include "qSlicerDataModuleExport.h"

class qSlicerSceneIOOptionsWidgetPrivate;

/// \ingroup Slicer_QtModules_Scene
class Q_SLICER_QTMODULES_DATA_EXPORT qSlicerSceneIOOptionsWidget
  : public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  qSlicerSceneIOOptionsWidget(QWidget *parent=0);
  virtual ~qSlicerSceneIOOptionsWidget();

protected slots:
  void updateProperties();

private:
  Q_DECLARE_PRIVATE(qSlicerSceneIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerSceneIOOptionsWidget);
};

#endif
