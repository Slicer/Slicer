/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKCoordinatesWidget.h"

#include <QHBoxLayout>
#include <QDoubleSpinBox>

//------------------------------------------------------------------------------
qCTKCoordinatesWidget::qCTKCoordinatesWidget(QWidget* _parent)
  :QWidget(_parent)
{
  this->Minimum = -100000.;
  this->Maximum = 100000.;
  this->SingleStep = 1.;
  this->Dimension = 3;
  this->Coordinates = new double [this->Dimension];
  
  QHBoxLayout* hboxLayout = new QHBoxLayout(this);
  this->setLayout(hboxLayout);
  for (int i = 0; i < this->Dimension; ++i)
    {
    this->Coordinates[i] = 0.;
    this->AddSpinBox();
    }
  hboxLayout->setContentsMargins(0, 0, 0, 0);
}

//------------------------------------------------------------------------------
qCTKCoordinatesWidget::~qCTKCoordinatesWidget()
{
  delete [] this->Coordinates;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::AddSpinBox()
{
  QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
  spinBox->setMinimum(this->Minimum);
  spinBox->setMaximum(this->Maximum);
  spinBox->setSingleStep(this->SingleStep);
  connect( spinBox, SIGNAL(valueChanged(double)), 
           this, SLOT(coordinateChanged(double)));
  this->layout()->addWidget(spinBox);
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::setDimension(int dim)
{
  if (dim < 1)
    {
    return;
    }
  double* newPos = new double[dim];
  if (dim > this->Dimension)
    {
    memcpy(newPos, this->Coordinates, this->Dimension * sizeof(double));
    for (int i = this->Dimension; i < dim; ++i)
      {
      newPos[i] = 0.;
      this->AddSpinBox();
      }
    }
  else
    {
    memcpy(newPos, this->Coordinates, dim * sizeof(double));
    for (int i = this->Dimension - 1 ; i >= dim; --i)
      {
      QLayoutItem* item = this->layout()->takeAt(i);
      QWidget* widget = item ? item->widget() : 0;
      delete item;
      delete widget;
      }
    }
  delete [] this->Coordinates;
  this->Coordinates = newPos;
  this->Dimension = dim;

  this->updateGeometry();
  
  this->coordinatesChanged();
}

//------------------------------------------------------------------------------
int qCTKCoordinatesWidget::dimension() const
{
  return this->Dimension;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::setMinimum(double min)
{
  for (int i = 0; this->layout()->itemAt(i); ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QDoubleSpinBox* spinBox = item ? dynamic_cast<QDoubleSpinBox*>(
      item->widget()) : 0;
    if (spinBox)
      {
      spinBox->setMinimum(min);
      }
    }
  this->Minimum = min;
}

//------------------------------------------------------------------------------
double qCTKCoordinatesWidget::minimum() const
{
  return this->Minimum;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::setMaximum(double max)
{
  for (int i = 0; this->layout()->itemAt(i); ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QDoubleSpinBox* spinBox = item ? dynamic_cast<QDoubleSpinBox*>(
      item->widget()) : 0;
    if (spinBox)
      {
      spinBox->setMaximum(max);
      }
    }
  this->Maximum = max;
}

//------------------------------------------------------------------------------
double qCTKCoordinatesWidget::maximum() const
{
  return this->Maximum;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::setSingleStep(double step)
{
  for (int i = 0; this->layout()->itemAt(i); ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QDoubleSpinBox* spinBox = item ? dynamic_cast<QDoubleSpinBox*>(
      item->widget()) : 0;
    if (spinBox)
      {
      spinBox->setSingleStep(step);
      }
    }
  this->SingleStep = step;
}

//------------------------------------------------------------------------------
double qCTKCoordinatesWidget::singleStep() const
{
  return this->SingleStep;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::setCoordinatesAsString(QString _pos)
{
  QStringList posList = _pos.split(',');
  if (posList.count() != this->Dimension)
    {
    return;
    }
  double* newPos = new double[this->Dimension];
  for (int i = 0; i < this->Dimension; ++i)
    {
    newPos[i] = posList[i].toDouble();
    }
  this->setCoordinates(newPos);
  delete [] newPos;
}

//------------------------------------------------------------------------------
QString qCTKCoordinatesWidget::coordinatesAsString()const
{
  QString res;
  for (int i = 0; i < this->Dimension; ++i)
    {
    if (i != 0)
      {
      res += ",";
      }
    res += QString::number(this->Coordinates[i]);
    }
  return res;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::setCoordinates(double* _pos)
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    this->Coordinates[i] = _pos[i];
    }
  bool blocked = this->blockSignals(true);
  for (int i = 0; i < this->Dimension; ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QDoubleSpinBox* spinBox = 
      item ? dynamic_cast<QDoubleSpinBox*>(item->widget()) : 0;
    if (spinBox)
      {
      spinBox->setValue(this->Coordinates[i]);
      }
    }
  this->blockSignals(blocked);
  emit valueChanged(this->Coordinates);
}

//------------------------------------------------------------------------------
double* qCTKCoordinatesWidget::coordinates()const
{
  return this->Coordinates;
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::coordinateChanged(double coordinate)
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QObject* spinBox = 
      item ? dynamic_cast<QObject*>(item->widget()) : 0;
    if ( spinBox && spinBox == this->sender())
      {
      this->Coordinates[i] = coordinate;
      }
    }
  emit valueChanged(this->Coordinates);
}

//------------------------------------------------------------------------------
void qCTKCoordinatesWidget::coordinatesChanged()
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QSpinBox* spinBox = 
      item ? dynamic_cast<QSpinBox*>(item->widget()) : 0;
    if ( spinBox)
      {
      this->Coordinates[i] = spinBox->value();
      }
    }
  emit valueChanged(this->Coordinates);
}
