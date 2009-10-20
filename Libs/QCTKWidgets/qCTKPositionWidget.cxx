#include "qCTKPositionWidget.h"

#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <iostream>

qCTKPositionWidget::qCTKPositionWidget(QWidget* parent)
  :QWidget(parent)
{
  this->Dimension = 3;
  this->Position = new double [this->Dimension];
  
  QHBoxLayout* hboxLayout = new QHBoxLayout(this);

  for (int i = 0; i < this->Dimension; ++i)
    {
    this->Position[i] = 0.;
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
    connect( spinBox, SIGNAL(valueChanged(double)), 
             this, SLOT(coordinateChanged(double)));
    hboxLayout->addWidget(spinBox);
    }
  hboxLayout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(hboxLayout);
}

qCTKPositionWidget::~qCTKPositionWidget()
{
  delete [] this->Position;
}

// Description:
// Set/Get the dimension of the point (3D by default)
void qCTKPositionWidget::setDimension(int dim)
{
  if (dim < 1)
    {
    return;
    }
  double* newPos = new double[dim];
  if (dim > this->Dimension)
    {
    memcpy(newPos, this->Position, this->Dimension * sizeof(double));
    for (int i = this->Dimension; i < dim; ++i)
      {
      newPos[i] = 0.;
      QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
      connect( spinBox, SIGNAL(valueChanged(double)), 
               this, SLOT(coordinateChanged(double)));
      this->layout()->addWidget(spinBox);
      }
    }
  else
    {
    memcpy(newPos, this->Position, dim * sizeof(double));
    for (int i = this->Dimension - 1 ; i >= dim; --i)
      {
      QLayoutItem* item = this->layout()->takeAt(i);
      QWidget* widget = item ? item->widget() : 0;
      delete item;
      delete widget;
      }
    }
  delete [] this->Position;
  this->Position = newPos;
  this->Dimension = dim;

  this->updateGeometry();
  
  this->coordinatesChanged();
}

int qCTKPositionWidget::dimension() const
{
  return this->Dimension;
}

// Description:
// Set/Get the single step of the QDoubleSpinBoxes 
void qCTKPositionWidget::setSingleStep(double step)
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
}

double qCTKPositionWidget::singleStep() const
{
  QLayoutItem* item = this->layout()->itemAt(0);
  QDoubleSpinBox* spinBox = item ?dynamic_cast<QDoubleSpinBox*>(
    item->widget()) : 0;
  if (spinBox)
    {
    return spinBox->singleStep();
    }
  return 1.;
}

// Description:
// Set/Get the position. Use commas between numbers
void qCTKPositionWidget::setPositionAsString(QString pos)
{
  QStringList posList = pos.split(',');
  if (posList.count() != this->Dimension)
    {
    return;
    }
  double* newPos = new double[this->Dimension];
  for (int i = 0; i < this->Dimension; ++i)
    {
    newPos[i] = posList[i].toDouble();
    }
  this->setPosition(newPos);
  delete [] newPos;
}

QString qCTKPositionWidget::positionAsString()const
{
  QString res;
  for (int i = 0; i < this->Dimension; ++i)
    {
    if (i != 0)
      {
      res += ",";
      std::cout << "," ;
      }
    res += QString::number(this->Position[i]);
    std::cout << this->Position[i] ;
    }
  std::cout << std::endl;
  return res;
}

// Description:
// Set/Get the position
void qCTKPositionWidget::setPosition(double* pos)
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    this->Position[i] = pos[i];
    }
  bool blocked = this->blockSignals(true);
  for (int i = 0; i < this->Dimension; ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QDoubleSpinBox* spinBox = 
      item ? dynamic_cast<QDoubleSpinBox*>(item->widget()) : 0;
    if (spinBox)
      {
      spinBox->setValue(this->Position[i]);
      }
    }
  this->blockSignals(blocked);
  emit valueChanged(this->Position);
}

double* qCTKPositionWidget::position()const
{
  return this->Position;
}

void qCTKPositionWidget::coordinateChanged(double coordinate)
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QObject* spinBox = 
      item ? dynamic_cast<QObject*>(item->widget()) : 0;
    if ( spinBox && spinBox == this->sender())
      {
      this->Position[i] = coordinate;
      }
    }
  emit valueChanged(this->Position);
}

void qCTKPositionWidget::coordinatesChanged()
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    QLayoutItem* item = this->layout()->itemAt(i);
    QSpinBox* spinBox = 
      item ? dynamic_cast<QSpinBox*>(item->widget()) : 0;
    if ( spinBox)
      {
      this->Position[i] = spinBox->value();
      }
    }
  emit valueChanged(this->Position);
}
