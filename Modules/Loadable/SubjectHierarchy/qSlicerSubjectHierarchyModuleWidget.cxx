/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Subject Hierarchy includes
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "ui_qSlicerSubjectHierarchyModule.h"

#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qMRMLSceneSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// SlicerQt includes
#include "qSlicerApplication.h"

// Qt includes
#include <QSettings>
#include <QMessageBox>

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerSubjectHierarchyModuleWidgetPrivate: public Ui_qSlicerSubjectHierarchyModule
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyModuleWidget);
protected:
  qSlicerSubjectHierarchyModuleWidget* const q_ptr;
public:
  qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object);
  ~qSlicerSubjectHierarchyModuleWidgetPrivate();
  vtkSlicerSubjectHierarchyModuleLogic* logic() const;
public:
  /// Using this flag prevents overriding the parameter set node contents when the
  ///   QMRMLCombobox selects the first instance of the specified node type when initializing
  bool ModuleWindowInitialized;

  /// Helper flag ensuring consistency when deleting branches
  bool DeleteBranchInProgress;

  /// Flag determining whether subject hierarchy nodes are automatically created upon
  /// adding a supported data node in the scene, or just when entering the module.
  bool AutoCreateSubjectHierarchy;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::qSlicerSubjectHierarchyModuleWidgetPrivate(qSlicerSubjectHierarchyModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
  , DeleteBranchInProgress(false)
  , AutoCreateSubjectHierarchy(false)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidgetPrivate::~qSlicerSubjectHierarchyModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerSubjectHierarchyModuleLogic*
qSlicerSubjectHierarchyModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSubjectHierarchyModuleWidget);
  return vtkSlicerSubjectHierarchyModuleLogic::SafeDownCast(q->logic());
}


//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidget::qSlicerSubjectHierarchyModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSubjectHierarchyModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModuleWidget::~qSlicerSubjectHierarchyModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::exit()
{
  this->Superclass::exit();

  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->SubjectHierarchyTreeView->setMRMLScene(NULL);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onEnter()
{
  if (!this->mrmlScene())
    {
    return;
    }

  Q_D(qSlicerSubjectHierarchyModuleWidget);

  d->ModuleWindowInitialized = true;
  d->SubjectHierarchyTreeView->setMRMLScene(this->mrmlScene());

  this->onLogicModified();

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setup()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Make connections for the checkboxes and buttons
  connect( d->DisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMRMLIDsVisible(bool)) );
  connect( d->DisplayTransformsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTransformsVisible(bool)) );

  // Make MRML connections
  // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
  qvtkConnect( this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
  // Connect scene node added event so that the associated subject hierarchy node can be deleted too
  qvtkConnect( this->mrmlScene(), vtkMRMLScene::NodeAboutToBeRemovedEvent, this, SLOT( onNodeAboutToBeRemoved(vtkObject*,vtkObject*) ) );

  // Set up tree view
  qMRMLSceneSubjectHierarchyModel* sceneModel = (qMRMLSceneSubjectHierarchyModel*)d->SubjectHierarchyTreeView->sceneModel();
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  d->SubjectHierarchyTreeView->header()->resizeSection(sceneModel->transformColumn(), 60);

  connect( d->SubjectHierarchyTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)), d->MRMLNodeAttributeTableWidget, SLOT(setMRMLNode(vtkMRMLNode*)) );
  connect( d->SubjectHierarchyTreeView->sceneModel(), SIGNAL(invalidateFilter()), d->SubjectHierarchyTreeView->model(), SLOT(invalidate()) );

  this->setMRMLIDsVisible(d->DisplayMRMLIDsCheckBox->isChecked());
  this->setTransformsVisible(d->DisplayTransformsCheckBox->isChecked());

  // Assemble help text for question mark tooltip
  QString aggregatedHelpText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">    <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">    p, li   { white-space: pre-wrap;   }  </style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">");
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    // Add help text from each plugin
    QString pluginHelpText = plugin->helpText();
    if (!pluginHelpText.isEmpty())
      {
      aggregatedHelpText.append(QString("\n") + pluginHelpText);
      }
    }
  aggregatedHelpText.append(QString("</body></html>"));
  d->label_Help->setToolTip(aggregatedHelpText);

  // Load settings
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (!settings)
    {
    qWarning() << "qSlicerSubjectHierarchyModuleWidget::setup: Invalid application settings!";
    }
  else
    {
    if (settings->contains("SubjectHierarchy/AutoCreateSubjectHierarchy"))
      {
      d->AutoCreateSubjectHierarchy = (bool)settings->value("SubjectHierarchy/AutoCreateSubjectHierarchy").toInt();
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  // Expand to depth 4
  d->SubjectHierarchyTreeView->expandToDepth(4);

  // Check if there are supported data nodes in the scene that are not in subject hierarchy
  if (this->isThereSupportedNodeOutsideSubjectHierarchy())
    {
    // This should only happen if auto-creation is off. Report error in this case, because it's a bug
    if (d->AutoCreateSubjectHierarchy)
      {
      qCritical() << "qSlicerSubjectHierarchyModuleWidget::updateWidgetFromMRML: Subject hierarchy auto-creation is on, still there are supported data nodes outside the hierarchy. This is a bug, please report with reproducible steps. Thanks!";
      return;
      }

    // Ask the user if they want subject hierarchy to be created, otherwise it's unusable
    QMessageBox::StandardButton answer =
      QMessageBox::question(NULL, tr("Do you want to create subject hierarchy?"),
      tr("Supported nodes have been found outside the hierarchy. Do you want to create subject hierarchy?\n\nIf you choose No, subject hierarchy will not be usable.\nIf you choose yes, then this question will appear every time you enter this module and not all supported nodes are in the hierarchy\nIf you choose Yes to All, this question never appears again, and all supported data nodes are automatically added to the hierarchy"),
      QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll,
      QMessageBox::Yes);
    // Create subject hierarchy if the user some form of yes
    if (answer == QMessageBox::Yes || answer == QMessageBox::YesToAll)
      {
      this->addSupportedNodesToSubjectHierarchy();
      }
    // Save auto-creation flag in settings
    if (answer == QMessageBox::YesToAll)
      {
      d->AutoCreateSubjectHierarchy = true;
      QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
      settings->setValue("SubjectHierarchy/AutoCreateSubjectHierarchy", "1");
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->sceneModel()->idColumn(), !visible);

  d->DisplayMRMLIDsCheckBox->blockSignals(true);
  d->DisplayMRMLIDsCheckBox->setChecked(visible);
  d->DisplayMRMLIDsCheckBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setTransformsVisible(bool visible)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  qMRMLSceneSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSceneSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->sceneModel());
  d->SubjectHierarchyTreeView->setColumnHidden(sceneModel->transformColumn(), !visible);
  d->SubjectHierarchyTreeView->header()->resizeSection(sceneModel->transformColumn(), 60);

  d->DisplayTransformsCheckBox->blockSignals(true);
  d->DisplayTransformsCheckBox->setChecked(visible);
  d->DisplayTransformsCheckBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
qMRMLSceneSubjectHierarchyModel* qSlicerSubjectHierarchyModuleWidget::subjectHierarchySceneModel()const
{
  Q_D(const qSlicerSubjectHierarchyModuleWidget);

  qMRMLSceneSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSceneSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->sceneModel());
  return sceneModel;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyModuleWidget::subjectHierarchyPluginByName(QString name)const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName(name);
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyModuleWidget::currentSubjectHierarchyNode()const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::setCurrentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentNode(node);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onLogicModified()
{
  vtkMRMLScene* scene = this->mrmlScene();
  vtkMRMLScene* currentScene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();

  if (scene != currentScene)
    {
    // Set the new scene to the plugin handler
    qSlicerSubjectHierarchyPluginHandler::instance()->setScene(scene);

    // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
    qvtkReconnect( scene, vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
    // Connect scene node added event so that the associated subject hierarchy node can be deleted too
    qvtkReconnect( scene, vtkMRMLScene::NodeAboutToBeRemovedEvent, this, SLOT( onNodeAboutToBeRemoved(vtkObject*,vtkObject*) ) );
    // Connect scene import ended event so that subject hierarchy nodes can be created for supported data nodes if missing (backwards compatibility)
    qvtkReconnect( scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportEnded(vtkObject*) ) );
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onNodeAdded(vtkObject* sceneObject, vtkObject* nodeObject)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(nodeObject);
  // If subject hierarchy node, take care of owner plugins and auto-assignment of it when node is changed
  if (subjectHierarchyNode)
    {
    // Keep 'owner plugin changed' connections up-to date (reconnect to the new plugin)
    qvtkConnect( subjectHierarchyNode, vtkMRMLSubjectHierarchyNode::OwnerPluginChangedEvent,
      qSlicerSubjectHierarchyPluginHandler::instance(), SLOT( reconnectOwnerPluginChanged(vtkObject*,void*) ) );

    // Find plugin for current subject hierarchy node and "claim" it
    if (!scene->IsImporting())
      {
      qSlicerSubjectHierarchyPluginHandler::instance()->findAndSetOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);
      }

    // See if owner plugin has to be changed when a note is modified
    qvtkConnect( subjectHierarchyNode, vtkCommand::ModifiedEvent, this, SLOT( onSubjectHierarchyNodeModified(vtkObject*) ) );
    }
  // If data node
  else
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodeObject);

    // Observe HierarchyModifiedEvent so that we can switch to nested association (see vtkMRMLSubjectHierarchyNode header)
    // if the data node is associated independently to another hierarchy node
    qvtkConnect( node, vtkMRMLNode::HierarchyModifiedEvent, this, SLOT( onMRMLNodeHierarchyModified(vtkObject*) ) );

    // If auto-creation is enabled, then add subject hierarchy node for the added data node
    // Don't add to subject hierarchy automatically one-by-one if importing scene, because the SH nodes may be stored in the scene and loaded
    // Also abort if invalid or hidden node or if explicitly excluded from subject hierarchy before even adding to the scene
    if ( !d->AutoCreateSubjectHierarchy
      || scene->IsImporting()
      || !node
      || node->GetHideFromEditors()
      || node->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyExcludeFromTreeAttributeName().c_str()) )
      {
      return;
      }

    // If there is a plugin that can add the data node to subject hierarchy, then add
    QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingToSubjectHierarchyForNode(node, NULL);
    qSlicerSubjectHierarchyAbstractPlugin* selectedPlugin = NULL;
    if (foundPlugins.size() > 1)
      {
      // Let the user choose a plugin if more than one returned the same non-zero confidence value
      QString textToDisplay = QString("Equal confidence number found for more than one subject hierarchy plugin for adding new node to subject hierarchy.\n\nSelect plugin to add node named\n'%1'\n(type %2)").arg(node->GetName()).arg(node->GetNodeTagName());
      selectedPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->selectPluginFromDialog(textToDisplay, foundPlugins);
      }
     else if (foundPlugins.size() == 1)
      {
      selectedPlugin = foundPlugins[0];
      }
    // Have the selected plugin add the new node to subject hierarchy
    if (selectedPlugin)
      {
      bool successfullyAddedByPlugin = selectedPlugin->addNodeToSubjectHierarchy(node, NULL);
      if (!successfullyAddedByPlugin)
        {
        qWarning() << "qSlicerSubjectHierarchyModuleWidget::onNodeAdded: Failed to add node "
          << node->GetName() << " through plugin '" << selectedPlugin->name().toLatin1().constData() << "'";
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onNodeAboutToBeRemoved(vtkObject* sceneObject, vtkObject* nodeObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  // Do nothing if scene is closing
  if (scene->IsClosing())
    {
    return;
    }

  Q_D(qSlicerSubjectHierarchyModuleWidget);

  vtkMRMLNode* dataNode = vtkMRMLNode::SafeDownCast(nodeObject);
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(nodeObject);

  if (subjectHierarchyNode)
    {
    // Remove associated data node if any
    vtkMRMLNode* associatedDataNode = subjectHierarchyNode->GetAssociatedNode();
    if (associatedDataNode && !subjectHierarchyNode->GetDisableModifiedEvent())
      {
      subjectHierarchyNode->DisableModifiedEventOn();
      subjectHierarchyNode->SetAssociatedNodeID(NULL);
      scene->RemoveNode(associatedDataNode);
      }

    // Check if node has children and ask if branch is to be removed
    std::vector<vtkMRMLHierarchyNode*> childrenNodes;
    subjectHierarchyNode->GetAllChildrenNodes(childrenNodes);
    if (!childrenNodes.empty() && !d->DeleteBranchInProgress)
      {
      QMessageBox::StandardButton answer =
        QMessageBox::question(NULL, tr("Delete branch?"),
        tr("The deleted node has children. Do you want to remove those too?\n\nIf you choose yes, the whole branch will be deleted, including all children."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
      // Delete branch if the user chose yes
      if (answer == QMessageBox::Yes)
        {
        d->DeleteBranchInProgress = true;
        for (std::vector<vtkMRMLHierarchyNode*>::iterator childrenIt = childrenNodes.begin();
          childrenIt != childrenNodes.end(); ++childrenIt)
          {
          scene->RemoveNode(*childrenIt);
          }
        d->DeleteBranchInProgress = false;
        }
      }
    }
  else if (dataNode)
    {
    // Remove associated subject hierarchy node if any
    vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(dataNode, scene);
    if (subjectHierarchyNode)
      {
      subjectHierarchyNode->DisableModifiedEventOn();
      subjectHierarchyNode->SetAssociatedNodeID(NULL);
      scene->RemoveNode(subjectHierarchyNode);
      }
    // Remove associated other hierarchy node if any (if there is a nested association)
    vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, dataNode->GetID());
    if (hierarchyNode)
      {
      scene->RemoveNode(hierarchyNode);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onSubjectHierarchyNodeModified(vtkObject* nodeObject)
{
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(nodeObject);
  if (subjectHierarchyNode && subjectHierarchyNode->GetOwnerPluginAutoSearch())
    {
    // Find plugin for current subject hierarchy node and "claim" it if the
    // owner plugin is not manually overridden by the user
    QString pluginBefore( subjectHierarchyNode->GetOwnerPluginName() );
    qSlicerSubjectHierarchyPluginHandler::instance()->findAndSetOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);
    QString pluginAfter( subjectHierarchyNode->GetOwnerPluginName() );
    //if (pluginBefore.compare(pluginAfter))
    //  {
    //  qDebug() << "qSlicerSubjectHierarchyModuleWidget::onSubjectHierarchyNodeModified: Subject hierarchy node '" <<
    //    subjectHierarchyNode->GetName() << "' has been modified, plugin search performed, and owner plugin changed from '" <<
    //    pluginBefore << "' to '" << pluginAfter << "'";
    //  }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onSceneImportEnded(vtkObject* sceneObject)
{
  Q_D(qSlicerSubjectHierarchyModuleWidget);

  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  if (d->AutoCreateSubjectHierarchy)
    {
    // Only auto-create subject hierarchy if it's enabled
    this->addSupportedNodesToSubjectHierarchy();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::onMRMLNodeHierarchyModified(vtkObject* nodeObject)
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodeObject);
  if (!node || node->IsA("vtkMRMLHierarchyNode"))
    // Only handle this event for non-hierarchy nodes, as the same event is fired for both
    // hierarchy and associated nodes, and also on other occasions, not just associations.
    {
    return;
    }
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(node->GetScene());
  if (!scene)
    {
    return;
    }

  // Resolve possible conflict by creating nested association if necessary

  // Look for hierarchy nodes that are associated to the node in question
  std::vector<vtkMRMLNode*> hierarchyNodes;
  std::vector<vtkMRMLHierarchyNode*> associatedHierarchyNodes;
  scene->GetNodesByClass("vtkMRMLHierarchyNode", hierarchyNodes);
  for (std::vector<vtkMRMLNode*>::iterator hierarchyNodeIt = hierarchyNodes.begin(); hierarchyNodeIt != hierarchyNodes.end(); ++hierarchyNodeIt)
    {
    vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(*hierarchyNodeIt);
    if ( hierarchyNode && hierarchyNode->GetAssociatedNodeID()
      && !strcmp(hierarchyNode->GetAssociatedNodeID(), node->GetID()) )
      {
      associatedHierarchyNodes.push_back(hierarchyNode);
      }
    }

  // If more than one hierarchy nodes are associated with the data node, then create nested association.
  if (associatedHierarchyNodes.size() > 1)
    {
    // We cannot handle multi-level nesting yet, it's only used by subject hierarchy, so it must be a bug!
    if (associatedHierarchyNodes.size() > 2)
      {
      qCritical() << "qSlicerSubjectHierarchyModuleWidget::onMRMLNodeHierarchyModified: Multi-level nested associations detected for node "
        << node->GetName() << "! This is probably a bug, please report.";
      return;
      }

    // Create nested association
    vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = NULL;
    vtkMRMLHierarchyNode* otherHierarchyNode = NULL;
    if (associatedHierarchyNodes[0]->IsA("vtkMRMLSubjectHierarchyNode"))
      {
      subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(associatedHierarchyNodes[0]);
      otherHierarchyNode = associatedHierarchyNodes[1];
      }
    else if (associatedHierarchyNodes[1]->IsA("vtkMRMLSubjectHierarchyNode"))
      {
      subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(associatedHierarchyNodes[1]);
      otherHierarchyNode = associatedHierarchyNodes[0];
      }
    else // Neither is subject hierarchy. This should never happen
      {
      qCritical() << "qSlicerSubjectHierarchyModuleWidget::onMRMLNodeHierarchyModified: Invalid nested associations detected for node "
        << node->GetName() << "! This is probably a bug, please report.";
      return;
      }

      // Create nested association: (SH -> node <- OtherH)  ==>  (SH -> OtherH -> node)
      subjectHierarchyNode->SetAssociatedNodeID(otherHierarchyNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModuleWidget::addSupportedNodesToSubjectHierarchy()
{
  // Traverse all storable nodes in the scene (only storable nodes can be saved to the scene and thus
  // imported, so it does not make sense to go through non-storable ones)
  vtkMRMLScene* scene = this->mrmlScene();
  std::vector<vtkMRMLNode*> storableNodes;
  scene->GetNodesByClass("vtkMRMLStorableNode", storableNodes);
  for (std::vector<vtkMRMLNode*>::iterator storableNodeIt = storableNodes.begin(); storableNodeIt != storableNodes.end(); ++storableNodeIt)
    {
    vtkMRMLNode* node = (*storableNodeIt);
    // Do not add into subject hierarchy if hidden or already added
    if ( node->GetHideFromEditors()
      || vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(node, scene) )
      {
      continue;
      }

    // If there is a plugin that can add the data node to subject hierarchy, then add
    QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingToSubjectHierarchyForNode(node, NULL);
    qSlicerSubjectHierarchyAbstractPlugin* selectedPlugin = NULL;
    if (foundPlugins.size() > 0)
      {
      // Choose first plugin in case of confidence equality not to annoy user (it can be changed later in subject hierarchy)
      selectedPlugin = foundPlugins[0];
      }
    // Have the selected plugin add the new node to subject hierarchy
    if (selectedPlugin)
      {
      bool successfullyAddedByPlugin = selectedPlugin->addNodeToSubjectHierarchy(node, NULL);
      if (!successfullyAddedByPlugin)
        {
        // Should never happen! If a plugin answers positively to the canOwn question (condition of
        // reaching this point), then it has to be able to add it.
        qCritical() << "qSlicerSubjectHierarchyModuleWidget::onNodeAdded: Failed to add node "
          << node->GetName() << " through plugin '" << selectedPlugin->name().toLatin1().constData() << "'";
        }
      }
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyModuleWidget::isThereSupportedNodeOutsideSubjectHierarchy()
{
  vtkMRMLScene* scene = this->mrmlScene();
  std::vector<vtkMRMLNode*> storableNodes;
  scene->GetNodesByClass("vtkMRMLStorableNode", storableNodes);
  for (std::vector<vtkMRMLNode*>::iterator storableNodeIt = storableNodes.begin(); storableNodeIt != storableNodes.end(); ++storableNodeIt)
    {
    vtkMRMLNode* node = (*storableNodeIt);
    // Non-hidden and not in subject hierarchy, let's see if it's supported
    if ( !node->GetHideFromEditors()
      && !vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(node, scene) )
      {
      QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
        qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingToSubjectHierarchyForNode(node, NULL);
      if (foundPlugins.size() > 0)
        {
        // It is supported, should be in subject hierarchy in order for it to be usable
        return true;
        }
      }
    }

  return false;
}
