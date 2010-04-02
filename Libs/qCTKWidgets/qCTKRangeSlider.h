/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKRangeSlider_h
#define __qCTKRangeSlider_h

#include <QSlider>

#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class QStylePainter;
class qCTKRangeSliderPrivate;

///
/// A qCTKRangeSlider is a slider that lets you input 2 values instead of one
/// (see QSlider). These values are typically a lower and upper bound.
/// Values are comprised between the range of the slider. (see setRange(), 
/// minimum() and maximum()). The upper bound can't be smaller than the 
/// lower bound and vice-versa.
/// FIXME: support triggerAction(QAbstractSlider::SliderSingleStepSub) that
/// moves both values at a time.
class QCTK_WIDGETS_EXPORT qCTKRangeSlider : public QSlider
{
  Q_OBJECT
  Q_PROPERTY(int minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(int maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(int minimumPosition READ minimumPosition WRITE setMinimumPosition)
  Q_PROPERTY(int maximumPosition READ maximumPosition WRITE setMaximumPosition)

public:
  // Superclass typedef
  typedef QSlider Superclass;
  // Constructors
  explicit qCTKRangeSlider( Qt::Orientation o, QWidget* par= 0 );
  explicit qCTKRangeSlider( QWidget* par = 0 );
  virtual ~qCTKRangeSlider();

  /// 
  /// This property holds the slider's current minimum value.
  /// The slider forces the minimum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the minimumValue also changes the minimumPosition.
  int minimumValue() const;

  /// 
  /// This property holds the slider's current maximum value.
  /// The slider forces the maximum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the maximumValue also changes the maximumPosition.
  int maximumValue() const;

  /// 
  /// This property holds the current slider minimum position.
  /// If tracking is enabled (the default), this is identical to minimumValue.
  int minimumPosition() const;
  void setMinimumPosition(int min);

  /// 
  /// This property holds the current slider maximum position.
  /// If tracking is enabled (the default), this is identical to maximumValue.
  int maximumPosition() const;
  void setMaximumPosition(int max);

  ///
  /// Utility function that set the minimum position and
  /// maximum position at once.
  void setPositions(int min, int max);

signals:
  ///
  /// This signal is emitted when the slider minimum value has changed, 
  /// with the new slider value as argument.
  void minimumValueChanged(int min);
  ///
  /// This signal is emitted when the slider maximum value has changed, 
  /// with the new slider value as argument.
  void maximumValueChanged(int max);
  ///
  /// Utility signal that is fired when minimum or maximum values have changed.
  void valuesChanged(int min, int max);

  ///
  /// This signal is emitted when sliderDown is true and the slider moves. 
  /// This usually happens when the user is dragging the minimum slider. 
  /// The value is the new slider minimum position.
  /// This signal is emitted even when tracking is turned off.
  void minimumPositionChanged(int min);

  ///
  /// This signal is emitted when sliderDown is true and the slider moves. 
  /// This usually happens when the user is dragging the maximum slider. 
  /// The value is the new slider maximum position.
  /// This signal is emitted even when tracking is turned off.
  void maximumPositionChanged(int max);

  ///
  /// Utility signal that is fired when minimum or maximum positions 
  /// have changed.
  void positionsChanged(int min, int max);

public slots:
  /// 
  /// This property holds the slider's current minimum value.
  /// The slider forces the minimum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the minimumValue also changes the minimumPosition.
  void setMinimumValue(int min);

  /// 
  /// This property holds the slider's current maximum value.
  /// The slider forces the maximum value to be within the legal range: 
  /// minimum <= minvalue <= maxvalue <= maximum.
  /// Changing the maximumValue also changes the maximumPosition.
  void setMaximumValue(int max);

  ///
  /// Utility function that set the minimum value and maximum value at once.
  void setValues(int min, int max);

protected slots:
  void onRangeChanged(int minimum, int maximum);

protected:
  // Description:
  // Standard Qt UI events
  virtual void mousePressEvent(QMouseEvent* ev);
  virtual void mouseMoveEvent(QMouseEvent* ev);
  virtual void mouseReleaseEvent(QMouseEvent* ev);

  // Description:
  // Rendering is done here.
  virtual void paintEvent(QPaintEvent* ev);

private:
  QCTK_DECLARE_PRIVATE(qCTKRangeSlider);
};

#endif

