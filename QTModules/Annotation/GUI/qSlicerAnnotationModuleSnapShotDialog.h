#ifndef __qSlicerAnnotationModuleSnapShotDialog_h
#define __qSlicerAnnotationModuleSnapShotDialog_h

#include "ui_qSlicerAnnotationModuleSnapShotDialog.h"

class qSlicerAnnotationModuleSnapShotDialog : public QDialog
{
  Q_OBJECT
public:
  qSlicerAnnotationModuleSnapShotDialog();
  ~qSlicerAnnotationModuleSnapShotDialog(){};

protected:
  /*
protected slots:

signals:
*/

private:

    Ui::qSlicerAnnotationModuleSnapShotDialog ui;
    void createConnection();

    QString m_filename;

};

#endif
