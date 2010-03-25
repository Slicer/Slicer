/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKDoubleSlider_h
#define __qCTKDoubleSlider_h

/// QT includes
#include <QSlider>
#include <QWidget>

/// qCTK includes
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKDoubleSliderPrivate;

class QCTK_WIDGETS_EXPORT qCTKDoubleSlider : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double value READ value WRITE setValue)
  Q_PROPERTY(double sliderPosition READ sliderPosition WRITE setSliderPosition)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval)
  Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// 
  /// Constructors
  /// Vertical by default
  explicit qCTKDoubleSlider(QWidget* parent = 0);
  explicit qCTKDoubleSlider(Qt::Orientation orient, QWidget* parent = 0);
  /// Destructor
  virtual ~qCTKDoubleSlider();

  /// 
  /// This property holds the sliders's minimum value.
  /// When setting this property, the maximum is adjusted if necessary to
  /// ensure that the range remains valid. Also the slider's current value
  /// is adjusted to be within the new range. 
  void setMinimum(double min);
  double minimum()const;

  /// 
  /// This property holds the slider's maximum value.
  /// When setting this property, the minimum is adjusted if necessary to 
  /// ensure that the range remains valid. Also the slider's current value
  /// is adjusted to be within the new range.
  void setMaximum(double max);
  double maximum()const;

  /// 
  /// Sets the slider's minimum to min and its maximum to max.
  /// If max is smaller than min, min becomes the only legal value.
  void setRange(double min, double max);

  /// 
  /// This property holds the slider's current value.
  /// The slider forces the value to be within the legal range: 
  /// minimum <= value <= maximum.
  /// Changing the value also changes the sliderPosition.
  double value()const;

  /// 
  /// This property holds the single step.
  /// The smaller of two natural steps that an abstract sliders provides and
  /// typically corresponds to the user pressing an arrow key
  void setSingleStep(double step);
  double singleStep()const;

  /// 
  /// This property holds the interval between tickmarks.
  /// This is a value interval, not a pixel interval. If it is 0, the slider
  /// will choose between lineStep() and pageStep().
  /// The default value is 0.
  void setTickInterval(double ti);
  double tickInterval()const;

  /// 
  /// This property holds the current slider position.
  /// If tracking is enabled (the default), this is identical to value.
  double sliderPosition()const;
  void setSliderPosition(double);

  /// 
  /// This property holds whether slider tracking is enabled.
  /// If tracking is enabled (the default), the slider emits the valueChanged()
  /// signal while the slider is being dragged. If tracking is disabled, the 
  /// slider emits the valueChanged() signal only when the user releases the
  /// slider.
  void setTracking(bool enable);
  bool hasTracking()const;
  
  ///
  /// Triggers a slider action. Possible actions are SliderSingleStepAdd, 
  /// SliderSingleStepSub, SliderPageStepAdd, SliderPageStepSub, 
  /// SliderToMinimum, SliderToMaximum, and SliderMove.
  void triggerAction(QAbstractSlider::SliderAction action);

  ///
  /// This property holds the orientation of the slider.
  /// The orientation must be Qt::Vertical (the default) or Qt::Horizontal.
  Qt::Orientation orientation()const;

public slots:
  /// 
  /// This property holds the slider's current value.
  /// The slider forces the value to be within the legal range: 
  /// minimum <= value <= maximum.
  /// Changing the value also changes the sliderPosition.
  void setValue(double value);

  ///
  /// This property holds the orientation of the slider.
  /// The orientation must be Qt::Vertical (the default) or Qt::Horizontal.
  void setOrientation(Qt::Orientation orientation);

protected slots:
  void onValueChanged(int value);
  void onSliderMoved(int position);

signals:
  ///
  /// This signal is emitted when the slider value has changed, with the new
  /// slider value as argument.
  void valueChanged(double value);

  ///
  /// This signal is emitted when sliderDown is true and the slider moves. 
  /// This usually happens when the user is dragging the slider. The value
  /// is the new slider position.
  /// This signal is emitted even when tracking is turned off.
  void sliderMoved(double position);
  
  ///
  /// This signal is emitted when the user presses the slider with the mouse,
  /// or programmatically when setSliderDown(true) is called.
  void sliderPressed();
  
  /// 
  /// This signal is emitted when the user releases the slider with the mouse, 
  /// or programmatically when setSliderDown(false) is called.
  void sliderReleased();
  
private:
  QCTK_DECLARE_PRIVATE(qCTKDoubleSlider);
};

#endif
