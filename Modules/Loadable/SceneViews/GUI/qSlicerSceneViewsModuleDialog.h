#ifndef __qSlicerSceneViewsModuleDialog_h
#define __qSlicerSceneViewsModuleDialog_h

#include <vtkSlicerSceneViewsModuleLogic.h>

#include "qMRMLScreenShotDialog.h"


class qSlicerSceneViewsModuleDialogPrivate;

class qSlicerSceneViewsModuleDialog : public qMRMLScreenShotDialog
{
  Q_OBJECT
public:
  typedef qMRMLScreenShotDialog Superclass;

  qSlicerSceneViewsModuleDialog(QWidget* parent = nullptr);
  ~qSlicerSceneViewsModuleDialog() override;

  /// Set the SceneViews module logic.
  void setLogic(vtkSlicerSceneViewsModuleLogic* logic);

  /// Initialize this dialog with values from an existing scene view.
  void loadSceneViewInfo(int index);
  /// Reset the dialog and give it a unique name.
  void reset();

  void accept() override;

private:
    Q_DECLARE_PRIVATE(qSlicerSceneViewsModuleDialog);

    vtkSlicerSceneViewsModuleLogic* m_Logic;

    QScopedPointer<qSlicerSceneViewsModuleDialogPrivate> d_ptr;
};

#endif
