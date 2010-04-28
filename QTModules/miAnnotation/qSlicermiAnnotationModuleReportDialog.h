#ifndef __qSlicermiAnnotationModuleReportDialog_h
#define __qSlicermiAnnotationModuleReportDialog_h

#include "ui_qSlicermiAnnotationModuleReportDialog.h"

class qSlicermiAnnotationModuleReportDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicermiAnnotationModuleReportDialog();
  ~qSlicermiAnnotationModuleReportDialog(){};

  Ui::qSlicermiAnnotationModuleReportDialog getReportDialogUi();
  QString getFileName();


protected:
  
protected slots:
  // Description:
  // obsolete
    void onSaveReportButtonClicked();
    void onCancelReportButtonClicked();


signals:
    void filenameSelected();

private:

    Ui::qSlicermiAnnotationModuleReportDialog ui;
    void createConnection();

    QString m_filename;

};

#endif
