#include "qSlicerAnnotationModuleScreenShotDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>


qSlicerAnnotationModuleScreenShotDialog::qSlicerAnnotationModuleScreenShotDialog()
{
    ui.setupUi(this);
    createConnection();

}

void qSlicerAnnotationModuleScreenShotDialog::createConnection()
{
    connect(ui.saveScreenShotButton,   SIGNAL(clicked()), this, SLOT(onSaveScreenShotButtonClicked()));
    connect(ui.cancelScreenShotButton, SIGNAL(clicked()), this, SLOT(onCancelScreenShotButtonClicked()) );


}

Ui::qSlicerAnnotationModuleScreenShotDialog qSlicerAnnotationModuleScreenShotDialog::getScreenShotDialogUi()
{
    return ui;
}

QString qSlicerAnnotationModuleScreenShotDialog::getFileName()
{
    return m_filename;
}

void qSlicerAnnotationModuleScreenShotDialog::onSaveScreenShotButtonClicked()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save ScreenShot",
        QDir::currentPath(),
        "ScreenShot (*.png)");

    // save the documents...
    if ( !filename.isNull())
    {
        m_filename = filename;
        emit filenameSelected();
    }

}

void qSlicerAnnotationModuleScreenShotDialog::onCancelScreenShotButtonClicked()
{
    this->close();    
}
