#ifndef __qSlicerAnnotationModuleScreenShotDialog_h
#define __qSlicerAnnotationModuleScreenShotDialog_h

#include "ui_qSlicerAnnotationModuleScreenShotDialog.h"

class qSlicerAnnotationModuleScreenShotDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerAnnotationModuleScreenShotDialog();
  ~qSlicerAnnotationModuleScreenShotDialog(){};

  Ui::qSlicerAnnotationModuleScreenShotDialog getScreenShotDialogUi();
  QString getFileName();


protected:
  
protected slots:
  // Description:
  // obsolete
    void onSaveScreenShotButtonClicked();
    void onCancelScreenShotButtonClicked();


signals:
    void filenameSelected();

private:

    Ui::qSlicerAnnotationModuleScreenShotDialog ui;
    void createConnection();

    QString m_filename;

};

#endif
