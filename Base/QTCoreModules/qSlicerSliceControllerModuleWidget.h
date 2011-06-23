/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSliceControllerModuleWidget_h
#define __qSlicerSliceControllerModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerSliceControllerModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerSliceControllerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSliceControllerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSliceControllerModuleWidget();

public slots:
  virtual void setMRMLScene(vtkMRMLScene *newScene);
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onLayoutChanged(int);

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerSliceControllerModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSliceControllerModuleWidget);
  Q_DISABLE_COPY(qSlicerSliceControllerModuleWidget);
};

#endif
