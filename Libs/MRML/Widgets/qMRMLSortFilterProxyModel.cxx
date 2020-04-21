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
#include <vtkMRMLScene.h>

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
  QStringList                      VisibleNodeIDs;
  QString                          HideNodesUnaffiliatedWithNodeID;
  typedef QPair<QString, QVariant> AttributeType;
  QHash<QString, AttributeType>    Attributes;
  qMRMLSortFilterProxyModel::FilterType Filter;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterProxyModelPrivate::qMRMLSortFilterProxyModelPrivate()
{
  this->ShowHidden = false;
  this->ShowChildNodeTypes = true;
  this->Filter = qMRMLSortFilterProxyModel::UseFilters;
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

  this->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::~qMRMLSortFilterProxyModel() = default;

// -----------------------------------------------------------------------------
QStandardItem* qMRMLSortFilterProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  if (sceneModel == nullptr)
    {
    //Q_ASSERT(sceneModel);
    return nullptr;
    }
  return sourceIndex.isValid() ? sceneModel->itemFromIndex(sourceIndex) : sceneModel->invisibleRootItem();
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
      (d->Attributes.value(nodeType).first == attributeName &&
       d->Attributes.value(nodeType).second == attributeValue))
    {
    return;
    }
  d->Attributes[nodeType] =
    qMRMLSortFilterProxyModelPrivate::AttributeType(attributeName, attributeValue);
  this->invalidateFilter();
}

//------------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::removeAttribute(const QString& nodeType,
                                              const QString& attributeName)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (!d->NodeTypes.contains(nodeType) ||
      d->Attributes.value(nodeType).first != attributeName)
    {
    return;
    }
  d->Attributes.remove(nodeType);
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QVariant qMRMLSortFilterProxyModel::attributeFilter(const QString& nodeType,
                                                    const QString& attributeName) const
{
  Q_UNUSED(attributeName);
  Q_D(const qMRMLSortFilterProxyModel);
  return d->Attributes.value(nodeType).second;
}

//------------------------------------------------------------------------------
//bool qMRMLSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent)const;

//------------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
  QStandardItem* parentItem = this->sourceItem(source_parent);
  if (parentItem == nullptr)
    {
    //Q_ASSERT(parentItem);
    return false;
    }
  QStandardItem* item = nullptr;
  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex = 0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(source_row, childIndex);
    if (item)
      {
      break;
      }
    }
  if (item == nullptr)
    {
    //Q_ASSERT(item);
    return false;
    }
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  vtkMRMLNode* node = sceneModel->mrmlNodeFromItem(item);
  AcceptType accept = this->filterAcceptsNode(node);
  bool acceptRow = (accept == Accept);
  if (accept == AcceptButPotentiallyRejectable)
    {
    acceptRow = this->QSortFilterProxyModel::filterAcceptsRow(source_row,
                                                              source_parent);
    }
  if (node &&
      sceneModel->listenNodeModifiedEvent() == qMRMLSceneModel::OnlyVisibleNodes &&
      accept != Reject)
    {
    sceneModel->observeNode(node);
    }
  return acceptRow;
}

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::AcceptType qMRMLSortFilterProxyModel
::filterAcceptsNode(vtkMRMLNode* node)const
{
  Q_D(const qMRMLSortFilterProxyModel);
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  if (!node || !node->GetID())
    {
    return Accept;
    }
  if (this->showAll())
    {
    return Accept;
    }
  if (this->hideAll())
    {
    return Reject;
    }
  if (d->HiddenNodeIDs.contains(node->GetID()))
    {
    return Reject;
    }
  if (d->VisibleNodeIDs.contains(node->GetID()))
    {
    return Accept;
    }
  // HideFromEditors property
  if (!d->ShowHidden && node->GetHideFromEditors())
    {
    bool hide = true;
    foreach(const QString& nodeType, d->ShowHiddenForTypes)
      {
      if (node->IsA(nodeType.toUtf8()))
        {
        hide = false;
        break;
        }
      }
    if (hide)
      {
      return Reject;
      }
    }

  if (!d->HideNodesUnaffiliatedWithNodeID.isEmpty())
    {
    vtkMRMLNode* theNode = sceneModel->mrmlScene()->GetNodeByID(
      d->HideNodesUnaffiliatedWithNodeID.toUtf8());
    bool affiliated = sceneModel->isAffiliatedNode(node, theNode);
    if (!affiliated)
      {
      return Reject;
      }
    }

  // Accept all the nodes if no type has been set
  if (d->NodeTypes.isEmpty())
    {
    // Apply filter if any
    return AcceptButPotentiallyRejectable;
    }
  foreach(const QString& nodeType, d->NodeTypes)
    {
    // filter by node type
    if (!node->IsA(nodeType.toUtf8().data()))
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
        if (node->IsA(hideChildNodeType.toUtf8().data()))
          {
          return Reject;
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

      QString attributeName = d->Attributes[nodeType].first;
      const char *nodeAttribute = node->GetAttribute(attributeName.toUtf8());
      QString nodeAttributeQString = node->GetAttribute(attributeName.toUtf8());
      QString testAttribute = d->Attributes[nodeType].second.toString();

      //std::cout << "attribute name = " << qPrintable(attributeName) << "\n\ttestAttribute = " << qPrintable(testAttribute) << "\n\t" << node->GetID() << " nodeAttributeQString = " << qPrintable(nodeAttributeQString) << "\n\t\tas char str = " << (nodeAttribute ? nodeAttribute : "null") << "." << std::endl;
      // fail if the attribute isn't defined on the node at all
      if (nodeAttribute == nullptr)
        {
        return RejectButPotentiallyAcceptable;
        }
      // if the filter value is null, any node attribute value will match
      if (!d->Attributes[nodeType].second.isNull())
        {
        // otherwise, the node and filter attributes have to match
        if (testAttribute != nodeAttribute)
          {
          return RejectButPotentiallyAcceptable;
          }
        }
      }
    // Apply filter if any
    return AcceptButPotentiallyRejectable;
    }
  return Reject;
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

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setVisibleNodeIDs(const QStringList& nodeIDs)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (nodeIDs == d->VisibleNodeIDs)
    {
    return;
    }
  d->VisibleNodeIDs = nodeIDs;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::visibleNodeIDs()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->VisibleNodeIDs;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel
::setHideNodesUnaffiliatedWithNodeID(const QString& nodeID)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (nodeID == d->HideNodesUnaffiliatedWithNodeID)
    {
    return;
    }
  d->HideNodesUnaffiliatedWithNodeID = nodeID;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
QString qMRMLSortFilterProxyModel::hideNodesUnaffiliatedWithNodeID()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->HideNodesUnaffiliatedWithNodeID;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel
::setFilterType(FilterType filterType)
{
  Q_D(qMRMLSortFilterProxyModel);
  if (filterType == d->Filter)
    {
    return;
    }
  d->Filter = filterType;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
qMRMLSortFilterProxyModel::FilterType qMRMLSortFilterProxyModel
::filterType()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->Filter;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel
::setShowAll(bool show)
{
  if (show == this->showAll())
    {
    return;
    }
  this->setFilterType((show ? qMRMLSortFilterProxyModel::ShowAll :
                       (this->hideAll() ? qMRMLSortFilterProxyModel::HideAll :
                        qMRMLSortFilterProxyModel::UseFilters)));
}

// --------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::showAll()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->Filter == qMRMLSortFilterProxyModel::ShowAll;
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel
::setHideAll(bool hide)
{
  if (hide == this->hideAll())
    {
    return;
    }
  this->setFilterType((hide ? qMRMLSortFilterProxyModel::HideAll :
                       (this->showAll() ? qMRMLSortFilterProxyModel:: ShowAll :
                        qMRMLSortFilterProxyModel::UseFilters)));
}

// --------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::hideAll()const
{
  Q_D(const qMRMLSortFilterProxyModel);
  return d->Filter == qMRMLSortFilterProxyModel::HideAll;
}

// --------------------------------------------------------------------------
qMRMLSceneModel* qMRMLSortFilterProxyModel::sceneModel()const
{
  return qobject_cast<qMRMLSceneModel*>(this->sourceModel());
}
