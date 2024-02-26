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
#include <QFileInfo>
#include <QStyle>
#include <QDateTime>

// SlicerApp includes
#include "qSlicerErrorReportDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "ui_qSlicerErrorReportDialog.h"

//-----------------------------------------------------------------------------
class qSlicerErrorReportDialogPrivate : public Ui_qSlicerErrorReportDialog
{
public:
};

//-----------------------------------------------------------------------------
// qSlicerErrorReportDialogPrivate methods

//-----------------------------------------------------------------------------
// qSlicerErrorReportDialog methods
qSlicerErrorReportDialog::qSlicerErrorReportDialog(QWidget* parentWidget)
  : QDialog(parentWidget)
  , d_ptr(new qSlicerErrorReportDialogPrivate)
{
  Q_D(qSlicerErrorReportDialog);
  d->setupUi(this);

  QString instructionsText = d->InstructionsLabel->text();
  QString appNameVersionPlatform = QString("%1 %2 %3")
                                     .arg(qSlicerApplication::application()->applicationName())
                                     .arg(qSlicerApplication::application()->applicationVersion())
                                     .arg(qSlicerApplication::application()->platform());
  instructionsText.replace(QString("[appname-version-platform]"), QUrl::toPercentEncoding(appNameVersionPlatform));
  d->InstructionsLabel->setText(instructionsText);

  QStringList headers;
  headers << "App Name"
          << "Version"
          << "Revision"
          << "Date"
          << "Time"
          << "File Path";
  d->RecentLogFilesComboBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
  d->RecentLogFilesComboBox->setColumnCount(6);
  d->RecentLogFilesComboBox->setHorizontalHeaderLabels(headers);
  for (int i = 0; i < d->RecentLogFilesComboBox->columnCount(); i++)
  {
    d->RecentLogFilesComboBox->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
  }
  QLocale locale = qSlicerCoreApplication::application()->applicationLocale();
  QStringList logFilePaths = qSlicerApplication::application()->recentLogFiles();
  foreach (const QString& path, logFilePaths)
  {
    QTableWidgetItem* itemApp = new QTableWidgetItem(path);
    QTableWidgetItem* itemVersion = new QTableWidgetItem(path);
    QTableWidgetItem* itemRevision = new QTableWidgetItem(path);
    QTableWidgetItem* itemDate = new QTableWidgetItem(path);
    QTableWidgetItem* itemTime = new QTableWidgetItem(path);
    QTableWidgetItem* itemPath = new QTableWidgetItem(path);

    QVariant fileString(path);
    QFileInfo fi(path);
    QString fileName = fi.fileName();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList stringList = fileName.split("_", Qt::SkipEmptyParts);
#else
    QStringList stringList = fileName.split("_", QString::SkipEmptyParts);
#endif
    itemApp->setText(stringList.at(0));
    itemApp->setData(Qt::UserRole, fileString);
    if (stringList.size() >= 6) // compatibility for log files with and without app version in filename
    {
      QDateTime dt = QDateTime::fromString(QString(stringList.at(3)), "yyyyMMdd");
      QDateTime tm = QDateTime::fromString(QString(stringList.at(4)), "hhmmss");
      itemVersion->setText(stringList.at(1));
      itemRevision->setText(stringList.at(2));
      itemDate->setText(locale.toString(dt, "ddd yyyy-MM-dd"));
      itemTime->setText(locale.toString(tm, "hh:mm:ss"));
    }
    else
    {
      QDateTime dt = QDateTime::fromString(QString(stringList.at(2)), "yyyyMMdd");
      QDateTime tm = QDateTime::fromString(QString(stringList.at(3)), "hhmmss");
      itemVersion->setText(QString("unknown"));
      itemRevision->setText(stringList.at(1));
      itemDate->setText(locale.toString(dt, "ddd yyyy-MM-dd"));
      itemTime->setText(locale.toString(tm, "hh:mm:ss"));
    }
    itemPath->setText(path);
    itemRevision->setToolTip(path);
    itemDate->setToolTip(path);
    itemTime->setToolTip(path);
    itemApp->setToolTip(path);
    itemVersion->setToolTip(path);

    // Create a new entry
    int row = d->RecentLogFilesComboBox->rowCount();
    d->RecentLogFilesComboBox->insertRow(row);
    d->RecentLogFilesComboBox->setItem(row, 0, itemApp);
    d->RecentLogFilesComboBox->setItem(row, 1, itemVersion);
    d->RecentLogFilesComboBox->setItem(row, 2, itemRevision);
    d->RecentLogFilesComboBox->setItem(row, 3, itemDate);
    d->RecentLogFilesComboBox->setItem(row, 4, itemTime);
    d->RecentLogFilesComboBox->setItem(row, 5, itemPath);
  }
  if (d->RecentLogFilesComboBox->rowCount() > 0)
  {
    d->RecentLogFilesComboBox->setCurrentIndex(d->RecentLogFilesComboBox->model()->index(0, 0));
  }

  // QObject::connect(d->RecentLogFilesComboBox, SIGNAL(currentTextChanged(QString)), this,
  // SLOT(onLogFileSelectionChanged()));
  QObject::connect(d->RecentLogFilesComboBox,
                   SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
                   this,
                   SLOT(onLogFileSelectionChanged()));
  QObject::connect(d->LogCopyToClipboardPushButton, SIGNAL(clicked()), this, SLOT(onLogCopy()));
  QObject::connect(d->LogOpenFileLocationPushButton, SIGNAL(clicked()), this, SLOT(onLogFileLocationOpen()));
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
  int row = d->RecentLogFilesComboBox->currentRow();
  QTableWidgetItem* eventItem = d->RecentLogFilesComboBox->item(row, 0);
  QFile f(eventItem->data(Qt::UserRole).toString());
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
void qSlicerErrorReportDialog::onLogFileLocationOpen()
{
  Q_D(qSlicerErrorReportDialog);
  int row = d->RecentLogFilesComboBox->currentRow();
  QTableWidgetItem* eventItem = d->RecentLogFilesComboBox->item(row, 0);
  QFileInfo fileInfo(eventItem->data(Qt::UserRole).toString());
  QDesktopServices::openUrl(QUrl(fileInfo.absolutePath(), QUrl::TolerantMode));
}

// --------------------------------------------------------------------------
void qSlicerErrorReportDialog::onLogFileOpen()
{
  Q_D(qSlicerErrorReportDialog);
  int row = d->RecentLogFilesComboBox->currentRow();
  QTableWidgetItem* eventItem = d->RecentLogFilesComboBox->item(row, 0);
  QString f(eventItem->data(Qt::UserRole).toString());
  QDesktopServices::openUrl(QUrl::fromLocalFile(f));
}

// --------------------------------------------------------------------------
void qSlicerErrorReportDialog::onLogFileEditClicked(bool editable)
{
  Q_D(qSlicerErrorReportDialog);
  d->LogText->setReadOnly(!editable);
}
