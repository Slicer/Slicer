#include "qSlicerAnnotationModuleReportDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>


qSlicerAnnotationModuleReportDialog::qSlicerAnnotationModuleReportDialog()
{
    ui.setupUi(this);
    createConnection();

}

void qSlicerAnnotationModuleReportDialog::createConnection()
{
    connect(ui.saveReportButton,   SIGNAL(clicked()), this, SLOT(onSaveReportButtonClicked()));
    connect(ui.cancelReportButton, SIGNAL(clicked()), this, SLOT(onCancelReportButtonClicked()) );


}

Ui::qSlicerAnnotationModuleReportDialog qSlicerAnnotationModuleReportDialog::getReportDialogUi()
{
    return ui;
}

QString qSlicerAnnotationModuleReportDialog::getFileName()
{
    return m_filename;
}

void qSlicerAnnotationModuleReportDialog::onSaveReportButtonClicked()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Report",
        QDir::currentPath(),
        "Reports (*.html)");

    // save the documents...
    if ( !filename.isNull())
    {
        m_filename = filename;
        emit filenameSelected();
    }

}

void qSlicerAnnotationModuleReportDialog::onCancelReportButtonClicked()
{
    this->close();    
}
