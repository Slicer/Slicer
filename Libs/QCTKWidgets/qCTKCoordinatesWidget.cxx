#include "qCTKCoordinatesWidget.h"

#include <QHBoxLayout>
#include <QDoubleSpinBox>

qCTKCoordinatesWidget::qCTKCoordinatesWidget(QWidget* parent)
  :QWidget(parent)
{
  this->Dimension = 3;
  this->Coordinates = new double [this->Dimension];
  
  QHBoxLayout* hboxLayout = new QHBoxLayout(this);

  for (int i = 0; i < this->Dimension; ++i)
    {
    this->Coordinates[i] = 0.;
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
    connect( spinBox, SIGNAL(valueChanged(double)), 
             this, SLOT(coordinateChanged(double)));
    hboxLayout->addWidget(spinBox);
    }
  hboxLayout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(hboxLayout);
}

qCTKCoordinatesWidget::~qCTKCoordinatesWidget()
{
  delete [] this->Coordinates;
}

// Description:
// Set/Get the dimension of the point (3D by default)
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
      QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
      connect( spinBox, SIGNAL(valueChanged(double)), 
               this, SLOT(coordinateChanged(double)));
      this->layout()->addWidget(spinBox);
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

int qCTKCoordinatesWidget::dimension() const
{
  return this->Dimension;
}

// Description:
// Set/Get the single step of the QDoubleSpinBoxes 
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
}

double qCTKCoordinatesWidget::singleStep() const
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
// Set/Get the coordinates. Use commas between numbers
void qCTKCoordinatesWidget::setCoordinatesAsString(QString pos)
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
  this->setCoordinates(newPos);
  delete [] newPos;
}

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

// Description:
// Set/Get the coordinates
void qCTKCoordinatesWidget::setCoordinates(double* pos)
{
  for (int i = 0; i < this->Dimension; ++i)
    {
    this->Coordinates[i] = pos[i];
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

double* qCTKCoordinatesWidget::coordinates()const
{
  return this->Coordinates;
}

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
