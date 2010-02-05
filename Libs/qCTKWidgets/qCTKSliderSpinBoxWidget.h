/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKSliderSpinBoxWidget_h
#define __qCTKSliderSpinBoxWidget_h

/// qCTK includes
#include "qCTKPimpl.h"

/// QT includes
#include <QSlider>

#include "qCTKWidgetsExport.h"

class qCTKSliderSpinBoxWidgetPrivate;

class QCTK_WIDGETS_EXPORT qCTKSliderSpinBoxWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double value READ value WRITE setValue)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval)
  Q_PROPERTY(bool autoSpinBoxWidth READ isAutoSpinBoxWidth WRITE setAutoSpinBoxWidth)

public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qCTKSliderSpinBoxWidget(QWidget* parent = 0);
  virtual ~qCTKSliderSpinBoxWidget(){}

  /// 
  /// Set/Get range
  double minimum()const;
  double maximum()const;
  void setMinimum(double minimum);
  void setMaximum(double maximum);
  /// Description
  /// Utility function that set the min/max in once
  void setRange(double min, double max);

  /// 
  /// Set/Get slider position
  double sliderPosition()const;
  void setSliderPosition(double position);
  //double previousSliderPosition();

  /// 
  /// Set/Get value
  double value()const;

  /// 
  /// Set/Get single step
  double singleStep()const;
  void setSingleStep(double step);

  /// 
  /// Set/Get tick interval
  double tickInterval()const;
  void setTickInterval(double ti);

  /// 
  /// Set/Get the auto spinbox width
  /// When the autoSpinBoxWidth property is on, the width of the SpinBox is
  /// set to the same width of the largest QSpinBox of its
  // qCTKSliderSpinBoxWidget siblings.
  bool isAutoSpinBoxWidth()const;
  void setAutoSpinBoxWidth(bool autoWidth);

public slots:
  /// 
  /// Reset the slider and spinbox to zero (value and position)
  void reset();
  void setValue(double value);

signals:
  /// Use with care:
  /// sliderMoved is emitted only when the user moves the slider
  void sliderMoved(double position);
  void valueChanged(double value);
private:
  QCTK_DECLARE_PRIVATE(qCTKSliderSpinBoxWidget);

};

#endif
