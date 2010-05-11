/// Qt includes
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QUrl>

/// CTK includes
#include <ctkCheckableHeaderView.h>
#include <ctkDirectoryButton.h>

/// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerSaveDataDialog.h"
#include "qSlicerSaveDataDialog_p.h"
#include "qSlicerIOManager.h"
#include "qSlicerIOOptionsWidget.h"

/// MRML includes
#include <vtkDataFileFormatHelper.h> // for GetFileExtensionFromFormatString()
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

/// VTK includes
#include <vtkStringArray.h>

//-----------------------------------------------------------------------------
qSlicerSaveDataDialogPrivate::qSlicerSaveDataDialogPrivate(QWidget* _parent)
  :QDialog(_parent)
{
  this->setupUi(this);

  // checkable headers.
  this->FileWidget->model()->setHeaderData(NodeNameColumn, Qt::Horizontal, Qt::Unchecked, Qt::CheckStateRole);
  QHeaderView* previousHeaderView = this->FileWidget->horizontalHeader();
  ctkCheckableHeaderView* headerView = new ctkCheckableHeaderView(Qt::Horizontal, this->FileWidget);
  headerView->setClickable(previousHeaderView->isClickable());
  headerView->setMovable(previousHeaderView->isMovable());
  headerView->setHighlightSections(previousHeaderView->highlightSections());
  headerView->setPropagateToItems(true);
  this->FileWidget->setHorizontalHeader(headerView);
  
  connect(this->DirectoryButton, SIGNAL(directoryChanged(const QString&)), this, SLOT(setDirectory(const QString&)));
  connect(this->SelectSceneDataButton, SIGNAL(clicked()), this, SLOT(selectModifiedSceneData()));
  connect(this->SelectDataButton, SIGNAL(clicked()), this, SLOT(selectModifiedData()));

}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialogPrivate::~qSlicerSaveDataDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::setDirectory(const QString& newDirectory)
{
  QDir newDir(newDirectory);
  Q_ASSERT(newDir.exists());

  const int rowCount = this->FileWidget->rowCount();
  for( int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* fileItem = this->FileWidget->item(row, NodeNameColumn);
    Q_ASSERT(fileItem);
    if (fileItem->checkState() == Qt::Unchecked)
      {
      continue;
      }
    ctkDirectoryButton* directoryItemButton = qobject_cast<ctkDirectoryButton*>(
      this->FileWidget->cellWidget(row, FileDirectoryColumn));
    Q_ASSERT(directoryItemButton);
    directoryItemButton->setDirectory(newDir.path());
    }
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::populateItems(vtkMRMLScene* scene)
{
  this->FileWidget->setRowCount(0);
  if (scene == 0)
    {
    return;
    }
  bool sortingEnabled = this->FileWidget->isSortingEnabled();
  this->FileWidget->setSortingEnabled(false);

  int row = 0;
  this->DirectoryButton->setDirectory(scene->GetRootDirectory());
  this->FileWidget->insertRow(row);
  // Scene Name
  QTableWidgetItem* sceneNameItem = new QTableWidgetItem("(Scene Description)");
  sceneNameItem->setFlags((sceneNameItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
  sceneNameItem->setCheckState(Qt::Checked);
  this->FileWidget->setItem(row, NodeNameColumn, sceneNameItem);
  
  // Scene Type
  QTableWidgetItem* sceneTypeItem = new QTableWidgetItem("(SCENE)");
  sceneTypeItem->setFlags(sceneTypeItem->flags() & ~Qt::ItemIsEditable);
  this->FileWidget->setItem(row, NodeTypeColumn, sceneTypeItem);
  // Scene Status
  QTableWidgetItem* sceneModifiedItem = new QTableWidgetItem("Modified");
  sceneModifiedItem->setFlags(sceneModifiedItem->flags() & ~Qt::ItemIsEditable);
  this->FileWidget->setItem(row, NodeStatusColumn, sceneModifiedItem);
  // Scene Format
  QComboBox* sceneComboBoxWidget = new QComboBox(this->FileWidget);
  sceneComboBoxWidget->addItem("MRML (.mrml)");
  sceneComboBoxWidget->setEnabled(false);
  this->FileWidget->setCellWidget(row, FileFormatColumn, sceneComboBoxWidget);
  // Scene FileName
  if (scene->GetURL())
    {
    QFileInfo sceneFileInfo;
    sceneFileInfo = QFileInfo(QDir(scene->GetRootDirectory()),
                              scene->GetURL());
    this->FileWidget->setItem(row, FileNameColumn, 
                              new QTableWidgetItem(sceneFileInfo.fileName()));
    }
  else
    {
    this->FileWidget->setItem(row, FileNameColumn, 
                              new QTableWidgetItem("SlicerScene1"));

    }
  // Scene Directory
  ctkDirectoryButton* sceneDirectoryButton = 
    new ctkDirectoryButton(scene->GetRootDirectory(), this->FileWidget);
  this->FileWidget->setCellWidget(row, FileDirectoryColumn, sceneDirectoryButton);
  const int nodeCount = scene->GetNumberOfNodesByClass("vtkMRMLStorableNode");
  for (int i = 0 ; i < nodeCount; ++i)
    {
    vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(
      scene->GetNthNodeByClass(i, "vtkMRMLStorableNode"));
    Q_ASSERT(node);
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLStorageNode* storageNode = node->CreateDefaultStorageNode();
    if (storageNode == 0)
      {
      continue;
      }
    
    vtkMRMLStorageNode* snode = node->GetStorageNode();
    if (snode == 0) 
      {
      storageNode->SetScene(scene);
      scene->AddNode(storageNode);  
      node->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }
    
    if (snode->GetFileName() == 0 && !this->DirectoryButton->directory().isEmpty()) 
      {
      QString fileExtension = snode->GetDefaultWriteFileExtension();
      if (fileExtension.isEmpty())
        {
        fileExtension = QString(".") + fileExtension;
        }
      QFileInfo fileName(QDir(this->DirectoryButton->directory()), 
                         QString(node->GetName()) + fileExtension);
      snode->SetFileName(fileName.absoluteFilePath().toLatin1().data());
      }

    // get absolute filename
    QFileInfo fileInfo;
    if (scene->IsFilePathRelative(snode->GetFileName()))
      {
      fileInfo = QFileInfo(QDir(scene->GetRootDirectory()),
                     snode->GetFileName());
      }
    else
      {
      fileInfo = QFileInfo(snode->GetFileName());
      }

    this->FileWidget->insertRow(++row);

    // Node name
    QTableWidgetItem *nodeNameItem = new QTableWidgetItem(node->GetName());
    nodeNameItem->setFlags( (nodeNameItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
    // the tooltip is used to store the id of the nodes
    nodeNameItem->setData(Qt::ToolTipRole, QString(node->GetID()) + " " + snode->GetID() );
    this->FileWidget->setItem(row, NodeNameColumn, nodeNameItem);

    // Node type
    QTableWidgetItem *nodeTypeItem = new QTableWidgetItem(node->GetNodeTagName());
    nodeTypeItem->setFlags( nodeTypeItem->flags() & ~Qt::ItemIsEditable);
    this->FileWidget->setItem(row, NodeTypeColumn, nodeTypeItem);

    // Node status (modified / not modified)
    // As a safety measure:
    // If the data is sitting in cache, it's vulnerable to overwriting or deleting.
    // Mark the node as modified since read so that a user will be more likely
    // to save it to a reliable location on local (or remote) disk.
    if ( scene->GetCacheManager() )
      {
      if ( scene->GetCacheManager()->GetRemoteCacheDirectory() )
        {
        QString cacheDir = scene->GetCacheManager()->GetRemoteCacheDirectory();
        int pos = fileInfo.absoluteFilePath().indexOf(cacheDir);
        if ( pos != -1)
          {
          node->ModifiedSinceReadOn();
          }
        }
      else
        {
        qWarning() << "Warning saving data: cannot get a default cache "
"directory, so not able to check whether any datafiles are residing in cache "
"and should be marked for save by default. Please take care when saving data.";
        }
      }
    else
      {
      qWarning() << "Warning saving data: cannot get a default cache "
"directory, so not able to check whether any datafiles are residing in cache "
"and should be marked for save by default. Please take care when saving data.";
      }

    QTableWidgetItem *nodeModifiedItem = 
      new QTableWidgetItem(node->GetModifiedSinceRead() ? 
                           tr("Modified") : tr("Not Modified"));
    nodeModifiedItem->setFlags( nodeModifiedItem->flags() & ~Qt::ItemIsEditable);
    this->FileWidget->setItem(row, NodeStatusColumn, nodeModifiedItem);
    // select modified nodes by default
    nodeNameItem->setCheckState(
      node->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);
    
    // File format
    QComboBox* fileFormats = new QComboBox(this->FileWidget);
    const int formatCount = snode->GetSupportedWriteFileTypes()->GetNumberOfValues();
    int currentFormat = -1;
    for (int formatIt = 0; formatIt < formatCount; ++formatIt)
      {
      vtkStdString format = 
        snode->GetSupportedWriteFileTypes()->GetValue(formatIt);
      fileFormats->addItem(format.c_str());
      if (QString(vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
                    format.c_str())) == (QString(".") + fileInfo.suffix()))
        {
        currentFormat = formatIt;
        }
      }
    fileFormats->setCurrentIndex(currentFormat);
    QObject::connect(fileFormats, SIGNAL(currentIndexChanged(int)), 
                     this, SLOT(formatChanged()));
    this->FileWidget->setCellWidget(row, FileFormatColumn, fileFormats);
    // File name
    QTableWidgetItem *fileNameItem =
      new QTableWidgetItem(fileInfo.fileName());
    this->FileWidget->setItem(row, FileNameColumn, fileNameItem);
    // File Directory
    ctkDirectoryButton* directoryButton = 
      new ctkDirectoryButton(fileInfo.absolutePath(),this->FileWidget);
    this->FileWidget->setCellWidget(row, FileDirectoryColumn, directoryButton);
    }
  this->FileWidget->setSortingEnabled(sortingEnabled);
  this->FileWidget->resizeColumnsToContents();
  this->resize(this->sizeHint());
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOProperties> qSlicerSaveDataDialogPrivate::selectedFiles()
{
  QList<qSlicerIO::IOProperties> files;
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    qDebug() << "row: " << row;
    qSlicerIO::IOProperties properties;
    QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
    //QTableWidgetItem* typeItem = this->FileWidget->item(row, TypeColumn);
    //qSlicerIOOptionsWidget* optionsItem = dynamic_cast<qSlicerIOOptionsWidget*>(
    //  this->FileWidget->cellWidget(row, OptionsColumn));
    Q_ASSERT(nodeNameItem);
    //Q_ASSERT(typeItem);
    if (nodeNameItem->checkState() != Qt::Checked)
      {
      qDebug() << "unchecked" ;
      continue;
      }
    /*
    properties["fileName"] = fileItem->text();
    properties["fileType"] = typeItem->text().toInt();
    if (optionsItem)
      {
      properties.unite(optionsItem->options());
      }
    files << properties;
    */
    }
  return files;
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::selectModifiedData()
{  
  const int rowCount = this->FileWidget->rowCount();
  for (int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* statusItem = this->FileWidget->item(row, NodeStatusColumn);
    QTableWidgetItem* typeItem = this->FileWidget->item(row, NodeTypeColumn);
    Q_ASSERT(statusItem); 
    Q_ASSERT(typeItem);
    QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
    Q_ASSERT(nodeNameItem);
    nodeNameItem->setCheckState(
      statusItem->text() == tr("Modified") && 
      typeItem->text() != tr("(SCENE)") ? Qt::Checked : Qt::Unchecked);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::selectModifiedSceneData()
{
  const int rowCount = this->FileWidget->rowCount();
  for (int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* statusItem = this->FileWidget->item(row, NodeStatusColumn);
    Q_ASSERT(statusItem);
    if (statusItem->text() != tr("Modified"))
      {
      continue;
      }
    QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
    Q_ASSERT(nodeNameItem);
    nodeNameItem->setCheckState(
      statusItem->text() == tr("Modified") ? Qt::Checked : Qt::Unchecked);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::formatChanged()
{
  //Search the item whose format has changed
  QComboBox* changedComboBox = qobject_cast<QComboBox*>(this->sender());
  Q_ASSERT(changedComboBox);
  const int rowCount = this->FileWidget->rowCount();
  int row = 0;
  for (; row < rowCount; ++row)
    {
    if (changedComboBox == this->FileWidget->cellWidget(row, FileFormatColumn))
      {
      break;
      }
    }
  Q_ASSERT(row < rowCount);
  // Update the name with the new extension
  QString extension = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
    changedComboBox->currentText().toStdString().c_str());
  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  Q_ASSERT(fileNameItem);
  fileNameItem->setText(QFileInfo(fileNameItem->text()).baseName() + extension);
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialog::qSlicerSaveDataDialog(QObject* _parent)
  :qSlicerFileDialog(_parent)
{
  // FIXME give qSlicerSaveDataDialog as a parent of qSlicerSaveDataDialogPrivate;
  CTK_INIT_PRIVATE(qSlicerSaveDataDialog);
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialog::~qSlicerSaveDataDialog()
{
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSaveDataDialog::fileType()const
{
  // FIXME: not really a scene file, but more a collection of files
  return qSlicerIO::NoFile;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerSaveDataDialog::action()const
{
  return qSlicerFileDialog::Write;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialog::exec(const qSlicerIO::IOProperties& readerProperties)
{
  CTK_D(qSlicerSaveDataDialog);
  Q_ASSERT(!readerProperties.contains("fileName"));
#ifdef Slicer3_USE_KWWIDGETS
  d->setWindowFlags(d->windowFlags() | Qt::WindowStaysOnTopHint);
#endif
  d->populateItems(qSlicerCoreApplication::application()->mrmlScene());
  bool res = false;
  if (d->exec() != QDialog::Accepted)
    {
    return res;
    }
  QList<qSlicerIO::IOProperties> files = d->selectedFiles();
  foreach(qSlicerIO::IOProperties properties, files)
    {
    properties.unite(readerProperties);
    res = qSlicerCoreApplication::application()->coreIOManager()
      ->saveNodes(properties["fileType"].toInt(),
                  properties) || res;
    }
  return res;
}

