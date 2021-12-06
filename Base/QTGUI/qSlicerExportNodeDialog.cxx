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

==============================================================================*/

/// Qt includes
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

/// CTK includes
#include <ctkPathLineEdit.h>

/// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileWriterOptionsWidget.h"
#include "qSlicerExportNodeDialog_p.h"

/// MRML includes
#include <vtkDataFileFormatHelper.h> // for GetFileExtensionFromFormatString()
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLSceneViewNode.h>


//-----------------------------------------------------------------------------
static QString forceFileNameExtension(const QString& fileName, const QString& extension, vtkMRMLNode* node)
{
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  QString strippedFileName = qSlicerCoreIOManager::forceFileNameValidCharacters(fileName);
  strippedFileName = coreIOManager->stripKnownExtension(strippedFileName, node) + extension;
  return strippedFileName;
}


//-----------------------------------------------------------------------------
qSlicerExportNodeDialogPrivate::qSlicerExportNodeDialogPrivate(QWidget* parentWidget)
  : QDialog(parentWidget),
    lastUsedDirectory{},
    lastUsedFormats{},
    lastUsedHardenTransform{false},
    formatToOptionsWidget{}
{
  this->setupUi(this);

  connect(this->FilenameLineEdit, SIGNAL(editingFinished()),
          this, SLOT(onFilenameEditingFinished()));

  QObject::connect(this->ExportFormatComboBox, &QComboBox::currentTextChanged,
                   this, &qSlicerExportNodeDialogPrivate::formatChangedSlot);

  // Set up DirectoryPathLineEdit widget to be a directory selector
  this->DirectoryPathLineEdit->setLabel("Output folder");
  this->DirectoryPathLineEdit->setFilters(ctkPathLineEdit::Dirs);
}

//-----------------------------------------------------------------------------
qSlicerExportNodeDialogPrivate::~qSlicerExportNodeDialogPrivate() = default;

//-----------------------------------------------------------------------------
bool qSlicerExportNodeDialogPrivate::setup(vtkMRMLScene* scene, vtkMRMLNode* node)
{

  this->MRMLScene = scene;
  this->MRMLNode = node;

  this->DirectoryPathLineEdit->setCurrentPath(
    this->lastUsedDirectory.isEmpty() ? this->MRMLScene->GetRootDirectory() : this->lastUsedDirectory
  );

  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  if (!coreIOManager)
    {
    qCritical() << Q_FUNC_INFO << " failed: Core IO manager not found.";
    return false;
    }

  this->setWindowTitle(QString("Export ")+QString(this->MRMLNode->GetName()));

  // Now we will work on filling up the export formats dropdown box
  // with the various choices and initializaing a choice

  this->ExportFormatComboBox->clear();

  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(this->MRMLNode);
  if (!storableNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: Node is not storable.";
    return false;
    }

  // Get the favored extension according to the storage node (if there is one)
  QString extensionInStorageNode;
  vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
  if (storageNode)
    {
    if (storageNode->GetDefaultWriteFileExtension() != nullptr)
      {
      extensionInStorageNode = storageNode->GetDefaultWriteFileExtension();
      }
    }
  else { // If there wasn't a storage node, we make a temporary one just to get default extension information from it
    const std::string defaultStorageNodeClassName = storableNode->GetDefaultStorageNodeClassName();
    if (!defaultStorageNodeClassName.empty())
      {
      vtkSmartPointer<vtkMRMLNode> defaultStorageNodeAsNode = vtkSmartPointer<vtkMRMLNode>::Take(
        this->MRMLScene->CreateNodeByClass(defaultStorageNodeClassName.c_str())
      );
      vtkMRMLStorageNode* defaultStorageNode = vtkMRMLStorageNode::SafeDownCast(defaultStorageNodeAsNode);
      extensionInStorageNode = defaultStorageNode->GetDefaultWriteFileExtension();
      }
  }

  QString currentExtension = storageNode ?
    coreIOManager->completeSlicerWritableFileNameSuffix(storableNode) : QString(".");
  // (Checking that storageNode is not null allows us to dodge a warning from completeSlicerWritableFileNameSuffix)
  int suggestedFormatIndex = -1; // will be index corresponding to format corresponding to currentExtension
  foreach(QString nameFilter, coreIOManager->fileWriterExtensions(storableNode))
  {
    // extract extension (e.g. ".ext") from format description string (e.g. "Blahblahblah (.ext)")
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toUtf8()));

    // add the entry to the dropdown menu
    this->ExportFormatComboBox->addItem(nameFilter, extension);

    // if it was the current extension, remember the index
    if (extension == currentExtension)
      {
      suggestedFormatIndex = this->ExportFormatComboBox->count() - 1;
      }
  }

  // If it turns out currentExtension was not any of the choices,
  // then look for extensionInStorageNode
  if (suggestedFormatIndex == -1 && !extensionInStorageNode.isEmpty())
    {
    for (int i = 0; i < this->ExportFormatComboBox->count(); ++i)
      {
      if (this->ExportFormatComboBox->itemData(i).toString() ==  QString('.') + extensionInStorageNode)
        {
        suggestedFormatIndex = i;
        break;
        }
      }
    }

  // Look for the most recently used extension that is part of the dropdown list
  int previouslyUsedFormatIndex = -1;
  for (const QString & format : this->lastUsedFormats)
    {
    int formatIndex = this->ExportFormatComboBox->findText(format);
    if (formatIndex != -1)
      {
      previouslyUsedFormatIndex = formatIndex;
      break;
      }
    }

  // Prefer to use last used extension, then suggested extension found above,
  // then if all else fails just leave a prompt for the user to select a format
  if (previouslyUsedFormatIndex != -1)
    {
    this->ExportFormatComboBox->setCurrentIndex(previouslyUsedFormatIndex);
    }
  else if (suggestedFormatIndex != -1)
    {
    this->ExportFormatComboBox->setCurrentIndex(suggestedFormatIndex);
    }
  else
    {
    this->ExportFormatComboBox->blockSignals(true); // avoid triggering a formatChanged here
    this->ExportFormatComboBox->setCurrentIndex(-1);
    this->ExportFormatComboBox->setEditable(true); // so we can set custom text below
    this->ExportFormatComboBox->lineEdit()->setReadOnly(true);
    this->ExportFormatComboBox->setEditText("Select a format");
    this->ExportFormatComboBox->blockSignals(false);
    }

  // Initialize filename
  const QString unsafeNodeName(this->MRMLNode->GetName() ? this->MRMLNode->GetName() : "");
  const QString safeNodeName = qSlicerCoreIOManager::forceFileNameValidCharacters(unsafeNodeName);
  this->FilenameLineEdit->setText(
    forceFileNameExtension(safeNodeName, this->extension(), this->MRMLNode)
  );

  // Now that we have initialized the ExportFormatComboBox with a format,
  // we can initialize the options widget
  this->updateOptionsWidget();

  // Set keyboard focus to the filename textbox
  this->FilenameLineEdit->setFocus(Qt::ActiveWindowFocusReason);

  // Set coordinate system option visibility and initial state
  vtkMRMLTransformableNode* nodeAsTransformable = vtkMRMLTransformableNode::SafeDownCast(this->MRMLNode);
  if (nodeAsTransformable && nodeAsTransformable->GetParentTransformNode())
    {
    this->HardenTransformCheckBox->show();
    this->HardenTransformCheckBox->setChecked(this->lastUsedHardenTransform);
    }
  else
    {
    this->HardenTransformCheckBox->hide();
    this->HardenTransformCheckBox->setChecked(false);
    }


  return true; // success
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::accept()
{
  this->saveWidgetStates();

  if (!this->exportNode())
    {
    return;
    }
  this->done(QDialog::Accepted);
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::saveWidgetStates()
{
  this->lastUsedDirectory = this->DirectoryPathLineEdit->currentPath();

  if (this->HardenTransformCheckBox->isVisible())
    {
    this->lastUsedHardenTransform = this->HardenTransformCheckBox->isChecked();
    }

  // Save export format dropdown data into front of lastUsedFormats,
  // and pop the back if the list is long
  this->lastUsedFormats.push_front(this->formatText());
  if (this->lastUsedFormats.size()>20)
    {
    this->lastUsedFormats.pop_back();
    }
  // QList is similar to std::deque, prepend and append are fast:
  // https://doc.qt.io/qt-5/containers.html#algorithmic-complexity
}

//-----------------------------------------------------------------------------
bool qSlicerExportNodeDialogPrivate::exportNode()
{
  // If the current filename isn't the suggested one, give a prompt that suggests changing it
  // The user can either proceed without change
  // or the user can accept the change (in which case we make the change and proceed normally)
  // or the user can cancel the export (in which case we return false here, so they can return to the export dialog)
  QString betterFilename = this->recommendedFilename();
  if (this->FilenameLineEdit->text() != betterFilename)
    {
    QMessageBox messageBox(
      QMessageBox::Warning, // icon
      tr("Filename not standard"), // title
      tr("The following filename is recommended:\n")+betterFilename, // message text
      QMessageBox::NoButton, // buttons; they will be added after
      this // parent
    );
    QAbstractButton* acceptButton = messageBox.addButton("Accept recommended", QMessageBox::YesRole);
    messageBox.addButton("Keep my filename", QMessageBox::NoRole);
    QAbstractButton* cancelButton = messageBox.addButton(QMessageBox::Cancel);
    messageBox.exec();

    if (messageBox.clickedButton() == acceptButton)
      {
      this->FilenameLineEdit->setText(betterFilename);
      }
    else if (messageBox.clickedButton() == cancelButton)
      {
      return false;
      }
    }

  // file properties
  QFileInfo file = this->file();
  QString formatText = this->formatText();
  qSlicerFileWriterOptionsWidget* options = this->getOptionsWidget();

  if (file.fileName().isEmpty())
    {
    QMessageBox::critical(this, tr("Export error"),
        tr("Failed to export node %1; filename is empty.")
          .arg(this->MRMLNode->GetName())
    );
    return false;
    }

  if (file.exists())
    {
    QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Exporting node..."),
      tr("The file %1 already exists. Do you want to replace it?").arg(file.absoluteFilePath()),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::Yes
    );
    if (answer != QMessageBox::Yes)
      {
      return false;
      }
    }

  // export the node
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();

  QList<QString> nodesToExport;
  nodesToExport.push_back(QString(this->MRMLNode->GetID()));

  QList<QString> filePaths;
  filePaths.push_back(file.absoluteFilePath());

  qSlicerIO::IOProperties savingParameters;
  if (options)
    {
    savingParameters = options->properties();
    }
  savingParameters["fileFormat"] = formatText;
  savingParameters["hardenTransforms"] = this->HardenTransformCheckBox->isChecked();

  vtkNew<vtkMRMLMessageCollection> userMessages;

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  bool success = coreIOManager->exportNodes(nodesToExport, filePaths, savingParameters, userMessages);
  QApplication::restoreOverrideCursor();

  if (!success)
    {
    // Make sure an error message is added if saving returns with error
    userMessages->AddMessage(vtkCommand::ErrorEvent,
      (tr("Cannot write data file: %1.").arg(file.absoluteFilePath())).toStdString());

    QString messagesStr = QString::fromStdString(userMessages->GetAllMessagesAsString());

    // display messagesStr as an error message
    QMessageBox::critical(this, tr("Export error"), messagesStr);
    }
  // In case there are any errors or warnings in storage node, make sure to alert even in case of success:
  else if (userMessages->GetNumberOfMessages() > 0)
    {
    bool warningFound = false;
    bool errorFound = false;
    QString messagesStr = QString::fromStdString(userMessages->GetAllMessagesAsString(&errorFound, &warningFound));

    if (errorFound)
      {
      QMessageBox::critical(this, tr("Export error"), messagesStr);
      // If there was an error, this should never have been considered a success.
      success = false;
      qWarning() << Q_FUNC_INFO << " warning: node write returned success, while there were error messages during write.";
      }
    else if (warningFound)
      {
      QMessageBox::warning(this, tr("Export warning"), messagesStr);
      }
    else
      {
      QMessageBox::information(this, tr("Export information"), messagesStr);
      }
    }

  return success;
}

//-----------------------------------------------------------------------------
QFileInfo qSlicerExportNodeDialogPrivate::file() const
{
  QDir directory = this->DirectoryPathLineEdit->currentPath();
  QString filename = this->FilenameLineEdit->text();
  return QFileInfo(directory, filename);
}

//-----------------------------------------------------------------------------
QString qSlicerExportNodeDialogPrivate::formatText() const
{
  return this->ExportFormatComboBox->currentText();
}

//-----------------------------------------------------------------------------
QString qSlicerExportNodeDialogPrivate::extension() const
{
  return this->ExportFormatComboBox->itemData(
    this->ExportFormatComboBox->currentIndex()
  ).toString();
}

//-----------------------------------------------------------------------------
qSlicerFileWriterOptionsWidget* qSlicerExportNodeDialogPrivate::getOptionsWidget() const
{
  if (this->OptionsContainer->findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly).size() > 1)
    {
    qCritical() << Q_FUNC_INFO << " failed: OptionsContainer should always have at most one child.";
    return nullptr;
    }

  QWidget* optionsContainerChild = this->OptionsContainer->findChild<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  if (!optionsContainerChild)
    {
    return nullptr;
    }

  qSlicerFileWriterOptionsWidget* options = dynamic_cast<qSlicerFileWriterOptionsWidget*>(optionsContainerChild);
  if (!options)
    {
    qCritical() << Q_FUNC_INFO << " failed: child of OptionsContainer should always be a qSlicerFileWriterOptionsWidget";
    }

  return options;
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::formatChangedSlot()
{
  this->formatChanged(true);
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::formatChanged(bool updateFilename)
{
  // In case the combobox was editable (hack to display custom text), we now
  // don't need this property anymore.
  this->ExportFormatComboBox->setEditable(false);

  if (updateFilename)
    {
    this->FilenameLineEdit->setText(this->recommendedFilename());
    }

  this->updateOptionsWidget();
}

//-----------------------------------------------------------------------------
QString qSlicerExportNodeDialogPrivate::recommendedFilename() const
{
  // Get the extension associated with the currently selected format
  QString extension = QString::fromStdString(vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
    this->formatText().toUtf8()));
  if (extension == "*")
    {
    extension = QString();
    }

  return forceFileNameExtension(this->FilenameLineEdit->text(), extension, this->MRMLNode);
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::updateOptionsWidget()
{
  // If there is already an options widget corresponding to the format currently being used,
  // then just use that. Otherwise we go ahead and create a new options widget.
  auto iterator = this->formatToOptionsWidget.find(this->formatText());
  if (iterator != this->formatToOptionsWidget.end())
    {
    this->setOptionsWidget(*iterator);
    return;
    }


  // Create new options widget for the present node type
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  qSlicerIOOptions* options = coreIOManager->fileWriterOptions(this->MRMLNode, this->formatText());

  // We can only use options that are also widgets. If the following cast succeeds, we pass ownership
  // of the options widget to the export dialog, and if it fails we immediatly delete and give up.
  qSlicerFileWriterOptionsWidget* optionsWidget = dynamic_cast<qSlicerFileWriterOptionsWidget*>(options);
  if (!optionsWidget)
    {
    delete options;
    return;
    }

  // The optionsWidget can use the filename to initialize some options.
  optionsWidget->setFileName(this->file().absoluteFilePath());
  optionsWidget->setObject(this->MRMLNode);

  this->setOptionsWidget(optionsWidget); // pass ownership of optionsWidget to the qSlicerExportNodeDialog widget

  this->formatToOptionsWidget.insert(this->formatText(),optionsWidget);
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::setOptionsWidget(qSlicerFileWriterOptionsWidget* optionsWidget)
{
  if (!optionsWidget)
    {
    qCritical() << Q_FUNC_INFO << " failed: optionsWidget is null";
    return;
    }

  // If there's already a widget in the container, set it aside
  QWidget* optionsContainerChild = this->OptionsContainer->findChild<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  if (optionsContainerChild)
    {
    // This "stows away" the options widget. It still has its memory managed by Qt.
    optionsContainerChild->hide();
    optionsContainerChild->setParent(this);
    }

  // Container should be empty now; it should never contain more than one widget
  if ( ! this->OptionsContainer->findChildren<QWidget*>().isEmpty())
    {
    qWarning() << Q_FUNC_INFO << " assumption failing: OptionsContainer should now have no children.";
    }

  // Now ready to inject the new optionsWidget into the container
  optionsWidget->setParent(this->OptionsContainer); // passes ownership of optionsWidget
  optionsWidget->show(); // setting parent made it invisible

  // Make it so that tabbing into OptionsContainer sets focus to the first item in optionsWidget
  this->OptionsContainer->setFocusProxy(optionsWidget);
  QWidget* firstTabbableOptionChild = optionsWidget->nextInFocusChain();
  if (firstTabbableOptionChild) // unclear whether this can be null; check just in case
    {
    this->OptionsContainer->setFocusPolicy(Qt::TabFocus);
    optionsWidget->setFocusProxy(firstTabbableOptionChild);
    }
  else
    {
    this->OptionsContainer->setFocusPolicy(Qt::NoFocus);
    }

  // Sometimes optionsWidget isn't wide enough to show the full label for each option:
  optionsWidget->setMinimumSize(optionsWidget->sizeHint());
  OptionsContainer->setMinimumSize(optionsWidget->sizeHint());
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::onFilenameEditingFinished()
{
  // When the filename in the text box is changed, we check whether it matches a supported
  // file extension. If so, we update the file format selector.
  // If not, then the current file extension will be added
  // (this way when the user just enters extensionless filename,
  // the extension is added automatically).
  QString strippedFileName = qSlicerCoreIOManager::forceFileNameValidCharacters(this->FilenameLineEdit->text());

  // Determine current file extension
  // (switch to lowercase because the data in the ExportFormatComboBox entries are all lowercase)
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  QString currentExtensionLower = coreIOManager->extractKnownExtension(strippedFileName.toLower(), this->MRMLNode);

  // Update file format selector according to current extension
  int newFormat = this->ExportFormatComboBox->findData(currentExtensionLower);
  if (newFormat >= 0)
    {
    // current extension matches a supported format, update the format selector widget accordingly
    this->ExportFormatComboBox->blockSignals(true);
    this->ExportFormatComboBox->setCurrentIndex(newFormat);
    this->formatChanged(false); // We blocked signals so we can manually call formatChanged
    // with updateFilename set to false. This way the filename that the user just typed is not rudely changed.
    this->ExportFormatComboBox->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
qSlicerExportNodeDialog::qSlicerExportNodeDialog(QObject* parentObject)
  : qSlicerFileDialog(parentObject)
  , d_ptr(new qSlicerExportNodeDialogPrivate(nullptr))
{
}

//-----------------------------------------------------------------------------
qSlicerExportNodeDialog::~qSlicerExportNodeDialog() = default;

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerExportNodeDialog::fileType()const
{
  return QString("GenericNodeExport");
}

//-----------------------------------------------------------------------------
QString qSlicerExportNodeDialog::description()const
{
  return tr("Export an individual node");
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerExportNodeDialog::action()const
{
  return qSlicerFileDialog::Write;
}

//-----------------------------------------------------------------------------
bool qSlicerExportNodeDialog::exec(const qSlicerIO::IOProperties& readerProperties)
{
  Q_D(qSlicerExportNodeDialog);

  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();

  QString nodeID;
  if (readerProperties.contains("nodeID"))
    {
    nodeID = readerProperties["nodeID"].toString();
    }


  if (nodeID.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << " failed: No nodeID property found.";
    return false;
    }

  vtkMRMLNode * node = scene->GetNodeByID(nodeID.toUtf8().constData());

  if (!d->setup(scene, node))
    {
    return false;
    }

  if (d->exec() != QDialog::Accepted)
    {
    return false;
    }

  return true;
}
