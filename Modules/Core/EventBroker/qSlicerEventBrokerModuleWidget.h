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

#ifndef __qSlicerEventBrokerModuleWidget_h
#define __qSlicerEventBrokerModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerModulesCoreExport.h"

class qSlicerEventBrokerModuleWidgetPrivate;

class Q_SLICER_MODULES_CORE_EXPORT qSlicerEventBrokerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerEventBrokerModuleWidget(QWidget *parent=nullptr);
  ~qSlicerEventBrokerModuleWidget() override;

protected slots:
  void onCurrentObjectChanged(vtkObject* );

protected:
  QScopedPointer<qSlicerEventBrokerModuleWidgetPrivate> d_ptr;
  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerEventBrokerModuleWidget);
  Q_DISABLE_COPY(qSlicerEventBrokerModuleWidget);
};

#endif
