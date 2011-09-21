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

#ifndef __qSlicerCamerasModuleWidget_h
#define __qSlicerCamerasModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMRMLNode;
class vtkMRMLViewNode;
class qSlicerCamerasModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCamerasModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCamerasModuleWidget();

public slots:
  /// 
  /// Inherited from qSlicerWidget. Reimplemented for refresh issues.
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  virtual void setup();
  void synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode);

protected slots:
  void onCurrentViewNodeChanged(vtkMRMLNode*);
  void setCameraToCurrentView(vtkMRMLNode*);
  void onCameraNodeAdded(vtkMRMLNode*);
  void onCameraNodeRemoved(vtkMRMLNode*);
  void synchronizeCameraWithView();

protected:
  QScopedPointer<qSlicerCamerasModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCamerasModuleWidget);
  Q_DISABLE_COPY(qSlicerCamerasModuleWidget);
};

#endif
