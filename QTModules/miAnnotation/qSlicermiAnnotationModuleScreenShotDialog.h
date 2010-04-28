#ifndef __qSlicermiAnnotationModuleScreenShotDialog_h
#define __qSlicermiAnnotationModuleScreenShotDialog_h

#include "ui_qSlicermiAnnotationModuleScreenShotDialog.h"

class qSlicermiAnnotationModuleScreenShotDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicermiAnnotationModuleScreenShotDialog();
  ~qSlicermiAnnotationModuleScreenShotDialog(){};

  Ui::qSlicermiAnnotationModuleScreenShotDialog getScreenShotDialogUi();
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

    Ui::qSlicermiAnnotationModuleScreenShotDialog ui;
    void createConnection();

    QString m_filename;

};

#endif
