/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Qt includes
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QList>
#include <QUrl>

/// CTK includes
#include <ctkCheckableHeaderView.h>
#include <ctkLogger.h>

/// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerDataDialog.h"
#include "qSlicerDataDialog_p.h"
#include "qSlicerIOManager.h"
#include "qSlicerIOOptionsWidget.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qSlicerDataDialog");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
qSlicerDataDialogPrivate::qSlicerDataDialogPrivate(QWidget* _parent)
  :QDialog(_parent)
{
  this->setupUi(this);
  // checkable headers.
  this->FileWidget->model()->setHeaderData(FileColumn, Qt::Horizontal, Qt::Unchecked, Qt::CheckStateRole);
  QHeaderView* previousHeaderView = this->FileWidget->horizontalHeader();
  ctkCheckableHeaderView* headerView = new ctkCheckableHeaderView(Qt::Horizontal, this->FileWidget);
  headerView->setClickable(previousHeaderView->isClickable());
  headerView->setMovable(previousHeaderView->isMovable());
  headerView->setHighlightSections(previousHeaderView->highlightSections());
  //headerView->setModel(previousHeaderView->model());
  //headerView->setSelectionModel(previousHeaderView->selectionModel());
  headerView->setPropagateToItems(true);
  this->FileWidget->setHorizontalHeader(headerView);
  /*
  connect(this->FileWidget->model(), SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
          this, SLOT(updateCheckBoxes(Qt::Orientation, int, int)));
  connect(this->FileWidget, SIGNAL(cellChanged(int,int)),
          this, SLOT(updateCheckBoxHeader(int,int)));
  */
  connect(this->ShowOptionsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(showOptions(bool)));
  // hide the options by default;
  this->showOptions(false);

  connect(this->AddDirectoryButton, SIGNAL(clicked()), this, SLOT(addDirectory()));
  connect(this->AddFilesButton, SIGNAL(clicked()), this, SLOT(addFiles()));
  QPushButton* resetButton = this->ButtonBox->button(QDialogButtonBox::Reset);
  connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
}

//-----------------------------------------------------------------------------
qSlicerDataDialogPrivate::~qSlicerDataDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::addDirectory()
{
  QString directory = QFileDialog::getExistingDirectory(this);
  if (directory.isNull())
    {
    return;
    }
  bool sortingEnabled = this->FileWidget->isSortingEnabled();
  this->FileWidget->setSortingEnabled(false);
  this->addDirectory(directory);
  this->FileWidget->setSortingEnabled(sortingEnabled);
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::addFiles()
{
  QStringList files = QFileDialog::getOpenFileNames(this);
  foreach(QString file, files)
    {
    this->addFile(file);
    }
  this->FileWidget->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::addDirectory(const QDir& directory)
{
  bool recursive = true;
  QDir::Filters filters =
    QDir::AllDirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
  foreach(QFileInfo entry, directory.entryInfoList(filters))
    {
    if (entry.isFile())
      {
      this->addFile(entry);
      }
    else if (entry.isDir() && recursive)
      {
      this->addDirectory(entry.absoluteFilePath());
      }
    }
  this->FileWidget->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::addFile(const QFileInfo& file)
{
  if (!file.isFile() || !file.exists() || !file.isReadable())
    {
    return;
    }
  if (!this->FileWidget->findItems(file.absoluteFilePath(),
                                   Qt::MatchExactly).isEmpty())
    {// file already exists
    qDebug() <<"already exists";
    return;
    }
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  QList<qSlicerIO::IOFileType> fileTypes =
    coreIOManager->fileTypes(file.absoluteFilePath());
  if (fileTypes.isEmpty())
    {
    return;
    }

  bool sortingEnabled = this->FileWidget->isSortingEnabled();
  this->FileWidget->setSortingEnabled(false);
  int row = this->FileWidget->rowCount();
  this->FileWidget->insertRow(row);
  // File name
  QTableWidgetItem *fileItem = new QTableWidgetItem(file.absoluteFilePath());
  fileItem->setFlags( (fileItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
  fileItem->setCheckState(Qt::Checked);
  this->FileWidget->setItem(row, FileColumn, fileItem);
  // Description
  QComboBox* descriptionComboBox = new QComboBox(this->FileWidget);
  foreach(const qSlicerIO::IOFileType fileType, fileTypes)
    {
    descriptionComboBox->addItem(coreIOManager->fileDescription(fileType),
                                 fileType);
    }
  // adding items to the combobox automatically selects the first item
  // let's select none, connect the signal and then selecting the first will
  // automatically create the option widget
  descriptionComboBox->setCurrentIndex(-1);
  QObject::connect(descriptionComboBox, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onFileTypeChanged()));
  this->FileWidget->setCellWidget(row, TypeColumn, descriptionComboBox);
  descriptionComboBox->setCurrentIndex(0);
    this->FileWidget->setSortingEnabled(sortingEnabled);
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::reset()
{
  this->FileWidget->setRowCount(0);
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::showOptions(bool show)
{
  this->ShowOptionsCheckBox->setChecked(show);
  this->FileWidget->setColumnHidden(OptionsColumn, !show);
  this->FileWidget->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOProperties> qSlicerDataDialogPrivate::selectedFiles()const
{
  QList<qSlicerIO::IOProperties> files;
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    logger.trace(QString("selectedFiles - row: %1").arg(row));
    qSlicerIO::IOProperties properties;
    QTableWidgetItem* fileItem = this->FileWidget->item(row, FileColumn);
    QComboBox* descriptionComboBox = 
      qobject_cast<QComboBox*>(this->FileWidget->cellWidget(row, TypeColumn));
    Q_ASSERT(fileItem);
    Q_ASSERT(descriptionComboBox);
    if (fileItem->checkState() != Qt::Checked)
      {
      logger.trace(QString("selectedFiles - row: %1 - UnChecked").arg(row));
      continue;
      }
    properties["fileType"] = descriptionComboBox->itemData(
      descriptionComboBox->currentIndex()).toInt();
    qSlicerIOOptionsWidget* optionsItem = dynamic_cast<qSlicerIOOptionsWidget*>(
      this->FileWidget->cellWidget(row, OptionsColumn));
    if (optionsItem)
      {
      // The optionsItem contains all the file properties including "fileName"
      properties.unite(optionsItem->properties());
      }
    else
      {
      properties["fileName"] = fileItem->text();
      }
    files << properties;
    }
  return files;
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::onFileTypeChanged()
{
  QComboBox* comboBox = qobject_cast<QComboBox*>(this->sender());
  if (!comboBox)
    {
    qCritical() << "qSlicerDataDialogPrivate::onFileTypeChanged must be called"
                << "by a QComboBox signal";
    return;
    }
  int row = -1;
  for (int i = 0; i < this->FileWidget->rowCount(); ++i)
    {
    if (this->FileWidget->cellWidget(i, TypeColumn) == comboBox)
      {
      row = i;
      break;
      }
    }
  if (row < 0)
    {
    qCritical() << "Can't find the item to update";
    return;
    }
  QString fileName = this->FileWidget->item(row, FileColumn)->text();
  qSlicerIO::IOFileType fileType =
    static_cast<qSlicerIO::IOFileType>(
      comboBox->itemData(comboBox->currentIndex()).toInt());
  this->setFileOptions(row, fileName, fileType);
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::setFileOptions(
  int row, const QString& fileName, const qSlicerIO::IOFileType& fileType)
{
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  // Options
  qSlicerIOOptions* options = coreIOManager->fileOptions(fileType);
  qSlicerIOOptionsWidget* optionsWidget =
    dynamic_cast<qSlicerIOOptionsWidget*>(options);
  if (optionsWidget)
    {
    // TODO: support uneven rows. Until that day, we want to make sure the whole
    // widget is visible
    optionsWidget->setMinimumWidth(optionsWidget->sizeHint().width());
    // The optionsWidget can use the filename to initialize some options.
    optionsWidget->setFileName(fileName);
    // TODO: connect signal validChanged(bool) with the accept button
    }
  else
    {
    delete options;
    }
  this->FileWidget->setCellWidget(row, OptionsColumn, optionsWidget);
}
/*
//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::updateCheckBoxes(Qt::Orientation orientation, int first, int last)
{
  if (orientation != Qt::Horizontal ||
      first > FileColumn || last < FileColumn)
    {
    return;
    }
  Qt::CheckState headerState = this->FileWidget->horizontalHeaderItem(FileColumn)->checkState();
  if (headerState == Qt::PartiallyChecked)
    {
    return;
    }
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    this->FileWidget->item(row, FileColumn)->setCheckState(headerState);
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::updateCheckBoxHeader(int itemRow, int itemColumn)
{
  if (itemColumn != FileColumn)
    {
    return;
    }
  Qt::CheckState headerCheckState = this->FileWidget->item(itemRow,itemColumn)->checkState();
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    if (this->FileWidget->item(row, FileColumn)->checkState() !=
        headerCheckState)
      {
      this->FileWidget->model()->setHeaderData(FileColumn, Qt::Horizontal, Qt::PartiallyChecked, Qt::CheckStateRole);
      return;
      }
    }
  this->FileWidget->model()->setHeaderData(FileColumn, Qt::Horizontal, headerCheckState, Qt::CheckStateRole);
}*/

//-----------------------------------------------------------------------------
qSlicerDataDialog::qSlicerDataDialog(QObject* _parent)
  : qSlicerFileDialog(_parent)
  , d_ptr(new qSlicerDataDialogPrivate(0))
{
  // FIXME give qSlicerDataDialog as a parent of qSlicerDataDialogPrivate;
}

//-----------------------------------------------------------------------------
qSlicerDataDialog::~qSlicerDataDialog()
{
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerDataDialog::fileType()const
{
  // FIXME: not really a scene file, but more a collection of files
  return qSlicerIO::NoFile;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerDataDialog::action()const
{
  return qSlicerFileDialog::Read;
}

//-----------------------------------------------------------------------------
bool qSlicerDataDialog::exec(const qSlicerIO::IOProperties& readerProperties)
{
  Q_D(qSlicerDataDialog);
  Q_ASSERT(!readerProperties.contains("fileName"));
#ifdef Slicer_USE_KWWIDGETS
  d->setWindowFlags(d->windowFlags() | Qt::WindowStaysOnTopHint);
#endif
  bool res = false;
  if (d->exec() != QDialog::Accepted)
    {
    d->reset();
    return res;
    }
  QList<qSlicerIO::IOProperties> files = d->selectedFiles();
  foreach(qSlicerIO::IOProperties properties, files)
    {
    properties.unite(readerProperties);
    res = qSlicerCoreApplication::application()->coreIOManager()
      ->loadNodes(static_cast<qSlicerIO::IOFileType>(
        properties["fileType"].toInt()), properties) || res;
    }
  d->reset();
  return res;
}

