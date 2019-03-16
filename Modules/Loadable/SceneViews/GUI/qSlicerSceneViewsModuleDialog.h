#ifndef __qSlicerSceneViewsModuleDialog_h
#define __qSlicerSceneViewsModuleDialog_h

#include <vtkSlicerSceneViewsModuleLogic.h>

#include "qMRMLScreenShotDialog.h"

/// \ingroup Slicer_QtModules_SceneViews
class qSlicerSceneViewsModuleDialog : public qMRMLScreenShotDialog
{
  Q_OBJECT
public:
  typedef qMRMLScreenShotDialog Superclass;

  qSlicerSceneViewsModuleDialog();
  ~qSlicerSceneViewsModuleDialog() override;

  /// Set the SceneViews module logic.
  void setLogic(vtkSlicerSceneViewsModuleLogic* logic);

  /// Initialize this dialog with values from an existing annotation Snapshot node.
  void loadNode(const QString& nodeId);
  /// Reset the dialog and give it a unique name.
  void reset();

  void accept() override;

private:
    vtkSlicerSceneViewsModuleLogic* m_Logic;
};

#endif
