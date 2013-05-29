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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerUnitsModuleWidget_h
#define __qSlicerUnitsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

// CTK includes
#include <ctkVTKObject.h>

// Unit includes
#include "qSlicerUnitsModuleExport.h"
class qSlicerUnitsModuleWidgetPrivate;

class Q_SLICER_QTMODULES_UNITS_EXPORT qSlicerUnitsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerUnitsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerUnitsModuleWidget();

protected slots:
  void updateWidgetFromNode();
  void updateNodeQuantityFromWidget();
  void setPrefix(const QString &);
  void setSuffix(const QString &);
  void setPrecision(int);
  void setMinimum(double);
  void setMaximum(double);

protected:
  QScopedPointer<qSlicerUnitsModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerUnitsModuleWidget);
  Q_DISABLE_COPY(qSlicerUnitsModuleWidget);
};

#endif
