#ifndef __qVTKAbstractMatrixWidget_p_h
#define __qVTKAbstractMatrixWidget_p_h

/// QT includes
#include <QObject>

/// VTK includes
#include <vtkWeakPointer.h>

/// qCTKWidgets includes
#include <qCTKPimpl.h>

/// qVTKWidgets includes
#include "qVTKAbstractMatrixWidget.h"

class vtkMatrix4x4;

class qVTKAbstractMatrixWidgetPrivate: public QObject, public qCTKPrivate<qVTKAbstractMatrixWidget>
{
  Q_OBJECT
  QVTK_OBJECT
public:  
  qVTKAbstractMatrixWidgetPrivate();
  void init();

  void setMatrix(vtkMatrix4x4* matrix);
  vtkMatrix4x4* matrix()const;

public slots:
  /// 
  /// Triggered upon VTK transform modified event
  void updateMatrix();

protected:
  vtkWeakPointer<vtkMatrix4x4> Matrix;
};

#endif 
