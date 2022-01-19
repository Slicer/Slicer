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
#include <QToolButton>

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
static QString defaultFilename(vtkMRMLNode* node, QString extension)
{
  const QString unsafeNodeName(node->GetName() ? node->GetName() : "");
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  if (!coreIOManager)
    {
    qCritical() << Q_FUNC_INFO << "failed: Core IO manager not found.";
    return QString();
    }
  const QString safeNodeName = qSlicerCoreIOManager::forceFileNameValidCharacters(unsafeNodeName);
  return forceFileNameExtension(safeNodeName, extension, node);
}

//-----------------------------------------------------------------------------
// Return whether list a has any elements that list b doesn't have.
template <typename T>
static bool setDifferenceIsNonempty(const QList<T>& a, const QList<T>& b)
{
  for (const T & item : a)
    {
    if (!b.contains(item))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
// Return whether all widgets that are part of the given layout are invisible
// Visibility is considered relative to the given widget relativeTo
static bool layoutWidgetsAllInvisible(const QLayout* layout, const QWidget* relativeTo)
{
  for (int i = 0; i < layout->count(); ++i)
    {
    QWidget* widget = layout->itemAt(i)->widget();
    if (widget && widget->isVisibleTo(relativeTo))
      {
      return false;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
// Set up the given ctkPathLineEdit to make it easier to use in a tabbing order.
// ctkPathLineEdit has a default focus policy of NoFocus. It will usable for tab ordering
// if it has TabFocus and sets one of its member widgets as the focus proxy.
// Really this should probably be a fix in the implementation of ctkPathLineEdit, but this is fine for now.
static void fixCtkPathLineEditTabbing(ctkPathLineEdit* w)
{
  // Unfortunately, ctkPathLineEdit does not give direct access to its underlying widgets,
  // so we have to do this fiddly thing with QObject::findChildren.
  auto lineEdits = w->findChildren<QLineEdit*>();
  if (lineEdits.size()!=1)
    { // This might happen if ctkPathLineWidget ever changes its internals.
    qWarning() << "Export dialog warning: unable to set up ctkPathLineEdit keyboard focus rules. Tabbing might be weird.";
    return;
    }

  w->setFocusPolicy(Qt::TabFocus);
  w->setFocusProxy(lineEdits[0]);
}

//-----------------------------------------------------------------------------
NodeTypeWidgetSet::NodeTypeWidgetSet(QWidget* parent, vtkMRMLStorableNode* storableNode, vtkMRMLScene* scene)
  : prototypeNode{storableNode}
{
  this->nodeType = storableNode->GetClassName();
  this->frame = new QFrame(parent);
  this->frameLayout = new QVBoxLayout(this->frame);
  this->frame->setLayout(this->frameLayout);
  this->exportFormatComboBox = new QComboBox(this->frame);
  this->optionsStackedWidget = new QStackedWidget(this->frame);
  this->optionsStackedWidget->addWidget(new QWidget()); // Add a blank widget in the 0 position (see makeOptionsStackedWidgetBlank)
  this->frameLayout->addWidget(this->exportFormatComboBox);
  this->frameLayout->addWidget(this->optionsStackedWidget);

  // --------------------
  // Fill up the export formats dropdown box with the various choices and initialize a choice
  // --------------------

  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  if (!coreIOManager)
    {
    throw std::runtime_error("NodeTypeWidgetSet: Core IO manager not found.");
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
  else // If there wasn't a storage node, we make a temporary one just to get default extension information from it
    {
    const std::string defaultStorageNodeClassName = storableNode->GetDefaultStorageNodeClassName();
    if (!defaultStorageNodeClassName.empty())
      {
      vtkSmartPointer<vtkMRMLNode> defaultStorageNodeAsNode = vtkSmartPointer<vtkMRMLNode>::Take(
        scene->CreateNodeByClass(defaultStorageNodeClassName.c_str())
      );
      vtkMRMLStorageNode* defaultStorageNode = vtkMRMLStorageNode::SafeDownCast(defaultStorageNodeAsNode);
      extensionInStorageNode = defaultStorageNode->GetDefaultWriteFileExtension();
      }
    }

  // (Checking that storageNode is not null allows us to dodge a warning from completeSlicerWritableFileNameSuffix)
  QString currentExtension = storageNode ?
    coreIOManager->completeSlicerWritableFileNameSuffix(storableNode) : QString(".");
  int suggestedFormatIndex = -1; // will be index corresponding to format corresponding to currentExtension
  foreach(QString nameFilter, coreIOManager->fileWriterExtensions(storableNode))
  {
    // extract extension (e.g. ".ext") from format description string (e.g. "Blahblahblah (.ext)")
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toUtf8()));

    // add the entry to the dropdown menu
    this->exportFormatComboBox->addItem(nameFilter, extension);

    // if it was the current extension, remember the index
    if (extension == currentExtension)
      {
      suggestedFormatIndex = this->exportFormatComboBox->count() - 1;
      }
  }

  // If it turns out currentExtension was not any of the choices,
  // then look for extensionInStorageNode
  if (suggestedFormatIndex == -1 && !extensionInStorageNode.isEmpty())
    {
    for (int i = 0; i < this->exportFormatComboBox->count(); ++i)
      {
      if (this->exportFormatComboBox->itemData(i).toString() ==  QString('.') + extensionInStorageNode)
        {
        suggestedFormatIndex = i;
        break;
        }
      }
    }

  // Prefer to use suggested extension found above, otherwise just leave a prompt for the user to select a format.
  if (suggestedFormatIndex != -1)
    {
    this->exportFormatComboBox->setCurrentIndex(suggestedFormatIndex);
    }
  else
    {
    this->exportFormatComboBox->setCurrentIndex(-1);
    this->exportFormatComboBox->setEditable(true); // so we can set custom text below
    this->exportFormatComboBox->lineEdit()->setReadOnly(true);
    this->exportFormatComboBox->setEditText("Select a format");
    }

  // We make this connection now, not earlier, so that we don't trigger the signal with the above initialization.
  QObject::connect(this->exportFormatComboBox, &QComboBox::currentTextChanged,
                    this, &NodeTypeWidgetSet::formatChangedSlot);

  // --------------------
  // Setup optionsWidget, now that exportFormatComboBox is initialized
  // --------------------

  if (!this->updateOptionsWidget())
    {
    throw std::runtime_error(std::string("NodeTypeWidgetSet: Unable to initialize options widget for ")+storableNode->GetName());
    }

  // --------------------
  // Setup the QLabels
  // --------------------

  this->label = new QLabel("", parent);
  this->setLabelText("");
}

void NodeTypeWidgetSet::makeOptionsStackedWidgetBlank()
{
  // We added a blank QWidget to the stacked widget before adding anything else, so 0 is the correct index.
  this->optionsStackedWidget->setCurrentIndex(0);
}

//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::setLabelText(QString nodeTypeDisplayName)
{
  if (nodeTypeDisplayName.isEmpty())
    {
    this->label->setText("Export Format");
    this->label->setIndent(0);
    }
  else
    {
    this->label->setText(nodeTypeDisplayName);
    this->label->setIndent(17);
    }
}

//-----------------------------------------------------------------------------
bool NodeTypeWidgetSet::updateOptionsWidget()
{
  auto iterator = this->formatToOptionsWidget.find(this->formatText());
  if (iterator != this->formatToOptionsWidget.end())
    {
    this->changeCurrentOptionsWidget(*iterator);
    }
  else
    {
    // Create new options widget for the present node type
    qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
    if (!coreIOManager)
      {
      qCritical() << Q_FUNC_INFO << "failed: Core IO manager not found.";
      return false;
      }
    qSlicerIOOptions* options = coreIOManager->fileWriterOptions(this->prototypeNode, this->formatText());

    if (!options)
      {
      this->changeCurrentOptionsWidget(nullptr); // It's okay if there's no options widget. We just don't show one.
      return true;
      }

    // We can only use options that are also widgets. If the following cast succeeds, we pass ownership
    // of the options widget to the QStackedWidget, and if it fails we immediatly delete and give up.
    qSlicerFileWriterOptionsWidget* newOptionsWidget = dynamic_cast<qSlicerFileWriterOptionsWidget*>(options);
    if (!newOptionsWidget)
      {
      delete options;
      qCritical() << Q_FUNC_INFO << "failed: Unable to get options widget for" << prototypeNode->GetName();
      this->changeCurrentOptionsWidget(nullptr);
      return false;
      }
    this->optionsStackedWidget->addWidget(newOptionsWidget);

    // Assumption: setObject below can initialize reasonable options for the widget using prototypeNode as a prototype object,
    // even when the options widget is perhaps going to be used for saving other unrelated nodes of the same type
    newOptionsWidget->setObject(this->prototypeNode);

    // optionsWidgets are not always wide enough to show the full label for each option
    newOptionsWidget->setMinimumSize(newOptionsWidget->sizeHint());

    // Set first item in options widget as its focus proxy
    QWidget* firstTabbableOptionChild = newOptionsWidget->nextInFocusChain();
    if (firstTabbableOptionChild) // unclear whether this can be null; check just in case
      {
      newOptionsWidget->setFocusPolicy(Qt::TabFocus);
      newOptionsWidget->setFocusProxy(firstTabbableOptionChild);
      }
    else
      {
      newOptionsWidget->setFocusPolicy(Qt::NoFocus);
      }

    this->changeCurrentOptionsWidget(newOptionsWidget);

    this->formatToOptionsWidget.insert(this->formatText(), newOptionsWidget);
    }

  return true; //success
}

//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::changeCurrentOptionsWidget(qSlicerFileWriterOptionsWidget* newOptionsWidget)
{
  this->optionsWidget = newOptionsWidget;

  // Null indicates there is no options widget, so we will not show one.
  if (!newOptionsWidget)
    {
    this->makeOptionsStackedWidgetBlank();
    this->optionsStackedWidget->setFocusPolicy(Qt::NoFocus);
    }
  else
    {
    int index = this->optionsStackedWidget->indexOf(newOptionsWidget);

    if (index == -1)
      { // There's a possibility of memory issues if we run into this error, because newOptionsWidget may never have been given a parent
      qCritical() << Q_FUNC_INFO << "has detected an error: options widget is not part of its associated stacked widget.";

      // Go to the "no options widget" state
      this->optionsWidget = nullptr;
      this->makeOptionsStackedWidgetBlank();
      }
    else
      {
      this->optionsStackedWidget->setCurrentIndex(index);
      this->optionsStackedWidget->setFocusPolicy(Qt::TabFocus);
      this->optionsStackedWidget->setFocusProxy(newOptionsWidget);
      }
    }
}

//-----------------------------------------------------------------------------
QString NodeTypeWidgetSet::getTypeDisplayName() const
{
  return this->prototypeNode->GetTypeDisplayName();
}

//-----------------------------------------------------------------------------
QString NodeTypeWidgetSet::extension() const
{
  return this->exportFormatComboBox->itemData(this->exportFormatComboBox->currentIndex()).toString();
}


QString NodeTypeWidgetSet::formatText() const
{
  return this->exportFormatComboBox->currentText();
}
//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::formatChangedSlot()
{
  this->exportFormatComboBox->setEditable(false); // In case we had set the format dropdown as editable to set our custom text, it's now not needed.
  this->updateOptionsWidget();
}

//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::setMemberWidgetVisibility(bool visible)
{
  if (!this->frame || !this->label)
    {
    qCritical() << Q_FUNC_INFO << "failed: Member widgets are invalid!";
    return;
    }
  this->frame->setVisible(visible);
  this->label->setVisible(visible);
}

//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::insertWidgetsAtRow(int row, QFormLayout* formLayout)
{
  formLayout->insertRow(row, this->label, this->frame);
  this->setMemberWidgetVisibility(true);
}

//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::notifyRemovedFromDialog()
{
  this->setMemberWidgetVisibility(false);
}

//-----------------------------------------------------------------------------
void NodeTypeWidgetSet::setFrameStyle(NodeTypeWidgetSet::FrameStyle frameStyle)
{
  if (frameStyle == NodeTypeWidgetSet::FrameStyle::Frame)
    {
    this->frame->setFrameShape(QFrame::StyledPanel);
    this->frameLayout->setContentsMargins(9,9,9,9);
    }
  else
    {
    this->frame->setFrameShape(QFrame::NoFrame);
    this->frameLayout->setContentsMargins(0,0,0,0);
    }
}

//-----------------------------------------------------------------------------
qSlicerExportNodeDialogPrivate::qSlicerExportNodeDialogPrivate(QWidget* parentWidget)
  : QDialog(parentWidget),
    lastUsedHardenTransform{false},
    lastUsedRecursiveChildren{true},
    lastUsedIncludeChildren{true},
    protectFilenameLineEdit{false}
{
  this->setupUi(this);

  connect(this->FilenameLineEdit, SIGNAL(editingFinished()), this, SLOT(onFilenameEditingFinished()));
  connect(this->RecursiveChildrenCheckBox, &QCheckBox::stateChanged, this, &qSlicerExportNodeDialogPrivate::onNodeInclusionCheckboxStateChanged);
  connect(this->IncludeChildrenCheckBox, &QCheckBox::stateChanged, this, &qSlicerExportNodeDialogPrivate::onNodeInclusionCheckboxStateChanged);

  // Set up DirectoryPathLineEdit widget to be a directory selector
  this->DirectoryPathLineEdit->setLabel("Output folder");
  this->DirectoryPathLineEdit->setFilters(ctkPathLineEdit::Dirs);
  this->DirectoryPathLineEdit->setMinimumSize(this->DirectoryPathLineEdit->sizeHint());
  fixCtkPathLineEditTabbing(this->DirectoryPathLineEdit);

  // Find the row of the QFormLayout after which NodeTypeWidgetSets should start getting populated
  int exportFormatsLabelRow;
  this->formLayout->getWidgetPosition(this->ExportFormatsLabel, &exportFormatsLabelRow, nullptr); // (returns to second parameter)
  if (exportFormatsLabelRow >= 0)
    {
    this->nodeTypeWidgetSetStartRow = exportFormatsLabelRow+1;
    }
  else
    {
    qCritical() << Q_FUNC_INFO << "error: Could not find the expected placeholder widget for building the UI.";
    }

}

//-----------------------------------------------------------------------------
qSlicerExportNodeDialogPrivate::~qSlicerExportNodeDialogPrivate() = default;

//-----------------------------------------------------------------------------
const QList<vtkMRMLStorableNode*> & qSlicerExportNodeDialogPrivate::nodeList() const
{
  if (!this->IncludeChildrenCheckBox->isChecked())
    {
    return this->nodesSelectedOnly;
    }
  else
    {
    return this->RecursiveChildrenCheckBox->isChecked() ? this->nodesRecursive : this->nodesNonrecursive;
    }
}

//-----------------------------------------------------------------------------
bool qSlicerExportNodeDialogPrivate::setup(
  vtkMRMLScene* scene,
  const QList<vtkMRMLStorableNode*> & nodesNonrecursive,
  const QList<vtkMRMLStorableNode*> & nodesRecursive,
  vtkMRMLStorableNode* selectedNode /*=nullptr*/
)
{
  this->MRMLScene = scene;
  this->nodesRecursive = nodesRecursive;
  this->nodesNonrecursive = nodesNonrecursive;
  this->nodesSelectedOnly.clear();
  if (selectedNode)
    {
    this->nodesSelectedOnly.push_back(selectedNode);
    }

  // Initialize children inclusion checkbox visibility and state
  this->IncludeChildrenCheckBox->blockSignals(true); // don't trigger onNodeInclusionCheckboxStateChanged
  if (this->nodesSelectedOnly.empty())
    {
    // There would be nothing to export if didn't include children, so force checked and hide
    this->IncludeChildrenCheckBox->setChecked(true);
    this->IncludeChildrenCheckBox->hide();
    }
  else if (this->nodesRecursive.size()==1)
    {
    // There are no children so it makes no difference-- force unchecked and hide
    this->IncludeChildrenCheckBox->setChecked(false);
    this->IncludeChildrenCheckBox->hide();
    }
  else
    {
    // There are children to include and it makes a difference whether we include them-- show checkbox
    this->IncludeChildrenCheckBox->setChecked(this->lastUsedIncludeChildren);
    this->IncludeChildrenCheckBox->show();
    }
  this->IncludeChildrenCheckBox->blockSignals(false);

  // Initialize recursive checkbox visibility and state
  this->RecursiveChildrenCheckBox->blockSignals(true); // don't trigger onNodeInclusionCheckboxStateChanged
  if (this->IncludeChildrenCheckBox->isChecked())
    {
    this->RecursiveChildrenCheckBox->setChecked(this->lastUsedRecursiveChildren);
    this->RecursiveChildrenCheckBox->setEnabled(true);
    }
  else
    {
    this->RecursiveChildrenCheckBox->setChecked(false);
    this->RecursiveChildrenCheckBox->setEnabled(false);
    }
  this->RecursiveChildrenCheckBox->setVisible(setDifferenceIsNonempty(nodesRecursive, nodesNonrecursive));
  this->RecursiveChildrenCheckBox->blockSignals(false);

  // Initialize coordinate system option visibility and state
  bool someNodeHasTransform = false;
  for (vtkMRMLStorableNode* node : this->nodeList())
    {
    vtkMRMLTransformableNode* nodeAsTransformable = vtkMRMLTransformableNode::SafeDownCast(node);
    if (nodeAsTransformable && nodeAsTransformable->GetParentTransformNode())
      {
      someNodeHasTransform = true;
      break;
      }
    }
  if (someNodeHasTransform)
    {
    this->HardenTransformCheckBox->show();
    this->HardenTransformCheckBox->setChecked(this->lastUsedHardenTransform);
    }
  else
    {
    this->HardenTransformCheckBox->hide();
    this->HardenTransformCheckBox->setChecked(false);
    }

  // Intitialize directory input widget
  this->DirectoryPathLineEdit->setCurrentPath(
    this->lastUsedDirectory.isEmpty() ? this->MRMLScene->GetRootDirectory() : this->lastUsedDirectory
  );

  if (this->nodeTypeWidgetSetStartRow < 0)
    {
    qCritical() << Q_FUNC_INFO << "failed: No row index available for the expected placeholder widget.";
    return false;
    }

  if (!this->populateNodeTypeWidgetSets())
    {
    qCritical() << Q_FUNC_INFO << "failed: Unable to populate node-type-specific widgets.";
    return false;
    }

  return true; // success
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::onNodeInclusionCheckboxStateChanged(int state)
{
  Q_UNUSED(state);

  // Enable/disable recursive checkbox based on children inclusion checkbox
  this->RecursiveChildrenCheckBox->blockSignals(true); // prevent signal recursion
  if (this->IncludeChildrenCheckBox->isChecked())
    {
    this->RecursiveChildrenCheckBox->setEnabled(true);
    }
  else
    {
    this->RecursiveChildrenCheckBox->setChecked(false);
    this->RecursiveChildrenCheckBox->setEnabled(false);
    }
  this->RecursiveChildrenCheckBox->blockSignals(false);

  // Repopulate widgets based on the now possibly different node types occuring among the nodes slated for export
  this->populateNodeTypeWidgetSets();
}

//-----------------------------------------------------------------------------
bool qSlicerExportNodeDialogPrivate::populateNodeTypeWidgetSets()
{
  // Clear the form layout of node-type-specific rows before we start filling it with new ones.
  // We use takeRow, not removeRow, so that widgets are removed from the layout without being deleted.
  if (this->nodeTypeWidgetSetStartRow + this->nodeTypesInDialog.size() > this->formLayout->rowCount())
    {
    qCritical() << Q_FUNC_INFO << "failed: The list that tracks node-type-specific widgets is too long; it cannot be valid.";
    return false;
    }
  for (int i = this->nodeTypesInDialog.size()-1; i >= 0; --i)
    {
    NodeTypeWidgetSet* nodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(this->nodeTypesInDialog[i], true);
    if (!nodeTypeWidgetSet)
      {
      return false;
      }

    nodeTypeWidgetSet->notifyRemovedFromDialog();

    // The order of these two lines matters
    this->formLayout->takeRow(this->nodeTypeWidgetSetStartRow + i);
    }
  this->nodeTypesInDialog.clear();

  // Go through the list of nodes to export, and look at each node type that shows up.
  // For each unique node type, get the associated NodeTypeWidgetSet, or construct one if there isn't one yet.
  // Mark the NodeTypeWidgetSet for insertion into the form layout by adding its node type to nodeTypesToAddToDialog.
  QStringList nodeTypesToAddToDialog;
  for (vtkMRMLStorableNode* node : this->nodeList())
    {
    NodeTypeWidgetSet::NodeType nodeType = node->GetClassName();
    if (nodeTypesToAddToDialog.contains(nodeType))
      {
      continue;
      }

    if (!this->nodeTypeToNodeTypeWidgetSet.contains(nodeType))
      {
      try
        {
        NodeTypeWidgetSet* nodeTypeWidgetSet = new NodeTypeWidgetSet(this, node, this->MRMLScene);
        this->nodeTypeToNodeTypeWidgetSet[nodeType] = nodeTypeWidgetSet;
        connect(nodeTypeWidgetSet->exportFormatComboBox, &QComboBox::currentTextChanged, this, &qSlicerExportNodeDialogPrivate::formatChangedSlot);
        }
      catch(std::runtime_error & error)
        {
        qCritical() << Q_FUNC_INFO << "failed: Encountered NodeTypeWidgetSet constructor error:" << error.what();
        return false;
        }
      }
    else
      {
      // The old prototype node pointer may have become invalid since the export dialog was last raised, so we must update it here.
      this->nodeTypeToNodeTypeWidgetSet[nodeType]->prototypeNode = node;
      }

    nodeTypesToAddToDialog.push_back(nodeType);
    }

  // Get the list into a standard order. Not doing this can result in some odd-looking behavior when
  // different calls to populateNodeTypeWidgetSets work with the same set of node types but in different orders.
  // The node-type-specific widgets in the dialog would shuffle around in the dialog as the user triggers
  // populateNodeTypeWidgetSets, without any widgets being added or removed.
  nodeTypesToAddToDialog.sort(Qt::CaseInsensitive);

  // Go through the NodeTypeWidgetSets that have been marked for insertion and insert their member widgets
  // into the form layout. Update the list nodeTypesInDialog.
  for (const auto& nodeType : nodeTypesToAddToDialog)
    {
    NodeTypeWidgetSet* nodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(nodeType, true);
    if (!nodeTypeWidgetSet)
      {
      return false;
      }
    nodeTypeWidgetSet->insertWidgetsAtRow(this->nodeTypeWidgetSetStartRow + this->nodeTypesInDialog.size(), this->formLayout);
    this->nodeTypesInDialog.push_back(nodeType);
    }

  // Set the label text and frame style for each nodeTypeWidgetSet. When there are multiple, we include the type display name in the label text.
  if (this->nodeTypesInDialog.size() == 1)
    {
    this->theOnlyNodeTypeWidgetSet()->setFrameStyle(NodeTypeWidgetSet::FrameStyle::NoFrame);
    this->theOnlyNodeTypeWidgetSet()->setLabelText("");
    }
  else
    {
    for (const auto& nodeType : this->nodeTypesInDialog)
      {
      NodeTypeWidgetSet* nodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(nodeType, true);
      if (!nodeTypeWidgetSet)
        {
        return false;
        }
      nodeTypeWidgetSet->setFrameStyle(NodeTypeWidgetSet::FrameStyle::Frame);
      // Check if a type display name is shared with any other NodeTypeWidgetSet currently in the dialog.
      // Hopefully not, so we can use type display names in the label text.
      // If there's a collsion, resort to using node type identifier (i.e. class name) in the label text.
      bool typeDisplayNameCollison = false;
      for (const auto& otherNodeType : this->nodeTypesInDialog)
        {
        if (nodeType==otherNodeType)
          {
          continue;
          }
        NodeTypeWidgetSet* otherNodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(otherNodeType, true);
        if (!otherNodeTypeWidgetSet)
          {
          return false;
          }
        if (nodeTypeWidgetSet->getTypeDisplayName() == otherNodeTypeWidgetSet->getTypeDisplayName())
          {
          typeDisplayNameCollison = true;
          break;
          }
        }
      if (!typeDisplayNameCollison)
        {
        nodeTypeWidgetSet->setLabelText(nodeTypeWidgetSet->getTypeDisplayName());
        }
      else
        {
        nodeTypeWidgetSet->setLabelText(nodeTypeWidgetSet->nodeType);
        }
      }
    }

  // If there is more than one node to export, we disable the filename text entry box.
  // If there is exactly one node to export, we allow filename entry and initialize a default filename.
  if (this->nodeList().size()>1)
    {
    this->FilenameLineEdit->setEnabled(false);
    this->FilenameLineEdit->setToolTip("When exporting multiple nodes, filenames are automatically set");

    this->FilenameLineEdit->setText("<automatic>");

    this->ButtonBox->setFocus(Qt::ActiveWindowFocusReason);

    this->setWindowTitle(QString("Export ") + QString::number(this->nodeList().size()) + QString(" Nodes"));
    }
  else if (this->nodeList().size()==1)
    {
    this->FilenameLineEdit->setEnabled(true);
    this->FilenameLineEdit->setToolTip("");

    this->FilenameLineEdit->setText(defaultFilename(this->theOnlyNode(), this->theOnlyNodeTypeWidgetSet()->extension()));

    this->FilenameLineEdit->setFocus(Qt::ActiveWindowFocusReason);

    this->setWindowTitle(QString("Export ")+QString(this->theOnlyNode()->GetName()));
    }
  else
    {
    qCritical() << Q_FUNC_INFO << "failed: There is nothing to export.";
    return false;
    }

  // Depending on whether there are multiple node types, make a few aesthetic touchups to the dialog
  if (this->nodeTypesInDialog.size() > 1)
    {
    this->ExportFormatsLabel->show();
    this->formLayout->invalidate();
    }
  else if (this->nodeTypesInDialog.size() == 1)
    {
    this->ExportFormatsLabel->hide();
    this->formLayout->invalidate();
    }
  else
    { // This should really never happen, because we have already caught the case of no nodes to export.
    qCritical() << Q_FUNC_INFO << "failed: Somehow, the list of detected node types is empty!";
    return false;
    }

  this->GeneralOptionsLabel->setVisible(!layoutWidgetsAllInvisible(this->GeneralOptionsLayout, this));
  this->adjustTabbingOrder();
  this->adjustSize();

  return true; // success
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::adjustTabbingOrder()
{
  // For all this to work, it is important that focus policies and focus proxies have been
  // properly set up for DirectoryPathLineEdit and for each optionsStackedWidget

  setTabOrder(this->FilenameLineEdit, this->DirectoryPathLineEdit);

  for (int i = 0; i < this->nodeTypesInDialog.size(); ++i)
    {
    NodeTypeWidgetSet* nodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(this->nodeTypesInDialog[i], true);
    if (!nodeTypeWidgetSet)
      {
      qWarning() << Q_FUNC_INFO << "error: could not find widgets while setting tabbing order.";
      return;
      }
    if (i==0)
      {
      setTabOrder(this->DirectoryPathLineEdit, nodeTypeWidgetSet->exportFormatComboBox);
      }
    else
      {
      NodeTypeWidgetSet* previousNodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(this->nodeTypesInDialog[i-1], true);
      setTabOrder(previousNodeTypeWidgetSet->optionsStackedWidget, nodeTypeWidgetSet->exportFormatComboBox);
      }
    setTabOrder(nodeTypeWidgetSet->exportFormatComboBox, nodeTypeWidgetSet->optionsStackedWidget);
    if (i == this->nodeTypesInDialog.size()-1)
      {
    setTabOrder(nodeTypeWidgetSet->optionsStackedWidget, this->IncludeChildrenCheckBox);
      }
    }
  setTabOrder(this->IncludeChildrenCheckBox,this->RecursiveChildrenCheckBox);
  setTabOrder(this->RecursiveChildrenCheckBox, this->HardenTransformCheckBox);
  setTabOrder(this->HardenTransformCheckBox,ButtonBox);
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::accept()
{
  this->saveWidgetStates();

  if (!this->exportNodes())
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

  if (this->IncludeChildrenCheckBox->isVisible())
    {
    this->lastUsedIncludeChildren = this->IncludeChildrenCheckBox->isChecked();
    }

  if (this->RecursiveChildrenCheckBox->isVisible())
    {
    this->lastUsedRecursiveChildren = this->RecursiveChildrenCheckBox->isChecked();
    }
}

//-----------------------------------------------------------------------------
bool qSlicerExportNodeDialogPrivate::exportNodes()
{
  // Validate user-chosen filename in the single node case
  if (this->nodeList().size()==1 && this->FilenameLineEdit->isEnabled())
    {
    // If the current filename isn't the suggested one, give a prompt that suggests changing it
    // The user can either proceed without change
    // or the user can accept the change (in which case we make the change and proceed normally)
    // or the user can cancel the export (in which case we return false here, so they can return to the export dialog)
    QString betterFilename = this->recommendedFilename(this->theOnlyNode());
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
    }

  // Build lists that can be passed to qSlicerCoreIOManager::exportNodes
  QList<QString> filePaths;
  QList<QString> formatTexts;
  QList<qSlicerIO::IOProperties> savingParameterMaps;
  bool replaceYesToAll = false;
  bool replaceNoToAll = false;
  for (vtkMRMLStorableNode* node : this->nodeList())
    {
    // Get NodeTypeWidgetSet associated to node
    NodeTypeWidgetSet::NodeType nodeType = node->GetClassName();
    NodeTypeWidgetSet* nodeTypeWidgetSet = this->getNodeTypeWidgetSetSafe(nodeType);
    if (!nodeTypeWidgetSet)
      {
      qCritical() << Q_FUNC_INFO << "error: Map of NodeTypeWidgetSet was not properly populated. Skipping" << node->GetName();
      continue;
      }
    if (!this->nodeTypesInDialog.contains(nodeType))
      { // (validation step-- we don't want wrongly invisible widgets to influence the export outcome)
      qCritical() << Q_FUNC_INFO << "error: Failed to add NodeTypeWidgetSet for" << nodeTypeWidgetSet->nodeType << "into dialog. Skipping" << node->GetName();
      }

    // Construct file path for node
    QDir directory = this->DirectoryPathLineEdit->currentPath();
    QString filename;
    if (this->FilenameLineEdit->isEnabled())
      {
      if (this->nodeList().size()==1)
        {
        filename = this->FilenameLineEdit->text();
        }
      else
        {
        // This should never happen, but if it does we ignore what was in the filename textbox
        qCritical() << Q_FUNC_INFO << "error: The filename text box should not have been editable when multiple nodes are being exported.";
        filename = defaultFilename(node, nodeTypeWidgetSet->extension());
        }
      }
    else
      {
      filename = defaultFilename(node, nodeTypeWidgetSet->extension());
      }
    QFileInfo fileInfo(directory, filename);

    // This can happen if there is one node to export and the user insists on leaving the filename box empty (user error)
    // It can also happen if defaultFilename for some reason reutrns empty (program error)
    if (fileInfo.fileName().isEmpty())
      {
      if (this->FilenameLineEdit->isEnabled()) // user error
        {
        QMessageBox::critical(this, tr("Export Error"), tr("Failed to export node %1; filename is empty.").arg(node->GetName()));
        return false;
        }
      else // program error
        {
        qCritical() << Q_FUNC_INFO << "error: Failed to generate a filename for" << node->GetName();
        continue;
        }
      }

    // If file already exists, ask whether to replace.
    if (fileInfo.exists() && !replaceYesToAll)
      {
      if (replaceNoToAll)
        {
        continue;
        }
      QMessageBox::StandardButtons replaceQuestionButtons;
      QMessageBox::StandardButton answer = QMessageBox::question(this, tr("File Exists"),
        tr("The file %1 already exists. Do you want to replace it?").arg(fileInfo.absoluteFilePath()),
        this->nodeList().size()>1
          ? QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel
          : QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes
      );
      if (answer == QMessageBox::Cancel)
        {
        return false;
        }
      else if (answer == QMessageBox::No)
        {
        continue;
        }
      else if (answer == QMessageBox::YesToAll)
        {
        replaceYesToAll = true;
        }
      else if (answer == QMessageBox::NoToAll)
        {
        replaceNoToAll = true;
        continue;
        }
      }

      // Get additional saving parameters from options widget
      qSlicerFileWriterOptionsWidget* options = nodeTypeWidgetSet->optionsWidget;
      qSlicerIO::IOProperties savingParameters;
      if (options)
        {
        savingParameters = options->properties();
        if (!options->isVisibleTo(this))
          {
          qCritical() << Q_FUNC_INFO << "error: attempted to use options from an invisible widget.";
          return false;
          }
        }

      // Fill saving parameters with the gathered information
      savingParameters["nodeID"] = node->GetID();
      savingParameters["fileName"] = fileInfo.absoluteFilePath();
      savingParameters["fileFormat"] = nodeTypeWidgetSet->formatText();

      // sadly, this copies; if we move to Qt6 then this should be emplace_back with a move
      savingParameterMaps.push_back(savingParameters);
    }

  if (savingParameterMaps.isEmpty())
    {
    QMessageBox::information(this, tr("Export Information"), tr("Nothing was exported."));
    return false;
    }

  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  if (!coreIOManager)
    {
    qCritical() << Q_FUNC_INFO << "failed: Core IO manager not found.";
    return false;
    }

  vtkNew<vtkMRMLMessageCollection> userMessages;

  // Actual exporting is here
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  bool success = coreIOManager->exportNodes(savingParameterMaps, this->HardenTransformCheckBox->isChecked(), userMessages);
  QApplication::restoreOverrideCursor();

  if (!success)
    {
    // Make sure at least one error message is in userMessages if saving returns with error
    if (userMessages->GetNumberOfMessages() == 0)
      {
      userMessages->AddMessage(vtkCommand::ErrorEvent, tr("Error encountered while exporting.").toStdString());
      }

    QString messagesStr = QString::fromStdString(userMessages->GetAllMessagesAsString());

    // display messagesStr as an error message
    QMessageBox::critical(this, tr("Export Error"), messagesStr);
    }
  // In case there are any errors or warnings in storage node, make sure to alert even in case of success:
  else if (userMessages->GetNumberOfMessages() > 0)
    {
    bool warningFound = false;
    bool errorFound = false;
    QString messagesStr = QString::fromStdString(userMessages->GetAllMessagesAsString(&errorFound, &warningFound));

    if (errorFound)
      {
      QMessageBox::critical(this, tr("Export Error"), messagesStr);
      // If there was an error, this should never have been considered a success.
      success = false;
      qWarning() << Q_FUNC_INFO << " warning: node write returned success, while there were error messages during write.";
      }
    else if (warningFound)
      {
      QMessageBox::warning(this, tr("Export Warning"), messagesStr);
      }
    else
      {
      QMessageBox::information(this, tr("Export Information"), messagesStr);
      }
    }

  return success;
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::formatChangedSlot()
{
  if (this->nodeList().size()==1 && !this->protectFilenameLineEdit)
    {
    this->FilenameLineEdit->setText(this->recommendedFilename(this->theOnlyNode()));
    }
  this->adjustTabbingOrder(); // Tabbing order may need to be updated if options widgets have changed.
}

//-----------------------------------------------------------------------------
QString qSlicerExportNodeDialogPrivate::recommendedFilename(vtkMRMLStorableNode* node) const
{
  if (!this->FilenameLineEdit->isEnabled())
    {
    qWarning() << Q_FUNC_INFO << "should not be used when the filename text box is disabled.";
    return QString();
    }

  // Get the extension associated with the currently selected format
  QString extension = QString::fromStdString(vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
    this->theOnlyNodeTypeWidgetSet()->formatText().toUtf8()));
  if (extension == "*")
    {
    extension = QString();
    }

  return forceFileNameExtension(this->FilenameLineEdit->text(), extension, node);
}

//-----------------------------------------------------------------------------
vtkMRMLStorableNode* qSlicerExportNodeDialogPrivate::theOnlyNode() const
{
  if (this->nodeList().size()!=1)
    {
    qCritical() << Q_FUNC_INFO << "failed: Expected there to be exactly one node to be exported.";
    return nullptr;
    }
  return this->nodeList()[0];
}

//-----------------------------------------------------------------------------
NodeTypeWidgetSet* qSlicerExportNodeDialogPrivate::getNodeTypeWidgetSetSafe(NodeTypeWidgetSet::NodeType nodeType, bool logError) const
{
  if (logError && !this->nodeTypeToNodeTypeWidgetSet.contains(nodeType))
    {
    qCritical() << Q_FUNC_INFO << "failed: Map of node-type-specific widgets is missing the node type" << nodeType;
    }
  return this->nodeTypeToNodeTypeWidgetSet.value(nodeType, nullptr);
}

//-----------------------------------------------------------------------------
NodeTypeWidgetSet* qSlicerExportNodeDialogPrivate::theOnlyNodeTypeWidgetSet() const
{
  if (this->nodeTypesInDialog.size()!=1)
    {
    qCritical() << Q_FUNC_INFO << "failed: Expected exactly one node type.";
    return nullptr;
    }
  return this->getNodeTypeWidgetSetSafe(this->nodeTypesInDialog[0], true);
}

//-----------------------------------------------------------------------------
QComboBox* qSlicerExportNodeDialogPrivate::theOnlyExportFormatComboBox() const
{
  if (this->nodeTypesInDialog.size()!=1)
    {
    qCritical() << Q_FUNC_INFO << "failed: Expected exactly one node type.";
    return nullptr;
    }
  return this->theOnlyNodeTypeWidgetSet()->exportFormatComboBox;
}

//-----------------------------------------------------------------------------
void qSlicerExportNodeDialogPrivate::onFilenameEditingFinished()
{
  // This slot should only be activated when there is one node to export, because otherwise the filename entry box should be disabled
  if (this->nodeList().size()!=1 || this->nodeTypesInDialog.size()!=1)
    {
    qCritical() << Q_FUNC_INFO << "detected an error: This should not be called when there are multiple nodes to export.";
    return;
    }

  // When the filename in the text box is changed, we check whether it matches a supported file extension.
  // If so, we update the file format selector. If not, then the current file extension will be added
  // (this way when the user just enters extensionless filename, the extension is added automatically).
  QString strippedFileName = qSlicerCoreIOManager::forceFileNameValidCharacters(this->FilenameLineEdit->text());

  // Determine current file extension
  // (switch to lowercase because the data in the ExportFormatComboBox entries are all lowercase)
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  QString currentExtensionLower = coreIOManager->extractKnownExtension(strippedFileName.toLower(), this->theOnlyNode());

  // Update file format selector according to current extension
  int newFormat = this->theOnlyExportFormatComboBox()->findData(currentExtensionLower);
  if (newFormat >= 0)
    {
    // Current extension matches a supported format, update the format selector widget accordingly
    this->protectFilenameLineEdit = true; // Block formatChangedSlot from rudely changing the filename the user just typed in
    this->theOnlyExportFormatComboBox()->setCurrentIndex(newFormat);
    this->protectFilenameLineEdit = false;
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
bool qSlicerExportNodeDialog::exec(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerExportNodeDialog);

  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();

  // The following case occurs for example when trying to export a folder item that has children,
  // but none of its descendant items are storable nodes.
  if (!properties.contains("childIdsNonrecursive") &&
      !properties.contains("childIdsRecursive") &&
      !properties.contains("selectedNodeID"))
    {
    QMessageBox::critical(d, tr("Export error"), tr("There is nothing to export."));
    return false;
    }

  QVariantList childIdsNonrecursive, childIdsRecursive;

  if (properties.contains("childIdsNonrecursive"))
    {
    childIdsNonrecursive = properties["childIdsNonrecursive"].toList();
    }
  if (properties.contains("childIdsRecursive"))
    {
    childIdsRecursive = properties["childIdsRecursive"].toList();
    }

  // This will remain null if there is no "selectedNodeID", or it will become a pointer to the selected node
  vtkMRMLStorableNode* selectedNode = nullptr;

  // Construct list of all nodes to be exported, one list for each potential value of the "recursive" checkbox
  QList<vtkMRMLStorableNode*> nodesNonrecursive, nodesRecursive;
  if (properties.contains("selectedNodeID"))
    {
    QString selectedNodeID = properties["selectedNodeID"].toString();
    selectedNode = vtkMRMLStorableNode::SafeDownCast(scene->GetNodeByID(selectedNodeID.toUtf8().constData()));
    if (selectedNode)
      {
      nodesNonrecursive.push_back(selectedNode);
      nodesRecursive.push_back(selectedNode);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Received node ID " << selectedNodeID << ", but unable to get an associated storable node.";
      }
    }
  for (const QVariant & childID : childIdsNonrecursive)
    {
    QString childIDString = childID.toString();
    vtkMRMLStorableNode* n = vtkMRMLStorableNode::SafeDownCast(scene->GetNodeByID(childIDString.toUtf8().constData()));
    if (n)
      {
      nodesNonrecursive.push_back(n);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Received node ID " << childIDString << ", but unable to get an associated storable node.";
      }
    }
  for (const QVariant & childID : childIdsRecursive)
    {
    QString childIDString = childID.toString();
    vtkMRMLStorableNode* n = vtkMRMLStorableNode::SafeDownCast(scene->GetNodeByID(childIDString.toUtf8().constData()));
    if (n)
      {
      nodesRecursive.push_back(n);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Received node ID " << childIDString << ", but unable to get an associated storable node.";
      }
    }

  // It is again possible for there to be "nothing to export," if there are errors retreiving storable nodes above.
  if (nodesNonrecursive.isEmpty() && nodesRecursive.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << "failed: There is nothing left to export.";
    return false;
    }

  if (!d->setup(scene, nodesNonrecursive, nodesRecursive, selectedNode))
    {
    return false;
    }

  if (d->exec() != QDialog::Accepted)
    {
    return false;
    }

  return true;
}
