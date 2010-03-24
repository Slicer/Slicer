#include "qCTKDoubleRangeSlider.h"
#include <QDebug>
// --------------------------------------------------------------------------
qCTKDoubleRangeSlider::qCTKDoubleRangeSlider(QWidget* parent) 
  :qCTKRangeSlider(parent)
{
  this->m_DblTickInterval = 1.0; 
  
  this->m_DblRangeMinimum = 10; 
  this->m_DblRangeMaximum = 90; 
  this->m_DblRangeMinimumValue = 10; 
  this->m_DblRangeMaximumValue = 90; 
  
  connect(this, SIGNAL(rangeValuesChanged(int, int)), 
          this, SLOT(onRangeValuesChanged(int, int)));
          
  connect(this, SIGNAL(rangeMinimumValuePositionChanged(int)), 
          this, SLOT(onRangeMinimumValuePositionChanged(int)));
          
  connect(this, SIGNAL(rangeMaximumValuePositionChanged(int)), 
          this, SLOT(onRangeMaximumValuePositionChanged(int)));
}

// --------------------------------------------------------------------------
qCTKDoubleRangeSlider::qCTKDoubleRangeSlider( Qt::Orientation o, 
                                              QWidget* parentWidget ) 
  :qCTKRangeSlider(o, parentWidget)
{
  this->m_DblTickInterval = 1.0; 
  
  this->m_DblRangeMinimum = 10; 
  this->m_DblRangeMaximum = 90; 
  this->m_DblRangeMinimumValue = 10; 
  this->m_DblRangeMaximumValue = 90; 
          
  connect(this, SIGNAL(rangeMinimumValuePositionChanged(int)), 
          this, SLOT(onRangeMinimumValuePositionChanged(int)));
          
  connect(this, SIGNAL(rangeMaximumValuePositionChanged(int)), 
          this, SLOT(onRangeMaximumValuePositionChanged(int)));
}

//-----------------------------------------------------------------------------

void qCTKDoubleRangeSlider::setTickIntervalAsDbl(double ti)
{
  // if 'ti' is an integer, set DoubleMode to False
  this->m_DblTickInterval = ti; 
  
  this->Superclass::setTickInterval(1);
  this->setRangeBoundsAsDbl(this->rangeMinimumAsDbl(), this->rangeMaximumAsDbl());
}

//-----------------------------------------------------------------------------
double qCTKDoubleRangeSlider::tickIntervalAsDbl()const
{
  return this->m_DblTickInterval;
}

//-----------------------------------------------------------------------------
double qCTKDoubleRangeSlider::rangeMinimumAsDbl() const
{
  return this->m_DblRangeMinimum; 
}

//-----------------------------------------------------------------------------
double qCTKDoubleRangeSlider::rangeMaximumAsDbl() const
{
  return this->m_DblRangeMaximum; 
}

//-----------------------------------------------------------------------------
double qCTKDoubleRangeSlider::rangeMinimumValuePositionAsDbl() const
{
  return this->m_DblRangeMinimumValue; 
}

//-----------------------------------------------------------------------------
double qCTKDoubleRangeSlider::rangeMaximumValuePositionAsDbl() const
{
  return this->m_DblRangeMaximumValue; 
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::setRangeMinimumAsDbl(double rangeMinimum)
{
  this->setRangeBoundsAsDbl(rangeMinimum, this->rangeMaximumAsDbl());
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::setRangeMaximumAsDbl(double rangeMaximum)
{
  this->setRangeBoundsAsDbl(this->rangeMinimumAsDbl(), rangeMaximum);
}

//-----------------------------------------------------------------------------
int qCTKDoubleRangeSlider::fromDouble2Int(double value)const
{
  return static_cast<int>(value / this->m_DblTickInterval);
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::setRangeBoundsAsDbl(double rangeMinimum, double rangeMaximum)
{
  this->m_DblRangeMinimum = qMin(rangeMinimum, rangeMaximum); 
  this->m_DblRangeMaximum = qMax(rangeMinimum, rangeMaximum); 
  
  // QSlider range
  this->Superclass::setRange(
    this->fromDouble2Int(m_DblRangeMinimum),
    this->fromDouble2Int(m_DblRangeMaximum)); 

  this->Superclass::setRangeBounds(
    this->fromDouble2Int(m_DblRangeMinimum),
    this->fromDouble2Int(m_DblRangeMaximum));

  this->setRangeMinimumPositionAsDbl(
    qBound(this->m_DblRangeMinimum, this->m_DblRangeMinimumValue, this->m_DblRangeMaximum));
  this->setRangeMaximumPositionAsDbl(
    qBound(this->m_DblRangeMinimum, this->m_DblRangeMaximumValue, this->m_DblRangeMaximum));

  emit this->rangeValuesAsDblChanged(this->rangeMinimumAsDbl(), this->rangeMaximumAsDbl());
  emit this->rangeMinimumAsDblChanged(this->rangeMinimumAsDbl());
  emit this->rangeMaximumAsDblChanged(this->rangeMaximumAsDbl());
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::setRangeMinimumPositionAsDbl(double rangeMinimumPosition)
{
  this->m_DblRangeMinimumValue = 
    qBound(this->m_DblRangeMinimum, rangeMinimumPosition,this->m_DblRangeMaximum); 
  this->Superclass::setRangeMinimumPosition(
    this->fromDouble2Int(m_DblRangeMinimumValue));
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::setRangeMaximumPositionAsDbl(double rangeMaximumPosition)
{
  this->m_DblRangeMaximumValue = 
    qBound(this->m_DblRangeMinimum, rangeMaximumPosition, this->m_DblRangeMaximum); 
  this->Superclass::setRangeMaximumPosition(
    this->fromDouble2Int(m_DblRangeMaximumValue));
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::onRangeMinimumValuePositionChanged(int value)
{
  this->m_DblRangeMinimumValue =  value * this->m_DblTickInterval;
  emit rangeMinimumValuePositionAsDblChanged(this->m_DblRangeMinimumValue);
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::onRangeMaximumValuePositionChanged(int value)
{
  this->m_DblRangeMaximumValue =  value * this->m_DblTickInterval;
  emit rangeMaximumValuePositionAsDblChanged(this->m_DblRangeMaximumValue);
}

//-----------------------------------------------------------------------------
void qCTKDoubleRangeSlider::onRangeValuesChanged(int min, int max)
{
  this->setRangeBounds(this->fromDouble2Int(min), 
    this->fromDouble2Int(max));
}

