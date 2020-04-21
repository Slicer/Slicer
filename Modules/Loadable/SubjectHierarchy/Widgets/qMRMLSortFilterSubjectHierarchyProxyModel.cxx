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

#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

// MRML include
#include "vtkMRMLSubjectHierarchyNode.h"

// Subject Hierarchy includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qMRMLSubjectHierarchyModel.h"

// Qt includes
#include <QStandardItem>

// -----------------------------------------------------------------------------
// qMRMLSortFilterSubjectHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
/// \ingroup Slicer_MRMLWidgets
class qMRMLSortFilterSubjectHierarchyProxyModelPrivate
{
public:
  qMRMLSortFilterSubjectHierarchyProxyModelPrivate();

  QString NameFilter;
  QString AttributeNameFilter;
  QString AttributeValueFilter;
  QStringList LevelFilter;
  QStringList NodeTypes;
  QStringList HideChildNodeTypes;
  vtkIdType HideItemsUnaffiliatedWithItemID;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModelPrivate::qMRMLSortFilterSubjectHierarchyProxyModelPrivate()
  : NameFilter(QString())
  , AttributeNameFilter(QString())
  , AttributeValueFilter(QString())
  , LevelFilter(QStringList())
  , NodeTypes(QStringList())
  , HideChildNodeTypes(QStringList())
  , HideItemsUnaffiliatedWithItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterSubjectHierarchyProxyModel

// -----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, nameFilter, NameFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, attributeNameFilter, AttributeNameFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, attributeValueFilter, AttributeValueFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QStringList, levelFilter, LevelFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QStringList, nodeTypes, NodeTypes);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QStringList, hideChildNodeTypes, HideChildNodeTypes);

//------------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel::qMRMLSortFilterSubjectHierarchyProxyModel(QObject *vparent)
 : QSortFilterProxyModel(vparent)
 , d_ptr(new qMRMLSortFilterSubjectHierarchyProxyModelPrivate)
{
  // For speed issue, we might want to disable the dynamic sorting however
  // when having source models using QStandardItemModel, drag&drop is handled
  // in 2 steps, first a new row is created (which automatically calls
  // filterAcceptsRow() that returns false) and then set the row with the
  // correct values (which doesn't call filterAcceptsRow() on the up to date
  // value unless DynamicSortFilter is true).
  this->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel::~qMRMLSortFilterSubjectHierarchyProxyModel() = default;

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLSortFilterSubjectHierarchyProxyModel::mrmlScene()const
{
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return model->mrmlScene();
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qMRMLSortFilterSubjectHierarchyProxyModel::subjectHierarchyNode()const
{
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return model->subjectHierarchyNode();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setNameFilter(QString filter)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->NameFilter == filter)
    {
    return;
    }
  d->NameFilter = filter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setAttributeNameFilter(QString filter)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->AttributeNameFilter == filter)
    {
    return;
    }
  d->AttributeNameFilter = filter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setAttributeValueFilter(QString filter)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->AttributeValueFilter == filter)
    {
    return;
    }
  d->AttributeValueFilter = filter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setLevelFilter(QStringList filter)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->LevelFilter == filter)
    {
    return;
    }
  d->LevelFilter = filter;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setNodeTypes(const QStringList& types)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->NodeTypes == types)
    {
    return;
    }
  d->NodeTypes = types;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setHideChildNodeTypes(const QStringList& types)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->HideChildNodeTypes == types)
    {
    return;
    }
  d->HideChildNodeTypes = types;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
vtkIdType qMRMLSortFilterSubjectHierarchyProxyModel::hideItemsUnaffiliatedWithItemID()
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  return d->HideItemsUnaffiliatedWithItemID;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setHideItemsUnaffiliatedWithItemID(vtkIdType itemID)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->HideItemsUnaffiliatedWithItemID == itemID)
    {
    return;
    }
  d->HideItemsUnaffiliatedWithItemID = itemID;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterSubjectHierarchyProxyModel::subjectHierarchySceneIndex()const
{
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->subjectHierarchySceneIndex());
}

//-----------------------------------------------------------------------------
vtkIdType qMRMLSortFilterSubjectHierarchyProxyModel::subjectHierarchyItemFromIndex(const QModelIndex& index)const
{
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  return sceneModel->subjectHierarchyItemFromIndex( this->mapToSource(index) );
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterSubjectHierarchyProxyModel::indexFromSubjectHierarchyItem(vtkIdType itemID, int column)const
{
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->indexFromSubjectHierarchyItem(itemID, column));
}

//------------------------------------------------------------------------------
int qMRMLSortFilterSubjectHierarchyProxyModel::acceptedItemCount(vtkIdType rootItemID)const
{
  vtkMRMLSubjectHierarchyNode* shNode = this->subjectHierarchyNode();
  if (!shNode)
    {
    return 0;
    }

  // Count the accepted items under the root item
  int itemCount = 0;
  std::vector<vtkIdType> childItemIDs;
  shNode->GetItemChildren(rootItemID, childItemIDs, true);
  for (std::vector<vtkIdType>::iterator childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
    {
    if (this->filterAcceptsItem(*childIt))
      {
      itemCount++;
      }
    }
  return itemCount;
}

//-----------------------------------------------------------------------------
QStandardItem* qMRMLSortFilterSubjectHierarchyProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return nullptr;
    }
  return sourceIndex.isValid() ? model->itemFromIndex(sourceIndex) : model->invisibleRootItem();
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterSubjectHierarchyProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const
{
  QStandardItem* parentItem = this->sourceItem(sourceParent);
  if (!parentItem)
    {
    return false;
    }
  QStandardItem* item = nullptr;

  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex=0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(sourceRow, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == nullptr)
    {
    return false;
    }
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  vtkIdType itemID = model->subjectHierarchyItemFromItem(item);
  return this->filterAcceptsItem(itemID);
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterSubjectHierarchyProxyModel::filterAcceptsItem(vtkIdType itemID,
                                                                  bool canAcceptIfAnyChildIsAccepted/*=true*/)const
{
  Q_D(const qMRMLSortFilterSubjectHierarchyProxyModel);

  if (!itemID)
    {
    return true;
    }
  vtkMRMLSubjectHierarchyNode* shNode = this->subjectHierarchyNode();
  if (!shNode)
    {
    return true;
    }
  if (itemID == shNode->GetSceneItemID())
    {
    // Always accept scene
    return true;
    }
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());

  // Declare condition flag that is set to true if an item check fails.
  // Needed because if an item would be filtered out based on the criteria but any of its children are shown,
  // the item needs to be shown (so that there are no orphan items in the filtered model)
  bool onlyAcceptIfAnyChildIsAccepted = false;

  // Handle hiding unaffiliated item
  // Used when the root item needs to be shown in the tree but not its siblings or other branches in the tree
  if (d->HideItemsUnaffiliatedWithItemID)
    {
    if (!model->isAffiliatedItem(itemID, d->HideItemsUnaffiliatedWithItemID))
      {
      return false;
      }
    }

  // Filter by data node properties
  vtkMRMLNode* dataNode = shNode->GetItemDataNode(itemID);
  if (dataNode)
    {
    // Filter by hide from editor property
    if (dataNode->GetHideFromEditors())
      {
      return false;
      }

    // Filter by exclude attribute
    if (dataNode->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyExcludeFromTreeAttributeName().c_str()))
      {
      return false;
      }

    // Filter by node type
    bool nodeTypeAccepted = false;
    if (!d->NodeTypes.isEmpty())
      {
      QString dataNodeClass(dataNode->GetClassName());
      foreach (const QString& nodeType, d->NodeTypes)
        {
        if (dataNode->IsA(nodeType.toUtf8().data()))
          {
          nodeTypeAccepted = true;
          break;
          }
        }
      }
    else
      {
      nodeTypeAccepted = true;
      }
    if (nodeTypeAccepted)
      {
      foreach (const QString& hideChildNodeType, d->HideChildNodeTypes)
        {
        if (dataNode->IsA(hideChildNodeType.toUtf8().data()))
          {
          nodeTypeAccepted = false;
          }
        }
      }
    if (!nodeTypeAccepted)
      {
      if (canAcceptIfAnyChildIsAccepted)
        {
        // If node type was requested but is different, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      else
        {
        return false;
        }
      }
    } // If data node

  // Filter by level
  bool itemLevelAccepted = false;
  if (!d->LevelFilter.isEmpty())
    {
    QString itemLevel(shNode->GetItemLevel(itemID).c_str());
    foreach (const QString& levelFilter, d->LevelFilter)
      {
      if (itemLevel == levelFilter)
        {
        itemLevelAccepted = true;
        break;
        }
      }
    }
  else
    {
    itemLevelAccepted = true;
    }
  if (!itemLevelAccepted)
    {
    if (canAcceptIfAnyChildIsAccepted)
      {
      // If level was requested but is different, then only show if any of its children are shown
      onlyAcceptIfAnyChildIsAccepted = true;
      }
    else
      {
      return false;
      }
    }

  // Do not show items in virtual branches if their parent is not accepted for any reason
  vtkIdType parentItemID = shNode->GetItemParent(itemID);
  if (parentItemID && shNode->IsItemVirtualBranchParent(parentItemID))
    {
    if (!this->filterAcceptsItem(parentItemID, false))
      {
      return false;
      }
    }

  // Filter by item attribute
  if (!d->AttributeNameFilter.isEmpty())
    {
    std::string attributeNameFilterStr(d->AttributeNameFilter.toUtf8().constData());
    if (!shNode->HasItemAttribute(itemID, attributeNameFilterStr))
      {
      if (canAcceptIfAnyChildIsAccepted)
        {
        // If attribute was requested but missing, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      else
        {
        return false;
        }
      }
    else if (!d->AttributeValueFilter.isEmpty())
      {
      std::string attributeValueFilterStr(d->AttributeValueFilter.toUtf8().constData());
      std::string attributeValue = shNode->GetItemAttribute(itemID, attributeNameFilterStr);
      if (attributeValue.compare(attributeValueFilterStr))
        {
        if (canAcceptIfAnyChildIsAccepted)
          {
          // If attribute value check was requested but failed, then only show if any of its children are shown
          onlyAcceptIfAnyChildIsAccepted = true;
          }
        else
          {
          return false;
          }
        }
      }
    }

  // Filter by item name
  if (!d->NameFilter.isEmpty())
    {
    QString itemName(shNode->GetItemName(itemID).c_str());
    if (!itemName.contains(d->NameFilter, Qt::CaseInsensitive))
      {
      if (canAcceptIfAnyChildIsAccepted)
        {
        // If item name was requested but different, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      else
        {
        return false;
        }
      }
    }

  // If the visibility of an item depends on whether any of its children are shown, then evaluate that condition
  if (onlyAcceptIfAnyChildIsAccepted)
    {
    bool isChildShown = false;
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(itemID, childItemIDs, true);
    for (std::vector<vtkIdType>::iterator childIt=childItemIDs.begin(); childIt!=childItemIDs.end(); ++childIt)
      {
      if (this->filterAcceptsItem(*childIt))
        {
        isChildShown = true;
        break;
        }
      }
    if (!isChildShown)
      {
      return false;
      }
    }

  // All criteria were met
  return true;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSortFilterSubjectHierarchyProxyModel::flags(const QModelIndex & index)const 
{
  vtkIdType itemID = this->subjectHierarchyItemFromIndex(index);
  bool isSelectable = this->filterAcceptsItem(itemID, false);
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  QStandardItem* item = sceneModel->itemFromSubjectHierarchyItem(itemID, index.column());
  if (!item)
    {
    return Qt::ItemFlags();
    }

  if (isSelectable)
    {
    return item->flags() | Qt::ItemIsSelectable;
    }
  else
    {
    return item->flags() & ~Qt::ItemIsSelectable;
    }
}
