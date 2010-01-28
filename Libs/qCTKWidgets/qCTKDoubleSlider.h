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

// QT includes
#include <QSlider>
#include <QWidget>

// qCTK includes
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKDoubleSliderPrivate;

class QCTK_WIDGETS_EXPORT qCTKDoubleSlider : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double value READ value WRITE setValue)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval)
  Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
  // Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  /// Vertical by default
  explicit qCTKDoubleSlider(QWidget* parent = 0);
  explicit qCTKDoubleSlider(Qt::Orientation orient, QWidget* parent = 0);
  /// Destructor
  virtual ~qCTKDoubleSlider();

  // Description:
  // Get range
  void setMinimum(double min);
  double minimum()const;

  void setMaximum(double max);
  double maximum()const;

  // Description:
  // Set range
  void setRange(double min, double max);

  // Description:
  // Set/Get value
  double value()const;

  // Description:
  // Set/Get single step
  void setSingleStep(double step);
  double singleStep()const;

  // Description:
  // Set/Get tick interval
  void setTickInterval(double ti);
  double tickInterval()const;

  double sliderPosition()const;
  void setSliderPosition(double);
  
  void setTracking(bool enable);
  bool hasTracking()const;
  
  void triggerAction(QAbstractSlider::SliderAction action);

  /// Orientation of the slider (Vertical by default)
  Qt::Orientation orientation()const;

public slots:
  void setValue(double value);
  void setOrientation(Qt::Orientation orientation);

protected slots:
  void onValueChanged(int value);
  void onSliderMoved(int position);

signals:
  void valueChanged(double value);
  /// Same as QSlider::sliderMoved, the signal is emitted ONLY if the slider
  // is manually moved by the user.
  void sliderMoved(double position);
  
private:
  QCTK_DECLARE_PRIVATE(qCTKDoubleSlider);
};

#endif
