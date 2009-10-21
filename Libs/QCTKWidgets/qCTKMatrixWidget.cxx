
#include "qCTKMatrixWidget.h"

#include <Qt>
#include <QHeaderView>
#include <QVariant>
#include <QTableWidgetItem>
#include <QResizeEvent>
#include <QDebug>

#include <iostream>

//-----------------------------------------------------------------------------
class qCTKMatrixWidget::qInternal
{
public:
  qInternal()
    {
    }
};

// --------------------------------------------------------------------------
qCTKMatrixWidget::qCTKMatrixWidget(QWidget* parent) : Superclass(4, 4, parent)
{
  this->Internal = new qInternal;  
    
  // Set Read-only
  this->setEditTriggers(qCTKMatrixWidget::NoEditTriggers);
  
  // Hide headers
  this->verticalHeader()->hide(); 
  this->horizontalHeader()->hide(); 
  
  // Disable scrollBars
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
  // Define prototype item 
  QTableWidgetItem* item = new QTableWidgetItem(); 
  item->setData(Qt::DisplayRole, QVariant(0.0)); 
  item->setTextAlignment(Qt::AlignCenter);
  
  this->setItemPrototype(item);
  
  // Initialize 
  this->reset(); 
}

// --------------------------------------------------------------------------
qCTKMatrixWidget::~qCTKMatrixWidget()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::resizeEvent(QResizeEvent * event)
{
  this->Superclass::resizeEvent(event);
  this->adjustRowsColumnsSize(event->size().width(), event->size().height());
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::adjustRowsColumnsSize(int width, int height)
{
  int colwidth = width / this->columnCount();
  //qDebug() << "width:" << width << ",col-width:" << colwidth;
  for (int j=0; j < this->columnCount(); j++)
    {
    this->setColumnWidth(j, colwidth);
    }
    
  int rowheight = height / this->rowCount();
  //qDebug() << "height:" << height << ", row-height:" << rowheight;
  for (int i=0; i < this->rowCount(); i++)
    {
    this->setRowHeight(i, rowheight); 
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
void qCTKMatrixWidget::setValue(int i, int j, double value)
{
  if (i<0 || i>=(this->rowCount()) || j<0 || j>=this->columnCount()) { return; }
  
  this->item(i, j)->setData(Qt::DisplayRole, QVariant(value)); 
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
