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

#ifndef __qMRMLRangeWidget_h
#define __qMRMLRangeWidget_h

// CTK includes
#include <ctkDoubleRangeSlider.h>
#include <ctkRangeSlider.h>
#include <ctkRangeWidget.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLRangeSliderPrivate;
class qMRMLSpinBox;
class vtkMRMLScene;

/// qMRMLRangeWidget is a wrapper around a ctkRangeWidget
/// It adds QSpinBoxes (in a menu) for controlling the range of the values
/// and supports for the units.
class QMRML_WIDGETS_EXPORT qMRMLRangeWidget : public ctkRangeWidget
{
  Q_OBJECT
  Q_PROPERTY(QPalette minimumHandlePalette READ minimumHandlePalette WRITE setMinimumHandlePalette)
  Q_PROPERTY(QPalette maximumHandlePalette READ maximumHandlePalette WRITE setMaximumHandlePalette)
  Q_PROPERTY(vtkMRMLScene* mrmlScene READ mrmlScene WRITE setMRMLScene)
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity)

public:
  /// Constructor
  /// If \li parent is null, qMRMLRangeWidget will be a top-level widget
  /// \note The \li parent can be set later using QWidget::setParent()
  explicit qMRMLRangeWidget(QWidget* parent = nullptr);

  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

  vtkMRMLScene* mrmlScene()const;
  QString quantity()const;

  /// When symmetricMoves is true, moving a handle will move the other handle
  /// symmetrically, otherwise the handles are independent. False by default.
  void setSymmetricMoves(bool symmetry) override;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);

  /// Set the quantity the widget should look for.
  /// \sa quantity()
  void setQuantity(const QString& baseName);

  /// Set the scene the spinboxes listens to.
  /// \sa mrmlScene()
  virtual void setMRMLScene(vtkMRMLScene* scene);

protected slots:
  void updateSpinBoxRange(double min, double max);
  void updateRange();
  void updateSymmetricMoves(bool symmetric);

protected:
  qMRMLSpinBox* MinSpinBox;
  qMRMLSpinBox* MaxSpinBox;
  QAction* SymmetricAction;
};

/// qMRMLDoubleRangeSlider is a wrapper around a ctkDoubleRangeSlider
class QMRML_WIDGETS_EXPORT qMRMLDoubleRangeSlider : public ctkDoubleRangeSlider
{
  Q_OBJECT;
public:
  qMRMLDoubleRangeSlider(QWidget* parentWidget);
  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

  vtkMRMLScene* mrmlScene()const;
  QString quantity()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
};

/// qMRMLRangeSlider is a wrapper around a ctkRangeSlider
class QMRML_WIDGETS_EXPORT qMRMLRangeSlider : public ctkRangeSlider
{
  Q_OBJECT;
public:
  qMRMLRangeSlider(QWidget* parentWidget);
  ~qMRMLRangeSlider() override;
  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
protected:
  void initMinimumSliderStyleOption(QStyleOptionSlider* option) const override;
  void initMaximumSliderStyleOption(QStyleOptionSlider* option) const override;

protected:
  QScopedPointer<qMRMLRangeSliderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLRangeSlider);
  Q_DISABLE_COPY(qMRMLRangeSlider);
};

#endif
