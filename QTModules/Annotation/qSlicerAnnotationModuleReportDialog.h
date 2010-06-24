#ifndef __qSlicerAnnotationModuleReportDialog_h
#define __qSlicerAnnotationModuleReportDialog_h

#include "ui_qSlicerAnnotationModuleReportDialog.h"

class qSlicerAnnotationModuleReportDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerAnnotationModuleReportDialog();
  ~qSlicerAnnotationModuleReportDialog(){};

  Ui::qSlicerAnnotationModuleReportDialog getReportDialogUi();
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

    Ui::qSlicerAnnotationModuleReportDialog ui;
    void createConnection();

    QString m_filename;

};

#endif
