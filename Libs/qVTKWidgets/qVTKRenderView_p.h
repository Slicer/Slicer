#ifndef __qVTKRenderView_p_h
#define __qVTKRenderView_p_h

/// qVTK includes
#include "qVTKObject.h"
#include "qVTKRenderView.h"

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QObject>

/// VTK includes
#include <QVTKWidget.h>
#include <vtkAxesActor.h>
#include <vtkCornerAnnotation.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkRenderWindowInteractor;

//-----------------------------------------------------------------------------
class qVTKRenderViewPrivate : public QObject,
                              public qCTKPrivate<qVTKRenderView>
{
  Q_OBJECT
  QCTK_DECLARE_PUBLIC(qVTKRenderView);
public:
  qVTKRenderViewPrivate();

  /// Convenient setup methods
  void setupCornerAnnotation();
  void setupRendering();
  void setupDefaultInteractor();

  QVTKWidget*                                   VTKWidget;
  vtkSmartPointer<vtkRenderer>                  Renderer;
  vtkSmartPointer<vtkRenderWindow>              RenderWindow;
  bool                                          RenderPending;
  
  vtkSmartPointer<vtkAxesActor>                 Axes;
  vtkSmartPointer<vtkOrientationMarkerWidget>   Orientation;
  vtkSmartPointer<vtkCornerAnnotation>          CornerAnnotation;

  vtkWeakPointer<vtkRenderWindowInteractor>     CurrentInteractor;

};

#endif
