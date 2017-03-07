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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QStandardItem>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyFolderPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyFolderPlugin);
protected:
  qSlicerSubjectHierarchyFolderPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyFolderPluginPrivate(qSlicerSubjectHierarchyFolderPlugin& object);
  ~qSlicerSubjectHierarchyFolderPluginPrivate();
  void init();
public:
  QIcon FolderIcon;

  QAction* CreateFolderUnderSceneAction;
  QAction* CreateFolderUnderNodeAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyFolderPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPluginPrivate::qSlicerSubjectHierarchyFolderPluginPrivate(qSlicerSubjectHierarchyFolderPlugin& object)
: q_ptr(&object)
{
  this->FolderIcon = QIcon(":Icons/Folder.png");

  this->CreateFolderUnderSceneAction = NULL;
  this->CreateFolderUnderNodeAction = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyFolderPlugin);

  this->CreateFolderUnderSceneAction = new QAction("Create new folder",q);
  QObject::connect(this->CreateFolderUnderSceneAction, SIGNAL(triggered()), q, SLOT(createFolderUnderScene()));

  this->CreateFolderUnderNodeAction = new QAction("Create child folder",q);
  QObject::connect(this->CreateFolderUnderNodeAction, SIGNAL(triggered()), q, SLOT(createFolderUnderCurrentNode()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPluginPrivate::~qSlicerSubjectHierarchyFolderPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPlugin::qSlicerSubjectHierarchyFolderPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyFolderPluginPrivate(*this) )
{
  this->m_Name = QString("Folder");

  Q_D(qSlicerSubjectHierarchyFolderPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPlugin::~qSlicerSubjectHierarchyFolderPlugin()
{
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyFolderPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Input item is invalid!";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if ( hierarchyNode
    && (!hierarchyNode->GetAssociatedNodeID() || !strcmp(hierarchyNode->GetAssociatedNodeID(), "")) )
    {
    // Folder with a hierarchy node associated that has no data node associated to it
    // (which usually means it's an intermediate node with children)
    return 0.7;
    }
  else if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    // Folder with no hierarchy node
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyFolderPlugin::roleForPlugin()const
{
  // Get current node to determine role
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current node!";
    return "Error!";
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return "Error!";
    }

  // Folder level
  if (shNode->IsItemLevel(currentItemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    return "Folder";
    }

  return QString("Error!");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyFolderPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Input item is invalid!";
    return QIcon();
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyFolderPlugin);

  // Subject and Folder icon
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    return d->FolderIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyFolderPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyFolderPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyFolderPlugin);

  QList<QAction*> actions;
  actions << d->CreateFolderUnderNodeAction;
  return actions;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyFolderPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyFolderPlugin);

  QList<QAction*> actions;
  actions << d->CreateFolderUnderSceneAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyFolderPlugin);
  this->hideAllContextMenuActions();

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Scene
  if (itemID == shNode->GetSceneItemID())
    {
    d->CreateFolderUnderSceneAction->setVisible(true);
    return;
    }

  // Folder can be created under any node
  if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    d->CreateFolderUnderNodeAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::editProperties(vtkIdType itemID)
{
  qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->editProperties(itemID);
}

//---------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyFolderPlugin::createFolderUnderItem(vtkIdType parentItemID)
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Create folder subject hierarchy node
  std::string name = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewNodeNamePrefix()
    + vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder();
  name = shNode->GenerateUniqueItemName(name);
  vtkIdType childItemID = shNode->CreateItem(
    parentItemID, name, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder() );
  emit requestExpandItem(childItemID);

  return childItemID;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::createFolderUnderScene()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  this->createFolderUnderItem(shNode->GetSceneItemID());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::createFolderUnderCurrentNode()
{
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current node!";
    return;
    }

  this->createFolderUnderItem(currentItemID);
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyFolderPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(node);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }
  else if (hierarchyNode && (!hierarchyNode->GetAssociatedNodeID() || !strcmp(hierarchyNode->GetAssociatedNodeID(), "")))
    {
    // Node is a hierarchy and it has no associated data node
    // (which usually means it's an intermediate node with children)
    return 0.7;
    }
  return 0.0;
}

//----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyFolderPlugin::addNodeToSubjectHierarchy(
  vtkMRMLNode* nodeToAdd, vtkIdType parentItemID, std::string level/*=""*/)
{
  if (!nodeToAdd)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid node to add";
    return false;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  // If parent is invalid, then add it under the scene
  if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    parentItemID = shNode->GetSceneItemID();
    }

  // If added node is a hierarchy node with a data node associated, then the data node should be added instead
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(nodeToAdd);
  if (!hierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Folder plugin cannot add non-hierarchy nodes";
    return false;
    }
  if (hierarchyNode->GetAssociatedNodeID() && strcmp(hierarchyNode->GetAssociatedNodeID(), ""))
    {
    qCritical() << Q_FUNC_INFO << ": Data node is associated to the hierarchy node " << nodeToAdd->GetName()
      << " that is being attempted to add to subject hierarchy. The data node should be added to subject hierarchy instead";
    return false;
    }

  // Level will be folder no matter what is given
  level = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder();

  // Create subject hierarchy item with added hierarchy node
  vtkIdType addedItemID = shNode->CreateItem(parentItemID, nodeToAdd, level);
  if (addedItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to add subject hierarchy item for hierarchy node " << nodeToAdd->GetName();
    return false;
    }

  // Observe vtkMRMLHierarchyNode::ChildNodeAddedEvent so that we can reparent subject hierarchy items according to node hierarchy
  qvtkConnect( hierarchyNode, vtkMRMLHierarchyNode::ChildNodeAddedEvent, this, SLOT( onHierarchyNodeChildNodeAdded(vtkObject*,vtkObject*) ) );

  // Set owner plugin to make sure the plugin that adds the item is the owner
  shNode->SetItemOwnerPluginName(addedItemID, this->m_Name.toLatin1().constData());

  // Add the item for the hierarchy node to the proper place in subject hierarchy.
  // If the parent node hierarchy item does not exist yet, add that too, all the way to the scene
  return this->resolveHierarchyForItem(addedItemID);
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyFolderPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
{
  // Synchronize parents in the node hierarchy only if the new parent also corresponds to a hierarchy node.
  // Reparenting, however, is enabled in subject hierarchy even otherwise, in which case the node in the
  // corresponding hierarchy will be reparented under the scene
  Q_UNUSED(parentItemID);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  // Reparent items with associated data nodes that have an associated hierarchy node
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(shNode->GetScene(), associatedNode->GetID()))
    {
    return 0.9;
    }

  // Reparent items with hierarchy nodes
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(associatedNode);
  if (hierarchyNode && (!hierarchyNode->GetAssociatedNodeID() || !strcmp(hierarchyNode->GetAssociatedNodeID(), "")))
    {
    return 0.9;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyFolderPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return false;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  // Get hierarchy node associated to the reparented item
  vtkMRMLHierarchyNode* hierarchyNode = this->hierarchyNodeForItem(itemID);
  if (!hierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Synchronized reparenting failed for item "
      << itemID << "(" << shNode->GetItemName(itemID).c_str() << ") due to missing associated hierarchy node";
    return false;
    }

  // Get parent item's associated hierarchy node
  vtkMRMLHierarchyNode* parentHierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));

  // Synchronize parents in the node hierarchy only if the new parent also corresponds to a hierarchy node.
  // Reparenting, however, is enabled in subject hierarchy even otherwise, in which case the node in the
  // corresponding hierarchy will be reparented under the scene
  if (!parentHierarchyNode)
    {
    qWarning() << Q_FUNC_INFO << ": No hierarchy node associated to subject hierarchy item " << parentItemID << "("
      << shNode->GetItemName(parentItemID).c_str() << "). Reparenting hierarchy node under the scene";

    // Reparent item normally
    shNode->SetItemParent(itemID, parentItemID);
    // Reparent hierarchy node under the scene
    hierarchyNode->SetParentNodeID(NULL);
    }
  else
    {
    // Reparent hierarchy node to the new hierarchy node
    hierarchyNode->SetParentNodeID(parentHierarchyNode->GetID());
    }

  return true;
}

//---------------------------------------------------------------------------
vtkMRMLHierarchyNode* qSlicerSubjectHierarchyFolderPlugin::hierarchyNodeForItem(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return false;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  // Get hierarchy node associated to the item
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(associatedNode);
  if (!hierarchyNode)
    {
    // The hierarchy node may be associated to the data node associated to the item
    hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(shNode->GetScene(), associatedNode->GetID());
    }

  return hierarchyNode;
}

//---------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyFolderPlugin::itemForHierarchyNode(vtkMRMLHierarchyNode* hierarchyNode)
{
  if (!hierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input hierarchy node";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // If hierarchy node has an associated data node then find the item for that
  vtkMRMLNode* associatedNode = hierarchyNode->GetAssociatedNode();
  if (associatedNode)
    {
    return shNode->GetItemByDataNode(associatedNode);
    }

  return shNode->GetItemByDataNode(hierarchyNode);
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyFolderPlugin::resolveHierarchyForItem(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return false;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  // Get hierarchy node associated to the item
  vtkMRMLHierarchyNode* hierarchyNode = this->hierarchyNodeForItem(itemID);
  if (!hierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Item " << itemID << " is not associated to a hierarchy node";
    return false;
    }

  // Get parent hierarchy node under which the subject hierarchy item needs to be reparented
  vtkMRMLHierarchyNode* parentHierarchyNode = hierarchyNode->GetParentNode();
  vtkMRMLNode* associatedParentDataNode = NULL;
  vtkIdType parentItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (!parentHierarchyNode)
    {
    // Nothing to resolve in case of a top-level hierarchy node
    return true;
    }

  // See if a data node is associated to the parent hierarchy node.
  // It should not be, as in node hierarchies the parent nodes must not be associated to a data node.
  // In any case, get the subject hierarchy item for that
  associatedParentDataNode = parentHierarchyNode->GetAssociatedNode();
  if (associatedParentDataNode)
    {
    qWarning() << Q_FUNC_INFO << ": Parent hierarchy node has an associated data node, which should not be, as in "
      << "node hierarchies the parent nodes must not be associated to a data node";
    parentItemID = shNode->GetItemByDataNode(associatedParentDataNode);
    }
  else
    {
    parentItemID = shNode->GetItemByDataNode(parentHierarchyNode);
    }

  // Parent hierarchy node was not in subject hierarchy, need to add it
  if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    if (associatedParentDataNode)
      {
      qWarning() << Q_FUNC_INFO << ": Parent hierarchy node has an associated data node, which is not in subject "
        << "hierarchy yet. It needs to be added first, then resolved explicitly";
      return false;
      }

      // Add hierarchy node to subject hierarchy.
      // Scene is specified as parent, until it will be automatically resolved after adding
      return this->addNodeToSubjectHierarchy(parentHierarchyNode, shNode->GetSceneItemID());
    }

  // Nothing to do if determined parent item is the same as the current parent
  if (parentItemID != shNode->GetItemParent(itemID))
    {
    shNode->SetItemParent(itemID, parentItemID);
    }

  return true;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyFolderPlugin::resolveHierarchies()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  std::vector<vtkMRMLNode*> hierarchyNodes;
  scene->GetNodesByClass("vtkMRMLHierarchyNode", hierarchyNodes);
  for (std::vector<vtkMRMLNode*>::iterator nodeIt = hierarchyNodes.begin(); nodeIt != hierarchyNodes.end(); ++nodeIt)
    {
    // Get subject hierarchy item for hierarchy node
    vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(*nodeIt);
    vtkIdType itemID = this->itemForHierarchyNode(hierarchyNode);
    if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for hierarchy node "
        << (hierarchyNode ? hierarchyNode->GetName() : "NULL");
      continue;
      }

    // Get subject hierarchy item for parent
    vtkMRMLHierarchyNode* parentHierarchyNode = hierarchyNode->GetParentNode();
    vtkIdType parentItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (!parentHierarchyNode)
      {
      parentItemID = shNode->GetSceneItemID();
      }
    else
      {
      parentItemID = this->itemForHierarchyNode(parentHierarchyNode);
      if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for parent hierarchy node "
          << parentHierarchyNode->GetName();
        continue;
        }
      }

    // Nothing to do if parents match
    if (parentItemID != shNode->GetItemParent(itemID))
      {
      shNode->SetItemParent(itemID, parentItemID);
      emit requestExpandItem(parentItemID);
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::onHierarchyNodeChildNodeAdded(vtkObject* parentNodeObject, vtkObject* childNodeObject)
{
  Q_UNUSED(parentNodeObject);

  vtkMRMLHierarchyNode* childNode = vtkMRMLHierarchyNode::SafeDownCast(childNodeObject);
  if (!childNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid child hierarchy node";
    return;
    }

  // Get subject hierarchy item for the child node
  vtkIdType childItemID = this->itemForHierarchyNode(childNode);
  this->resolveHierarchyForItem(childItemID);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::onDataNodeAssociatedToHierarchyNode(vtkObject* dataNodeObject)
{
  vtkMRMLNode* dataNode = vtkMRMLNode::SafeDownCast(dataNodeObject);
  if (!dataNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data node";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get hierarchy node associated to the data node
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, dataNode->GetID());
  if (!hierarchyNode)
    {
    return;
    }

  // Get item for hierarchy node (and not for data node, hence not calling itemForHierarchyNode)
  vtkIdType hierarchyItemID = shNode->GetItemByDataNode(hierarchyNode);
  if (hierarchyItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // If there is a separate item for the hierarchy node then remove it, as it is now represented
    // by the associated data node in subject hierarchy
    shNode->RemoveItem(hierarchyItemID, false, false);
    }
}
