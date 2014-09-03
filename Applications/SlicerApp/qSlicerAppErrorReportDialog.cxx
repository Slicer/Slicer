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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QUrl>

// SlicerApp includes
#include "qSlicerAppErrorReportDialog.h"
#include "qSlicerApplication.h"
#include "ui_qSlicerAppErrorReportDialog.h"

//-----------------------------------------------------------------------------
class qSlicerAppErrorReportDialogPrivate: public Ui_qSlicerAppErrorReportDialog
{
public:
};

//-----------------------------------------------------------------------------
// qSlicerAppErrorReportDialogPrivate methods

//-----------------------------------------------------------------------------
// qSlicerAppErrorReportDialog methods
qSlicerAppErrorReportDialog::qSlicerAppErrorReportDialog(QWidget* parentWidget)
 :QDialog(parentWidget)
  , d_ptr(new qSlicerAppErrorReportDialogPrivate)
{
  Q_D(qSlicerAppErrorReportDialog);
  d->setupUi(this);

  QStringList logFilePaths = qSlicerApplication::application()->recentLogFiles();
  d->RecentLogFilesComboBox->addItems(logFilePaths);

  QObject::connect(d->RecentLogFilesComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLogFileSelectionChanged()));
  QObject::connect(d->LogCopyToClipboardPushButton, SIGNAL(clicked()), this, SLOT(onLogCopy()));
  QObject::connect(d->LogFileOpenPushButton, SIGNAL(clicked()), this, SLOT(onLogFileOpen()));

  connect(d->ButtonBox, SIGNAL(rejected()), this, SLOT(close()));

  onLogFileSelectionChanged(); // update log messages textbox

}

//-----------------------------------------------------------------------------
qSlicerAppErrorReportDialog::~qSlicerAppErrorReportDialog()
{
}

// --------------------------------------------------------------------------
void qSlicerAppErrorReportDialog::onLogCopy()
{
  Q_D(qSlicerAppErrorReportDialog);
  QApplication::clipboard()->setText(d->LogText->toPlainText());
}

// --------------------------------------------------------------------------
void qSlicerAppErrorReportDialog::onLogFileSelectionChanged()
{
  Q_D(qSlicerAppErrorReportDialog);
  QFile f(d->RecentLogFilesComboBox->currentText());
  if (f.open(QFile::ReadOnly | QFile::Text))
    {
    QTextStream in(&f);
    QString logText = in.readAll();
    d->LogText->setPlainText(logText);
    }
  else
    {
    d->LogText->clear();
    }
}

// --------------------------------------------------------------------------
void qSlicerAppErrorReportDialog::onLogFileOpen()
{
  Q_D(qSlicerAppErrorReportDialog);
  QDesktopServices::openUrl(QUrl("file:///"+d->RecentLogFilesComboBox->currentText(), QUrl::TolerantMode));
}
