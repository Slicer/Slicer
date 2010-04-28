#include "qSlicermiAnnotationModuleScreenShotDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>


qSlicermiAnnotationModuleScreenShotDialog::qSlicermiAnnotationModuleScreenShotDialog()
{
    ui.setupUi(this);
    createConnection();

}

void qSlicermiAnnotationModuleScreenShotDialog::createConnection()
{
    connect(ui.saveScreenShotButton,   SIGNAL(clicked()), this, SLOT(onSaveScreenShotButtonClicked()));
    connect(ui.cancelScreenShotButton, SIGNAL(clicked()), this, SLOT(onCancelScreenShotButtonClicked()) );


}

Ui::qSlicermiAnnotationModuleScreenShotDialog qSlicermiAnnotationModuleScreenShotDialog::getScreenShotDialogUi()
{
    return ui;
}

QString qSlicermiAnnotationModuleScreenShotDialog::getFileName()
{
    return m_filename;
}

void qSlicermiAnnotationModuleScreenShotDialog::onSaveScreenShotButtonClicked()
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

void qSlicermiAnnotationModuleScreenShotDialog::onCancelScreenShotButtonClicked()
{
    this->close();    
}
