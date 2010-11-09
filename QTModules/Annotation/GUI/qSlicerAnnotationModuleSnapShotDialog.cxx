#include "qSlicerAnnotationModuleSnapShotDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>


qSlicerAnnotationModuleSnapShotDialog::qSlicerAnnotationModuleSnapShotDialog()
{
    ui.setupUi(this);
    createConnection();

}

void qSlicerAnnotationModuleSnapShotDialog::createConnection()
{
//    connect(ui.saveScreenShotButton,   SIGNAL(clicked()), this, SLOT(onSaveScreenShotButtonClicked()));
//    connect(ui.cancelScreenShotButton, SIGNAL(clicked()), this, SLOT(onCancelScreenShotButtonClicked()) );


}

