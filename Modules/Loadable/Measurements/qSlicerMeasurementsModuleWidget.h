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

#ifndef __qSlicerMeasurementsModuleWidget_h
#define __qSlicerMeasurementsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsModuleWidgetPrivate;

/// \ingroup Slicer_QtModules_Measurements
class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMeasurementsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerMeasurementsModuleWidget();

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerMeasurementsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMeasurementsModuleWidget);
  Q_DISABLE_COPY(qSlicerMeasurementsModuleWidget);
};

#endif
