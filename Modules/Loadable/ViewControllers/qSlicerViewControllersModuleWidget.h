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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerViewControllersModuleWidget_h
#define __qSlicerViewControllersModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// View Controllers includes
#include "qSlicerViewControllersModuleExport.h"

class qSlicerViewControllersModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_VIEWCONTROLLERS_EXPORT qSlicerViewControllersModuleWidget
  : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerViewControllersModuleWidget(QWidget *parent=nullptr);
  ~qSlicerViewControllersModuleWidget() override;

public slots:
  void setMRMLScene(vtkMRMLScene *newScene) override;
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onLayoutChanged(int);

protected slots:
  void onAdvancedViewNodeChanged(vtkMRMLNode*);

protected:
  void setup() override;

protected:
  QScopedPointer<qSlicerViewControllersModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerViewControllersModuleWidget);
  Q_DISABLE_COPY(qSlicerViewControllersModuleWidget);
};

#endif
