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
#include <ctkVTKWidgetsUtils.h>

/// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileWriterOptionsWidget.h"
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
  this->MRMLScene = 0;
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
    QString extension = index.data(qSlicerSaveDataDialogPrivate::FileExtensionRole).toString();
    lineEdit->setValidator(
      new QRegExpValidator(qSlicerFileNameItemDelegate::fileNameRegExp(extension), lineEdit));
    }
  return widget;
}

//-----------------------------------------------------------------------------
void qSlicerFileNameItemDelegate
::setModelData(QWidget *editor, QAbstractItemModel *model,
               const QModelIndex &index) const
{
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
  if (lineEdit)
    {
    QString extension = model->data(index, qSlicerSaveDataDialogPrivate::FileExtensionRole).toString();
    QString nodeID = model->data(index, qSlicerSaveDataDialogPrivate::UIDRole).toString();
    lineEdit->setText(
      qSlicerFileNameItemDelegate::fixupFileName(lineEdit->text(), extension, this->MRMLScene, nodeID));
    }
  this->Superclass::setModelData(editor, model, index);
}

//-----------------------------------------------------------------------------
QString qSlicerFileNameItemDelegate::fixupFileName(const QString& fileName, const QString& extension,
                                                   vtkMRMLScene* mrmlScene, const QString& nodeID)
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

  QString strippedFileName = fixup;
  if(mrmlScene)
    {
    vtkObject * object = mrmlScene;
    if (!nodeID.isEmpty())
      {
      object = qSlicerSaveDataDialogPrivate::getNodeByID(nodeID.toLatin1().data(), mrmlScene);
      }
    if (!object)
      {
      qCritical() << Q_FUNC_INFO << " failed: node not found by ID " << qPrintable(nodeID);
      return QString();
      }
    strippedFileName = qSlicerSaveDataDialogPrivate::stripKnownExtension(fixup, object);
    strippedFileName += extension;
    }
  return strippedFileName;
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
  : QDialog(parentWidget)
{
  this->MRMLScene = 0;

  this->setupUi(this);
  this->FileWidget->setItemDelegateForColumn(
    FileNameColumn, new qSlicerFileNameItemDelegate(this));
  this->FileWidget->verticalHeader()->setVisible(false);

  // Checkable headers.
  // We replace the current FileWidget header view with a checkable header view.
  // Checked files (rows) will be saved, unchecked files will be discarded.
  // In order to have a column checkable, we need to manually set a checkstate
  // to a column. No checkstate (null QVariant) means uncheckable.
  this->FileWidget->model()->setHeaderData(SelectColumn, Qt::Horizontal,
                                           Qt::Unchecked, Qt::CheckStateRole);
  QHeaderView* previousHeaderView = this->FileWidget->horizontalHeader();
  ctkCheckableHeaderView* headerView = new ctkCheckableHeaderView(Qt::Horizontal, this->FileWidget);
  // Copy the previous behavior of the header into the new checkable header view
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  headerView->setClickable(previousHeaderView->isClickable());
  headerView->setMovable(previousHeaderView->isMovable());
#else
  headerView->setSectionsClickable(previousHeaderView->sectionsClickable());
  headerView->setSectionsMovable(previousHeaderView->sectionsMovable());
#endif
  headerView->setHighlightSections(previousHeaderView->highlightSections());
  headerView->setStretchLastSection(previousHeaderView->stretchLastSection());
  // Propagate to top-level items only (depth = 1),no need to go deeper
  // (depth = -1 or 2, 3...) as it is a flat list.
  headerView->checkableModelHelper()->setPropagateDepth(1);
  // Finally assign the new header to the view
  this->FileWidget->setHorizontalHeader(headerView);

  // Connect push buttons to associated actions
  connect(this->DirectoryButton, SIGNAL(directorySelected(QString)),
          this, SLOT(setDirectory(QString)));
  connect(this->SelectSceneDataButton, SIGNAL(clicked()),
          this, SLOT(selectModifiedSceneData()));
  connect(this->SelectDataButton, SIGNAL(clicked()),
          this, SLOT(selectModifiedData()));
  connect(this->DataBundleButton, SIGNAL(clicked()),
          this, SLOT(saveSceneAsDataBundle()));
  connect(this->ShowMoreCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(showMoreColumns(bool)));
  this->showMoreColumns(this->ShowMoreCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
qSlicerSaveDataDialogPrivate::~qSlicerSaveDataDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  this->MRMLScene = scene;

  qSlicerFileNameItemDelegate * fileNameItemDelegate =
      dynamic_cast<qSlicerFileNameItemDelegate*>(this->FileWidget->itemDelegateForColumn(Self::FileNameColumn));
  Q_ASSERT(fileNameItemDelegate);
  fileNameItemDelegate->MRMLScene = scene;

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
  for (int row = 0; row < rowCount; ++row)
    {
    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    Q_ASSERT(selectItem);
    if (selectItem->checkState() == Qt::Unchecked)
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

  QDir newDir(this->MRMLScene->GetRootDirectory());
  if (!newDir.exists())
    {
    this->MRMLScene->SetRootDirectory(".");
    }

  // sorting the table while doing insertions is dangerous
  // Moreover, we want to have the MRML scene to be the first item.
  this->FileWidget->setSortingEnabled(false);

  this->DirectoryButton->setDirectory(this->MRMLScene->GetRootDirectory());

  this->populateScene();

  // get all storable nodes in the main scene
  // and store them in the map by ID to avoid duplicates for the scene views
  std::map<std::string, vtkMRMLNode *> storableNodes;
  std::vector<vtkMRMLNode *> nodes;
  this->MRMLScene->GetNodesByClass("vtkMRMLStorableNode", nodes);
  std::vector<vtkMRMLNode *>::iterator it;

  for (it = nodes.begin(); it != nodes.end(); it++)
    {
    vtkMRMLNode* node = (*it);
    this->populateNode(node);
    storableNodes[std::string(node->GetID())] = node;
    }

  // get all additioanl storable nodes for all scene views except "Master Scene View"
  nodes.clear();
  this->MRMLScene->GetNodesByClass("vtkMRMLSceneViewNode", nodes);
  for (it = nodes.begin(); it != nodes.end(); it++)
    {
    vtkMRMLSceneViewNode *svNode = vtkMRMLSceneViewNode::SafeDownCast(*it);
    // skip "Master Scene View" since it contains the same ndoes as the scene
    if (svNode->GetName() && std::string("Master Scene View") == std::string(svNode->GetName()))
      {
      continue;
      }
    std::vector<vtkMRMLNode *> snodes;
    svNode->GetNodesByClass("vtkMRMLStorableNode", snodes);
    std::vector<vtkMRMLNode *>::iterator sit;
    for (sit = snodes.begin(); sit != snodes.end(); sit++)
      {
      vtkMRMLNode* node = (*sit);
      if (storableNodes.find(std::string(node->GetID())) == storableNodes.end())
        {
        // process only new storable nodes
        this->populateNode(node);
        storableNodes[std::string(node->GetID())] = node;
        }
      }
    }


  // Here we could have restore the sorting property but we want to keep the
  // MRML scene the first item of the list so we don't do restore the sorting.
  // this->FileWidget->setSortingEnabled(oldSortingEnabled);

  // Enable/disable nodes depending on the scene file format
  this->onSceneFormatChanged();

  this->updateSize();
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::populateScene()
{
  // Create a new entry
  int row = this->FileWidget->rowCount();
  this->FileWidget->insertRow(row);

  // Get absolute filename
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
                                 "yyyy-MM-dd") + "-Scene.mrml");
    }

  // Scene Name
  QTableWidgetItem* sceneNameItem = new QTableWidgetItem("");
  sceneNameItem->setFlags(sceneNameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  this->FileWidget->setItem(row, NodeNameColumn, sceneNameItem);

  // Scene Type
  QTableWidgetItem* sceneTypeItem = new QTableWidgetItem("");
  sceneTypeItem->setData(Self::SceneTypeRole, QString("Scene"));
  sceneTypeItem->setFlags(sceneTypeItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  this->FileWidget->setItem(row, NodeTypeColumn, sceneTypeItem);

  // Scene Status
  QTableWidgetItem* sceneModifiedItem = new QTableWidgetItem(
    this->MRMLScene->GetModifiedSinceRead() ? "Modified" : "Not Modified");
  sceneModifiedItem->setFlags(sceneModifiedItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  this->FileWidget->setItem(row, NodeStatusColumn, sceneModifiedItem);

  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();

  // Scene Format
  QComboBox* sceneComboBoxWidget = new QComboBox(this->FileWidget);
  int currentFormat = -1;
  QString currentExtension = Self::extractKnownExtension(sceneFileInfo.fileName(), this->MRMLScene);
  foreach(const QString& nameFilter,
          coreIOManager->fileWriterExtensions(this->MRMLScene))
    {
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toLatin1()));
    sceneComboBoxWidget->addItem(nameFilter, extension);
    if (extension == currentExtension)
      {
      currentFormat = sceneComboBoxWidget->count() - 1;
      }
    }
  sceneComboBoxWidget->setCurrentIndex(currentFormat);

  this->FileWidget->setCellWidget(row, FileFormatColumn, sceneComboBoxWidget);
  QObject::connect(sceneComboBoxWidget, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(formatChanged()));
  QObject::connect(sceneComboBoxWidget, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onSceneFormatChanged()));

  // Scene FileName
  QTableWidgetItem* fileNameItem = this->createFileNameItem(sceneFileInfo, ".mrml", /* nodeID = */ QString());
  this->FileWidget->setItem( row, FileNameColumn, fileNameItem);

  // Scene Directory
  ctkDirectoryButton* sceneDirectoryButton =
      this->createFileDirectoryWidget(sceneFileInfo);

  this->FileWidget->setCellWidget(row, FileDirectoryColumn, sceneDirectoryButton);

  // Scene Selected
  QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
  selectItem->setFlags(selectItem->flags() | Qt::ItemIsUserCheckable);
  selectItem->setCheckState(
    this->MRMLScene->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);

  // Options
  this->updateOptionsWidget(row);

  // Set current scene file format based on last successful scene save
  int lastFormatIndex = sceneComboBoxWidget->findText(this->LastMRMLSceneFileFormat);
  if (lastFormatIndex != -1)
    {
    sceneComboBoxWidget->setCurrentIndex(lastFormatIndex);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::populateNode(vtkMRMLNode* node)
{
  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(node);
  // Don't show if the node doesn't want to (internal node)
  if (!storableNode ||
    storableNode->GetHideFromEditors() || !storableNode->GetSaveWithScene())
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
  // Get absolute filename and create storage node if needed
  QFileInfo fileInfo = this->nodeFileInfo(storableNode);
  if (fileInfo == QFileInfo())
    {
    return;
    }

  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  Q_ASSERT(coreIOManager);
  // Must be called after nodeFileInfo() as it creates a storage node
  // that is mandatory for fileWriterFileType()
  if (coreIOManager->fileWriterFileType(node) == QString("NoFile"))
    {
    return;
    }

  if (!storableNode->GetStorageNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: storage node not found for node "
      << (storableNode->GetID() ? storableNode->GetID() : "(unknown)")
      << ". The node will not be shown in the save data dialog.";
    return;
    }

  // Create a new entry
  int row = this->FileWidget->rowCount();
  this->FileWidget->insertRow(row);

  // Node name
  QTableWidgetItem *nodeNameItem = this->createNodeNameItem(storableNode);
  this->FileWidget->setItem(row, NodeNameColumn, nodeNameItem);

  // Node type
  QTableWidgetItem *nodeTypeItem = this->createNodeTypeItem(storableNode);
  this->FileWidget->setItem(row, NodeTypeColumn, nodeTypeItem);

  // Node status
  QTableWidgetItem *nodeModifiedItem = this->createNodeStatusItem(storableNode, fileInfo);
  this->FileWidget->setItem(row, NodeStatusColumn, nodeModifiedItem);

  // File format
  QComboBox* fileFormatsWidget = qobject_cast<QComboBox*>(
    this->createFileFormatsWidget(storableNode, fileInfo));
  this->FileWidget->setCellWidget(row, FileFormatColumn, fileFormatsWidget);
  QString extension = fileFormatsWidget->itemData(
    fileFormatsWidget->currentIndex()).toString();

  // File name
  QTableWidgetItem *fileNameItem = this->createFileNameItem(fileInfo, extension, QString(node->GetID()));
  this->FileWidget->setItem(row, FileNameColumn, fileNameItem);

  // File Directory
  QWidget* directoryWidget = this->createFileDirectoryWidget(fileInfo);
  this->FileWidget->setCellWidget(row, FileDirectoryColumn, directoryWidget);

  // Select modified nodes by default
  QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
  selectItem->setCheckState(
    storableNode->GetModifiedSinceRead() ? Qt::Checked : Qt::Unchecked);

  // Options
  this->updateOptionsWidget(row);
}

//-----------------------------------------------------------------------------
QFileInfo qSlicerSaveDataDialogPrivate::nodeFileInfo(vtkMRMLStorableNode* node)
{
  // Remove characters from node name that cannot be used in file names
  // (same method as in qSlicerFileNameItemDelegate::fixupFileName)
  QString inputNodeName(node->GetName() ? node->GetName() : "");
  QString safeNodeName;
  QRegExp regExp = qSlicerFileNameItemDelegate::fileNameRegExp();
  for (int i = 0; i < inputNodeName.size(); ++i)
    {
    if (regExp.exactMatch(QString(inputNodeName[i])))
      {
      safeNodeName += inputNodeName[i];
      }
    }

  vtkMRMLStorageNode* snode = node->GetStorageNode();
  if (snode == 0)
    {
    bool success = node->AddDefaultStorageNode();
    if (!success)
      {
      qCritical() << Q_FUNC_INFO << " failed: error while trying to add storage node";
      return QFileInfo();
      }
    snode = node->GetStorageNode();
    if (!snode)
      {
      // no error and no storage node means that
      // there is no need for storage node, the node can be stored in the scene
      return QFileInfo();
      }
    }
  else
    {
    // a file name exists, but we want to update the filename to match the current
    // node name
    if (snode->GetFileName() && node->GetName())
      {
      std::string filenameWithoutExtension = snode->GetFileNameWithoutExtension();
      // Only reset the file name if the user has set the name explicitly (that is,
      // if the name isn't the default created by qSlicerVolumesIOOptionsWidget::setFileNames
      // TODO: this logic relies on the GUI so we should consider moving it into MRML proper
      // with a way for storage nodes to generate their default node names from a given filename
      if (QString(filenameWithoutExtension.c_str()) != safeNodeName)
        {
        QFileInfo existingInfo(snode->GetFileName());
        std::string extension = snode->GetSupportedFileExtension();
        QFileInfo newInfo(existingInfo.absoluteDir(), safeNodeName + QString(extension.c_str()));
        snode->SetFileName(newInfo.absoluteFilePath().toLatin1());
        node->StorableModified();
        }
      }
    }
  if (snode->GetFileName() == 0 && !this->DirectoryButton->directory().isEmpty())
    {
    QString fileExtension = snode->GetDefaultWriteFileExtension();
    if (!fileExtension.isEmpty())
      {
      fileExtension = QString(".") + fileExtension;
      }

    QFileInfo fileName(QDir(this->DirectoryButton->directory()),
                       safeNodeName + fileExtension);
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
  nodeNameItem->setFlags( nodeNameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  // the tooltip is used to store the id of the nodes
  Q_ASSERT(node->GetStorageNode());
  nodeNameItem->setData(Qt::ToolTipRole, QString(node->GetID()) + " " + node->GetStorageNode()->GetID() );
  return nodeNameItem;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerSaveDataDialogPrivate::createNodeTypeItem(vtkMRMLStorableNode* node)
{
  QTableWidgetItem* nodeTypeItem = new QTableWidgetItem(node->GetNodeTagName());
  nodeTypeItem->setFlags( nodeTypeItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  // TODO: add icon based on the type
  return nodeTypeItem;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qSlicerSaveDataDialogPrivate
::createNodeStatusItem(vtkMRMLStorableNode* node, const QFileInfo& fileInfo)
{
  Q_UNUSED(fileInfo);
  // Node status (modified / not modified)
  // As a safety measure:
  // If the data is sitting in cache, it's vulnerable to overwriting or deleting.
  // Mark the node as modified since read so that a user will be more likely
  // to save it to a reliable location on local (or remote) disk.
  /*
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
  */
  QTableWidgetItem *nodeModifiedItem =
    new QTableWidgetItem(node->GetModifiedSinceRead() ?
                         tr("Modified") : tr("Not Modified"));
  nodeModifiedItem->setFlags( nodeModifiedItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled);
  return nodeModifiedItem;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSaveDataDialogPrivate::createFileFormatsWidget(vtkMRMLStorableNode* node, QFileInfo& fileInfo)
{
  vtkMRMLStorageNode* snode = node->GetStorageNode();
  Q_ASSERT(snode);
  QComboBox* fileFormats = new QComboBox(this->FileWidget);
  // Add custom qSlicerSaveFile
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  int currentFormat = -1;
  QString currentExtension = coreIOManager->completeSlicerWritableFileNameSuffix(node);
  foreach(QString nameFilter, coreIOManager->fileWriterExtensions(node))
    {
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toLatin1()));
    fileFormats->addItem(nameFilter, extension);
    if (extension == currentExtension)
      {
      currentFormat = fileFormats->count() - 1;
      }
    }
  // The existing file name doesn't contain an existing extension, pick the
  // default extension if any
  if (currentFormat == -1 &&
      snode->GetDefaultWriteFileExtension() != 0)
    {
    for (int i = 0; i < fileFormats->count(); ++i)
      {
      if (fileFormats->itemData(i).toString() ==
          QString('.') + QString(snode->GetDefaultWriteFileExtension()))
        {
        currentFormat = i;
        fileInfo = QFileInfo(fileInfo.dir(),
                             fileInfo.completeBaseName() +
                               fileFormats->itemData(i).toString());
        break;
        }
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
::createFileNameItem(const QFileInfo& fileInfo, const QString& extension, const QString& nodeID)
{
  QTableWidgetItem* fileNameItem = new QTableWidgetItem(
    qSlicerFileNameItemDelegate::fixupFileName(fileInfo.fileName(), extension, this->mrmlScene(), nodeID));
  if (!extension.isEmpty())
    {
    fileNameItem->setData(Self::FileExtensionRole, extension);
    }
  fileNameItem->setData(Self::UIDRole, nodeID);
  return fileNameItem;
}

//-----------------------------------------------------------------------------
ctkDirectoryButton* qSlicerSaveDataDialogPrivate::createFileDirectoryWidget(const QFileInfo& fileInfo)
{
  // TODO: use QSignalMapper
  ctkDirectoryButton * directoryButton = new ctkDirectoryButton(fileInfo.absolutePath(),this->FileWidget);
  directoryButton->setOptions(ctkDirectoryButton::DontUseNativeDialog);
  directoryButton->setAcceptMode(QFileDialog::AcceptSave);
  return directoryButton;
}

//-----------------------------------------------------------------------------
QString qSlicerSaveDataDialogPrivate::extractKnownExtension(const QString& fileName, vtkObject* object)
{
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();

  foreach(const QString& nameFilter,
          coreIOManager->fileWriterExtensions(object))
    {
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toLatin1()));
    if (!extension.isEmpty() && fileName.endsWith(extension))
      {
      return extension;
      }
    }
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerSaveDataDialogPrivate::stripKnownExtension(const QString& fileName, vtkObject* object)
{
  QString strippedFileName(fileName);

  QString knownExtension = Self::extractKnownExtension(fileName, object);
  if (!knownExtension.isEmpty())
    {
    strippedFileName.chop(knownExtension.length());
    // check that the extension wasn't doubled by having the file name be
    // constructed from a node name that included the extension
    if (strippedFileName.endsWith(knownExtension))
      {
      return Self::stripKnownExtension(strippedFileName, object);
      }
    return strippedFileName;
    }
  return strippedFileName;
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
  if (this->mustSceneBeSaved())
    {
    if (!this->saveScene())
      {
      return false;
      }
    }
  else
    {
    // restore the root directory only if the scene is not saved
    this->restoreAfterSaving();
    }
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::saveNodes()
{
  QMessageBox::StandardButton forceOverwrite = QMessageBox::Ignore;
  QList<qSlicerIO::IOProperties> files;
  const int sceneRow = this->findSceneRow();
  for (int row = 0; row < this->FileWidget->rowCount(); ++row)
    {
    // only save nodes here
    if (row == sceneRow)
      {
      continue;
      }

    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
    QTableWidgetItem* nodeStatusItem = this->FileWidget->item(row, NodeStatusColumn);

    Q_ASSERT(selectItem);
    Q_ASSERT(nodeNameItem);

    // don't save unchecked nodes
    if (selectItem->checkState() != Qt::Checked ||
        (selectItem->flags() & Qt::ItemIsEnabled) == 0)
      {
      continue;
      }

    // file properties
    QFileInfo file = this->file(row);
    QString format = this->format(row);
    qSlicerIOOptions* options = this->options(row);
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(this->object(row));

    if (file.fileName().isEmpty())
      {
      QMessageBox::warning(this, tr("Saving node..."),
                           tr("Node %1 not saved, file name is empty.").arg(nodeNameItem->text()));
      continue;
      }

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
    qSlicerCoreIOManager* coreIOManager =
      qSlicerCoreApplication::application()->coreIOManager();
    Q_ASSERT(coreIOManager);
    qSlicerIO::IOFileType fileType = coreIOManager->fileWriterFileType(node);
    qSlicerIO::IOProperties savingParameters;
    if (options)
      {
      // options properties nodeID and fileName will be overwritten
      // \todo fileName is wrong as it contains an obsolete directory
      savingParameters = options->properties();
      }
    savingParameters["nodeID"] = QString(node->GetID());
    savingParameters["fileName"] = file.absoluteFilePath();
    savingParameters["fileFormat"] = format;
    bool res = coreIOManager->saveNodes(fileType, savingParameters);

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
    nodeNameItem->setCheckState(Qt::Unchecked);
    nodeStatusItem->setText("Not Modified");
    }
  return true;
}

//-----------------------------------------------------------------------------
QFileInfo qSlicerSaveDataDialogPrivate::file(int row)const
{
  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  Q_ASSERT(fileNameItem);

  ctkDirectoryButton* fileDirectoryButton = qobject_cast<ctkDirectoryButton*>(
    this->FileWidget->cellWidget(row, FileDirectoryColumn));
  Q_ASSERT(fileDirectoryButton);

  QDir directory = fileDirectoryButton->directory();
  return QFileInfo(directory, fileNameItem->text());
}

//-----------------------------------------------------------------------------
vtkObject* qSlicerSaveDataDialogPrivate::object(int row)const
{
  if (this->type(row) == tr("Scene"))
    {
    return this->MRMLScene;
    }
  QTableWidgetItem* nodeNameItem = this->FileWidget->item(row, NodeNameColumn);
  Q_ASSERT(nodeNameItem);

  /// \todo support mrmlScene row
  QStringList nodeIDs = nodeNameItem->data(Qt::ToolTipRole).toString().split(" ");
  vtkMRMLNode *node = this->getNodeByID(nodeIDs[0].toLatin1().data());
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerSaveDataDialogPrivate::getNodeByID(char *id)const
{
  return qSlicerSaveDataDialogPrivate::getNodeByID(id, this->MRMLScene);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerSaveDataDialogPrivate::getNodeByID(char *id, vtkMRMLScene* scene)
{
  vtkMRMLNode *node = scene->GetNodeByID(id);
  if (node == 0)
    {
    // search in SceneView nodes
    std::string sID(id);
    std::vector<vtkMRMLNode *> nodes;
    scene->GetNodesByClass("vtkMRMLSceneViewNode", nodes);
    std::vector<vtkMRMLNode *>::iterator it;

    for (it = nodes.begin(); it != nodes.end(); it++)
      {
      vtkMRMLSceneViewNode *svNode = vtkMRMLSceneViewNode::SafeDownCast(*it);
      // skip "Master Scene View" since it contains the same ndoes as the scene
      if (svNode->GetName() && std::string("Master Scene View") == std::string(svNode->GetName()))
        {
        continue;
        }
      std::vector<vtkMRMLNode *> snodes;
      svNode->GetNodesByClass("vtkMRMLStorableNode", snodes);
      std::vector<vtkMRMLNode *>::iterator sit;
      for (sit = snodes.begin(); sit != snodes.end(); sit++)
        {
        vtkMRMLNode* snode = (*sit);
        if (std::string(snode->GetID()) == sID)
          {
          return snode;
          }
        }
      }
    }
  return node;
}

//-----------------------------------------------------------------------------
QString qSlicerSaveDataDialogPrivate::format(int row)const
{
  QComboBox* fileFormatComboBox = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(row, FileFormatColumn));
  Q_ASSERT(fileFormatComboBox);
  return fileFormatComboBox->currentText();
}

//-----------------------------------------------------------------------------
QString qSlicerSaveDataDialogPrivate::type(int row)const
{
  QTableWidgetItem* typeItem = this->FileWidget->item(row, NodeTypeColumn);
  Q_ASSERT(typeItem);
  return typeItem->data(Self::SceneTypeRole).toString();
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerSaveDataDialogPrivate::options(int row)const
{
  qSlicerIOOptionsWidget* optionsWidget = qobject_cast<qSlicerIOOptionsWidget*>(
    this->FileWidget->cellWidget(row, OptionsColumn));
  return optionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::mustSceneBeSaved()const
{
  QAbstractItemModel* model = this->FileWidget->model();
  QModelIndexList found = model->match(
    model->index(0, NodeTypeColumn),
    Self::SceneTypeRole, QString("Scene"), 1, Qt::MatchExactly);
  if (found.count() == 0 || !found[0].isValid())
    {
    return false;
    }
  QTableWidgetItem* selectItem =
    this->FileWidget->item(found[0].row(), SelectColumn);

  return selectItem->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
int qSlicerSaveDataDialogPrivate::findSceneRow()const
{
  QAbstractItemModel* model = this->FileWidget->model();
  QModelIndexList found = model->match(
    model->index(0, NodeTypeColumn),
    Self::SceneTypeRole, QString("Scene"), 1, Qt::MatchExactly);
  return found[0].row();
}

//-----------------------------------------------------------------------------
QFileInfo qSlicerSaveDataDialogPrivate::sceneFile()const
{
  // search the scene
  int sceneRow = this->findSceneRow();

  QTableWidgetItem* fileNameItem = this->FileWidget->item(sceneRow, FileNameColumn);
  ctkDirectoryButton* fileDirectoryButton = qobject_cast<ctkDirectoryButton*>(
    this->FileWidget->cellWidget(sceneRow, FileDirectoryColumn));

  QDir directory = fileDirectoryButton->directory();
  QFileInfo file = QFileInfo(directory, fileNameItem->text());
  if (file.fileName().isEmpty())
    {
    file = QFileInfo(QString(this->MRMLScene->GetURL()));
    }
  return file;
}

//-----------------------------------------------------------------------------
QString qSlicerSaveDataDialogPrivate::sceneFileFormat()const
{
  return this->format(this->findSceneRow());
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::prepareForSaving()
{
  QFileInfo file = this->sceneFile();
  if (file.exists() && this->mustSceneBeSaved())
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
  this->MRMLSceneRootDirectoryBeforeSaving = this->MRMLScene->GetRootDirectory();
  this->setSceneRootDirectory(file.absoluteDir().absolutePath().toLatin1());
  return true;
}


//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::restoreAfterSaving()
{
  this->setSceneRootDirectory(this->MRMLSceneRootDirectoryBeforeSaving);
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::setSceneRootDirectory(const QString& dir)
{
  this->MRMLScene->SetRootDirectory(dir.toLatin1());

  // update the root directory of scene snapshot nodes (not sure why)
  const int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode* node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSceneViewNode");
    vtkMRMLSceneViewNode *snode = vtkMRMLSceneViewNode::SafeDownCast(node);
    if (snode && snode->GetStoredScene())
      {
      snode->GetStoredScene()->SetRootDirectory(this->MRMLScene->GetRootDirectory());
      }
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSaveDataDialogPrivate::saveScene()
{
  qSlicerIO::IOProperties properties;
  QFileInfo file = this->sceneFile();
  properties["fileName"] = file.absoluteFilePath();

  // create a screenShot of the full layout
  if (qSlicerApplication::application()->layoutManager())
    {
    QWidget* widget = qSlicerApplication::application()->layoutManager()->viewport();
    this->hide();  // don't block the screenshot
    QImage screenShot = ctk::grabVTKWidget(widget);
    this->show();
    properties["screenShot"] = screenShot;
    }

  qSlicerIOOptions* options = this->options(this->findSceneRow());
  if (options)
    {
    properties.unite(options->properties());
    }

  bool res = qSlicerApplication::application()->coreIOManager()->saveNodes(
    QString("SceneFile"), properties);

  if (res)
    {
    this->LastMRMLSceneFileFormat = this->sceneFileFormat();
    }
  else
    {
    QMessageBox::StandardButton answer =
      QMessageBox::question(this, tr("Saving scene..."),
                            tr("Cannot write scene file: %1.\n"
                               "Do you want to ignore this error and close saving?").arg(file.absoluteFilePath()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer == QMessageBox::Yes)
      {
      res = true;
      }
    }

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
    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    Q_ASSERT(selectItem);
    selectItem->setCheckState(
      statusItem->text() == tr("Modified") &&
      typeItem->data(Self::SceneTypeRole).toString() != tr("Scene") ? Qt::Checked : Qt::Unchecked);
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
    QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
    Q_ASSERT(selectItem);
    selectItem->setCheckState(
      statusItem->text() == tr("Modified") ? Qt::Checked : Qt::Unchecked);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::saveSceneAsDataBundle()
{
  int sceneRow = this->findSceneRow();
  QComboBox* box = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(sceneRow, FileFormatColumn));
  int mrbIndex = box->findText("mrb", Qt::MatchContains);
  int mrmlIndex = box->findText("mrml", Qt::MatchContains);
  // Toggle between scene data bundle entry and mrml entry
  if (mrbIndex != -1)
    {
    if (box->currentIndex() != mrbIndex)
      {
      box->setCurrentIndex(mrbIndex);
      }
    else
      {
      if (mrmlIndex != -1)
        {
        box->setCurrentIndex(mrmlIndex);
        }
      }
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

  QTableWidgetItem* fileNameItem = this->FileWidget->item(row, FileNameColumn);
  Q_ASSERT(fileNameItem);

  // Set the new selected extension to the file name
  QString extension = QString::fromStdString(vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
    formatComboBox->currentText().toLatin1()));
  if (extension == "*")
    {
    extension = QString();
    }
  fileNameItem->setData(Self::FileExtensionRole, extension);

  // Update fileName based on new selected extension
  QString nodeID = fileNameItem->data(Self::UIDRole).toString();
  fileNameItem->setText(
        qSlicerFileNameItemDelegate::fixupFileName(fileNameItem->text(), extension, this->MRMLScene, nodeID));

  // If the user changed the format, that means he wants to save the node
  // Select the row to mark the node to be saved.
  QTableWidgetItem* selectItem = this->FileWidget->item(row, SelectColumn);
  Q_ASSERT(selectItem);
  selectItem->setCheckState(Qt::Checked);

  this->updateOptionsWidget(row);
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::updateOptionsWidget(int row)
{
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  qSlicerIOOptions* options =
    coreIOManager->fileWriterOptions(this->object(row), this->format(row));
  qSlicerFileWriterOptionsWidget* optionsWidget =
    dynamic_cast<qSlicerFileWriterOptionsWidget*>(options);
  if (optionsWidget)
    {
    // The optionsWidget can use the filename to initialize some options.
    optionsWidget->setFileName(this->file(row).absoluteFilePath());
    optionsWidget->setObject(this->object(row));
    // TODO: support uneven rows. Until that day, we want to make sure the whole
    // widget is visible
    optionsWidget->setMinimumWidth(optionsWidget->sizeHint().width());
    // TODO: connect signal validChanged(bool) with the accept button
    }
  else
    {
    // we can't use options that are not widgets
    delete options;
    }
  this->FileWidget->setCellWidget(row, OptionsColumn, optionsWidget);
  this->FileWidget->resizeColumnToContents(OptionsColumn);
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::showMoreColumns(bool show)
{
  this->FileWidget->setColumnHidden(OptionsColumn, !show);
  this->FileWidget->setColumnHidden(NodeNameColumn, !show);
  this->FileWidget->setColumnHidden(NodeTypeColumn, !show);
  this->FileWidget->setColumnHidden(NodeStatusColumn, !show);
  this->updateSize();
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::updateSize()
{
  // let's try to resize the columns according to their new contents
  this->FileWidget->resizeColumnsToContents();
  // let's try to show the whole table on screen
  // TODO: find a function in Qt that does it automatically.
  this->resize(this->layout()->contentsMargins().left()
               + this->FileWidget->frameWidth()
//               + this->FileWidget->verticalHeader()->sizeHint().width()
               + this->FileWidget->horizontalHeader()->length()
               + this->FileWidget->frameWidth()
               + this->layout()->contentsMargins().right(),
               this->sizeHint().height());
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::onSceneFormatChanged()
{
  int sceneRow = this->findSceneRow();
  QComboBox* box = qobject_cast<QComboBox*>(
    this->FileWidget->cellWidget(sceneRow, FileFormatColumn));
  // Gray out all the nodes when saving scene as bundle
  this->enableNodes(box->currentIndex() == 0);
}

//-----------------------------------------------------------------------------
void qSlicerSaveDataDialogPrivate::enableNodes(bool enable)
{
  int sceneRow = this->findSceneRow();
  for (int i = 0; i < this->FileWidget->rowCount(); ++i)
    {
    if (i == sceneRow)
      {
      continue;
      }
    QTableWidgetItem* item = this->FileWidget->item(i, FileNameColumn);
    if (enable)
      {
      item->setFlags(item->flags() | Qt::ItemIsEnabled);
      }
    else
      {
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
      }
    QWidget* fileFormatWidget = this->FileWidget->cellWidget(i, FileFormatColumn);
    fileFormatWidget->setEnabled(enable);
    QWidget* fileDirectoryWidget = this->FileWidget->cellWidget(i, FileDirectoryColumn);
    fileDirectoryWidget->setEnabled(enable);
    }
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
  return QString("NoFile");
}

//-----------------------------------------------------------------------------
QString qSlicerSaveDataDialog::description()const
{
  return tr("Any Data");
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
