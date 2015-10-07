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
#include "qSlicerSubjectHierarchyPluginLogic.h"

#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerSubjectHierarchyCloneNodePlugin.h"
#include "qSlicerSubjectHierarchyChartsPlugin.h"
#include "qSlicerSubjectHierarchyParseLocalDataPlugin.h"
#include "qSlicerSubjectHierarchyRegisterPlugin.h"
#include "qSlicerSubjectHierarchySegmentPlugin.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"

// SlicerQt includes
#include "qSlicerApplication.h"

// Qt includes
#include <QDebug>
#include <QString>
#include <QSettings>
#include <QMessageBox>

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerSubjectHierarchyPluginLogicPrivate
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyPluginLogic);
protected:
  qSlicerSubjectHierarchyPluginLogic* const q_ptr;
public:
  qSlicerSubjectHierarchyPluginLogicPrivate(qSlicerSubjectHierarchyPluginLogic& object);
  ~qSlicerSubjectHierarchyPluginLogicPrivate();
  void loadApplicationSettings();
public:
  /// Helper flag ensuring consistency when deleting branches
  bool DeleteBranchInProgress;

  /// Flag determining whether subject hierarchy nodes are automatically created upon
  /// adding a supported data node in the scene, or just when entering the module.
  bool AutoCreateSubjectHierarchy;

  /// Flag determining whether subject hierarchy children nodes are automatically
  /// deleted upon deleting a parent subject hierarchy node.
  bool AutoDeleteSubjectHierarchyChildren;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPluginLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogicPrivate::qSlicerSubjectHierarchyPluginLogicPrivate(qSlicerSubjectHierarchyPluginLogic& object)
  : q_ptr(&object)
  , DeleteBranchInProgress(false)
  , AutoCreateSubjectHierarchy(false)
  , AutoDeleteSubjectHierarchyChildren(false)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogicPrivate::~qSlicerSubjectHierarchyPluginLogicPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogicPrivate::loadApplicationSettings()
{
  // Load settings
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (!settings)
    {
    qWarning() << "qSlicerSubjectHierarchyPluginLogic::setup: Invalid application settings!";
    }
  else
    {
    if (settings->contains("SubjectHierarchy/AutoCreateSubjectHierarchy"))
      {
      this->AutoCreateSubjectHierarchy = (settings->value("SubjectHierarchy/AutoCreateSubjectHierarchy").toString().compare("true") == 0);
      }
    if (settings->contains("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren"))
      {
      this->AutoDeleteSubjectHierarchyChildren = (settings->value("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren").toString().compare("true") == 0);
      }
    }
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPluginLogic methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogic::qSlicerSubjectHierarchyPluginLogic(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSubjectHierarchyPluginLogicPrivate(*this) )
{
  Q_D(qSlicerSubjectHierarchyPluginLogic);
  d->loadApplicationSettings();

  // Register Subject Hierarchy core plugins
  this->registerCorePlugins();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogic::~qSlicerSubjectHierarchyPluginLogic()
{
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::registerCorePlugins()
{
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyFolderPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyParseLocalDataPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyCloneNodePlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyChartsPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyRegisterPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchySegmentPlugin());
}

//------------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginLogic::autoCreateSubjectHierarchy()const
{
  Q_D(const qSlicerSubjectHierarchyPluginLogic);
  return d->AutoCreateSubjectHierarchy;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::setAutoCreateSubjectHierarchy(bool flag)
{
  Q_D(qSlicerSubjectHierarchyPluginLogic);
  d->AutoCreateSubjectHierarchy = flag;
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginLogic::autoDeleteSubjectHierarchyChildren()const
{
  Q_D(const qSlicerSubjectHierarchyPluginLogic);
  return d->AutoDeleteSubjectHierarchyChildren;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::setAutoDeleteSubjectHierarchyChildren(bool flag)
{
  Q_D(qSlicerSubjectHierarchyPluginLogic);
  d->AutoDeleteSubjectHierarchyChildren = flag;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginLogic::subjectHierarchyPluginByName(QString name)const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName(name);
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyPluginLogic::currentSubjectHierarchyNode()const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::setCurrentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentNode(node);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::checkSupportedNodesInScene()
{
  Q_D(qSlicerSubjectHierarchyPluginLogic);

  // Check if there are supported data nodes in the scene that are not in subject hierarchy
  if (this->isThereSupportedNodeOutsideSubjectHierarchy())
    {
    // This should only happen if auto-creation is off. Report error in this case, because it's a bug
    if (d->AutoCreateSubjectHierarchy)
      {
      qCritical() << "qSlicerSubjectHierarchyPluginLogic::updateWidgetFromMRML: Subject hierarchy auto-creation is on, still there are supported data nodes outside the hierarchy. This is a bug, please report with reproducible steps. Thanks!";
      return;
      }

    // Ask the user if they want subject hierarchy to be created, otherwise it's unusable
    QMessageBox::StandardButton answer =
      QMessageBox::question(NULL, tr("Do you want to create subject hierarchy?"),
      tr("Supported nodes have been found outside the hierarchy. Do you want to create subject hierarchy?\n\nIf you choose No, subject hierarchy will not be usable.\nIf you choose yes, then this question will appear every time you enter this module and not all supported nodes are in the hierarchy\nIf you choose Yes to All, this question never appears again, and all supported data nodes are automatically added to the hierarchy. This can be later changed in Application Settings."),
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
      settings->setValue("SubjectHierarchy/AutoCreateSubjectHierarchy", "true");
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::setMRMLScene(vtkMRMLScene* scene)
{
  this->qSlicerObject::setMRMLScene(scene);

  // Set the new scene to the plugin handler
  qSlicerSubjectHierarchyPluginHandler::instance()->setScene(scene);

  // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
  qvtkReconnect( scene, vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
  // Connect scene node added event so that the associated subject hierarchy node can be deleted too
  qvtkReconnect( scene, vtkMRMLScene::NodeAboutToBeRemovedEvent, this, SLOT( onNodeAboutToBeRemoved(vtkObject*,vtkObject*) ) );
  // Connect scene import ended event so that subject hierarchy nodes can be created for supported data nodes if missing (backwards compatibility)
  qvtkReconnect( scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportEnded(vtkObject*) ) );
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::onNodeAdded(vtkObject* sceneObject, vtkObject* nodeObject)
{
  Q_D(qSlicerSubjectHierarchyPluginLogic);

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
        qWarning() << "qSlicerSubjectHierarchyPluginLogic::onNodeAdded: Failed to add node "
          << node->GetName() << " through plugin '" << selectedPlugin->name().toLatin1().constData() << "'";
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::onNodeAboutToBeRemoved(vtkObject* sceneObject, vtkObject* nodeObject)
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

  Q_D(qSlicerSubjectHierarchyPluginLogic);

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

    // Check if node has children and ask if branch is to be removed.
    // If node contains a virtual branch, then it is assumed to be taken care of by the owner plugin
    // (a virtual branch is a branch where the children nodes do not correspond to actual MRML data nodes,
    // but to implicit items contained by the parent MRML node, e.g. in case of Markups or Segmentations)
    if (!subjectHierarchyNode->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetVirtualBranchSubjectHierarchyNodeAttributeName().c_str()))
      {
      std::vector<vtkMRMLHierarchyNode*> childrenNodes;
      subjectHierarchyNode->GetAllChildrenNodes(childrenNodes);
      if (!childrenNodes.empty() && !d->DeleteBranchInProgress)
        {
        QMessageBox::StandardButton answer = QMessageBox::Yes;
        if (!d->AutoDeleteSubjectHierarchyChildren)
          {
          answer =
            QMessageBox::question(NULL, tr("Delete subject hierarchy branch?"),
            tr("The deleted subject hierarchy node has children. "
               "Do you want to remove those too?\n\n"
               "If you choose yes, the whole branch will be deleted, including all children.\n"
               "If you choose Yes to All, this question never appears again, and all subject hierarchy children are automatically deleted. This can be later changed in Application Settings."),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll,
            QMessageBox::No);
          }
        // Delete branch if the user chose yes
        if (answer == QMessageBox::Yes || answer == QMessageBox::YesToAll)
          {
          d->DeleteBranchInProgress = true;
          for (std::vector<vtkMRMLHierarchyNode*>::iterator childrenIt = childrenNodes.begin();
            childrenIt != childrenNodes.end(); ++childrenIt)
            {
            scene->RemoveNode(*childrenIt);
            }
          d->DeleteBranchInProgress = false;
          }
        // Save auto-creation flag in settings
        if (answer == QMessageBox::YesToAll)
          {
          d->AutoDeleteSubjectHierarchyChildren = true;
          QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
          settings->setValue("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren", "true");
          }
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
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::onSubjectHierarchyNodeModified(vtkObject* nodeObject)
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
    //  qDebug() << "qSlicerSubjectHierarchyPluginLogic::onSubjectHierarchyNodeModified: Subject hierarchy node '" <<
    //    subjectHierarchyNode->GetName() << "' has been modified, plugin search performed, and owner plugin changed from '" <<
    //    pluginBefore << "' to '" << pluginAfter << "'";
    //  }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::onSceneImportEnded(vtkObject* sceneObject)
{
  Q_D(qSlicerSubjectHierarchyPluginLogic);

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
void qSlicerSubjectHierarchyPluginLogic::onMRMLNodeHierarchyModified(vtkObject* nodeObject)
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
      qCritical() << "qSlicerSubjectHierarchyPluginLogic::onMRMLNodeHierarchyModified: Multi-level nested associations detected for node "
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
      qCritical() << "qSlicerSubjectHierarchyPluginLogic::onMRMLNodeHierarchyModified: Invalid nested associations detected for node "
        << node->GetName() << "! This is probably a bug, please report.";
      return;
      }

      // Create nested association: (SH -> node <- OtherH)  ==>  (SH -> OtherH -> node)
      subjectHierarchyNode->SetAssociatedNodeID(otherHierarchyNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginLogic::addSupportedNodesToSubjectHierarchy()
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
        qCritical() << "qSlicerSubjectHierarchyPluginLogic::onNodeAdded: Failed to add node "
          << node->GetName() << " through plugin '" << selectedPlugin->name().toLatin1().constData() << "'";
        }
      }
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginLogic::isThereSupportedNodeOutsideSubjectHierarchy()
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
