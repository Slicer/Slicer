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

#include "qCTKRangeSlider.h"

class qCTKDoubleRangeSlider : public qCTKRangeSlider
{
  Q_OBJECT    
  Q_PROPERTY(double RangeMinimumValuePositionAsDbl READ rangeMinimumValuePositionAsDbl WRITE setRangeMinimumPositionAsDbl)
  Q_PROPERTY(double RangeMaximumValuePositionAsDbl READ rangeMaximumValuePositionAsDbl WRITE setRangeMaximumPositionAsDbl)
  Q_PROPERTY(double RangeMinimumAsDbl READ rangeMinimumAsDbl WRITE setRangeMinimumAsDbl)
  Q_PROPERTY(double RangeMaximumAsDbl READ rangeMaximumAsDbl WRITE setRangeMaximumAsDbl)
  Q_PROPERTY(double TickIntervalAsDbl READ tickIntervalAsDbl WRITE setTickIntervalAsDbl);
public:
  // Superclass typedef
  typedef qCTKRangeSlider Superclass;
  
  // Constructors
  qCTKDoubleRangeSlider( Qt::Orientation o, QWidget* par= 0 );
  qCTKDoubleRangeSlider( QWidget* par = 0 );
  
  void setTickIntervalAsDbl(double ti);
  double tickIntervalAsDbl()const;
  
  double rangeMinimumAsDbl() const;
  double rangeMaximumAsDbl() const;

  double rangeMinimumValuePositionAsDbl() const;
  double rangeMaximumValuePositionAsDbl() const;

signals:
    void rangeValuesAsDblChanged(double rangeMinimum, double rangeMaximum);
    void rangeMinimumAsDblChanged(double rangeMinimum);
    void rangeMaximumAsDblChanged(double rangeMaximum);

    void rangeMinimumValuePositionAsDblChanged(double rangeMinimum);
    void rangeMaximumValuePositionAsDblChanged(double rangeMaximum);
    
public slots:

  void setRangeMinimumAsDbl(double rangeMinimum);
  void setRangeMaximumAsDbl(double rangeMaximum);
  void setRangeBoundsAsDbl(double rangeMinimum, double rangeMaximum);

  void setRangeMinimumPositionAsDbl(double rangeMinimumPosition);
  void setRangeMaximumPositionAsDbl(double rangeMaximumPosition);

protected slots:
  void onRangeMinimumValuePositionChanged(int value);
  void onRangeMaximumValuePositionChanged(int value);
  
  void onRangeValuesChanged(int min, int max);

protected:
  int fromDouble2Int(double value)const; 
  
private:
  
  // Description:
  // Interval between tick mark. 
  double m_DblTickInterval; 
  
  double m_DblRangeMinimum; 
  double m_DblRangeMaximum; 
  
  double m_DblRangeMinimumValue; 
  double m_DblRangeMaximumValue; 
};

#endif
