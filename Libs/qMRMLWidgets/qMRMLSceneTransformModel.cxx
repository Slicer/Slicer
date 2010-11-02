/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QDebug>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSceneModel_p.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkVariantArray.h>
#include <typeinfo>
/*
class qMRMLNodeItemHelperPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLTransformItemHelperFactory : public qMRMLSceneModelItemHelperFactory
{
public:
  virtual ~qMRMLTransformItemHelperFactory(){}
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column, int row = -1)const;
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
  qMRMLSceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
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
  qMRMLNodeItemHelper(vtkMRMLNode* node, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
};


// qMRMLTransformItemHelperFactory
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLTransformItemHelperFactory::createItem(vtkObject* object, int column, int row)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLScene"))
    {
    qMRMLAbstractItemHelper* scene =
      new qMRMLSceneItemHelper(vtkMRMLScene::SafeDownCast(object), column, this, 0);
    return new qMRMLExtraItemsHelper(this->preItems(), this->postItems(), scene);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLNodeItemHelper(
      vtkMRMLNode::SafeDownCast(object), column, this, row);
    }
  else if (object->IsA("vtkVariantArray"))
    {
    return new qMRMLVariantArrayItemHelper(
      vtkVariantArray::SafeDownCast(object), column, this, row);
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
                                           const qMRMLAbstractItemHelperFactory* _factory, int _row)
  :qMRMLAbstractSceneItemHelper(scene, _column, _factory, _row)
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
  return this->factory()->createItem(childNode,_column, _row);
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
qMRMLNodeItemHelper::qMRMLNodeItemHelper(vtkMRMLNode* node, int _column, const qMRMLAbstractItemHelperFactory* _factory, int _row)
  :qMRMLAbstractNodeItemHelper(node, _column, _factory, _row)
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
  if (dynamic_cast<qMRMLAbstractSceneItemHelper*>(newParent) ||
      dynamic_cast<qMRMLExtraItemsHelper*>(newParent))
    {
    _canReparent = qMRMLUtils::canReparent(this->mrmlNode(), 0);
    }
  else if (dynamic_cast<qMRMLNodeItemHelper*>(newParent))
    {
    vtkMRMLNode*  newParentNode = dynamic_cast<qMRMLNodeItemHelper*>(newParent)->mrmlNode();
    _canReparent = qMRMLUtils::canReparent(this->mrmlNode(), newParentNode);
    }
  else
    {
    qDebug() << "Drag&Drop: item not supported yet" << newParent << typeid(*newParent).name();
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
  return this->factory()->createItem(childNode, _column, _row);
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
    return this->factory()->createItem(this->mrmlNode()->GetScene(), 0, -1);
    }
  //return new qMRMLNodeItemHelper(parentNode, 0);
  //return parentNode;
  return this->factory()->createItem(parentNode, 0, -1);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::reparent(qMRMLAbstractItemHelper* newParent)
{ 
  vtkMRMLNode*  newParentNode = 
    newParent ? vtkMRMLNode::SafeDownCast(newParent->object()) : 0;
  return qMRMLUtils::reparent(this->mrmlNode(), newParentNode);
}
*/

//------------------------------------------------------------------------------
class qMRMLSceneTransformModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneTransformModel);
public:
  qMRMLSceneTransformModelPrivate(qMRMLSceneTransformModel& object);

};

//------------------------------------------------------------------------------
qMRMLSceneTransformModelPrivate
::qMRMLSceneTransformModelPrivate(qMRMLSceneTransformModel& object)
  : qMRMLSceneModelPrivate(object)
{

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneTransformModel::qMRMLSceneTransformModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneTransformModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneTransformModel::~qMRMLSceneTransformModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::populateScene()
{
  Q_D(qMRMLSceneTransformModel);
  Q_ASSERT(d->MRMLScene);
  // Add nodes
  vtkMRMLNode *node = 0;
  vtkCollectionSimpleIterator it;
  for (d->MRMLScene->GetCurrentScene()->InitTraversal(it);
       (node = (vtkMRMLNode*)d->MRMLScene->GetCurrentScene()->GetNextItemAsObject(it)) ;)
    {
    // WARNING: works only if the nodes are in the scene in the correct order:
    // parents are before children
    this->insertNode(node);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::insertNode(vtkMRMLNode* node)
{
  vtkMRMLNode* parentNode = qMRMLUtils::parentNode(node);
  QStandardItem* parentItem =
    parentNode ? this->itemFromNode(parentNode) : this->mrmlSceneItem();
  Q_ASSERT(parentItem);
  int min = this->preItems(parentItem).count();
  int max = parentItem->rowCount() - this->postItems(parentItem).count();
  this->insertNode(node, parentItem, qMin(min + qMRMLUtils::nodeIndex(node), max));
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  bool oldBlock = this->blockSignals(true);
  if (qMRMLUtils::canBeAChild(node))
    {
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    }
  if (qMRMLUtils::canBeAParent(node))
    {
    item->setFlags(item->flags() | Qt::ItemIsDropEnabled);
    }
  this->blockSignals(oldBlock);
  QStandardItem* parentItem = item->parent();
  QStandardItem* newParentItem = this->itemFromNode(qMRMLUtils::parentNode(node));
  if (newParentItem == 0)
    {
    newParentItem = this->mrmlSceneItem();
    }
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // and it will do it automatically.
  if (parentItem != 0 && (parentItem != newParentItem || qMRMLUtils::nodeIndex(node) != item->row()))
    {
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = qMin(min + qMRMLUtils::nodeIndex(node), max);
    newParentItem->insertRow(pos, children);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTransformModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  this->qMRMLSceneModel::updateNodeFromItem(node, item);
  Q_ASSERT(node != this->mrmlNodeFromItem(item->parent()));

  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). updateNodeFromItem is typically
  // called for every item changed, so it should be
  QStandardItem* parentItem = item->parent();
  for (int i = 0; i < parentItem->columnCount(); ++i)
    {
    if (parentItem->child(item->row(), i) == 0)
      {
      return;
      }
    }
  vtkMRMLNode* parent = this->mrmlNodeFromItem(parentItem);
  if (qMRMLUtils::parentNode(node) != parent)
    {
    qMRMLUtils::reparent(node, parent);
    }
  else if (qMRMLUtils::nodeIndex(node) != item->row())
    {
    this->updateItemFromNode(item, node, item->column());
    }
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneTransformModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
