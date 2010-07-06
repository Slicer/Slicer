// Qt includes
#include <QDebug>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLItemHelper.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTreeProxyModel.h"
#include "qMRMLUtils.h"

#include "qMRMLSortFilterProxyModel.h"

// VTK includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// -----------------------------------------------------------------------------
// qMRMLSortFilterProxyModelPrivate

// -----------------------------------------------------------------------------
class qMRMLSortFilterProxyModelPrivate: public ctkPrivate<qMRMLSortFilterProxyModel>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLSortFilterProxyModel);
  qMRMLSortFilterProxyModelPrivate();

  QStringList                      NodeTypes;
  bool                             ShowHidden;
  bool                             ShowChildNodeTypes;
  QStringList                      HideChildNodeTypes;
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
{
  CTK_INIT_PRIVATE(qMRMLSortFilterProxyModel);
}

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::~qMRMLSortFilterProxyModel()
{
}

// -----------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSortFilterProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLTreeProxyModel* treeModel = qobject_cast<qMRMLTreeProxyModel*>(this->sourceModel());
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sourceModel());
  // If treeModel != 0, it is safe to use qMRMLTreeProxyModel::proxyItem()
  // because we know for sure that source_parent model is the proxy model and
  // not the source model (which would be the scene model)
  return treeModel ? treeModel->proxyItem(sourceIndex) : sceneModel->item(sourceIndex);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSortFilterProxyModel::mrmlNode(const QModelIndex& proxyIndex)const
{
  QSharedPointer<qMRMLAbstractItemHelper> item =
    QSharedPointer<qMRMLAbstractItemHelper>(
      this->sourceItem(this->mapToSource(proxyIndex)));
  return vtkMRMLNode::SafeDownCast(item->object());
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::addAttribute(const QString& nodeType, 
                                     const QString& attributeName,
                                     const QVariant& attributeValue)
{
  CTK_D(qMRMLSortFilterProxyModel);
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
  CTK_D(const qMRMLSortFilterProxyModel);
  QSharedPointer<qMRMLAbstractItemHelper> parentItem =
    QSharedPointer<qMRMLAbstractItemHelper>(this->sourceItem(source_parent));
  if (parentItem == 0)
    {
    Q_ASSERT(parentItem);
    return false;
    }
  QSharedPointer<qMRMLAbstractItemHelper> item = 
    QSharedPointer<qMRMLAbstractItemHelper>(parentItem->child(source_row, 0));
  if (item.data() == 0)
    {
    Q_ASSERT(item);
    return false;
    }
  vtkObject* object = item->object();
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(object);
  if (!node)
    {
    return true;
    }
  if (!d->ShowHidden && node->GetHideFromEditors())
    {
    return false;
    }
  if (d->NodeTypes.isEmpty())
    {
    return true;
    }
  foreach(const QString& nodeType, d->NodeTypes)
    {
    // filter by node type
    if (!node->IsA(nodeType.toAscii().data()))
      {
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
      QString nodeAttribute = 
        node->GetAttribute(d->Attributes[nodeType].first.toLatin1().data());
      if (!nodeAttribute.isEmpty() && 
           nodeAttribute != d->Attributes[nodeType].second.toString())
        {
        return false;
        }
      }
    
    return true;
    }
  return false;
  
}

//-----------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::hideChildNodeTypes()const
{
  CTK_D(const qMRMLSortFilterProxyModel);
  return d->HideChildNodeTypes;
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right)const
{
  return Superclass::lessThan(left, right);
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel::nodeTypes()const
{
  CTK_D(const qMRMLSortFilterProxyModel);
  return d->NodeTypes;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setHideChildNodeTypes(const QStringList& _nodeTypes)
{
  CTK_D(qMRMLSortFilterProxyModel);
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
  CTK_D(qMRMLSortFilterProxyModel);
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
  CTK_D(qMRMLSortFilterProxyModel);
  if (_show == d->ShowChildNodeTypes)
    {
    return;
    }
  d->ShowChildNodeTypes = _show;
  invalidateFilter();
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel::setShowHidden(bool enable)
{
  CTK_D(qMRMLSortFilterProxyModel);
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
  CTK_D(const qMRMLSortFilterProxyModel);
  return d->ShowHidden;
}

//-----------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel::showChildNodeTypes()const
{
  CTK_D(const qMRMLSortFilterProxyModel);
  return d->ShowChildNodeTypes;
}
