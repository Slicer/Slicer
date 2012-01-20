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

// Qt includes

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"

// VTK includes
#include <vtkMRMLNode.h>

// -----------------------------------------------------------------------------
// qMRMLSortFilterProxyModelPrivate

// -----------------------------------------------------------------------------
class qMRMLSortFilterProxyModelPrivate
{
public:
  qMRMLSortFilterProxyModelPrivate();

  QStringList                      NodeTypes;
  bool                             ShowHidden;
  QStringList                      ShowHiddenForTypes;
  bool                             ShowChildNodeTypes;
  QStringList                      HideChildNodeTypes;
  QStringList                      HiddenNodeIDs;
  typedef QPair<QString, QVariant> AttributeType;
  QHash<QString, AttributeType>    Attributes;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterProxyModelPrivate::qMRMLSortFilterProxyModelPrivate()
{
  this->ShowHidden = false;
  this->ShowChildNodeTypes = true;
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::qMRMLSortFilterProxyModel(QObject *vparent)
  :QSortFilterProxyModel(vparent)
  , d_ptr(new qMRMLSortFilterProxyModelPrivate)
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
qMRMLSortFilterProxyModel::~qMRMLSortFilterProxyModel()
{
}

// -----------------------------------------------------------------------------
QStandardItem* qMRMLSortFilterProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  if (sceneModel == NULL)
    {
    //Q_ASSERT(sceneModel);
    return NULL;
    }
  else
    {
  return sourceIndex.isValid() ? sceneModel->itemFromIndex(sourceIndex) : sceneModel->invisibleRootItem();
    }
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLSortFilterProxyModel::mrmlScene()const
{
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  return sceneModel->mrmlScene();
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterProxyModel::mrmlSceneIndex()const
{
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->mrmlSceneIndex());
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSortFilterProxyModel::mrmlNodeFromIndex(const QModelIndex& proxyIndex)const
{
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  return sceneModel->mrmlNodeFromIndex(this->mapToSource(proxyIndex));
}

//-----------------------------------------------------------------------------
QModelIndex qMRMLSortFilterProxyModel::indexFromMRMLNode(vtkMRMLNode* node, int column)const
{
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  return this->mapFromSource(sceneModel->indexFromNode(node, column));
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::addAttribute(const QString& nodeType,
                                              const QString& attributeName,
                                              const QVariant& attributeValue)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (!d->NodeTypes.contains(nodeType) ||
      (d->Attributes[nodeType].first == attributeName &&
       d->Attributes[nodeType].second == attributeValue))
    {
    return;
    }
  d->Attributes[nodeType] =
    qMRMLSortFilterProxyModelPrivate::AttributeType(attributeName, attributeValue);
  this->invalidateFilter();
}

//------------------------------------------------------------------------------
//bool qMRMLSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent)const;

//------------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
  Q_D(const qMRMLSortFilterProxyModel);
  QStandardItem* parentItem = this->sourceItem(source_parent);
  if (parentItem == 0)
    {
    //Q_ASSERT(parentItem);
    return false;
    }
  QStandardItem* item = 0;
  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex = 0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(source_row, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == 0)
    {
    //Q_ASSERT(item);
    return false;
    }
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  vtkMRMLNode* node = sceneModel->mrmlNodeFromItem(item);
  if (!node)
    {
    return true;
    }
  if (d->HiddenNodeIDs.contains(node->GetID()))
    {
    return false;
    }
  // HideFromEditors property
  if (!d->ShowHidden && node->GetHideFromEditors())
    {
    bool hide = true;
    foreach(const QString& nodeType, d->ShowHiddenForTypes)
      {
      if (node->IsA(nodeType.toLatin1()))
        {
        hide = false;
        break;
        }
      }
    if (hide)
      {
      return false;
      }
    }

  // Accept all the nodes if no type has been set
  if (d->NodeTypes.isEmpty())
    {
    // Apply filter if any
    return this->QSortFilterProxyModel::filterAcceptsRow(source_row,
                                                         source_parent);
    }
  foreach(const QString& nodeType, d->NodeTypes)
    {
    // filter by node type
    if (!node->IsA(nodeType.toAscii().data()))
      {
      //std::cout << "Reject node: " << node->GetName() << "(" << node->GetID()
      //          << ") type: " << typeid(*node).name() <<std::endl;
      continue;
      }
    // filter by excluded child node types
    if (!d->ShowChildNodeTypes && nodeType != node->GetClassName())
      {
      continue;
      }
    // filter by HideChildNodeType
    if (d->ShowChildNodeTypes)
      {
      foreach(const QString& hideChildNodeType, d->HideChildNodeTypes)
        {
        if (node->IsA(hideChildNodeType.toAscii().data()))
          {
          return false;
          }
        }
      }

    // filter by attributes
    if (d->Attributes.contains(nodeType))
      {
      // can be optimized if the event is AttributeModifiedEvent instead of modifiedevent
      const_cast<qMRMLSortFilterProxyModel*>(this)->qvtkConnect(
        node, vtkCommand::ModifiedEvent,
        const_cast<qMRMLSortFilterProxyModel*>(this),
        SLOT(invalidate()),0., Qt::UniqueConnection);

      QString nodeAttribute =
        node->GetAttribute(d->Attributes[nodeType].first.toLatin1());
      if (!nodeAttribute.isEmpty() &&
           nodeAttribute != d->Attributes[nodeType].second.toString())
        {
        return false;
        }
      }
    // Apply filter if any
    return this->QSortFilterProxyModel::filterAcceptsRow(source_row,
                                                         source_parent);
    }
  return false;
}

//-----------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::hideChildNodeTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->HideChildNodeTypes;
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::nodeTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->NodeTypes;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setHideChildNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (_nodeTypes == d->HideChildNodeTypes)
    {
    return;
    }
  d->HideChildNodeTypes = _nodeTypes;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (d->NodeTypes == _nodeTypes)
    {
    return;
    }
  d->NodeTypes = _nodeTypes;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setShowChildNodeTypes(bool _show)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (_show == d->ShowChildNodeTypes)
    {
    return;
    }
  d->ShowChildNodeTypes = _show;
  invalidateFilter();
}

//-----------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::showChildNodeTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->ShowChildNodeTypes;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setShowHidden(bool enable)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (enable == d->ShowHidden)
    {
    return;
    }
  d->ShowHidden = enable;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::showHidden()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->ShowHidden;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setShowHiddenForTypes(const QStringList& types)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (types == d->ShowHiddenForTypes)
    {
    return;
    }
  d->ShowHiddenForTypes = types;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::showHiddenForTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->ShowHiddenForTypes;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setHiddenNodeIDs(const QStringList& nodeIDs)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (nodeIDs == d->HiddenNodeIDs)
    {
    return;
    }
  d->HiddenNodeIDs = nodeIDs;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::hiddenNodeIDs()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->HiddenNodeIDs;
}
