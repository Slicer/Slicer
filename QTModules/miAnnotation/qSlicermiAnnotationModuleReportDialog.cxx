#include "qSlicermiAnnotationModuleReportDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>


qSlicermiAnnotationModuleReportDialog::qSlicermiAnnotationModuleReportDialog()
{
    ui.setupUi(this);
    createConnection();

}

void qSlicermiAnnotationModuleReportDialog::createConnection()
{
    connect(ui.saveReportButton,   SIGNAL(clicked()), this, SLOT(onSaveReportButtonClicked()));
    connect(ui.cancelReportButton, SIGNAL(clicked()), this, SLOT(onCancelReportButtonClicked()) );


}

Ui::qSlicermiAnnotationModuleReportDialog qSlicermiAnnotationModuleReportDialog::getReportDialogUi()
{
    return ui;
}

QString qSlicermiAnnotationModuleReportDialog::getFileName()
{
    return m_filename;
}

void qSlicermiAnnotationModuleReportDialog::onSaveReportButtonClicked()
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

void qSlicermiAnnotationModuleReportDialog::onCancelReportButtonClicked()
{
    this->close();    
}
