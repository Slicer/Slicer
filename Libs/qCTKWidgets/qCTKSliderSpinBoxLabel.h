/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKSliderSpinBoxLabel_h
#define __qCTKSliderSpinBoxLabel_h

// qCTK includes
#include "qCTKPimpl.h"

// QT includes
#include <QSlider>

#include "qCTKWidgetsExport.h"

class qCTKSliderSpinBoxLabelPrivate;

class QCTK_WIDGETS_EXPORT qCTKSliderSpinBoxLabel : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
  Q_PROPERTY(double value READ value WRITE setValue)
  Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
  Q_PROPERTY(double tickInterval READ tickInterval WRITE setTickInterval)

public:
  // Superclass typedef
  typedef QWidget Superclass;

  // Constructors
  explicit qCTKSliderSpinBoxLabel(QWidget* parent = 0);
  virtual ~qCTKSliderSpinBoxLabel(){}

  // Description:
  // Set/Get range
  double minimum()const;
  double maximum()const;
  void setMinimum(double minimum);
  void setMaximum(double maximum);
  // Description
  // Utility function that set the min/max in once
  void setRange(double min, double max);

  // Description:
  // Set/Get slider position
  double sliderPosition()const;
  void setSliderPosition(double position);
  //double previousSliderPosition();

  // Description:
  // Set/Get value
  double value()const;

  // Description:
  // Set/Get single step
  double singleStep()const;
  void setSingleStep(double step);

  // Description:
  // Set/Get tick interval
  double tickInterval()const;
  void setTickInterval(double ti);

public slots:
  // Description:
  // Reset the slider and spinbox to zero (value and position)
  void reset();
  void setValue(double value);

signals:
  /// Use with care:
  /// sliderMoved is emitted only when the user moves the slider
  void sliderMoved(double position);
  void valueChanged(double value);

private:
  QCTK_DECLARE_PRIVATE(qCTKSliderSpinBoxLabel);

};

#endif
