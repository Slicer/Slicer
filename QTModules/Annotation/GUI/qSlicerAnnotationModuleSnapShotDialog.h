#ifndef __qSlicerAnnotationModuleSnapShotDialog_h
#define __qSlicerAnnotationModuleSnapShotDialog_h

#include "ui_qSlicerAnnotationModuleSnapShotDialog.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

class qSlicerAnnotationModuleSnapShotDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerAnnotationModuleSnapShotDialog();
  ~qSlicerAnnotationModuleSnapShotDialog();

  /// Set the Annotation module logic.
  void setLogic(vtkSlicerAnnotationModuleLogic* logic);

  /// Reset the dialog.
  void reset();

protected:

protected slots:

  void onDialogRejected();
  void onDialogAccepted();

  /// Slots for the four radio buttons to select screenShot area
  void onThreeDViewRadioClicked();
  void onRedSliceViewRadioClicked();
  void onYellowSliceViewRadioClicked();
  void onGreenSliceViewRadioClicked();

signals:
  void dialogRejected();
  void dialogAccepted();

private:

    Ui::qSlicerAnnotationModuleSnapShotDialog ui;
    void createConnection();

    /// Grab a screenshot of the 3DView or any sliceView.
    /// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for a ThreeDView
    void grabScreenShot(QString screenshotWindow);

    vtkSlicerAnnotationModuleLogic* m_Logic;

    vtkImageData* m_vtkImageData;

};

#endif
