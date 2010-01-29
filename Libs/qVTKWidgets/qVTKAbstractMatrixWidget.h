#ifndef __qVTKAbstractMatrixWidget_h
#define __qVTKAbstractMatrixWidget_h

/// qCTK includes
#include <qCTKMatrixWidget.h>
#include <qCTKPimpl.h>

/// qVTK includes
#include "qVTKObject.h"
#include "qVTKWidgetsExport.h"
 
class vtkMatrix4x4;
class qVTKAbstractMatrixWidgetPrivate;

class QVTK_WIDGETS_EXPORT qVTKAbstractMatrixWidget : public qCTKMatrixWidget
{
public:
  /// Self/Superclass typedef
  typedef qCTKMatrixWidget   Superclass;
  
  /// Constructors
  qVTKAbstractMatrixWidget(QWidget* parent);
  vtkMatrix4x4* matrix()const;

protected:
  void setMatrixInternal(vtkMatrix4x4* matrix);

private:
  QCTK_DECLARE_PRIVATE(qVTKAbstractMatrixWidget);
}; 

#endif
