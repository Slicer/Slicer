#include "qMRMLItems.h"

//------------------------------------------------------------------------------
struct qMRMLAbstractItemPrivate: public qCTKPrivate<qMRMLAbstractItem>
{
  QCTK_DECLARE_PUBLIC(qMRMLAbstractItem);
  int Column;
  QVector<qMRMLAbstractItem*> Children;
};

//------------------------------------------------------------------------------
qMRMLAbstractItem::qMRMLAbstractItem(int column)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractItem);
  qctk_d()->Column = column;
}

//------------------------------------------------------------------------------
qMRMLAbstractItem* qMRMLAbstractItem::child(int row, int column)
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItem::childCount() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItem::childIndex(qMRMLAbstracItem* child) const
{
  return -1;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItem::column() const
{
  return qctk_d()->Column;
}

//------------------------------------------------------------------------------
QVariant qMRMLAbstractItem::data(int role) const
{
  return QVariant();
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItem::hasChildren() const
{
  return false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItem* qMRMLAbstractItem::parent() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItem::row() const
{
  qMRMLAbstractItem* parent = this->parent();
  if (parent == 0)
    {
    return 0;
    }
  return parent->childIndex(this);
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItem::setData(const QVariant &value, int role)
{
  return false;
}

// qMRMLSceneItem
//------------------------------------------------------------------------------
struct qMRMLSceneItemPrivate: public qCTKPrivate<qMRMLSceneItem>
{
  QCTK_DECLARE_PUBLIC(qMRMLSceneItem); 
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
qMRMLSceneItem::qMRMLSceneItem(vtkMRMLScene* scene, int column)
  :qMRMLAbstractItem(column)
{
  QCTK_INIT_PRIVATE(qMRMLSceneItem);
  Q_ASSERT(scene);
  qctk_d()->MRMLScene = scene;
}

//------------------------------------------------------------------------------
qMRMLAbstractItem* qMRMLSceneItem::child(int row, int column) const
{
  QCTK_D(const qMRMLSceneItem);
  vtkMRMLNode* childNode = qMRMLUtils::topLevelNthNode(d->MRMLScene, row);
  Q_ASSERT(childNode);
  if (childNode == 0)
    {
    return 0;
    }
  qMRMLNodeItem* child = new qMRMLNodeItem(childNode, column);
  d->Children[row] = child;
  return child;
}

//------------------------------------------------------------------------------
int qMRMLSceneItem::childCount() const
{
  return qMRMLUtils::childCount(qctk_d()->MRMLScene);
}

//------------------------------------------------------------------------------
int qMRMLAbstractItem::childIndex(qMRMLAbstracItem* child) const
{
  qMRMLNodeItem* nodeItem = dynamic_cast<qMRMLNodeItem*>(child);
  Q_ASSERT(nodeItem);
  if (nodeItem == 0)
    {
    return -1;     
    }
  return qMRMLUtils::nodeIndex(nodeItem->mrmlNode());
}

//------------------------------------------------------------------------------
QVariant qMRMLSceneItem::data(int role) const
{
  QCTK_D(const qMRMLSceneItem);
  switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
      return QString(d->MRMLScene->URL.c_str());
      break;
    default:
      break;
    }
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLSceneItem::flags() const
{
  return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

//------------------------------------------------------------------------------
bool qMRMLSceneItem::hasChildren() const
{
  QCTK_D(const qMRMLSceneItem);
  return d->MRMLScene->GetNumberOfNodes() > 0;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLSceneItem::object() const
{
  QCTK_D(const qMRMLSceneItem);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
qMRMLAbstractItem* qMRMLSceneItem::parent() const
{
  return 0;
}

// qMRMLNodeItem
//------------------------------------------------------------------------------
struct qMRMLNodeItemPrivate: public qCTKPrivate<qMRMLNodeItem>
{
  QCTK_DECLARE_PUBLIC(qMRMLNodeItem); 
  vtkMRMLNode* MRMLNode;
};

//------------------------------------------------------------------------------
qMRMLNodeItem::qMRMLNodeItem(vtkMRMLNode* node, int column)
  :qMRMLAbstractItem(column)
{
  QCTK_INIT_PRIVATE(qMRMLNodeItem);
  Q_ASSERT(node);
  qctk_d()->MRMLNode = node;
}

//------------------------------------------------------------------------------
qMRMLAbstractItem* qMRMLNodeItem::child(int row, int column) const
{
  QCTK_D(const qMRMLNodeItem);
  vtkMRMLNode* childNode = qMRMLUtils::childNode(d->MRMLNode, row);
  Q_ASSERT(childNode);
  if (childNode == 0)
    {
    return 0;
    }
  qMRMLNodeItem* child = new qMRMLNodeItem(childNode, column);
  return child;
}

//------------------------------------------------------------------------------
int qMRMLNodeItem::childCount() const
{
  return qMRMLUtils::childCount(qctk_d()->MRMLNode);
}

//------------------------------------------------------------------------------
int qMRMLAbstractItem::childIndex(qMRMLAbstractItem* child) const
{
  qMRMLNodeItem* nodeItem = dynamic_cast<qMRMLNodeItem*>(child);
  Q_ASSERT(nodeItem);
  if (nodeItem == 0)
    {
    return -1;
    }
  return qMRMLUtils::nodeIndex(nodeItem->mrmlNode());
}

//------------------------------------------------------------------------------
QVariant qMRMLNodeItem::data(int role) const
{
  QCTK_D(const qMRMLNodeItem);
  switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
      switch (this->column())
        {
        case 0:
          return QString(d->MRMLNode->GetName());
          break;
        case 1:
          return QString(d->MRMLNode->GetId());
          break;
        default:
          break;
        }
      break;
    default:
      break;
    }
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLNodeItem::flags() const
{
  QCTK_D(const qMRMLNodeItem);
  Qt::ItemFlags f;
  if (!d->MRMLNode->GetHideFromEditors())
    {
    f |= Qt::ItemIsEnabled;
    }
  if (d->MRMLNode->GetSelectable())
    {
    f |= Qt::ItemIsSelectable;
    }
  if (qMRMLUtils::canBeAChild(d->MRMLNode))
    {
    f |= Qt::ItemIsDragEnabled;
    }
  if (this->column() == 0 && qMRMLUtils::canBeAParent(d->MRMLNode))
    {
    f |= Qt::ItemIsDropEnabled;
    }
  if (this->column() == 0)
    {
    f |= Qt::ItemIsEditable;
    }
  return f;
}

//------------------------------------------------------------------------------
bool qMRMLNodeItem::hasChildren() const
{
  QCTK_D(const qMRMLNodeItem);
  return this->column() == 0 && 
    qMRMLUtils::canBeAParent(d->MRMLNode) && 
    (qMRMLUtils::childNode(d->MRMLNode, 0) != 0);
}
//------------------------------------------------------------------------------
vtkObject* qMRMLNodeItem::object() const
{
  QCTK_D(const qMRMLNodeItem);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
qMRMLAbstractItem* qMRMLNodeItem::parent() const
{
  QCTK_D(const qMRMLNodeItem);
  vtkMRMLNode* parentNode = qMRMLUtils::parentNode(d->MRMLNode);
  if (parentNode == 0)
    {
    return new qMRMLSceneItem(d->MRMLNode->GetScene(), this->column());
    }
  return new qMRMLNodeItem(parentNode, this->column());
}
