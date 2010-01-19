/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKMatrixWidget.h"

#include <Qt>
#include <QHeaderView>
#include <QVariant>
#include <QTableWidgetItem>
#include <QResizeEvent>
#include <QDebug>

//-----------------------------------------------------------------------------
class qCTKMatrixWidgetPrivate: public qCTKPrivate<qCTKMatrixWidget>
{
};

// --------------------------------------------------------------------------
qCTKMatrixWidget::qCTKMatrixWidget(QWidget* _parent) : Superclass(4, 4, _parent)
{
  QCTK_INIT_PRIVATE(qCTKMatrixWidget);

  // Set Read-only
  this->setEditTriggers(qCTKMatrixWidget::NoEditTriggers);

  // Hide headers
  this->verticalHeader()->hide();
  this->horizontalHeader()->hide();

  // Disable scrollBars
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Define prototype item
  QTableWidgetItem* _item = new QTableWidgetItem();
  _item->setData(Qt::DisplayRole, QVariant(0.0));
  _item->setTextAlignment(Qt::AlignCenter);

  // The table takes ownership of the prototype.
  this->setItemPrototype(_item);

  // Initialize
  this->reset();
}

// --------------------------------------------------------------------------
QSize qCTKMatrixWidget::minimumSizeHint () const
{
  return QSize(this->columnCount() * 25, this->rowCount() * 25);
}

// --------------------------------------------------------------------------
QSize qCTKMatrixWidget::sizeHint () const
{
  return this->minimumSizeHint();
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::resizeEvent(QResizeEvent * _event)
{
  this->Superclass::resizeEvent(_event);
  this->adjustRowsColumnsSize(_event->size().width(), _event->size().height());
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::adjustRowsColumnsSize(int _width, int _height)
{
  int colwidth = _width / this->columnCount();
  int lastColwidth = colwidth + (_width - colwidth * this->columnCount());
  //qDebug() << "width:" << width << ",col-width:" << colwidth;
  for (int j=0; j < this->columnCount(); j++)
    {
    bool lastColumn = (j==(this->columnCount()-1));
    this->setColumnWidth(j, lastColumn ? lastColwidth : colwidth);
    }

  int rowheight = _height / this->rowCount();
  int lastRowheight = rowheight + (_height - rowheight * this->rowCount());
  //qDebug() << "height:" << height << ", row-height:" << rowheight;
  for (int i=0; i < this->rowCount(); i++)
    {
    bool lastRow = (i==(this->rowCount()-1));
    this->setRowHeight(i, lastRow ? lastRowheight : rowheight);
    }
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::reset()
{
  // Initialize 4x4 matrix
  for (int i=0; i < this->rowCount(); i++)
    {
    for (int j=0; j < this->columnCount(); j++)
      {
      this->setItem(i, j, this->itemPrototype()->clone());
      if (i == j)
        {
        this->setValue(i, j, 1);
        }
      }
    }
}

// --------------------------------------------------------------------------
double qCTKMatrixWidget::value(int i, int j)
{
  if (i<0 || i>=(this->rowCount()) || j<0 || j>=this->columnCount()) { return 0; }

  return this->item(i, j)->data(Qt::DisplayRole).toDouble();
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::setValue(int i, int j, double _value)
{
  if (i<0 || i>=(this->rowCount()) || j<0 || j>=this->columnCount()) { return; }

  this->item(i, j)->setData(Qt::DisplayRole, QVariant(_value));
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::setVector(const QVector<double> & vector)
{
  for (int i=0; i < this->rowCount(); i++)
    {
    for (int j=0; j < this->columnCount(); j++)
      {
      this->item(i,j)->setData(Qt::DisplayRole, QVariant(vector.at(i * this->columnCount() + j)));
      }
    }
}
