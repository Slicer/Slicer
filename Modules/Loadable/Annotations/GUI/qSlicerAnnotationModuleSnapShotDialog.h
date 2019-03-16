#ifndef __qSlicerAnnotationModuleSnapShotDialog_h
#define __qSlicerAnnotationModuleSnapShotDialog_h

class vtkSlicerAnnotationModuleLogic;

#include "qMRMLScreenShotDialog.h"

/// \ingroup Slicer_QtModules_Annotation
class qSlicerAnnotationModuleSnapShotDialog : public qMRMLScreenShotDialog
{
  Q_OBJECT
public:
  typedef qMRMLScreenShotDialog Superclass;
  qSlicerAnnotationModuleSnapShotDialog(QWidget* parent = nullptr);
  ~qSlicerAnnotationModuleSnapShotDialog() override;

  /// Set the Annotation module logic.
  void setLogic(vtkSlicerAnnotationModuleLogic* logic);

  /// Initialize this dialog with values from an existing annotation Snapshot node.
  void loadNode(const char* nodeId);
  /// Reset the dialog and give it a unique name.
  void reset();

  void accept() override;

private:
    vtkSlicerAnnotationModuleLogic* m_Logic;
};

#endif
