/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKRangeWidget_h
#define __qCTKRangeWidget_h

/// qCTK includes
#include "qCTKPimpl.h"

/// QT includes
#include <QSlider>

#include "qCTKWidgetsExport.h"

class qCTKRangeWidgetPrivate;

class QCTK_WIDGETS_EXPORT qCTKRangeWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(int decimals READ decimals WRITE setDecimals)
  Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
  Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval)
  Q_PROPERTY(bool autoSpinBoxWidth READ isAutoSpinBoxWidth WRITE setAutoSpinBoxWidth)
  Q_PROPERTY(Qt::Alignment spinBoxAlignment READ spinBoxAlignment WRITE setSpinBoxAlignment)
  Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)

public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qCTKRangeWidget(QWidget* parent = 0);

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
  /// This property holds the slider and spinbox minimum value.
  /// qCTKRangeWidget forces the value to be within the 
  /// legal range: minimum <= minimumValue <= maximumValue <= maximum.
  double minimumValue()const;

  /// 
  /// This property holds the slider and spinbox maximum value.
  /// qCTKRangeWidget forces the value to be within the 
  /// legal range: minimum <= minimumValue <= maximumValue <= maximum.
  double maximumValue()const;

  /// 
  /// This property holds the single step.
  /// The smaller of two natural steps that the 
  /// slider provides and typically corresponds to the 
  /// user pressing an arrow key.
  double singleStep()const;
  void setSingleStep(double step);

  /// 
  /// This property holds the precision of the spin box, in decimals.
  /// Sets how many decimals the spinbox will use for displaying and interpreting doubles.
  int decimals()const;
  void setDecimals(int decimals);

  ///
  /// This property holds the spin box's prefix.
  /// The prefix is prepended to the start of the displayed value. 
  /// Typical use is to display a unit of measurement or a currency symbol
  QString prefix()const;
  void setPrefix(const QString& prefix);

  ///
  /// This property holds the spin box's suffix.
  /// The suffix is appended to the end of the displayed value. 
  /// Typical use is to display a unit of measurement or a currency symbol
  QString suffix()const;
  void setSuffix(const QString& suffix);

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
  /// This property holds whether slider tracking is enabled.
  /// If tracking is enabled (the default), the widget emits the valueChanged() 
  /// signal while the slider or spinbox is being dragged. If tracking is 
  /// disabled, the widget emits the valueChanged() signal only when the user 
  /// releases the slider or spinbox.
  void setTracking(bool enable);
  bool hasTracking()const;

  /// 
  /// Set/Get the auto spinbox width
  /// When the autoSpinBoxWidth property is on, the width of the SpinBox is
  /// set to the same width of the largest QSpinBox of its
  // qCTKRangeWidget siblings.
  bool isAutoSpinBoxWidth()const;
  void setAutoSpinBoxWidth(bool autoWidth);

public slots:
  /// 
  /// Reset the slider and spinbox to zero (value and position)
  void reset();
  void setMinimumValue(double value);
  void setMaximumValue(double value);

signals:
  /// Use with care:
  /// sliderMoved is emitted only when the user moves the slider
  //void sliderMoved(double position);
  void minimumValueChanged(double value);
  void minimumValueIsChanging(double value);
  void maximumValueChanged(double value);
  void maximumValueIsChanging(double value);

protected slots:
  void startChanging();
  void stopChanging();
  void changeMinimumValue(double value);
  void changeMaximumValue(double value);
  void setMinimumToMaximumSpinBox(double minimum);
  void setMaximumToMinimumSpinBox(double maximum);
  
protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);
private:
  QCTK_DECLARE_PRIVATE(qCTKRangeWidget);

};

#endif
