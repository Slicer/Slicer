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

#ifndef __qMRMLUnitWidget_h
#define __qMRMLUnitWidget_h

// CTK includes
#include <ctkVTKObject.h>

// MRML includes
class vtkMRMLNode;

// qMRML includes
#include <qMRMLWidget.h>
class qMRMLNodeComboBox;

// Qt includes
class QWidget;

// Unit includes
#include "qSlicerUnitsModuleWidgetsExport.h"
class qMRMLUnitWidgetPrivate;
class vtkSlicerUnitsLogic;

class Q_SLICER_MODULE_UNITS_WIDGETS_EXPORT qMRMLUnitWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY suffixChanged)
  Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)

public:
  typedef qMRMLWidget Superclass;
  qMRMLUnitWidget(QWidget *parent=0);
  virtual ~qMRMLUnitWidget();

  QString prefix() const;
  QString suffix() const;
  int precision() const;
  double minimum() const;
  double maximum() const;

  /// Set the units logic scene for the preset comboxes
  virtual void setMRMLScene(vtkMRMLScene* unitsLogicScene);

  Q_INVOKABLE vtkMRMLNode* currentNode() const;

public slots:
  void setCurrentNode(vtkMRMLNode* unitNode);
  void updateWidgetFromNode();
  void setPrefix(const QString &);
  void setSuffix(const QString &);
  void setPrecision(int);
  void setMinimum(double);
  void setMaximum(double);
  void setUnitFromPreset(vtkMRMLNode* presetNode);

signals:
  void prefixChanged(QString);
  void suffixChanged(QString);
  void precisionChanged(int);
  void minimumChanged(double);
  void maximumChanged(double);

protected:
  QScopedPointer<qMRMLUnitWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLUnitWidget);
  Q_DISABLE_COPY(qMRMLUnitWidget);
};

#endif
