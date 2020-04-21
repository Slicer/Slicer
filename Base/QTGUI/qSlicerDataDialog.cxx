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

/// Qt includes
#include <QDebug>
#include <QComboBox>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QMessageBox>
#include <QTemporaryDir>

/// CTK includes
#include <ctkCheckableHeaderView.h>
#include <ctkCheckableModelHelper.h>

/// Slicer includes
#include "vtkArchive.h"
#include "vtkMRMLApplicationLogic.h"

/// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerDataDialog_p.h"
#include "qSlicerIOManager.h"
#include "qSlicerIOOptionsWidget.h"

//-----------------------------------------------------------------------------
qSlicerDataDialogPrivate::qSlicerDataDialogPrivate(QWidget* _parent)
  :QDialog(_parent)
{
  this->setupUi(this);

  // Checkable headers.
  // We replace the current FileWidget header view with a checkable header view.
  // Checked files (rows) will be loaded into the scene, unchecked files will be
  // discarded.
  // In order to have a column checkable, we need to manually set a checkstate
  // to a column. No checkstate (null QVariant) means uncheckable.
  this->FileWidget->model()->setHeaderData(
    FileColumn, Qt::Horizontal, Qt::Unchecked, Qt::CheckStateRole);
  QHeaderView* previousHeaderView = this->FileWidget->horizontalHeader();
  ctkCheckableHeaderView* headerView = new ctkCheckableHeaderView(
    Qt::Horizontal, this->FileWidget);
  // Copy the previous behavior of the header into the new checkable header view
  headerView->setSectionsClickable(previousHeaderView->sectionsClickable());
  headerView->setSectionsMovable(previousHeaderView->sectionsMovable());
  headerView->setHighlightSections(previousHeaderView->highlightSections());
  headerView->setStretchLastSection(previousHeaderView->stretchLastSection());
  // Propagate to top-level items only (depth = 1),no need to go deeper
  // (depth = -1 or 2, 3...) as it is a flat list.
  headerView->checkableModelHelper()->setPropagateDepth(1);
  // Finally assign the new header to the view
  this->FileWidget->setHorizontalHeader(headerView);

  headerView->setStretchLastSection(false);
  headerView->setSectionResizeMode(FileColumn, QHeaderView::Stretch);
  headerView->setSectionResizeMode(TypeColumn, QHeaderView::ResizeToContents);
  headerView->setSectionResizeMode(OptionsColumn, QHeaderView::ResizeToContents);

  this->FileWidget->sortItems(-1, Qt::AscendingOrder);

  // Connect the "Options" button with the visibility of the "Options" column.
  connect(this->ShowOptionsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(showOptions(bool)));
  // Hide the options by default;
  this->showOptions(false);

  connect(this->AddDirectoryButton, SIGNAL(clicked()), this, SLOT(addDirectory()));
  connect(this->AddFilesButton, SIGNAL(clicked()), this, SLOT(addFiles()));

  // Reset clears the FileWidget of all previously added files.
  QPushButton* resetButton = this->ButtonBox->button(QDialogButtonBox::Reset);
  connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

  // Authorize Drops action from outside
  this->setAcceptDrops(true);

  // Set up button focus default action:
  // * Space bar: clicks the button where the focus is on (e.g., if a user added data from
  //   file and wants to add another data from file then he should press space bar)
  // * Enter key: loads the selected data (unless cancel button has the focus, that case
  //   enter key cancels loading). It is important that after add data button was clicked
  //   and add data button has the focus enter key still loads selected data instead of
  //   opening the add data window. This allows the user to load data then just hit Enter
  //   key to load data.

  // All buttons have strong focus, so after clicking/tabbing on them hitting space bar
  // clicks them. However, we need to prevent all push-buttons (other than OK and Cancel)
  // to become default buttons.
  // Default button is the one that is clicked when user hits Enter key.
  resetButton->setAutoDefault(false);
  resetButton->setDefault(false);
  this->AddDirectoryButton->setDefault(false);
  this->AddDirectoryButton->setAutoDefault(false);
  this->AddFilesButton->setDefault(false);
  this->AddFilesButton->setAutoDefault(false);
}

//-----------------------------------------------------------------------------
qSlicerDataDialogPrivate::~qSlicerDataDialogPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::addDirectory()
{
  qSlicerStandardFileDialog fileDialog;
  QString directory = fileDialog.getExistingDirectory();
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
  qSlicerStandardFileDialog fileDialog;
  QStringList files = fileDialog.getOpenFileName();

  foreach(QString file, files)
    {
    this->addFile(file);
    }
  //this->FileWidget->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::addDirectory(const QDir& directory)
{
  bool recursive = true;
  QDir::Filters filters =
    QDir::AllDirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
  QFileInfoList fileInfoList = directory.entryInfoList(filters);

  //
  // check to see if any readers recognize the directory contents
  // and provide an archetype.
  //
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  QString readerDescription;
  qSlicerIO::IOProperties ioProperties;
  QFileInfo archetypeEntry;
  if (coreIOManager->examineFileInfoList(fileInfoList, archetypeEntry, readerDescription, ioProperties))
    {
    this->addFile(archetypeEntry);
    QString filePath = archetypeEntry.absoluteFilePath();
    QList<QTableWidgetItem *> items = this->FileWidget->findItems(filePath, Qt::MatchExactly);
    if (items.isEmpty())
      {
      qWarning() << "Couldn't add archetype widget for file: " << filePath;
      }
    else
      {
      QTableWidgetItem *item = items[0];
      QWidget *cellWidget = this->FileWidget->cellWidget(item->row(), TypeColumn);
      QComboBox *descriptionComboBox = dynamic_cast<QComboBox *>(cellWidget);
      descriptionComboBox->setCurrentIndex(descriptionComboBox->findText(readerDescription));
      cellWidget = this->FileWidget->cellWidget(item->row(), OptionsColumn);
      qSlicerIOOptionsWidget *ioOptionsWidget = dynamic_cast<qSlicerIOOptionsWidget *> (cellWidget);
      ioOptionsWidget->updateGUI(ioProperties);
      }
    }

  //
  // now add any files and directories that weren't filtered
  // out by the ioManager
  //
  foreach(QFileInfo entry, fileInfoList)
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
  //this->FileWidget->resizeColumnsToContents();
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
    {
    return; // file already exists
    }


  //
  // check for archive, and optionally open it
  //
  if (this->checkAndHandleArchive(file))
    {
    return; // file was an archive
    }

  //
  // use the IOManager to check for ways to load the data
  //
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  QStringList fileDescriptions =
    coreIOManager->fileDescriptions(file.absoluteFilePath());
  if (fileDescriptions.isEmpty())
    {
    return;
    }

  //
  // add the file to the dialog
  //
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
  foreach(const QString& fileDescription, fileDescriptions)
    {
    descriptionComboBox->addItem(fileDescription,
                                 QVariant(coreIOManager->fileTypeFromDescription(fileDescription)));
    }
  // adding items to the combobox automatically selects the first item
  // let's select none, connect the signal and then selecting the first will
  // automatically create the option widget
  descriptionComboBox->setCurrentIndex(-1);
  QObject::connect(descriptionComboBox, SIGNAL(currentIndexChanged(QString)),
                   this, SLOT(onFileTypeChanged(QString)));
  QObject::connect(descriptionComboBox, SIGNAL(activated(QString)),
                   this, SLOT(onFileTypeActivated(QString)));
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
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOProperties> qSlicerDataDialogPrivate::selectedFiles()const
{
  QList<qSlicerIO::IOProperties> files;
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    qSlicerIO::IOProperties properties;
    QTableWidgetItem* fileItem = this->FileWidget->item(row, FileColumn);
    QComboBox* descriptionComboBox =
      qobject_cast<QComboBox*>(this->FileWidget->cellWidget(row, TypeColumn));
    Q_ASSERT(fileItem);
    Q_ASSERT(descriptionComboBox);
    if (fileItem->checkState() != Qt::Checked)
      {
      continue;
      }
    // TBD: fileType is not good enough to describe what reader to use
    properties["fileType"] = descriptionComboBox->itemData(
      descriptionComboBox->currentIndex()).toString();
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
void qSlicerDataDialogPrivate::onFileTypeChanged(const QString& description)
{
  int row = this->senderRow();
  QString fileName = this->FileWidget->item(row, FileColumn)->text();
  this->setFileOptions(row, fileName, description);
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::onFileTypeActivated(const QString& description)
{
  int activatedRow = this->senderRow();
  if (this->propagateChange(activatedRow))
    {
    for(int row = 0; row < this->FileWidget->rowCount(); ++row)
      {
      if (!this->haveSameTypeOption(activatedRow, row))
        {
        continue;
        }
      QComboBox* selectedComboBox = qobject_cast<QComboBox*>(
        this->FileWidget->cellWidget(row, TypeColumn));
      int descriptionIndex =
        selectedComboBox ? selectedComboBox->findText(description) : -1;
      qDebug() << "id" << descriptionIndex;
      if (descriptionIndex != -1)
        {
        selectedComboBox->setCurrentIndex(descriptionIndex);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataDialogPrivate::setFileOptions(
  int row, const QString& fileName, const QString& fileDescription)
{
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  // Options
  qSlicerIOOptions* options = coreIOManager->fileOptions(fileDescription);
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
  this->FileWidget->resizeColumnToContents(OptionsColumn);
}

//-----------------------------------------------------------------------------
int qSlicerDataDialogPrivate::senderRow()const
{
  QComboBox* comboBox = qobject_cast<QComboBox*>(this->sender());
  if (!comboBox)
    {
    qCritical() << "qSlicerDataDialogPrivate::onFileTypeChanged must be called"
                << "by a QComboBox signal";
    return -1;
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
    }
  return row;
}

//-----------------------------------------------------------------------------
bool qSlicerDataDialogPrivate::haveSameTypeOption(int row1, int row2)const
{
  QComboBox* comboBox1 = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(row1, TypeColumn));
  QComboBox* comboBox2 = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(row2, TypeColumn));
  if (!comboBox1 || !comboBox2)
    {
    return false;
    }
  if (comboBox1->count() != comboBox2->count())
    {
    return false;
    }
  for (int i=0; i < comboBox1->count(); ++i)
    {
    if (comboBox1->itemText(i) != comboBox2->itemText(i))
      {
      return false;
      }
    }
  return true;
}
//-----------------------------------------------------------------------------
bool qSlicerDataDialogPrivate::propagateChange(int changedRow)const
{
  QTableWidgetItem* item = this->FileWidget->item(changedRow, FileColumn);
  bool fileSelected = item ? item->checkState() != Qt::Unchecked : false;
  return fileSelected
    && (QApplication::keyboardModifiers() & Qt::ShiftModifier);
}

//-----------------------------------------------------------------------------
bool qSlicerDataDialogPrivate::checkAndHandleArchive(const QFileInfo& file)
{
  if (file.suffix().toLower() == "zip")
    {
    if (QMessageBox::question(this, tr("Open archive?"), tr("The selected file is a .zip archive, open it and load contents?")))
      {
      this->temporaryArchiveDirectory.reset(new QTemporaryDir());
      if (this->temporaryArchiveDirectory->isValid())
        {
        if (vtkArchive::UnZip(file.absoluteFilePath().toStdString().c_str(), this->temporaryArchiveDirectory->path().toStdString().c_str()))
          {
          this->addDirectory(QDir(this->temporaryArchiveDirectory->path()));
          return true;
          }
        }
      }
    }
  return false;
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
  , d_ptr(new qSlicerDataDialogPrivate(nullptr))
{
  // FIXME give qSlicerDataDialog as a parent of qSlicerDataDialogPrivate;
}

//-----------------------------------------------------------------------------
qSlicerDataDialog::~qSlicerDataDialog() = default;

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerDataDialog::fileType()const
{
  // FIXME: not really a scene file, but more a collection of files
  return QString("NoFile");
}

//-----------------------------------------------------------------------------
QString qSlicerDataDialog::description()const
{
  return tr("Any Data");
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerDataDialog::action()const
{
  return qSlicerFileDialog::Read;
}

//---------------------------------------------------------------------------
bool qSlicerDataDialog::isMimeDataAccepted(const QMimeData* mimeData)const
{
  return mimeData->hasFormat("text/uri-list");
}

//-----------------------------------------------------------------------------
void qSlicerDataDialog::dropEvent(QDropEvent *event)
{
  Q_D(qSlicerDataDialog);
  bool pathAdded = false;
  foreach(QUrl url, event->mimeData()->urls())
    {
    if (!url.isValid() || url.isEmpty())
      {
      continue;
      }

    QString localPath = url.toLocalFile(); // convert QUrl to local path
    QFileInfo pathInfo;
    pathInfo.setFile(localPath); // information about the path

    if (pathInfo.isDir()) // if it is a directory we add the files to the dialog
      {
      d->addDirectory(QDir(localPath));
      pathAdded = true;
      }
    else if (pathInfo.isFile()) // if it is a file we simply add the file
      {
      d->addFile(pathInfo);
      pathAdded = true;
      }
    }
  if (pathAdded)
    {
    event->acceptProposedAction();
    }
}

//-----------------------------------------------------------------------------
bool qSlicerDataDialog::exec(const qSlicerIO::IOProperties& readerProperties)
{
  Q_D(qSlicerDataDialog);
  Q_ASSERT(!readerProperties.contains("fileName"));
  if (readerProperties.contains("fileNames"))
    {
    QStringList fileNames = readerProperties["fileNames"].toStringList();
    foreach(QString fileName, fileNames)
      {
      d->addFile(QFileInfo(fileName));
      }
    }

  bool res = false;
  if (d->exec() != QDialog::Accepted)
    {
    d->reset();
    return res;
    }
  QList<qSlicerIO::IOProperties> files = d->selectedFiles();
  for (int i = 0; i < files.count(); ++i)
    {
    files[i].unite(readerProperties);
    }
  res = qSlicerCoreApplication::application()->coreIOManager()
    ->loadNodes(files);
  d->reset();
  return res;
}

//-----------------------------------------------------------------------------
void qSlicerDataDialog::addFile(const QString filePath)
{
  Q_D(qSlicerDataDialog);
  d->addFile(QFileInfo(filePath));
}

//-----------------------------------------------------------------------------
void qSlicerDataDialog::addDirectory(const QString directoryPath)
{
  Q_D(qSlicerDataDialog);
  d->addDirectory(QDir(directoryPath));
}
