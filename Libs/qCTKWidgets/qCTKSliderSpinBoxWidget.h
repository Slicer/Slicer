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
  Q_PROPERTY(Qt::Alignment spinBoxAlignment READ spinBoxAlignment WRITE setSpinBoxAlignment)

public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qCTKSliderSpinBoxWidget(QWidget* parent = 0);
  virtual ~qCTKSliderSpinBoxWidget(){}

  /// 
  /// This property holds the sliders and spinbox minimum value.
  /// FIXME: Test following specs.
  /// When setting this property, the maximum is adjusted if necessary 
  /// to ensure that the range remains valid. 
  /// Also the slider's current value is adjusted to be within the new range.
  double minimum()const;
  void setMinimum(double minimum);
  
  /// 
  /// This property holds the sliders and spinbox minimum value.
  /// FIXME: Test following specs.
  /// When setting this property, the maximum is adjusted if necessary 
  /// to ensure that the range remains valid. 
  /// Also the slider's current value is adjusted to be within the new range.
  double maximum()const;
  void setMaximum(double maximum);
  /// Description
  /// Utility function that set the min/max in once
  void setRange(double min, double max);

  /// 
  /// This property holds the current slider position.
  /// If tracking is enabled (the default), this is identical to value.
  double sliderPosition()const;
  void setSliderPosition(double position);

  /// 
  /// This property holds the slider and spinbox current value.
  /// qCTKSliderSpinBoxWidget forces the value to be within the 
  /// legal range: minimum <= value <= maximum.
  double value()const;

  /// 
  /// This property holds the single step.
  /// The smaller of two natural steps that the 
  /// slider provides and typically corresponds to the 
  /// user pressing an arrow key.
  double singleStep()const;
  void setSingleStep(double step);

  /// 
  /// This property holds the interval between tickmarks.
  /// This is a value interval, not a pixel interval. 
  /// If it is 0, the slider will choose between lineStep() and pageStep().
  /// The default value is 0.
  double tickInterval()const;
  void setTickInterval(double ti);

  /// 
  /// This property holds the alignment of the spin box.
  /// Possible Values are Qt::AlignLeft, Qt::AlignRight, and Qt::AlignHCenter.
  /// By default, the alignment is Qt::AlignLeft
  void setSpinBoxAlignment(Qt::Alignment alignment);
  Qt::Alignment spinBoxAlignment()const;

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
