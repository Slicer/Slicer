#ifndef __qSlicerSceneViewsModuleDialog_h
#define __qSlicerSceneViewsModuleDialog_h

#include "ui_qSlicerSceneViewsModuleDialog.h"
#include <vtkSlicerSceneViewsModuleLogic.h>

/// \ingroup Slicer_QtModules_SceneViews
class qSlicerSceneViewsModuleDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerSceneViewsModuleDialog();
  ~qSlicerSceneViewsModuleDialog();

  /// Set the Annotation module logic.
  void setLogic(vtkSlicerSceneViewsModuleLogic* logic);

  /// Initialize this dialog with values from an existing annotation Snapshot node.
  void initialize(const QString& nodeId);

  /// Reset the dialog.
  void reset();

protected:

protected slots:

  void onDialogAccepted();

  /// Slots for the four radio buttons to select screenShot area
  void onThreeDViewRadioClicked();
  void onRedSliceViewRadioClicked();
  void onYellowSliceViewRadioClicked();
  void onGreenSliceViewRadioClicked();
  void onFullLayoutRadioClicked();

  /// Slots for the restore button
  void onRestoreButtonClicked();

private:

    Ui::qSlicerSceneViewsModuleDialog ui;
    void createConnection();

    /// Grab a screenshot of the 3DView or any sliceView.
    /// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for a ThreeDView
    void grabScreenShot(QString screenshotWindow);

    vtkSlicerSceneViewsModuleLogic* m_Logic;

    vtkImageData* m_vtkImageData;

    /// The ID of the associated snapshot node.
    /// This is NULL if the dialog has no associated snapshot node (== new snapshot mode).
    vtkStdString m_Id;

};

#endif
