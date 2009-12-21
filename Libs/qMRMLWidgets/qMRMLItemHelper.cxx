#include "qMRMLItemHelper.h"


#include "qMRMLUtils.h"
#include "qMRMLItemModel.h"
#include <vtkMRMLScene.h>
#include <QSharedPointer>
#include <QDebug>

//------------------------------------------------------------------------------
class qMRMLAbstractItemHelperPrivate: public qCTKPrivate<qMRMLAbstractItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLAbstractItemHelper);
  int Column;
};

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper::qMRMLAbstractItemHelper(int column)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractItemHelper);
  qctk_d()->Column = column;
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::canReparent(qMRMLAbstractItemHelper* newParent)const
{
  return false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLAbstractItemHelper::child(int row, int column) const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::childCount() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::childIndex(const qMRMLAbstractItemHelper* child) const
{
  return -1;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::column() const
{
  return qctk_d()->Column;
}

//------------------------------------------------------------------------------
QVariant qMRMLAbstractItemHelper::data(int role) const
{
  return QVariant();
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::hasChildren() const
{
  return false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLAbstractItemHelper::parent() const
{
  return 0;
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::reparent(qMRMLAbstractItemHelper* newParent)
{
  return false;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::row() const
{
  QSharedPointer<const qMRMLAbstractItemHelper> parent = 
    QSharedPointer<const qMRMLAbstractItemHelper>(this->parent());
  if (parent.isNull())
    {
    return 0;
    }
  return parent->childIndex(this);
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::setData(const QVariant &value, int role)
{
  return false;
}

// qMRMLSceneItemHelper
//------------------------------------------------------------------------------
class qMRMLSceneItemHelperPrivate: public qCTKPrivate<qMRMLSceneItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLSceneItemHelper); 
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
qMRMLSceneItemHelper::qMRMLSceneItemHelper(vtkMRMLScene* scene, int column)
  :qMRMLAbstractItemHelper(column)
{
  QCTK_INIT_PRIVATE(qMRMLSceneItemHelper);
  qctk_d()->MRMLScene = scene;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneItemHelper::child(int row, int column) const
{
  QCTK_D(const qMRMLSceneItemHelper);
  vtkMRMLNode* childNode = qMRMLUtils::topLevelNthNode(d->MRMLScene, row);
  if (childNode == 0)
    {
    return 0;
    }
  qMRMLNodeItemHelper* child = new qMRMLNodeItemHelper(childNode, column);
  return child;
}

//------------------------------------------------------------------------------
int qMRMLSceneItemHelper::childCount() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  return qMRMLUtils::childCount(qctk_d()->MRMLScene);
}

//------------------------------------------------------------------------------
int qMRMLSceneItemHelper::childIndex(const qMRMLAbstractItemHelper* child) const
{
  const qMRMLNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLNodeItemHelper*>(child);
  Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  return qMRMLUtils::nodeIndex(nodeItemHelper->mrmlNode());
}

//------------------------------------------------------------------------------
QVariant qMRMLSceneItemHelper::data(int role) const
{
  QCTK_D(const qMRMLSceneItemHelper);
  Q_ASSERT(d->MRMLScene);

  switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
      switch (this->column())
        {
        case 0:
          return QString("Scene");
          break;
        case 1:
          return QString(d->MRMLScene->GetURL());
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
Qt::ItemFlags qMRMLSceneItemHelper::flags() const
{
  return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

//------------------------------------------------------------------------------
bool qMRMLSceneItemHelper::hasChildren() const
{
  QCTK_D(const qMRMLSceneItemHelper);
  qDebug() << d->MRMLScene << (d->MRMLScene ? d->MRMLScene->GetNumberOfNodes() : -1);
  return d->MRMLScene ? d->MRMLScene->GetNumberOfNodes() > 0 : false;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLSceneItemHelper::object() const
{
  QCTK_D(const qMRMLSceneItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneItemHelper::mrmlScene() const
{
  QCTK_D(const qMRMLSceneItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneItemHelper::parent() const
{
  return 0;
}

// qMRMLNodeItemHelper
//------------------------------------------------------------------------------
class qMRMLNodeItemHelperPrivate: public qCTKPrivate<qMRMLNodeItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLNodeItemHelper); 
  vtkMRMLNode* MRMLNode;
};

//------------------------------------------------------------------------------
qMRMLNodeItemHelper::qMRMLNodeItemHelper(vtkMRMLNode* node, int column)
  :qMRMLAbstractItemHelper(column)
{
  QCTK_INIT_PRIVATE(qMRMLNodeItemHelper);
  Q_ASSERT(node);
  qctk_d()->MRMLNode = node;
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::canReparent(qMRMLAbstractItemHelper* newParent)const
{  
  QCTK_D(const qMRMLNodeItemHelper);
  vtkMRMLNode*  newParentNode = 
    newParent ? vtkMRMLNode::SafeDownCast(newParent->object()) : 0;
  return qMRMLUtils::canReparent(d->MRMLNode, newParentNode);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLNodeItemHelper::child(int row, int column) const
{
  QCTK_D(const qMRMLNodeItemHelper);
  vtkMRMLNode* childNode = qMRMLUtils::childNode(d->MRMLNode, row);
  if (childNode == 0)
    {
    return 0;
    }
  qMRMLNodeItemHelper* child = new qMRMLNodeItemHelper(childNode, column);
  return child;
}

//------------------------------------------------------------------------------
int qMRMLNodeItemHelper::childCount() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  return qMRMLUtils::childCount(qctk_d()->MRMLNode);
}

//------------------------------------------------------------------------------
int qMRMLNodeItemHelper::childIndex(const qMRMLAbstractItemHelper* child) const
{
  const qMRMLNodeItemHelper* nodeItemHelper = dynamic_cast<const qMRMLNodeItemHelper*>(child);
  Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  return qMRMLUtils::nodeIndex(nodeItemHelper->mrmlNode());
}

//------------------------------------------------------------------------------
QVariant qMRMLNodeItemHelper::data(int role) const
{
  QCTK_D(const qMRMLNodeItemHelper);
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
          return QString(d->MRMLNode->GetID());
          break;
        default:
          break;
        }
      break;
    case qMRML::UIDRole:
      return QString(d->MRMLNode->GetID());
      break;
    default:
      break;
    }
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLNodeItemHelper::flags() const
{
  QCTK_D(const qMRMLNodeItemHelper);
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
bool qMRMLNodeItemHelper::hasChildren() const
{
  QCTK_D(const qMRMLNodeItemHelper);
  return this->column() == 0 && 
    qMRMLUtils::canBeAParent(d->MRMLNode) && 
    (qMRMLUtils::childNode(d->MRMLNode, 0) != 0);
}

//------------------------------------------------------------------------------
vtkObject* qMRMLNodeItemHelper::object() const
{
  QCTK_D(const qMRMLNodeItemHelper);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeItemHelper::mrmlNode() const
{
  QCTK_D(const qMRMLNodeItemHelper);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLNodeItemHelper::parent() const
{
  QCTK_D(const qMRMLNodeItemHelper);
  vtkMRMLNode* parentNode = qMRMLUtils::parentNode(d->MRMLNode);
  if (parentNode == 0)
    {
    return new qMRMLSceneItemHelper(d->MRMLNode->GetScene(), 0);
    }
  return new qMRMLNodeItemHelper(parentNode, 0);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::reparent(qMRMLAbstractItemHelper* newParent)
{ 
  QCTK_D(qMRMLNodeItemHelper);
  vtkMRMLNode*  newParentNode = 
    newParent ? vtkMRMLNode::SafeDownCast(newParent->object()) : 0;
  return qMRMLUtils::reparent(d->MRMLNode, newParentNode);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::setData(const QVariant &value, int role)
{
  QCTK_D(qMRMLNodeItemHelper);
  bool changed = false;
  switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
      switch (this->column())
        {
        case 0:
          d->MRMLNode->SetName(value.toString().toAscii().data());
          changed = true;
          break;
        default:
          break;
        }
      break;
    default:
      break;
    }
  return changed;
}

// qMRMLRootItemHelper
//------------------------------------------------------------------------------
class qMRMLRootItemHelperPrivate: public qCTKPrivate<qMRMLRootItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLRootItemHelper);
  bool TopLevelScene;
};

//------------------------------------------------------------------------------
qMRMLRootItemHelper::qMRMLRootItemHelper(bool topLevelScene, vtkMRMLScene* scene)
  :qMRMLSceneItemHelper(scene, -1)
{
  QCTK_INIT_PRIVATE(qMRMLRootItemHelper);
  qctk_d()->TopLevelScene = topLevelScene;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLRootItemHelper::child(int row, int column) const
{
  if (!qctk_d()->TopLevelScene)
    {
    return this->qMRMLSceneItemHelper::child(row, column);
    }
  if (row == 0)
    {
    return new qMRMLSceneItemHelper(this->mrmlScene(), column);
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLRootItemHelper::childCount() const
{
  if (!qctk_d()->TopLevelScene)
    {
    return this->qMRMLSceneItemHelper::childCount();
    }
  return this->mrmlScene() ? 1 : 0;
}

//------------------------------------------------------------------------------
QVariant qMRMLRootItemHelper::data(int role) const
{
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLRootItemHelper::flags() const
{
  Q_ASSERT(false);
  return 0;
}

//------------------------------------------------------------------------------
bool qMRMLRootItemHelper::hasChildren() const
{
  if (!qctk_d()->TopLevelScene)
    {
    return this->qMRMLSceneItemHelper::hasChildren();
    }
  return this->mrmlScene();
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLRootItemHelper::parent() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLRootItemHelper::row() const
{
  Q_ASSERT(false);
  return -1;
}

//------------------------------------------------------------------------------
int qMRMLRootItemHelper::childIndex(const qMRMLAbstractItemHelper* child)const
{
  if (!qctk_d()->TopLevelScene)
    {
    return this->qMRMLSceneItemHelper::childIndex(child);
    }
  // we know for sure that child is a child of this, child is at index 0
  return 0;
}
