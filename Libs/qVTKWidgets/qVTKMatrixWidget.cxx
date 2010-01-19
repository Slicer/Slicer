#include "qVTKMatrixWidget.h"

// --------------------------------------------------------------------------
void qVTKMatrixWidget::setMatrix(vtkMatrix4x4* _matrix)
{
  this->setMatrixInternal(_matrix);
}

// --------------------------------------------------------------------------
qVTKMatrixWidget::qVTKMatrixWidget(QWidget* _parent) : Superclass(_parent)
{
}
