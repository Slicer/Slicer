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
  QString LevelFilter;
  vtkIdType HideItemsUnaffiliatedWithItemID;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModelPrivate::qMRMLSortFilterSubjectHierarchyProxyModelPrivate()
  : NameFilter(QString())
  , AttributeNameFilter(QString())
  , AttributeValueFilter(QString())
  , LevelFilter(QString())
  , HideItemsUnaffiliatedWithItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterSubjectHierarchyProxyModel

// -----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, nameFilter, NameFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, attributeNameFilter, AttributeNameFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, attributeValueFilter, AttributeValueFilter);
CTK_GET_CPP(qMRMLSortFilterSubjectHierarchyProxyModel, QString, levelFilter, LevelFilter);

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
qMRMLSortFilterSubjectHierarchyProxyModel::~qMRMLSortFilterSubjectHierarchyProxyModel()
{
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLSortFilterSubjectHierarchyProxyModel::mrmlScene()const
{
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return NULL;
    }
  return model->mrmlScene();
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qMRMLSortFilterSubjectHierarchyProxyModel::subjectHierarchyNode()const
{
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  if (!model)
    {
    return NULL;
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
void qMRMLSortFilterSubjectHierarchyProxyModel::setLevelFilter(QString filter)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  if (d->LevelFilter == filter)
    {
    return;
    }
  d->LevelFilter = filter;
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
    return NULL;
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
  QStandardItem* item = NULL;

  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex=0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(sourceRow, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == NULL)
    {
    return false;
    }
  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  vtkIdType itemID = model->subjectHierarchyItemFromItem(item);
  return this->filterAcceptsItem(itemID);
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterSubjectHierarchyProxyModel::filterAcceptsItem(vtkIdType itemID)const
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
    }

  // Filter by level
  if (!d->LevelFilter.isEmpty())
    {
    std::string levelFilterStr(d->LevelFilter.toLatin1().constData());
    if (!shNode->IsItemLevel(itemID, levelFilterStr))
      {
      // If level was requested but different, then only show if any of its children are shown
      onlyAcceptIfAnyChildIsAccepted = true;
      }
    }

  // Filter by item attribute
  if (!d->AttributeNameFilter.isEmpty())
    {
    std::string attributeNameFilterStr(d->AttributeNameFilter.toLatin1().constData());
    if (!shNode->HasItemAttribute(itemID, attributeNameFilterStr))
      {
      // If attribute was requested but missing, then only show if any of its children are shown
      onlyAcceptIfAnyChildIsAccepted = true;
      }
    else if (!d->AttributeValueFilter.isEmpty())
      {
      std::string attributeValueFilterStr(d->AttributeValueFilter.toLatin1().constData());
      std::string attributeValue = shNode->GetItemAttribute(itemID, attributeNameFilterStr);
      if (attributeValue.compare(attributeValueFilterStr))
        {
        // If attribute value check was requested but failed, then only show if any of its children are shown
        onlyAcceptIfAnyChildIsAccepted = true;
        }
      }
    }

  // Filter by item name
  if (!d->NameFilter.isEmpty())
    {
    QString itemName(shNode->GetItemName(itemID).c_str());
    if (!itemName.contains(d->NameFilter, Qt::CaseInsensitive))
      {
      onlyAcceptIfAnyChildIsAccepted = true;
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
