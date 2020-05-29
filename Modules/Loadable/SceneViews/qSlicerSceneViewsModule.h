#ifndef __qSlicerSceneViewsModule_h
#define __qSlicerSceneViewsModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerSceneViewsModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerSceneViewsModulePrivate;

/// \ingroup Slicer_QtModules_SceneViews
class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qSlicerSceneViewsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerSceneViewsModule(QObject *parent=nullptr);
  ~qSlicerSceneViewsModule() override;

  /// Return the help and acknowledgement text for the SceneViews module.
  QString helpText()const override ;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  /// Return the icon of the SceneViews module.
  QIcon icon()const override;
  QStringList categories()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

public slots:
    /// a public slot to open up the scene view capture
    /// dialog (get the module manager, get the module sceneviews, get the
    /// widget representation, then invoke this method
    /// \sa qSlicerIOManager::openSceneViewsDialog
    void showSceneViewDialog();

protected:

  /// All initialization code should be done in the setup
  void setup() override;

  // Description:
  // Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override;

  // Description:
  // Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerSceneViewsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSceneViewsModule);
  Q_DISABLE_COPY(qSlicerSceneViewsModule);
};

#endif
