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

#ifndef __qMRMLSettingsUnitWidget_h
#define __qMRMLSettingsUnitWidget_h

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
class vtkMRMLNode;

// qMRML includes
class qMRMLUnitWidget;
class qMRMLNodeComboBox;

// Qt includes
#include <QWidget>

// Unit includes
#include "qSlicerUnitsModuleWidgetsExport.h"
class qMRMLSettingsUnitWidgetPrivate;
class vtkSlicerUnitsLogic;

class Q_SLICER_MODULE_UNITS_WIDGETS_EXPORT qMRMLSettingsUnitWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qMRMLSettingsUnitWidget(QWidget *parent=nullptr);
  ~qMRMLSettingsUnitWidget() override;

  /// Set the units logic scene for the preset comboxes
  virtual void setUnitsLogic(vtkSlicerUnitsLogic* logic);

  qMRMLNodeComboBox* unitComboBox();
  qMRMLUnitWidget* unitWidget();

protected:
  QScopedPointer<qMRMLSettingsUnitWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSettingsUnitWidget);
  Q_DISABLE_COPY(qMRMLSettingsUnitWidget);
};

#endif
