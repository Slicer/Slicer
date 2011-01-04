#ifndef __qSlicerSceneViewsModule_h
#define __qSlicerSceneViewsModule_h

// SlicerQT includes
#include "qSlicerLoadableModule.h"

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerSceneViewsModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerSceneViewsModulePrivate;

class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qSlicerSceneViewsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule)
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerSceneViewsModule(QObject *parent=0);
  virtual ~qSlicerSceneViewsModule();

  ///
  /// Return the help and acknowledgement text for the SceneViews module.
  virtual QString helpText()const ;
  virtual QString acknowledgementText()const;

  ///
  /// Return the icon of the SceneViews module.
  virtual QIcon icon()const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  ///
  /// All initialization code should be done in the setup
  virtual void setup();

  // Description:
  // Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  // Description:
  // Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerSceneViewsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSceneViewsModule);
  Q_DISABLE_COPY(qSlicerSceneViewsModule);
};

#endif
