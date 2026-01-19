/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Steve Pieper, Isomics Inc.
  and was partially funded by NSF grant DBI 1759883

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
# include <QWebEngineDownloadRequest>
#else
# include <QWebEngineDownloadItem>
#endif

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerDataDialog.h"
#include "qSlicerWebDownloadWidget.h"

// --------------------------------------------------------------------------
qSlicerWebDownloadWidget::qSlicerWebDownloadWidget(QWidget* parent)
  : QDialog(parent)
{
}

// --------------------------------------------------------------------------
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void qSlicerWebDownloadWidget::handleDownload(QWebEngineDownloadRequest* download)
#else
void qSlicerWebDownloadWidget::handleDownload(QWebEngineDownloadItem* download)
#endif
{
  // need to use a modal dialog here because 'download' will be deleted
  // if we don't return 'accept from this slot
  QMessageBox* messageBox = new QMessageBox(this);
  messageBox->setWindowTitle(tr("Web download"));
  messageBox->setText(tr("Load data into Slicer or download to separate location."));
  QPushButton* loadButton = messageBox->addButton(tr("Load..."), QMessageBox::AcceptRole);
  messageBox->setDefaultButton(loadButton);
  loadButton->setToolTip(tr("Download the data and open in Slicer"));
  QPushButton* saveButton = messageBox->addButton(tr("Save..."), QMessageBox::AcceptRole);
  saveButton->setToolTip(tr("Save to an arbitrary location"));
  QPushButton* abortlButton = messageBox->addButton(tr("Cancel"), QMessageBox::AcceptRole);

  this->hide();
  messageBox->exec();

  if (messageBox->clickedButton() == loadButton)
  {
    // start the download into Slicer's temp directory
    qDebug() << "Load...";
    QString fileName = download->downloadFileName();
    QDir directory = QDir(qSlicerCoreApplication::application()->temporaryPath());
    download->setDownloadFileName(fileName);
    download->setDownloadDirectory(directory.absolutePath());
  }
  else if (messageBox->clickedButton() == saveButton)
  {
    qDebug() << "Save...";
    QString filePath = QFileInfo(download->downloadDirectory(), download->downloadFileName()).absoluteFilePath();
    filePath = QFileDialog::getSaveFileName(this, tr("Save File"), filePath);
    if (filePath.isEmpty())
    {
      download->cancel();
      return;
    }
    download->setDownloadFileName(QFileInfo(filePath).fileName());
    download->setDownloadDirectory(QFileInfo(filePath).absoluteDir().absolutePath());
    this->show();
  }
  else if (messageBox->clickedButton() == abortlButton)
  {
    qDebug() << "Cancel download...";
    download->cancel();
    return;
  }
  qDebug() << "Saving to " << QFileInfo(download->downloadDirectory(), download->downloadFileName()).absoluteFilePath();
  download->accept();
  this->show();

  //
  // setup progress and cancel UI and callbacks
  //
  QVBoxLayout* layout = new QVBoxLayout();
  this->setLayout(layout);

  QLabel* label = new QLabel();
  label->setText(tr("Downloading %1").arg(download->downloadFileName()));
  layout->addWidget(label);

  QProgressBar* progressBar = new QProgressBar();
  layout->addWidget(progressBar);

  QWidget* buttons = new QWidget(this);
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttons->setLayout(buttonLayout);
  layout->addWidget(buttons);

  QPushButton* cancelButton = new QPushButton(tr("Cancel"));
  buttonLayout->addWidget(cancelButton);

  // Progress
  connect(download,
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
          &QWebEngineDownloadRequest::receivedBytesChanged,
          [download, progressBar]()
          {
            auto bytesTotal = download->totalBytes();
            auto bytesReceived = download->receivedBytes();
#else
          &QWebEngineDownloadItem::downloadProgress,
          [=](qint64 bytesReceived, qint64 bytesTotal)
          {
#endif
            progressBar->setRange(0, bytesTotal);
            progressBar->setValue(bytesReceived);
          });

  // Cancel
  connect(cancelButton,
          &QPushButton::clicked,
          [=]()
          {
            qDebug() << "Download canceled";
            download->cancel();
            this->hide();
          });

  // Finish
  connect(download,
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
          &QWebEngineDownloadRequest::isFinishedChanged,
#else
          &QWebEngineDownloadItem::finished,
#endif
          [=]()
          {
            this->hide();
            if (messageBox->clickedButton() == loadButton)
            {
              qSlicerDataDialog* dataDialog = new qSlicerDataDialog(this->parent());
              dataDialog->addFile(QFileInfo(download->downloadDirectory(), download->downloadFileName()).absoluteFilePath());
              dataDialog->exec();
            }
            else
            {
              QMessageBox::information(this, tr("Web download"), tr("Download complete"));
            }
          });
}
