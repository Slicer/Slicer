#ifndef __qVTKRenderView_h
#define __qVTKRenderView_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QWidget>

#include "qVTKWidgetsExport.h"

class qVTKRenderViewPrivate;
class vtkRenderWindowInteractor;

class QVTK_WIDGETS_EXPORT qVTKRenderView : public QWidget
{
  Q_OBJECT
public:
  // Constructors
  typedef QWidget   Superclass;
  explicit qVTKRenderView(QWidget* parent = 0);
  virtual ~qVTKRenderView();

  // If a render has already been scheduled, this called is a no-op
  void scheduleRender();

  // Force a render even if a render is already ocurring
  void forceRender();

  // Set/Get window interactor
  vtkRenderWindowInteractor* interactor()const;
  void setInteractor(vtkRenderWindowInteractor* newInteractor);

  // Set corner annotation text
  void setCornerAnnotationText(const QString& text);

  // Set background color
  void setBackgroundColor(double r, double g, double b);

  void resetCamera();

  //virtual void setCornerText(const QString& text);
  
private:
  QCTK_DECLARE_PRIVATE(qVTKRenderView);
}; 

#endif
