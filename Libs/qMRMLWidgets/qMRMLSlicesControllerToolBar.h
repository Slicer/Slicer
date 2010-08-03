#ifndef __qMRMLSlicesControllerToolBar_h
#define __qMRMLSlicesControllerToolBar_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include "qMRMLWidgetsExport.h"

class qMRMLSlicesControllerToolBarPrivate;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLSlicesControllerToolBar : public QToolBar
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QToolBar Superclass;

  /// Constructors
  explicit qMRMLSlicesControllerToolBar(QWidget* parent = 0);
  virtual ~qMRMLSlicesControllerToolBar(){}

  vtkMRMLScene* mrmlScene()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);
  void setFiducialPointsVisible(bool visible);
  void setFiducialLabelsVisible(bool visible);
  void setForegroundGridVisible(bool visible);
  void setBackgroundGridVisible(bool visible);
  void setLabelGridVisible(bool visible);
  void fitToWindow();
  void setNavigatorVisible(bool visible);
  void setSliceIntersectionVisible(bool visible);
  void toggleBackgroundForeground();
  void showBackground();
  void showForeground();

protected slots:
  void onAnnotationTriggered(QAction*);
  void onCompositingActionTriggered(QAction*);
  void onCrosshairActionTriggered(QAction*);
  void onCrosshairThicknessActionTriggered(QAction*);
  void onSpatialUnitsActionTriggered(QAction*);

private:
  CTK_DECLARE_PRIVATE(qMRMLSlicesControllerToolBar);
};

#endif
