#ifndef __qSlicerAnnotationModuleReportDialog_h
#define __qSlicerAnnotationModuleReportDialog_h

#include "ui_qSlicerAnnotationModuleReportDialog.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include "vtkCollection.h"

class qSlicerAnnotationModuleReportDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerAnnotationModuleReportDialog();
  ~qSlicerAnnotationModuleReportDialog();

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
    bool saveReport();

    vtkSlicerAnnotationModuleLogic* m_Logic;

    vtkCollection* m_Annotations;

};

#endif
