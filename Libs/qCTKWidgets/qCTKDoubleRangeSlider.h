/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKDoubleRangeSlider_h
#define __qCTKDoubleRangeSlider_h

/// QT includes
#include <QWidget>
#include <QAbstractSlider>

/// qCTK includes
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKDoubleRangeSliderPrivate;
class QCTK_WIDGETS_EXPORT qCTKDoubleRangeSlider : public QWidget
{
  Q_OBJECT      
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(double minimumPosition READ minimumPosition WRITE setMinimumPosition)
  Q_PROPERTY(double maximumPosition READ maximumPosition WRITE setMaximumPosition)
  Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
  Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval)
public:
  // Superclass typedef
  typedef QWidget Superclass;
  
  // Constructors
  qCTKDoubleRangeSlider( Qt::Orientation o, QWidget* par= 0 );
  qCTKDoubleRangeSlider( QWidget* par = 0 );
  
  /// 
  /// This property holds the single step.
  /// The smaller of two natural steps that an abstract sliders provides and
  /// typically corresponds to the user pressing an arrow key
  void setSingleStep(double ss);
  double singleStep()const;
  
  /// 
  /// This property holds the interval between tickmarks.
  /// This is a value interval, not a pixel interval. If it is 0, the slider
  /// will choose between lineStep() and pageStep().
  /// The default value is 0.
  void setTickInterval(double ti);
  double tickInterval()const;
  
  /// 
  /// This property holds the sliders's minimum value.
  /// When setting this property, the maximum is adjusted if necessary to
  /// ensure that the range remains valid. Also the slider's current values
  /// are adjusted to be within the new range. 
  double minimum()const;
  void setMinimum(double min);

  /// 
  /// This property holds the slider's maximum value.
  /// When setting this property, the minimum is adjusted if necessary to 
  /// ensure that the range remains valid. Also the slider's current values
  /// are adjusted to be within the new range.
  double maximum()const;
  void setMaximum(double max);

  /// 
  /// Sets the slider's minimum to min and its maximum to max.
  /// If max is smaller than min, min becomes the only legal value.
  void setRange(double min, double max);

  /// 
  /// This property holds the slider's current minimum value.
  /// The slider forces the minimum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the minimumValue also changes the minimumPosition.
  double minimumValue() const;

  /// 
  /// This property holds the slider's current maximum value.
  /// The slider forces the maximum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the maximumValue also changes the maximumPosition.
  double maximumValue() const;
  
  /// 
  /// This property holds the current slider minimum position.
  /// If tracking is enabled (the default), this is identical to minimumValue.
  double minimumPosition() const;
  void setMinimumPosition(double minPos);

  /// 
  /// This property holds the current slider maximum position.
  /// If tracking is enabled (the default), this is identical to maximumValue.
  double maximumPosition() const;
  void setMaximumPosition(double maxPos);

  ///
  /// Utility function that set the minimum position and
  /// maximum position at once.
  void setPositions(double minPos, double maxPos);

  /// 
  /// This property holds whether slider tracking is enabled.
  /// If tracking is enabled (the default), the slider emits the minimumValueChanged()
  /// signal while the left/bottom handler is being dragged and the slider emits
  /// the maximumValueChanged() signal while the right/top handler is being dragged. 
  /// If tracking is disabled, the slider emits the minimumValueChanged() 
  /// and maximumValueChanged() signals only when the user releases the slider.
  void setTracking(bool enable);
  bool hasTracking()const;
  
  ///
  /// Triggers a slider action on the current slider. Possible actions are 
  /// SliderSingleStepAdd, SliderSingleStepSub, SliderPageStepAdd, 
  /// SliderPageStepSub, SliderToMinimum, SliderToMaximum, and SliderMove.
  void triggerAction(QAbstractSlider::SliderAction action);

  ///
  /// This property holds the orientation of the slider.
  /// The orientation must be Qt::Vertical (the default) or Qt::Horizontal.
  Qt::Orientation orientation()const;
  void setOrientation(Qt::Orientation orientation);
signals:
  ///
  /// This signal is emitted when the slider minimum value has changed, 
  /// with the new slider value as argument.
  void minimumValueChanged(double minVal);

  ///
  /// This signal is emitted when the slider maximum value has changed, 
  /// with the new slider value as argument.
  void maximumValueChanged(double maxVal);

  ///
  /// Utility signal that is fired when minimum or maximum values have changed.
  void valuesChanged(double minVal, double maxVal);

  ///
  /// This signal is emitted when sliderDown is true and the slider moves. 
  /// This usually happens when the user is dragging the minimum slider. 
  /// The value is the new slider minimum position.
  /// This signal is emitted even when tracking is turned off.
  void minimumPositionChanged(double minPos);

  ///
  /// This signal is emitted when sliderDown is true and the slider moves. 
  /// This usually happens when the user is dragging the maximum slider. 
  /// The value is the new slider maximum position.
  /// This signal is emitted even when tracking is turned off.
  void maximumPositionChanged(double maxPos);
  
  ///
  /// Utility signal that is fired when minimum or maximum positions 
  /// have changed.
  void positionsChanged(double minPos, double maxPos);

  /// 
  /// This signal is emitted when the user presses one slider with the mouse, 
  /// or programmatically when setSliderDown(true) is called.
  void sliderPressed();

  /// 
  /// This signal is emitted when the user releases one slider with the mouse,
  /// or programmatically when setSliderDown(false) is called. 
  void sliderReleased();

    
public slots:
  /// 
  /// This property holds the slider's current minimum value.
  /// The slider forces the minimum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the minimumValue also changes the minimumPosition.
  void setMinimumValue(double minVal);

  /// 
  /// This property holds the slider's current maximum value.
  /// The slider forces the maximum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the maximumValue also changes the maximumPosition.
  void setMaximumValue(double maxVal);
  
  ///
  /// Utility function that set the minimum value and maximum value at once.
  void setValues(double minVal, double maxVal);

protected slots:
  void onMinValueChanged(int value);
  void onMaxValueChanged(int value);
  void onValuesChanged(int min, int max);

  void onMinPosChanged(int value);
  void onMaxPosChanged(int value);
  void onPositionsChanged(int min, int max);
private:
  QCTK_DECLARE_PRIVATE(qCTKDoubleRangeSlider);
};

#endif
