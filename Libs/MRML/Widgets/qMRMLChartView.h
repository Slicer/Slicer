



#ifndef __qMRMLChartView_h
#define __qMRMLChartView_h

// Qt includes
#include <QWebView>

#include "qMRMLWidgetsExport.h"

class qMRMLChartViewPrivate;
class vtkMRMLScene;
class vtkMRMLChartViewNode;

class QMRML_WIDGETS_EXPORT qMRMLChartView : public QWebView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWebView Superclass;
  
  /// Constructors
  explicit qMRMLChartView(QWidget* parent = 0);
  virtual ~qMRMLChartView();

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 3D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 3D
  /// views (existing or future), you need to do it via
  /// vtkMRMLChartViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkMRMLCameraDisplayableManager,
  /// vtkMRMLViewDisplayableManager and vtkMRMLModelDisplayableManager are
  /// already registered.
  void addDisplayableManager(const QString& displayableManager);

  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// Get the 3D View node observed by view.
  //vtkMRMLViewNode* mrmlViewNode()const;
  vtkMRMLChartViewNode* mrmlChartViewNode()const;

  // /// Returns the interactor style of the view
  // vtkInteractorObserver* interactorStyle()const;

  // Redefine the sizeHint so layouts work properly.
  virtual QSize sizeHint() const;

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode);

signals:
  /// When designing custom qMRMLWidget in the designer, you can connect the
  /// mrmlSceneChanged signal directly to the aggregated MRML widgets that
  /// have a setMRMLScene slot.
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLChartViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLChartView);
  Q_DISABLE_COPY(qMRMLChartView);
};

#endif
