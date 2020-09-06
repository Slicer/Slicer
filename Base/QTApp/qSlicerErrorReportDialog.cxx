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
#include "qSlicerErrorReportDialog.h"
#include "qSlicerApplication.h"
#include "ui_qSlicerErrorReportDialog.h"

//-----------------------------------------------------------------------------
class qSlicerErrorReportDialogPrivate: public Ui_qSlicerErrorReportDialog
{
public:
};

//-----------------------------------------------------------------------------
// qSlicerErrorReportDialogPrivate methods

//-----------------------------------------------------------------------------
// qSlicerErrorReportDialog methods
qSlicerErrorReportDialog::qSlicerErrorReportDialog(QWidget* parentWidget)
 :QDialog(parentWidget)
  , d_ptr(new qSlicerErrorReportDialogPrivate)
{
  Q_D(qSlicerErrorReportDialog);
  d->setupUi(this);

  QString instructionsText = d->InstructionsLabel->text();
  QString appNameVersionPlatform = QString("%1 %2 %3").arg(
    qSlicerApplication::application()->applicationName()).arg(
    qSlicerApplication::application()->applicationVersion()).arg(
    qSlicerApplication::application()->platform());
  instructionsText.replace(QString("[appname-version-platform]"), QUrl::toPercentEncoding(appNameVersionPlatform));
  d->InstructionsLabel->setText(instructionsText);

  QStringList logFilePaths = qSlicerApplication::application()->recentLogFiles();
  d->RecentLogFilesComboBox->addItems(logFilePaths);

  QObject::connect(d->RecentLogFilesComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLogFileSelectionChanged()));
  QObject::connect(d->LogCopyToClipboardPushButton, SIGNAL(clicked()), this, SLOT(onLogCopy()));
  QObject::connect(d->LogFileOpenPushButton, SIGNAL(clicked()), this, SLOT(onLogFileOpen()));
  QObject::connect(d->LogFileEditCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLogFileEditClicked(bool)));

  connect(d->ButtonBox, SIGNAL(rejected()), this, SLOT(close()));

  onLogFileSelectionChanged(); // update log messages textbox

}

//-----------------------------------------------------------------------------
qSlicerErrorReportDialog::~qSlicerErrorReportDialog() = default;

// --------------------------------------------------------------------------
void qSlicerErrorReportDialog::onLogCopy()
{
  Q_D(qSlicerErrorReportDialog);
  QApplication::clipboard()->setText(d->LogText->toPlainText());
}

// --------------------------------------------------------------------------
void qSlicerErrorReportDialog::onLogFileSelectionChanged()
{
  Q_D(qSlicerErrorReportDialog);
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
void qSlicerErrorReportDialog::onLogFileOpen()
{
  Q_D(qSlicerErrorReportDialog);
  QDesktopServices::openUrl(QUrl("file:///"+d->RecentLogFilesComboBox->currentText(), QUrl::TolerantMode));
}

// --------------------------------------------------------------------------
void qSlicerErrorReportDialog::onLogFileEditClicked(bool editable)
{
  Q_D(qSlicerErrorReportDialog);
  d->LogText->setReadOnly(!editable);
}
