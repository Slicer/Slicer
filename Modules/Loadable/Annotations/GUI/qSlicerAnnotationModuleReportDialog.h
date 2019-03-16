#ifndef __qSlicerAnnotationModuleReportDialog_h
#define __qSlicerAnnotationModuleReportDialog_h

#include "ui_qSlicerAnnotationModuleReportDialog.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include "vtkCollection.h"

/// \ingroup Slicer_QtModules_Annotation
class qSlicerAnnotationModuleReportDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerAnnotationModuleReportDialog();
  ~qSlicerAnnotationModuleReportDialog() override;

  Ui::qSlicerAnnotationModuleReportDialog getReportDialogUi();

  /// Set the Annotation module logic.
  void setLogic(vtkSlicerAnnotationModuleLogic* logic);

  /// Set a collection of Annotations to be included in the report
  void setAnnotations(vtkCollection* collection);

  /// Update the report
  void updateReport();

protected:


protected slots:
  void onDialogRejected();
  void onDialogAccepted();
  void onTextEdited();
  void onPrintButtonClicked();

signals:
  void dialogRejected();
  void dialogAccepted();

private:

    Ui::qSlicerAnnotationModuleReportDialog ui;
    void createConnection();

    QString generateReport();
    void generateReportRecursive(int level, vtkMRMLAnnotationHierarchyNode* currentHierarchy);

    // check if a mrmlId is part of the selected annotations
    bool isSelected(const char* mrmlId);

    bool saveReport();

    vtkSlicerAnnotationModuleLogic* m_Logic;

    // the selected annotations
    vtkCollection* m_Annotations;
    // check if a mrmlId is part of the selected annotations
    bool isAnnotationSelected(const char* mrmlId);

    QString m_Html;

};

#endif
