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
#include "qMRMLTransformProxyModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

class qMRMLNodeItemHelperPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLTransformItemHelperFactory : public qMRMLAbstractItemHelperFactory
{
public:
  virtual ~qMRMLTransformItemHelperFactory(){}
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column)const;
  /// The default behavior for createRootItem() is good enough
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSceneItemHelper : public qMRMLAbstractSceneItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent()const;

protected:
  friend class qMRMLTransformItemHelperFactory;
  qMRMLSceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory);
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLNodeItemHelper : public qMRMLAbstractNodeItemHelper
{
public:

  virtual bool canReparent(qMRMLAbstractItemHelper* newParent)const;
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent() const;
  virtual bool reparent(qMRMLAbstractItemHelper* newParent);

protected:
  friend class qMRMLTransformItemHelperFactory;
  qMRMLNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory);
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
};


// qMRMLTransformItemHelperFactory
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformItemHelperFactory::createItem(vtkObject* object, int column)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLSceneItemHelper(vtkMRMLScene::SafeDownCast(object), column, this);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this);
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

// qMRMLSceneItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

qMRMLSceneItemHelper::qMRMLSceneItemHelper(vtkMRMLScene* scene, int _column, 
                                           const qMRMLAbstractItemHelperFactory* _factory)
  :qMRMLAbstractSceneItemHelper(scene, _column, _factory)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneItemHelper::child(int _row, int _column) const
{
  vtkMRMLNode* childNode = qMRMLUtils::topLevelNthNode(this->mrmlScene(), _row);
  if (childNode == 0)
    {
    return 0;
    }
  //qMRMLNodeItemHelper* _child = new qMRMLNodeItemHelper(childNode, _column);
  //return _child;
  //return childNode;
  return this->factory()->createItem(childNode,_column);
}

//------------------------------------------------------------------------------
int qMRMLSceneItemHelper::childCount() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  return qMRMLUtils::childCount(this->mrmlScene());
}

//------------------------------------------------------------------------------
int qMRMLSceneItemHelper::childIndex(const qMRMLAbstractItemHelper* _child) const
{
  const qMRMLAbstractNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLAbstractNodeItemHelper*>(_child);
  Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  return qMRMLUtils::nodeIndex(nodeItemHelper->mrmlNode());
}

//------------------------------------------------------------------------------
bool qMRMLSceneItemHelper::hasChildren() const
{
  return this->mrmlScene() ? this->mrmlScene()->GetNumberOfNodes() > 0 : false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneItemHelper::parent() const
{
  //return this->mrmlScene();
  return this->factory()->createRootItem(this->mrmlScene());
}

// qMRMLNodeItemHelper
//------------------------------------------------------------------------------
qMRMLNodeItemHelper::qMRMLNodeItemHelper(vtkMRMLNode* node, int _column, const qMRMLAbstractItemHelperFactory* _factory)
  :qMRMLAbstractNodeItemHelper(node, _column, _factory)
{
  Q_ASSERT(node);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::canReparent(qMRMLAbstractItemHelper* newParent)const
{
  if (this->qMRMLAbstractNodeItemHelper::canReparent(newParent))
    {
    return true;
    }
  bool _canReparent = false;
  if (dynamic_cast<qMRMLAbstractSceneItemHelper*>(newParent))
    {
    _canReparent = qMRMLUtils::canReparent(this->mrmlNode(), 0);
    }
  else if (dynamic_cast<qMRMLNodeItemHelper*>(newParent))
    {
    vtkMRMLNode*  newParentNode = dynamic_cast<qMRMLNodeItemHelper*>(newParent)->mrmlNode();
    _canReparent = qMRMLUtils::canReparent(this->mrmlNode(), newParentNode);
    }
  return _canReparent;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLNodeItemHelper::child(int _row, int _column) const
{
  vtkMRMLNode* childNode = qMRMLUtils::childNode(this->mrmlNode(), _row);
  if (childNode == 0)
    {
    return 0;
    }
  //qMRMLNodeItemHelper* _child = new qMRMLNodeItemHelper(childNode, _column);
  ///return _child;
  //return childNode;
  return this->factory()->createItem(childNode, _column);
}

//------------------------------------------------------------------------------
int qMRMLNodeItemHelper::childCount() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  return qMRMLUtils::childCount(this->mrmlNode());
}

//------------------------------------------------------------------------------
int qMRMLNodeItemHelper::childIndex(const qMRMLAbstractItemHelper* _child) const
{
  const qMRMLNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLNodeItemHelper*>(_child);
  Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  return qMRMLUtils::nodeIndex(nodeItemHelper->mrmlNode());
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLNodeItemHelper::flags() const
{
  Qt::ItemFlags f = this->qMRMLAbstractNodeItemHelper::flags();
  if (qMRMLUtils::canBeAChild(this->mrmlNode()))
    {
    f |= Qt::ItemIsDragEnabled;
    }
  if (this->column() == 0 && qMRMLUtils::canBeAParent(this->mrmlNode()))
    {
    f |= Qt::ItemIsDropEnabled;
    }
  return f;
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::hasChildren() const
{
  return this->column() == 0 && 
    qMRMLUtils::canBeAParent(this->mrmlNode()) && 
    (qMRMLUtils::childNode(this->mrmlNode(), 0) != 0);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLNodeItemHelper::parent() const
{
  vtkMRMLNode* parentNode = qMRMLUtils::parentNode(this->mrmlNode());
  if (parentNode == 0)
    {
    //return new qMRMLSceneItemHelper(this->mrmlNode()->GetScene(),0);
    //return this->mrmlNode()->GetScene();
    return this->factory()->createItem(this->mrmlNode()->GetScene(), 0);
    }
  //return new qMRMLNodeItemHelper(parentNode, 0);
  //return parentNode;
  return this->factory()->createItem(parentNode, 0);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::reparent(qMRMLAbstractItemHelper* newParent)
{ 
  vtkMRMLNode*  newParentNode = 
    newParent ? vtkMRMLNode::SafeDownCast(newParent->object()) : 0;
  return qMRMLUtils::reparent(this->mrmlNode(), newParentNode);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class qMRMLTransformProxyModelPrivate: public ctkPrivate<qMRMLTransformProxyModel>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLTransformProxyModel);
  qMRMLTransformProxyModelPrivate();
  virtual ~qMRMLTransformProxyModelPrivate();
  qMRMLTransformItemHelperFactory* ItemFactory;
};

//------------------------------------------------------------------------------
qMRMLTransformProxyModelPrivate::qMRMLTransformProxyModelPrivate()
{
  this->ItemFactory = new qMRMLTransformItemHelperFactory;
}

//------------------------------------------------------------------------------
qMRMLTransformProxyModelPrivate::~qMRMLTransformProxyModelPrivate()
{
  delete this->ItemFactory;
  this->ItemFactory = 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLTransformProxyModel::qMRMLTransformProxyModel(QObject *vparent)
  :qMRMLTreeProxyModel(vparent)
{
  CTK_INIT_PRIVATE(qMRMLTransformProxyModel);
}

//------------------------------------------------------------------------------
qMRMLTransformProxyModel::~qMRMLTransformProxyModel()
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelperFactory* qMRMLTransformProxyModel::itemFactory()const
{
  CTK_D(const qMRMLTransformProxyModel);
  return d->ItemFactory;
}
