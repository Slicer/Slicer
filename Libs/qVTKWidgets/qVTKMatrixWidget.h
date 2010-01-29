#ifndef __qVTKMatrixWidget_h
#define __qVTKMatrixWidget_h

/// qCTK includes
#include <qCTKPimpl.h>

/// qVTK includes
#include "qVTKAbstractMatrixWidget.h"
#include "qVTKWidgetsExport.h"
 
class vtkMatrix4x4;

class QVTK_WIDGETS_EXPORT qVTKMatrixWidget : public qVTKAbstractMatrixWidget
{
  Q_OBJECT
public:
  /// Self/Superclass typedef
  typedef qVTKMatrixWidget  Self;
  typedef qVTKAbstractMatrixWidget   Superclass;
  
  /// Constructors
  qVTKMatrixWidget(QWidget* parent);

public slots:
  void setMatrix(vtkMatrix4x4* matrix);
}; 

#endif
