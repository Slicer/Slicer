
#include "qCTKMatrixWidget.h"

#include <Qt>
#include <QHeaderView>
#include <QVariant>
#include <QTableWidgetItem>
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
  
  // Hide headers + enable last section stretch
  //this->verticalHeader()->hide(); 
  //this->verticalHeader()->setStretchLastSection(true);
  //this->horizontalHeader()->hide(); 
  //this->horizontalHeader()->setStretchLastSection(true);
  //QTableWidgetItem item;
  //this->setSizeHintForColumn( 30 );
  //this->setSizeHintForColumn( 30 );
  
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
void qCTKMatrixWidget::reset()
{
  // Initialize 4x4 matrix
  for (int i=0; i < 4; i++)
    {
    for (int j=0; j < 4; j++)
      {
      this->setItem(i, j, this->itemPrototype()->clone());
      if (i == j)
        {
        this->setValue(i, j, 1); 
        }
      }
    }
  //this->resizeColumnsToContents();
  this->resizeRowsToContents();
}

// --------------------------------------------------------------------------
double qCTKMatrixWidget::value(int i, int j)
{
  if (i<0 || i>3 || j<0 || j>3) { return 0; }
  
  return this->item(i, j)->data(Qt::DisplayRole).toDouble();
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::setValue(int i, int j, double value)
{
  if (i<0 || i>3 || j<0 || j>3) { return; }
  
  this->item(i, j)->setData(Qt::DisplayRole, QVariant(value)); 
}

// --------------------------------------------------------------------------
void qCTKMatrixWidget::setVector(const QVector<double> & vector)
{
  for (int i=0; i < 4; i++)
    {
    for (int j=0; j < 4; j++)
      {
      this->item(i,j)->setData(Qt::DisplayRole, QVariant(vector.at(i * 4 + j)));
      }
    }
    
}

// --------------------------------------------------------------------------
QSize qCTKMatrixWidget::minimumSizeHint() const
{
  QSize size( QTableWidget::sizeHint() );
  int width = 0;
  int height = 0;
  
  for (int c = 0; c < this->columnCount(); ++c)
    {
    width += this->columnWidth( c );
    }
  for (int r = 0; r < this->rowCount(); ++r)
    {
    height += this->rowHeight( r );
    }
  size.setHeight( height + 4 );
  //size.setWidth( width + 4 );
  return size;
}

// --------------------------------------------------------------------------
QSize qCTKMatrixWidget::sizeHint() const
{
  return this->minimumSizeHint();
}
