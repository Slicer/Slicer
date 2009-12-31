#include "qVTKMatrixWidget.h"

// --------------------------------------------------------------------------
void qVTKMatrixWidget::setMatrix(vtkMatrix4x4* matrix)
{
  this->setMatrixInternal(matrix);
}

// --------------------------------------------------------------------------
qVTKMatrixWidget::qVTKMatrixWidget(QWidget* parent) : Superclass(parent)
{
}
