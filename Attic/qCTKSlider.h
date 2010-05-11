/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKSlider_h
#define __qCTKSlider_h

// qCTK includes
#include "qCTKPimpl.h"

// QT includes
#include <QSlider>

#include "qCTKWidgetsExport.h"

class qCTKSliderPrivate;

class QCTK_WIDGETS_EXPORT qCTKSlider : public QSlider
{
  Q_OBJECT
  Q_PROPERTY(double SliderPosition READ sliderPositionAsDbl WRITE setSliderPosition)
  Q_PROPERTY(double Value READ valueAsDbl WRITE setValue)
  Q_PROPERTY(double SingleStep READ singleStepAsDbl WRITE setSingleStep)
  Q_PROPERTY(double TickInterval READ tickIntervalAsDbl WRITE setTickInterval)

public:
  // Superclass typedef
  typedef QSlider Superclass;

  // Constructors
  explicit qCTKSlider(QWidget* parent = 0);
  virtual ~qCTKSlider(){}

  // Description:
  // Get range
  double minimumAsDbl();
  double maximumAsDbl();

  // Description:
  // Set/Get slider position
  double sliderPositionAsDbl();
  void setSliderPosition(double position);
  double previousSliderPosition()const;

  // Description:
  // Set/Get value
  double valueAsDbl()const;
  void setValue(double value);

  // Description:
  // Set/Get single step
  double singleStepAsDbl()const;
  void setSingleStep(double step);

  // Description:
  // Set/Get tick interval
  double tickIntervalAsDbl()const;
  void setTickInterval(double ti);

public slots:
  // Description:
  // Reset the slider to zero (value and position)
  void reset();

  // Description:
  // Set range
  void setMinimumRange(double min);
  void setMaximumRange(double max);
  void setRange(double min, double max);

protected:
  int fromDoubleToInteger(double value);

protected slots:
  void onValueChanged(int value);
  void onSliderMoved(int position);

signals:
  void sliderMoved(double position);
  void valueChanged(double value);

private:
  CTK_DECLARE_PRIVATE(qCTKSlider);
};

#endif
