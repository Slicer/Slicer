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

#include "qMRMLSceneSubjectHierarchyModel.h"

// Subject Hierarchy includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "qMRMLSceneSubjectHierarchyModel_p.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QMessageBox>

//------------------------------------------------------------------------------
qMRMLSceneSubjectHierarchyModelPrivate::qMRMLSceneSubjectHierarchyModelPrivate(qMRMLSceneSubjectHierarchyModel& object)
: Superclass(object)
{
  this->NodeTypeColumn = -1;
  this->TransformColumn = -1;

  this->UnknownIcon = QIcon(":Icons/Unknown.png");
  this->WarningIcon = QIcon(":Icons/Warning.png");
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModelPrivate::init()
{
  Q_Q(qMRMLSceneSubjectHierarchyModel);
  this->Superclass::init();

  q->setNameColumn(0);
  q->setNodeTypeColumn(q->nameColumn());
  q->setVisibilityColumn(1);
  q->setTransformColumn(2);
  q->setIDColumn(3);

  q->setHorizontalHeaderLabels(
    QStringList() << "Node" << "Vis" << "Tr" << "IDs");

  q->horizontalHeaderItem(q->nameColumn())->setToolTip(QObject::tr("Node name and type"));
  q->horizontalHeaderItem(q->visibilityColumn())->setToolTip(QObject::tr("Show/hide branch or node"));
  q->horizontalHeaderItem(q->transformColumn())->setToolTip(QObject::tr("Applied transform"));
  q->horizontalHeaderItem(q->idColumn())->setToolTip(QObject::tr("Node ID"));

  // Set visibility icons from model to the default plugin
  qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDefaultVisibilityIcons(this->VisibleIcon, this->HiddenIcon, this->PartiallyVisibleIcon);
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneSubjectHierarchyModel::qMRMLSceneSubjectHierarchyModel(QObject *vparent)
: Superclass(new qMRMLSceneSubjectHierarchyModelPrivate(*this), vparent)
{
  Q_D(qMRMLSceneSubjectHierarchyModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSceneSubjectHierarchyModel::~qMRMLSceneSubjectHierarchyModel()
{
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneSubjectHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneSubjectHierarchyModel::parentNode(vtkMRMLNode* node)const
{
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  return subjectHierarchyNode ? subjectHierarchyNode->GetParentNode() : 0;
}

//------------------------------------------------------------------------------
int qMRMLSceneSubjectHierarchyModel::nodeIndex(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSceneSubjectHierarchyModel);
  if (!d->MRMLScene)
    {
    return -1;
    }

  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }

  int index = 0;

  // If the node is not top-level, then find only the index in the branch
  vtkMRMLSubjectHierarchyNode* parent = vtkMRMLSubjectHierarchyNode::SafeDownCast(this->parentNode(node));
  if (parent)
    {
    std::vector<vtkMRMLHierarchyNode*> childHierarchyNodes = parent->GetChildrenNodes();
    for (std::vector<vtkMRMLHierarchyNode*>::iterator childIt = childHierarchyNodes.begin(); childIt != childHierarchyNodes.end(); ++childIt)
      {
      vtkMRMLSubjectHierarchyNode* childNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(*childIt);
      if (childNode == node)
        {
        return index;
        }
      ++index;
      }
    }

  // Iterate through the scene and see if there is any matching node.
  // First try to find based on ptr value, as it's much faster than comparing string IDs.
  vtkCollection* nodes = d->MRMLScene->GetNodes();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it); (n = (vtkMRMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // Note: parent can be NULL, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      if (node==n)
        {
        // found the node
        return index;
        }
      ++index;
      }
    }

  // Not found by node ptr, try to find it by ID (much slower)
  const char* nId = 0;
  for (nodes->InitTraversal(it);
    (n = (vtkMRMLNode*)nodes->GetNextItemAsObject(it)) ;)
    {
    // Note: parent can be NULL, it means that the scene is the parent
    if (parent == this->parentNode(n))
      {
      ++index;
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
        return index;
        }
      }
    }

  // Not found
  return -1;
}

//------------------------------------------------------------------------------
bool qMRMLSceneSubjectHierarchyModel::canBeAChild(vtkMRMLNode* node)const
{
  return node;
}

//------------------------------------------------------------------------------
bool qMRMLSceneSubjectHierarchyModel::canBeAParent(vtkMRMLNode* node)const
{
  return node && node->IsA("vtkMRMLSubjectHierarchyNode");
}

//------------------------------------------------------------------------------
int qMRMLSceneSubjectHierarchyModel::nodeTypeColumn()const
{
  Q_D(const qMRMLSceneSubjectHierarchyModel);
  return d->NodeTypeColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::setNodeTypeColumn(int column)
{
  Q_D(qMRMLSceneSubjectHierarchyModel);
  d->NodeTypeColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneSubjectHierarchyModel::transformColumn()const
{
  Q_D(const qMRMLSceneSubjectHierarchyModel);
  return d->TransformColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::setTransformColumn(int column)
{
  Q_D(qMRMLSceneSubjectHierarchyModel);
  d->TransformColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneSubjectHierarchyModel::maxColumnId()const
{
  Q_D(const qMRMLSceneSubjectHierarchyModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->VisibilityColumn);
  maxId = qMax(maxId, d->NodeTypeColumn);
  maxId = qMax(maxId, d->TransformColumn);
  maxId = qMax(maxId, d->NameColumn);
  maxId = qMax(maxId, d->IDColumn);
  return maxId;
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneSubjectHierarchyModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);

  if (column == this->transformColumn() && node)
    {
    flags = flags | Qt::ItemIsEditable;
    }

  return flags;
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::onMRMLSceneImported(vtkMRMLScene* scene)
{
  Q_UNUSED(scene);
  this->updateScene();
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneSubjectHierarchyModel);

  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (!subjectHierarchyNode)
    {
    // If not subject hierarchy node (i.e. filtering is turned off),
    // then show as any node, except for a tooltip explaining how to add it to subject hierarchy
    if (column == this->nameColumn())
      {
      QString text = QString(node->GetName());
      item->setText(text);
      item->setToolTip(tr("To add into subject hierarchy, drag&drop under a subject hierarchy node"));
      }
    // ID column
    if (column == this->idColumn())
      {
      item->setText(QString(node->GetID()));
      }
    return;
    }

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = NULL;
  if (subjectHierarchyNode->GetOwnerPluginName())
    {
    ownerPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);
    if (!ownerPlugin)
      {
      // Set warning icon if the column is the node type column
      if ( column == this->nodeTypeColumn()
        && item->icon().cacheKey() != d->WarningIcon.cacheKey() ) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
        {
        item->setIcon(d->WarningIcon);
        }
      if (column == this->nameColumn())
        {
          item->setText(node->GetName());
          item->setToolTip(tr("Not owned by any plugin!"));
        }
        return;
      }
    }
  else
    {
    // Owner plugin name is not set for subject hierarchy node. Show it as a regular node
    //qDebug() << "qMRMLSceneSubjectHierarchyModel::updateItemDataFromNode: No owner plugin defined for subject hierarchy node '" << subjectHierarchyNode->GetName() << "'!";
    //Superclass::updateItemDataFromNode(item,node,column);
    return;
    }

  // Name column
  if (column == this->nameColumn())
    {
    // Have owner plugin set the name and the tooltip
    item->setText(ownerPlugin->displayedName(subjectHierarchyNode));
    item->setToolTip(ownerPlugin->tooltip(subjectHierarchyNode));
    }
  // ID column
  if (column == this->idColumn())
    {
    item->setText(QString(subjectHierarchyNode->GetID()));
    }
  // Visibility column
  if (column == this->visibilityColumn())
    {
    // Have owner plugin give the visibility state and icon
    int visible = ownerPlugin->getDisplayVisibility(subjectHierarchyNode);
    QIcon visibilityIcon = ownerPlugin->visibilityIcon(visible);

    // It should be fine to set the icon even if it is the same, but due
    // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
    // it would fire a superflous itemChanged() signal.
    if ( item->data(qMRMLSceneModel::VisibilityRole).isNull()
      || item->data(qMRMLSceneModel::VisibilityRole).toInt() != visible )
      {
      item->setData(visible, qMRMLSceneModel::VisibilityRole);
      if (!visibilityIcon.isNull())
        {
        item->setIcon(visibilityIcon);
        }
      }
    }
  // Node type column
  if (column == this->nodeTypeColumn())
    {
    // Have owner plugin give the icon
    QIcon icon = ownerPlugin->icon(subjectHierarchyNode);
    if (!icon.isNull())
      {
      if (item->icon().cacheKey() != icon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
        {
        item->setIcon(icon);
        }
      }
    else
      {
      if (item->icon().cacheKey() != d->UnknownIcon.cacheKey()) // Only set if it changed (https://bugreports.qt-project.org/browse/QTBUG-20248)
        {
        item->setIcon(d->UnknownIcon);
        }
      }
    }
  // Transform column
  if (column == this->transformColumn())
    {
    if (item->data(Qt::WhatsThisRole).toString().isEmpty())
      {
      item->setData( "Transform", Qt::WhatsThisRole );
      }

    vtkMRMLNode* associatedNode = subjectHierarchyNode->GetAssociatedNode();
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(associatedNode);
    if (transformableNode)
      {
      vtkMRMLTransformNode* parentTransformNode = ( transformableNode->GetParentTransformNode() ? transformableNode->GetParentTransformNode() : NULL );
      QString transformNodeId( parentTransformNode ? parentTransformNode->GetID() : "" );
      QString transformNodeName( parentTransformNode ? parentTransformNode->GetName() : "" );
      // Only change item if the transform itself changed
      if (item->text().compare(transformNodeName))
        {
        item->setData( transformNodeId, qMRMLSceneSubjectHierarchyModel::TransformIDRole );
        item->setText( transformNodeName );
        item->setToolTip( parentTransformNode ? tr("%1 (%2)").arg(parentTransformNode->GetName()).arg(parentTransformNode->GetID()) : "" );
        }
      }
    else
      {
      item->setToolTip(tr("No transform can be directly applied on non-transformable nodes,\nhowever a transform can be chosen to apply it on all the children"));
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (!subjectHierarchyNode)
    {
    Superclass::updateNodeFromItemData(node, item);
    return;
    }
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);

  // Name column
  if ( item->column() == this->nameColumn() )
    {
    subjectHierarchyNode->SetName(item->text().append(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NODE_NAME_POSTFIX.c_str()).toLatin1().constData());

    // Rename data node too
    vtkMRMLNode* associatedDataNode = subjectHierarchyNode->GetAssociatedNode();
    if (associatedDataNode)
      {
      associatedDataNode->SetName(item->text().toLatin1().constData());
      }
    }
  // Visibility column
  if ( item->column() == this->visibilityColumn()
    && !item->data(VisibilityRole).isNull() )
    {
    int visible = item->data(VisibilityRole).toInt();
    if (visible > -1 && visible != ownerPlugin->getDisplayVisibility(subjectHierarchyNode))
      {
      // Have owner plugin set the display visibility
      ownerPlugin->setDisplayVisibility(subjectHierarchyNode, visible);
      }
    }
  // Transform column
  if (item->column() == this->transformColumn())
    {
    QVariant transformIdData = item->data(qMRMLSceneSubjectHierarchyModel::TransformIDRole);
    std::string newParentTransformNodeIdStr = transformIdData.toString().toLatin1().constData();
    vtkMRMLTransformNode* newParentTransformNode =
      vtkMRMLTransformNode::SafeDownCast( this->mrmlScene()->GetNodeByID(newParentTransformNodeIdStr) );

    // No checks and questions when the transform is being removed
    if (!newParentTransformNode)
      {
      subjectHierarchyNode->TransformBranch(NULL, false);
      return;
      }

    // No action if the chosen transform is the same as the applied one
    vtkMRMLTransformableNode* dataNode = vtkMRMLTransformableNode:: SafeDownCast(
      subjectHierarchyNode->GetAssociatedNode() );
    if (dataNode && dataNode->GetParentTransformNode() == newParentTransformNode)
      {
      return;
      }

    // Ask the user if any child node in the tree is transformed with a transform different from the chosen one
    bool hardenExistingTransforms = true;
    if (subjectHierarchyNode->IsAnyNodeInBranchTransformed(newParentTransformNode))
      {
      QMessageBox::StandardButton answer =
        QMessageBox::question(NULL, tr("Some nodes in the branch are already transformed"),
        tr("Do you want to harden all already applied transforms before setting the new one?\n\n"
        "  Note: If you choose no, then the applied transform will simply be replaced."),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);
      if (answer == QMessageBox::No)
        {
        hardenExistingTransforms = false;
        }
      else if (answer == QMessageBox::Cancel)
        {
        //qDebug() << "qMRMLSceneSubjectHierarchyModel::updateNodeFromItemData: Transform branch cancelled";
        return;
        }
      }

    subjectHierarchyNode->TransformBranch(newParentTransformNode, hardenExistingTransforms);
    }
}

//------------------------------------------------------------------------------
bool qMRMLSceneSubjectHierarchyModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  if (!node || newParent == node)
    {
    qCritical() << "qMRMLSceneSubjectHierarchyModel::reparent: Invalid node to reparent!";
    return false;
    }

  vtkMRMLSubjectHierarchyNode* oldParent = vtkMRMLSubjectHierarchyNode::SafeDownCast(this->parentNode(node));
  if (oldParent == newParent)
    {
    return false;
    }

  if (!this->mrmlScene())
    {
    qCritical() << "qMRMLSceneSubjectHierarchyModel::reparent: Invalid MRML scene!";
    return false;
    }

  vtkMRMLSubjectHierarchyNode* parentSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(newParent);
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (!subjectHierarchyNode)
    {
    qCritical() << "qMRMLSceneSubjectHierarchyModel::reparent: Reparented node (" << node->GetName() << ") is not a subject hierarchy node!";
    }

  if (newParent && !this->canBeAParent(newParent))
    {
    qCritical() << "qMRMLSceneSubjectHierarchyModel::reparent: Target parent node (" << newParent->GetName() << ") is not a valid subject hierarchy parent node!";
    }

  // If dropped from within the subject hierarchy tree
  QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForReparentingInsideSubjectHierarchyForNode(subjectHierarchyNode, parentSubjectHierarchyNode);
  qSlicerSubjectHierarchyAbstractPlugin* selectedPlugin = NULL;
  if (foundPlugins.size() > 1)
    {
    // Let the user choose a plugin if more than one returned the same non-zero confidence value
    vtkMRMLNode* associatedNode = (subjectHierarchyNode->GetAssociatedNode() ? subjectHierarchyNode->GetAssociatedNode() : subjectHierarchyNode);
    QString textToDisplay = QString("Equal confidence number found for more than one subject hierarchy plugin for reparenting.\n\nSelect plugin to reparent node named\n'%1'\n(type %2)\nParent node: %3").arg(associatedNode->GetName()).arg(associatedNode->GetNodeTagName()).arg(parentSubjectHierarchyNode->GetName());
    selectedPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->selectPluginFromDialog(textToDisplay, foundPlugins);
    }
  else if (foundPlugins.size() == 1)
    {
    selectedPlugin = foundPlugins[0];
    }
  else
    {
    // Choose default plugin if all registered plugins returned confidence value 0
    selectedPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin();
    }

  // Have the selected plugin reparent the node
  bool successfullyReparentedByPlugin = selectedPlugin->reparentNodeInsideSubjectHierarchy(subjectHierarchyNode, parentSubjectHierarchyNode);
  if (!successfullyReparentedByPlugin)
    {
    subjectHierarchyNode->SetParentNodeID( subjectHierarchyNode->GetParentNodeID() );

    qCritical() << "qMRMLSceneSubjectHierarchyModel::reparent: Failed to reparent node "
      << subjectHierarchyNode->GetName() << " through plugin '" << selectedPlugin->name().toLatin1().constData() << "'";
    return false;
    }

  return true;
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::onHardenTransformOnBranchOfCurrentNode()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (currentNode)
    {
    currentNode->HardenTransformOnBranch();
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneSubjectHierarchyModel::onRemoveTransformsFromBranchOfCurrentNode()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (currentNode)
    {
    currentNode->TransformBranch(NULL, false);
    }
}
