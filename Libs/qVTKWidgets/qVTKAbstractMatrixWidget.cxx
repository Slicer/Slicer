#include "qVTKAbstractMatrixWidget.h"

// qVTKWidgets includes
#include "qVTKAbstractMatrixWidgetPrivate.h"

// VTK includes
#include <vtkMatrix4x4.h>

// QT includes
#include <QDebug>
#include <QVector>

// --------------------------------------------------------------------------
qVTKAbstractMatrixWidgetPrivate::qVTKAbstractMatrixWidgetPrivate()
  :QObject(0) // will be reparented in init()
{
}

void qVTKAbstractMatrixWidgetPrivate::init()
{
  QCTK_P(qVTKAbstractMatrixWidget);
  this->setParent(p);
  // there is no matrix to show/edit, disable the widget
  p->setEnabled(this->Matrix == 0);
}

// --------------------------------------------------------------------------
void qVTKAbstractMatrixWidgetPrivate::setMatrix(vtkMatrix4x4* matrix)
{
  qvtkReconnect(this->Matrix.GetPointer(), matrix, 
                vtkCommand::ModifiedEvent, this, SLOT(updateMatrix()));

  this->Matrix = matrix;
  this->updateMatrix();
}

// --------------------------------------------------------------------------
vtkMatrix4x4* qVTKAbstractMatrixWidgetPrivate::matrix() const
{
  return this->Matrix;
}

// --------------------------------------------------------------------------
void qVTKAbstractMatrixWidgetPrivate::updateMatrix()
{
  QCTK_P(qVTKAbstractMatrixWidget);
  // if there is no transform to show/edit, disable the widget
  p->setEnabled(this->Matrix != 0);

  if (this->Matrix == 0)
    {
    p->reset();
    return;
    }
  QVector<double> vector;
  //todo: fasten the loop
  for (int i=0; i < 4; i++)
    {
    for (int j=0; j < 4; j++)
      {
      vector.append(this->Matrix->GetElement(i,j)); 
      }
    }
  p->setVector( vector );
}

// --------------------------------------------------------------------------
qVTKAbstractMatrixWidget::qVTKAbstractMatrixWidget(QWidget* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qVTKAbstractMatrixWidget);
  qctk_d()->init();
}

// --------------------------------------------------------------------------
vtkMatrix4x4* qVTKAbstractMatrixWidget::matrix()const
{
  QCTK_D(const qVTKAbstractMatrixWidget);
  return d->matrix();
}

// --------------------------------------------------------------------------
void qVTKAbstractMatrixWidget::setMatrixInternal(vtkMatrix4x4* matrix)
{
  QCTK_D(qVTKAbstractMatrixWidget);
  d->setMatrix(matrix);
}
