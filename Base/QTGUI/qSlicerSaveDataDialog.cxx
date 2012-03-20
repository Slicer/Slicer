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
#include <QComboBox>
#include <QDate>
#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegExp>
#include <QRegExpValidator>

/// CTK includes
#include <ctkCheckableHeaderView.h>
#include <ctkCheckableModelHelper.h>

/// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerSaveDataDialog_p.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLUtils.h"

/// Logic includes
#include "vtkMRMLLogic.h"

/// MRML includes
#include <vtkCacheManager.h>
#include <vtkCollection.h>
#include <vtkDataFileFormatHelper.h> // for GetFileExtensionFromFormatString()
//#include <vtkMRMLHierarchyNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLSceneViewNode.h>

/// VTK includes
#include <vtkStringArray.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

// STD includes
#include <cstring> // for strlen

//-----------------------------------------------------------------------------
qSlicerFileNameItemDelegate::qSlicerFileNameItemDelegate( QObject * parent )
  : Superclass(parent)
{
}

//-----------------------------------------------------------------------------
QWidget* qSlicerFileNameItemDelegate
::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
               const QModelIndex & index ) const
{
  QWidget* widget =this->Superclass::createEditor(parent, option, index);
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
  if (lineEdit)
    {
    QString extension = index.data(Qt::UserRole).toString();
    lineEdit->setValidator(
      new QRegExpValidator(qSlicerFileNameItemDelegate::fileNameRegExp(extension), lineEdit));
    }
  return widget;
}

//-----------------------------------------------------------------------------
QString qSlicerFileNameItemDelegate::fixupFileName(const QString& fileName, const QString& extension)
{
  QString fixup;
  QRegExp regExp = qSlicerFileNameItemDelegate::fileNameRegExp();
  for (int i = 0; i < fileName.size(); ++i)
    {
    if (regExp.exactMatch(fileName[i]))
      {
      fixup += fileName[i];
      }
    }
  if (!extension.isEmpty() && !fixup.endsWith(extension))
    {
    fixup += extension;
    }
  return fixup;
}

//-----------------------------------------------------------------------------
QRegExp qSlicerFileNameItemDelegate::fileNameRegExp(const QString& extension)
{
  QRegExp regExp("[A-Za-z0-9\\ \\-\\_\\.\\(\\)\\$\\!\\~\\#\\'\\%\\^\\{\\}]{1,255}");

  if (!extension.isEmpty())
    {
    regExp.setPattern(regExp.pattern() + extension);
    }
  return regExp;
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialogPrivate::qSlicerSaveDataDialogPrivate(QWidget* parentWidget)
  :QDialog(parentWidget)
{
  this->MRMLScene = 0;

  this->setupUi(this);
  this->FileWidget->setItemDelegateForColumn(
    FileNameColumn, new qSlicerFileNameItemDelegate(this));

  // Checkable headers.
  // We replace the current FileWidget header view with a checkable header view.
  // Checked files (rows) will be saved, unchecked files will be discarded.
  // In order to have a column checkable, we need to manually set a checkstate
  // to a column. No checkstate (null QVariant) means uncheckable.
  this->FileWidget->model()->setHeaderData(NodeNameColumn, Qt::Horizontal,
                                           Qt::Unchecked, Qt::CheckStateRole);
  QHeaderView* previousHeaderView = this->FileWidget->horizontalHeader();
  ctkCheckableHeaderView* headerView = new ctkCheckableHeaderView(Qt::Horizontal, this->FileWidget);
  // Copy the previous behavior of the header into the new checkable header view
  headerView->setClickable(previousHeaderView->isClickable());
  headerView->setMovable(previousHeaderView->isMovable());
  headerView->setHighlightSections(previousHeaderView->highlightSections());
  headerView->setStretchLastSection(previousHeaderView->stretchLastSection());
  // Propagate to top-level items only (depth = 1),no need to go deeper
  // (depth = -1 or 2, 3...) as it is a flat list.
  headerView->checkableModelHelper()->setPropagateDepth(1);
  // Finally assign the new header to the view
  this->FileWidget->setHorizontalHeader(headerView);

  // Connect push buttons to associated actions
  connect(this->DirectoryButton, SIGNAL(directoryChanged(QString)),
          this, SLOT(setDirectory(QString)));
  connect(this->SelectSceneDataButton, SIGNAL(clicked()),
          this, SLOT(selectModifiedSceneData()));
  connect(this->SelectDataButton, SIGNAL(clicked()),
          this, SLOT(selectModifiedData()));
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialogPrivate::~qSlicerSaveDataDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  this->MRMLScene = scene;
  this->populateItems();
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerSaveDataDialogPrivate::mrmlScene()const
{
  return this->MRMLScene;
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
void qSlicerSaveDataDialogPrivate::populateItems()
{
  // clear the list
  this->FileWidget->setRowCount(0);
  if (this->MRMLScene == 0)
    {
    return;
    }

  // sorting the table while doing insertions is dangerous
  // Moreover, we want to have the MRML scene to be the first item.
  this->FileWidget->setSortingEnabled(false);

  this->DirectoryButton->setDirectory(this->MRMLScene->GetRootDirectory());

  this->populateScene();

  const int nodeCount = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLStorableNode");
  for (int i = 0 ; i < nodeCount; ++i)
    {
    vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(
      this->MRMLScene->GetNthNodeByClass(i, "vtkMRMLStorableNode"));
    Q_ASSERT(node);
    this->populateNode(node);
    }

  // Here we could have restore the sorting property but we want to keep the
  // MRML scene the first item of the list so we don't do restore the sorting.
  // this->FileWidget->setSortingEnabled(oldSortingEnabled);

  // let's try to resize the columns according to their new contents
  this->FileWidget->resizeColumnsToContents();
  // let's try to show the whole table on screen
  // TODO: find a function in Qt that does it automatically.
  this->resize(this->layout()->contentsMargins().left()
               + this->FileWidget->frameWidth()
               + this->FileWidget->verticalHeader()->sizeHint().width()
               + this->FileWidget->horizontalHeader()->length()
               + this->FileWidget->frameWidth()
               + this->layout()->contentsMargins().right(),
               this->sizeHint().height());
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::populateScene()
{
  int row = this->FileWidget->rowCount();
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
  QFileInfo sceneFileInfo;
  if (this->MRMLScene->GetURL() != 0 &&
      strlen(this->MRMLScene->GetURL()) > 0)
    {
    sceneFileInfo = QFileInfo( QDir(this->MRMLScene->GetRootDirectory()),
                               this->MRMLScene->GetURL());
    }
  else
    {
    sceneFileInfo = QFileInfo( QDir(this->MRMLScene->GetRootDirectory()),
                               QDate::currentDate().toString(
                                 "yyyy-MM-dd") + "-Scene");
    }
  QTableWidgetItem* fileNameItem = this->createFileNameItem(sceneFileInfo, ".mrml");
  this->FileWidget->setItem( row, FileNameColumn, fileNameItem);
    // Scene Directory
  ctkDirectoryButton* sceneDirectoryButton =
    new ctkDirectoryButton(this->MRMLScene->GetRootDirectory(), this->FileWidget);
  this->FileWidget->setCellWidget(row, FileDirectoryColumn, sceneDirectoryButton);
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::populateNode(vtkMRMLStorableNode* node)
{
  if (!node)
    {
    return;
    }

  // Don't show if the node doesn't want to (internal node)
  if (node->GetHideFromEditors())
    {
    return;
    }
/*
  // if the node is an annotation node and in a hierarchy, the hierarchy will
  // take care of writing it out
  if (node->IsA("vtkMRMLAnnotationNode"))
    {
    // not supporting rulers just yet
    if (!node->IsA("vtkMRMLAnnotationRulerNode"))
      {
      vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(node->GetScene(), node->GetID());
      if (hnode &&
          hnode->GetParentNodeID())
        {
        // std::cout << "Skipping node in a hierarchy: " << node->GetName() << std::endl;
        return;
        }
      }
    }
*/
  // get absolute filename
  QFileInfo fileInfo = this->nodeFileInfo(node);
  if (fileInfo == QFileInfo())
    {
    return;
    }

  // Create a new entry
  int row = this->FileWidget->rowCount();
  this->FileWidget->insertRow(row);

  // Node name
  QTableWidgetItem *nodeNameItem = this->createNodeNameItem(node);
  this->FileWidget->setItem(row, NodeNameColumn, nodeNameItem);

  // Node type
  QTableWidgetItem *nodeTypeItem = this->createNodeTypeItem(node);
  this->FileWidget->setItem(row, NodeTypeColumn, nodeTypeItem);

  // Node status
  QTableWidgetItem *nodeModifiedItem = this->createNodeStatusItem(node, fileInfo);
  this->FileWidget->setItem(row, NodeStatusColumn, nodeModifiedItem);

  // Select modified nodes by default
  nodeNameItem->setCheckState(
    node->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);

  // File format
  QWidget* fileFormatsWidget = this->createFileFormatsWidget(node, fileInfo);
  this->FileWidget->setCellWidget(row, FileFormatColumn, fileFormatsWidget);

  // File name
  QTableWidgetItem *fileNameItem = this->createFileNameItem(fileInfo);
  this->FileWidget->setItem(row, FileNameColumn, fileNameItem);

  // File Directory
  QWidget* directoryWidget = this->createFileDirectoryWidget(fileInfo);
  this->FileWidget->setCellWidget(row, FileDirectoryColumn, directoryWidget);
}

//-----------------------------------------------------------------------------
QFileInfo qSlicerSaveDataDialogPrivate::nodeFileInfo(vtkMRMLStorableNode* node)
{
  vtkMRMLStorageNode* storageNode = node->CreateDefaultStorageNode();
  if (storageNode == 0)
    {
    qDebug() << "nodeFileInfo: unable to create a new default storage node for node " << node->GetID();
    return QFileInfo();
    }
  
  vtkMRMLStorageNode* snode = node->GetStorageNode();
  if (snode == 0)
    {
    storageNode->SetScene(this->MRMLScene);
    this->MRMLScene->AddNode(storageNode);
    node->SetAndObserveStorageNodeID(storageNode->GetID());
    storageNode->Delete();
    snode = storageNode;
    }
  else
    {
    // there already is a storage node, don't need this new one
    storageNode->Delete();
    }
  if (snode->GetFileName() == 0 && !this->DirectoryButton->directory().isEmpty())
    {
    QString fileExtension = snode->GetDefaultWriteFileExtension();
    if (!fileExtension.isEmpty())
      {
      fileExtension = QString(".") + fileExtension;
      }
    QFileInfo fileName(QDir(this->DirectoryButton->directory()),
                       QString(node->GetName()) + fileExtension);
    snode->SetFileName(fileName.absoluteFilePath().toLatin1());
    }

  QFileInfo fileInfo;
  if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
    {
    fileInfo = QFileInfo(QDir(this->MRMLScene->GetRootDirectory()),
                         snode->GetFileName());
    }
  else
    {
    fileInfo = QFileInfo(snode->GetFileName());
    }
  return fileInfo;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerSaveDataDialogPrivate::createNodeNameItem(vtkMRMLStorableNode* node)
{
  QTableWidgetItem* nodeNameItem = new QTableWidgetItem(node->GetName());
  nodeNameItem->setFlags( (nodeNameItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
  // the tooltip is used to store the id of the nodes
  Q_ASSERT(node->GetStorageNode());
  nodeNameItem->setData(Qt::ToolTipRole, QString(node->GetID()) + " " + node->GetStorageNode()->GetID() );
  return nodeNameItem;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerSaveDataDialogPrivate::createNodeTypeItem(vtkMRMLStorableNode* node)
{
  QTableWidgetItem* nodeTypeItem = new QTableWidgetItem(node->GetNodeTagName());
  nodeTypeItem->setFlags( nodeTypeItem->flags() & ~Qt::ItemIsEditable);
  // TODO: add icon based on the type
  return nodeTypeItem;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerSaveDataDialogPrivate::createNodeStatusItem(vtkMRMLStorableNode* node, const QFileInfo& fileInfo)
{
  // Node status (modified / not modified)
  // As a safety measure:
  // If the data is sitting in cache, it's vulnerable to overwriting or deleting.
  // Mark the node as modified since read so that a user will be more likely
  // to save it to a reliable location on local (or remote) disk.
  if ( this->MRMLScene->GetCacheManager() )
    {
    if ( this->MRMLScene->GetCacheManager()->GetRemoteCacheDirectory() )
      {
      QString cacheDir = this->MRMLScene->GetCacheManager()->GetRemoteCacheDirectory();
      int pos = fileInfo.absoluteFilePath().indexOf(cacheDir);
      if ( pos != -1)
        {
        int disableModify = node->GetDisableModifiedEvent();
        node->SetDisableModifiedEvent(1);
        node->ModifiedSinceReadOn();
        node->SetDisableModifiedEvent(disableModify);
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
  return nodeModifiedItem;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSaveDataDialogPrivate::createFileFormatsWidget(vtkMRMLStorableNode* node, const QFileInfo& fileInfo)
{
  vtkMRMLStorageNode* snode = node->GetStorageNode();
  Q_ASSERT(snode);
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
  if (currentFormat == -1)
    {
    // In order to set a custom text (different than the combobox items),
    // we need to make the combobox editable, but we don't want the user to
    // edit the combobox line.
    fileFormats->setEditable(true);
    fileFormats->lineEdit()->setReadOnly(true);
    fileFormats->setEditText("Select a format");
    }

  // TODO: use QSignalMapper
  QObject::connect(fileFormats, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(formatChanged()));
  return fileFormats;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerSaveDataDialogPrivate
::createFileNameItem(const QFileInfo& fileInfo, const QString& extension)
{
  QTableWidgetItem* fileNameItem = new QTableWidgetItem(
    qSlicerFileNameItemDelegate::fixupFileName(fileInfo.fileName(), extension));
  if (!extension.isEmpty())
    {
    fileNameItem->setData(Qt::UserRole, extension);
    }
  return fileNameItem;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSaveDataDialogPrivate::createFileDirectoryWidget(const QFileInfo& fileInfo)
{
  // TODO: use QSignalMapper
  return new ctkDirectoryButton(fileInfo.absolutePath(),this->FileWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::accept()
{
  if (!this->save())
    {
    return;
    }
  this->done(QDialog::Accepted);
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::save()
{
  // Set the root directory to the scene so that the nodes paths are points
  // to the new scene path. It's important to have the right node paths
  // relatively to the scene in the saved mrml scene file.
  if (!this->prepareForSaving())
    {
    return false;
    }
  // Save the nodes first then the scene
  // Saving the nodes first ensures that the saved scene will points to the
  // potentially new path of the nodes.
  if (!this->saveNodes())
    {
    return false;
    }
  if (this->mustSceneBeSaved() && !this->saveScene())
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::saveNodes()
{
  QMessageBox::StandardButton forceOverwrite = QMessageBox::Ignore;
  QList<qSlicerIO::IOProperties> files;
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
    QTableWidgetItem* nodeTypeItem = this->FileWidget->item(row, NodeTypeColumn);
    QTableWidgetItem* nodeStatusItem = this->FileWidget->item(row, NodeStatusColumn);
    QComboBox* fileFormatComboBox = qobject_cast<QComboBox*>(
      this->FileWidget->cellWidget(row, FileFormatColumn));
    QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
    ctkDirectoryButton* fileDirectoryButton = qobject_cast<ctkDirectoryButton*>(
      this->FileWidget->cellWidget(row, FileDirectoryColumn));

    Q_ASSERT(nodeNameItem);
    Q_ASSERT(nodeTypeItem);
    Q_ASSERT(fileFormatComboBox);
    Q_ASSERT(fileNameItem);
    Q_ASSERT(fileDirectoryButton);

    // don't save unchecked nodes
    if (nodeNameItem->checkState() != Qt::Checked)
      {
      continue;
      }

    // only save nodes here
    if (nodeTypeItem->text() == "(SCENE)")
      {
      continue;
      }

    if (fileNameItem->text().isEmpty())
      {
      QMessageBox::warning(this, tr("Saving node..."),
                           tr("Node %1 not saved, file name is empty.").arg(nodeNameItem->text()));
      continue;
      }

    // file properties
    QDir directory = fileDirectoryButton->directory();
    QFileInfo file = QFileInfo(directory, fileNameItem->text());
    QString format = fileFormatComboBox->currentText();

    // node
    QStringList nodeIDs = nodeNameItem->data(Qt::ToolTipRole).toString().split(" ");
    vtkMRMLNode *node = this->MRMLScene->GetNodeByID(nodeIDs[0].toLatin1());
    vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(nodeIDs[1].toLatin1()));

    // check if the file already exists
    if (file.exists())
      {
      if (forceOverwrite == QMessageBox::NoToAll)
        {
        continue;
        }
      if (forceOverwrite == QMessageBox::Ignore)
        {
        QMessageBox::StandardButton answer =
          QMessageBox::question(this, tr("Saving node..."),
                                tr("The file: %1 already exists."
                                   " Do you want to replace it ?").arg(file.absoluteFilePath()),
                                QMessageBox::Yes | QMessageBox::No |
                                QMessageBox::YesToAll | QMessageBox::NoToAll,
                                QMessageBox::Yes);
        if (answer == QMessageBox::YesToAll)
          {
          forceOverwrite = QMessageBox::YesToAll;
          }
        else if (answer == QMessageBox::NoToAll)
          {
          forceOverwrite = QMessageBox::NoToAll;
          }
        if (answer == QMessageBox::No || answer == QMessageBox::NoToAll)
          {
          continue;
          }
        }
      }

    // save the node
    snode->SetFileName(file.absoluteFilePath().toLatin1());
    snode->SetWriteFileFormat(fileFormatComboBox->currentText().toLatin1());
    snode->SetURI(0);
    int res = snode->WriteData(node);

    // node has failed to be written
    if (!res)
      {
      QMessageBox::StandardButton answer =
        QMessageBox::question(this, tr("Saving node..."),
                              tr("Cannot write data file: %1.\n"
                                 "Do you want to continue saving?").arg(file.absoluteFilePath()),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
      if (answer == QMessageBox::No)
        {
        return false;
        }
      }

    // clean up node after saving
    int disableModify = node->GetDisableModifiedEvent();
    node->SetDisableModifiedEvent(1);
    node->SetModifiedSinceRead(0);
    node->SetDisableModifiedEvent(disableModify);
    nodeNameItem->setCheckState(Qt::Unchecked);
    nodeStatusItem->setText("Not Modified");
    }
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::mustSceneBeSaved()const
{
  QAbstractItemModel* model = this->FileWidget->model();
  QModelIndexList found = model->match(
    model->index(0, NodeTypeColumn),
    Qt::DisplayRole, QString("(SCENE)"), 1, Qt::MatchExactly);
  if (found.count() == 0 || !found[0].isValid())
    {
    return false;
    }
  QTableWidgetItem* nodeNameItem = 
    this->FileWidget->item(found[0].row(), NodeNameColumn);
  return nodeNameItem->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
QFileInfo qSlicerSaveDataDialogPrivate::sceneFile()const
{
  // search the scene
  QAbstractItemModel* model = this->FileWidget->model();
  QModelIndexList found = model->match(
    model->index(0, NodeTypeColumn),
    Qt::DisplayRole, QString("(SCENE)"), 1, Qt::MatchExactly);
  if (!found[0].isValid())
    {
    return QFileInfo(QString(this->MRMLScene->GetURL()));
    }

  QTableWidgetItem* fileNameItem = this->FileWidget->item(found[0].row(), FileNameColumn);
  ctkDirectoryButton* fileDirectoryButton = qobject_cast<ctkDirectoryButton*>(
    this->FileWidget->cellWidget(found[0].row(), FileDirectoryColumn));

  QDir directory = fileDirectoryButton->directory();
  QFileInfo file = QFileInfo(directory, fileNameItem->text());
  if (file.fileName().isEmpty())
    {
    file = QFileInfo(QString(this->MRMLScene->GetURL()));
    }
  if (file.suffix() != "mrml")
    {
    file = QFileInfo(directory, fileNameItem->text() + QString(".mrml"));
    }
  return file;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::prepareForSaving()
{
  QFileInfo file = this->sceneFile();
  if (file.exists())
    {
    QMessageBox::StandardButton answer = 
      QMessageBox::question(this, "Saving scene", "Scene file \""
                            + file.absoluteFilePath() +"\" already exists. "
                            "Do you want to replace it ?",
                            QMessageBox::Cancel | QMessageBox::Ok,  QMessageBox::Cancel);
    if (answer != QMessageBox::Ok)
      {
      return false;
      }
    }
  this->MRMLScene->SetRootDirectory(file.absoluteDir().absolutePath().toLatin1());

  // update the root directory of scene snapshot nodes (not sure why)
  const int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSceneSnapshotNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode* node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSceneSnapshotNode");
    vtkMRMLSceneViewNode *snode = vtkMRMLSceneViewNode::SafeDownCast(node);
    snode->GetNodes()->SetRootDirectory(this->MRMLScene->GetRootDirectory());
    }
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::saveScene()
{
  QFileInfo file = this->sceneFile();

  // save an explicit default scene view recording the state of the scene when
  // saved to file
  const char *defaultSceneName = "Master Scene View";
  vtkMRMLSceneViewNode * newSceneViewNode = NULL;
  vtkMRMLSceneViewNode *sceneViewNode = NULL;
  vtkSmartPointer<vtkCollection> oldSceneViewNodes;
  oldSceneViewNodes.TakeReference(this->MRMLScene->GetNodesByClassByName("vtkMRMLSceneViewNode", defaultSceneName));
  if (oldSceneViewNodes->GetNumberOfItems() == 0)
    {
    // make a new one
    newSceneViewNode = vtkMRMLSceneViewNode::New();
    newSceneViewNode->SetScene(this->MRMLScene);
    newSceneViewNode->SetName(defaultSceneName);
    newSceneViewNode->SetSceneViewDescription("Scene at MRML file save point");
    this->MRMLScene->AddNode(newSceneViewNode);

    // create a storage node
    vtkMRMLStorageNode *storageNode = newSceneViewNode->CreateDefaultStorageNode();
    // set the file name from the node name
    std::string fname = std::string(newSceneViewNode->GetName()) + std::string(".png");
    storageNode->SetFileName(fname.c_str());
    this->MRMLScene->AddNode(storageNode);
    newSceneViewNode->SetAndObserveStorageNodeID(storageNode->GetID());
    storageNode->Delete();

    // use the new one
    sceneViewNode = newSceneViewNode;
    }
  else
    {
    // take the first one and over write it
    sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(oldSceneViewNodes->GetItemAsObject(0));
    }
  // take a screen shot of the full layout
  sceneViewNode->SetScreenShotType(4);
  // create a screenShot of the full layout
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewport();
  // don't block the screenshot
  this->hide();
  QPixmap screenShot = QPixmap::grabWidget(widget);
  this->show();
  // convert to vtkImageData
  vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
  qMRMLUtils::qImageToVtkImageData(screenShot.toImage(), imageData);
  sceneViewNode->SetScreenShot(imageData);
  // mark it modified since read so that the screen shot will get saved to disk
  sceneViewNode->ModifiedSinceReadOn();
  sceneViewNode->StoreScene();
  
  // force a write
  sceneViewNode->GetStorageNode()->WriteData(sceneViewNode);
  // clean up
  if (newSceneViewNode)
    {
    newSceneViewNode->Delete();
    }
  
  // remove unreferenced nodes
  // TODO: the MRML Scene should deal with this on Commit or the
  // modules should respond to NodeRemoved events and keep the scene clean.
  /*
  vtkMRMLLogic *mrmlLogic = vtkMRMLLogic::New();
  mrmlLogic->SetScene(this->MRMLScene);
  mrmlLogic->RemoveUnreferencedDisplayNodes();
  mrmlLogic->RemoveUnreferencedStorageNodes();
  mrmlLogic->Delete();
  */
  
  this->MRMLScene->SetURL(file.absoluteFilePath().toLatin1());
  // TODO
  this->MRMLScene->SetVersion("Slicer4");
  bool res = this->MRMLScene->Commit();
  return res;
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
  // Select all the "modified" nodes.
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
  // Search for the item whose format has changed
  QComboBox* formatComboBox = qobject_cast<QComboBox*>(this->sender());
  Q_ASSERT(formatComboBox);
  const int rowCount = this->FileWidget->rowCount();
  int row = 0;
  for (; row < rowCount; ++row)
    {
    if (formatComboBox == this->FileWidget->cellWidget(row, FileFormatColumn))
      {
      break;
      }
    }
  Q_ASSERT(row < rowCount);

  // In case the combobox was editable (hack to display custom text), we now
  // don't need this property anymore.
  formatComboBox->setEditable(false);

  // Set the new selected extension to the file name
  QString extension = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
    formatComboBox->currentText().toLatin1());
  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  Q_ASSERT(fileNameItem);
  fileNameItem->setText(QFileInfo(fileNameItem->text()).baseName() + extension);
  fileNameItem->setData(Qt::UserRole, extension);

  // If the user changed the format, that means he wants to save the node
  // Select the row to mark the node to be saved.
  QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
  Q_ASSERT(nodeNameItem);
  nodeNameItem->setCheckState(Qt::Checked);
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialog::qSlicerSaveDataDialog(QObject* parentObject)
  : qSlicerFileDialog(parentObject)
  , d_ptr(new qSlicerSaveDataDialogPrivate(0))
{
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialog::~qSlicerSaveDataDialog()
{
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSaveDataDialog::fileType()const
{
  // FIXME: not really a nofile, but more a collection of files
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
  Q_D(qSlicerSaveDataDialog);
  Q_UNUSED(readerProperties);
  Q_ASSERT(!readerProperties.contains("fileName"));

  d->setMRMLScene(qSlicerCoreApplication::application()->mrmlScene());
  if (d->exec() != QDialog::Accepted)
    {
    return false;
    }
  return true;
}
