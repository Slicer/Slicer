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
#include "vtkMRMLSubjectHierarchyConstants.h"

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
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID HideItemsUnaffiliatedWithItemID;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModelPrivate::qMRMLSortFilterSubjectHierarchyProxyModelPrivate()
  : NameFilter(QString())
  , HideItemsUnaffiliatedWithItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterSubjectHierarchyProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel::qMRMLSortFilterSubjectHierarchyProxyModel(QObject *vparent)
 : QSortFilterProxyModel(vparent)
 , d_ptr(new qMRMLSortFilterSubjectHierarchyProxyModelPrivate)
{
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
QString qMRMLSortFilterSubjectHierarchyProxyModel::nameFilter()
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  return d->NameFilter;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setNameFilter(QString filter)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  d->NameFilter = filter;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID
qMRMLSortFilterSubjectHierarchyProxyModel::hideItemsUnaffiliatedWithItemID()
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  return d->HideItemsUnaffiliatedWithItemID;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterSubjectHierarchyProxyModel::setHideItemsUnaffiliatedWithItemID(
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID itemID)
{
  Q_D(qMRMLSortFilterSubjectHierarchyProxyModel);
  d->HideItemsUnaffiliatedWithItemID = itemID;
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterSubjectHierarchyProxyModel::subjectHierarchySceneIndex()const
{
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->subjectHierarchySceneIndex());
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID
qMRMLSortFilterSubjectHierarchyProxyModel::subjectHierarchyItemFromIndex(const QModelIndex& index)const
{
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  return sceneModel->subjectHierarchyItemFromIndex( this->mapToSource(index) );
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterSubjectHierarchyProxyModel::indexFromSubjectHierarchyItem(
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID itemID, int column)const
{
  qMRMLSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSubjectHierarchyModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->indexFromSubjectHierarchyItem(itemID, column));
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
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID itemID = model->subjectHierarchyItemFromItem(item);
  return this->filterAcceptsItem(itemID);
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterSubjectHierarchyProxyModel::filterAcceptsItem(
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID itemID )const
{
  Q_D(const qMRMLSortFilterSubjectHierarchyProxyModel);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    return true;
    }
  vtkMRMLSubjectHierarchyNode* shNode = this->subjectHierarchyNode();
  if (!shNode)
    {
    return true;
    }

  // Filtering by data node properties
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

  // Filter by name
  QString itemName(shNode->GetItemName(itemID).c_str());
  return itemName.contains(d->NameFilter, Qt::CaseInsensitive);
}
