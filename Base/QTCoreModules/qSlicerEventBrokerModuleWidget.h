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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEventBrokerModuleWidget_h
#define __qSlicerEventBrokerModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerEventBrokerModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerEventBrokerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerEventBrokerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerEventBrokerModuleWidget();

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerEventBrokerModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEventBrokerModuleWidget);
  Q_DISABLE_COPY(qSlicerEventBrokerModuleWidget);
};

#endif
