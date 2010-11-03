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
#include <QSharedPointer>
#include <QDebug>

// qMRMLWidgets includes
#include "qMRMLItemHelper.h"
#include "qMRMLUtils.h"
#include "qMRMLSceneModel.h" // needed for UIDRole

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkVariantArray.h>
#include <vtkStdString.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLAbstractItemHelperFactory::createRootItem(vtkMRMLScene* scene)const
{
  return new qMRMLRootItemHelper(scene, this);
}

//------------------------------------------------------------------------------
class qMRMLAbstractItemHelperPrivate
{
public:
  int Row;
  int Column;
  const qMRMLAbstractItemHelperFactory* ItemFactory;
};

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper::qMRMLAbstractItemHelper(int _column, const qMRMLAbstractItemHelperFactory* itemFactory, int _row)
  : d_ptr(new qMRMLAbstractItemHelperPrivate)
{
  Q_D(qMRMLAbstractItemHelper);
  d->Row = _row;
  d->Column = _column;
  d->ItemFactory = itemFactory;
}

qMRMLAbstractItemHelper::~qMRMLAbstractItemHelper()
{
  //std::cout << "+";
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::canReparent(qMRMLAbstractItemHelper* newParent)const
{
  Q_UNUSED(newParent);
  return false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLAbstractItemHelper::child(int _row, int _column) const
{
  Q_UNUSED(_row);
  Q_UNUSED(_column);
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::childCount() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::childIndex(const qMRMLAbstractItemHelper* _child) const
{
  Q_UNUSED(_child);
  return -1;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::column() const
{
  Q_D(const qMRMLAbstractItemHelper);
  return d->Column;
}

//------------------------------------------------------------------------------
QVariant qMRMLAbstractItemHelper::data(int role) const
{
  Q_UNUSED(role);
  return QVariant();
}

//------------------------------------------------------------------------------
const qMRMLAbstractItemHelperFactory* qMRMLAbstractItemHelper::factory()const
{
  Q_D(const qMRMLAbstractItemHelper);
  return d->ItemFactory;
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
  Q_UNUSED(newParent);
  return false;
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::row() const
{
  Q_D(const qMRMLAbstractItemHelper);
  QSharedPointer<const qMRMLAbstractItemHelper> _parent =
    QSharedPointer<const qMRMLAbstractItemHelper>(this->parent());
  if (_parent.isNull())
    {
    return 0;
    }
  if (d->Row != -1)
    {
    return d->Row;
    }
  return _parent->childIndex(this);
}

//------------------------------------------------------------------------------
int qMRMLAbstractItemHelper::row(const qMRMLAbstractItemHelper* child) const
{
  return this->childIndex(child);
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::setData(const QVariant &value, int role)
{
  Q_UNUSED(value);
  Q_UNUSED(role);
  return false;
}

//------------------------------------------------------------------------------
bool qMRMLAbstractItemHelper::operator==(const qMRMLAbstractItemHelper& helper)const
{
  return this->row() == helper.row() && 
         this->column() == helper.column() && 
         this->object() == helper.object();
}


// qMRMLAbstractSceneItemHelper
//------------------------------------------------------------------------------
class qMRMLAbstractSceneItemHelperPrivate
{
public:
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
qMRMLAbstractSceneItemHelper::qMRMLAbstractSceneItemHelper(vtkMRMLScene* scene, int _column, const qMRMLAbstractItemHelperFactory* itemFactory, int _row)
  :qMRMLAbstractItemHelper(_column, itemFactory, _row)
  , d_ptr(new qMRMLAbstractSceneItemHelperPrivate)
{
  Q_D(qMRMLAbstractSceneItemHelper);
  d->MRMLScene = scene;
}

//------------------------------------------------------------------------------
qMRMLAbstractSceneItemHelper::~qMRMLAbstractSceneItemHelper()
{
}

//------------------------------------------------------------------------------
QVariant qMRMLAbstractSceneItemHelper::data(int role) const
{
  Q_D(const qMRMLAbstractSceneItemHelper);
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
Qt::ItemFlags qMRMLAbstractSceneItemHelper::flags() const
{
  return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLAbstractSceneItemHelper::object() const
{
  Q_D(const qMRMLAbstractSceneItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLAbstractSceneItemHelper::mrmlScene() const
{
  Q_D(const qMRMLAbstractSceneItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
// qMRMLAbstractNodeItemHelper


//------------------------------------------------------------------------------
class qMRMLAbstractNodeItemHelperPrivate
{
public:
  vtkMRMLNode* MRMLNode;
};

//------------------------------------------------------------------------------
qMRMLAbstractNodeItemHelper::qMRMLAbstractNodeItemHelper(vtkMRMLNode* _node, int _column, const qMRMLAbstractItemHelperFactory* itemFactory, int _row)
  :qMRMLAbstractItemHelper(_column, itemFactory, _row)
  , d_ptr(new qMRMLAbstractNodeItemHelperPrivate)
{
  Q_ASSERT(_node);
  Q_D(qMRMLAbstractNodeItemHelper);
  d->MRMLNode = _node;
}

//------------------------------------------------------------------------------
qMRMLAbstractNodeItemHelper::~qMRMLAbstractNodeItemHelper()
{
}

//------------------------------------------------------------------------------
QVariant qMRMLAbstractNodeItemHelper::data(int role) const
{
  Q_D(const qMRMLAbstractNodeItemHelper);
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
Qt::ItemFlags qMRMLAbstractNodeItemHelper::flags() const
{
  Q_D(const qMRMLAbstractNodeItemHelper);
  Qt::ItemFlags f;
  f |= Qt::ItemIsEnabled;
  if (d->MRMLNode->GetSelectable())
    {
    f |= Qt::ItemIsSelectable;
    }
  if (this->column() == 0)
    {
    f |= Qt::ItemIsEditable;
    }
  return f;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLAbstractNodeItemHelper::object() const
{
  Q_D(const qMRMLAbstractNodeItemHelper);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLAbstractNodeItemHelper::mrmlNode() const
{
  Q_D(const qMRMLAbstractNodeItemHelper);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
bool qMRMLAbstractNodeItemHelper::setData(const QVariant &value, int role)
{
  Q_D(qMRMLAbstractNodeItemHelper);
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


//------------------------------------------------------------------------------
// qMRMLAbstractRootItemHelper
//------------------------------------------------------------------------------
class qMRMLRootItemHelperPrivate
{
public:
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
qMRMLRootItemHelper::qMRMLRootItemHelper(vtkMRMLScene* scene, 
                                         const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractItemHelper(-1, itemFactory, -1)
  , d_ptr(new qMRMLRootItemHelperPrivate)
{
  Q_D(qMRMLRootItemHelper);
  d->MRMLScene = scene;
}

//------------------------------------------------------------------------------
qMRMLRootItemHelper::~qMRMLRootItemHelper()
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLRootItemHelper::child(int _row, int _column) const
{
  Q_D(const qMRMLRootItemHelper);
  if (_row == 0)
    {
    //return ctk_d()->MRMLScene;
    return this->factory()->createItem(d->MRMLScene, _column);
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLRootItemHelper::childCount() const
{
  Q_D(const qMRMLRootItemHelper);
  return d->MRMLScene ? 1 : 0;
}

//------------------------------------------------------------------------------
QVariant qMRMLRootItemHelper::data(int role) const
{
  Q_UNUSED(role);
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
  Q_D(const qMRMLRootItemHelper);
  return d->MRMLScene != 0 ? true : false;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLRootItemHelper::mrmlScene() const
{
  Q_D(const qMRMLRootItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLRootItemHelper::object() const
{
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLRootItemHelper::parent() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLRootItemHelper::row() const
{
  //Q_ASSERT(false);
  return -1;
}

//------------------------------------------------------------------------------
int qMRMLRootItemHelper::childIndex(const qMRMLAbstractItemHelper* _child)const
{
  Q_UNUSED(_child);
  // we know for sure that child is a child of this, child is at index 0
  return 0;
}

//------------------------------------------------------------------------------
// vtkWeakObject: utility object that wraps a vtkWeakPointerBase into a vtkObject
class vtkWeakObject : public vtkObject
{
public: 
  static vtkWeakObject *New();
  vtkTypeMacro(vtkWeakObject, vtkObject);
  vtkObject* GetPointer()const
  { 
    return vtkObject::SafeDownCast(this->WeakPointer.GetPointer());
  }
  // TBD: not sure it
  vtkWeakPointerBase WeakPointer;
protected:
  vtkWeakObject(){}
private:
  vtkWeakObject(const vtkWeakObject&);  // Not implemented.
  void operator=(const vtkWeakObject&);  // Not implemented.
};
vtkStandardNewMacro(vtkWeakObject);

//------------------------------------------------------------------------------
// qMRMLVariantArrayItemHelper

//------------------------------------------------------------------------------
class qMRMLVariantArrayItemHelperPrivate
{
public:
  vtkSmartPointer<vtkVariantArray> VariantArray;
  bool isSeparator()const;
};

//------------------------------------------------------------------------------
bool qMRMLVariantArrayItemHelperPrivate::isSeparator()const
{
  const vtkVariant v = this->VariantArray->GetValue(1);
  return v.ToString() == "separator";
}

//------------------------------------------------------------------------------
qMRMLVariantArrayItemHelper::qMRMLVariantArrayItemHelper(vtkVariantArray* array, int column, const qMRMLAbstractItemHelperFactory* factory, int _row)
  :qMRMLAbstractItemHelper(column, factory, _row)
  , d_ptr(new qMRMLVariantArrayItemHelperPrivate)
{
  Q_D(qMRMLVariantArrayItemHelper);
  d->VariantArray = array;
}

//------------------------------------------------------------------------------
qMRMLVariantArrayItemHelper::~qMRMLVariantArrayItemHelper()
{
}

//------------------------------------------------------------------------------
QVariant qMRMLVariantArrayItemHelper::data(int role) const
{
  Q_D(const qMRMLVariantArrayItemHelper);
  switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      if (d->isSeparator())
        {
        return QString("");
        }
    case qMRML::UIDRole:
      if (this->column() == 0)
        {
        const vtkVariant v = d->VariantArray->GetValue(1);
        return QString(v.ToString());
        }
      else
        {
        return QString("");
        }
      break;
    case Qt::AccessibleDescriptionRole:
      if (d->isSeparator())
        {
        // QComboBox understands "separator"
        return QString("separator");
        }
    default:
      break;
    }
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLVariantArrayItemHelper::flags() const
{
  Q_D(const qMRMLVariantArrayItemHelper);
  return QFlag(d->VariantArray->GetValue(2).ToInt());
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLVariantArrayItemHelper::parent() const
{
  //Q_D(const qMRMLVariantArrayItemHelper);
  return this->factory()->createItem(this->mrmlScene(), 0);
}

//------------------------------------------------------------------------------
bool qMRMLVariantArrayItemHelper::setData(const QVariant &value, int role)
{
  //Q_D(const qMRMLVariantArrayItemHelper);
  return true;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLVariantArrayItemHelper::object()const
{
  Q_D(const qMRMLVariantArrayItemHelper);
  return d->VariantArray;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLVariantArrayItemHelper::mrmlScene()const
{
  Q_D(const qMRMLVariantArrayItemHelper);
  const vtkVariant v = d->VariantArray->GetValue(0);
  vtkWeakObject* weakParent = vtkWeakObject::SafeDownCast(v.ToVTKObject());
  vtkMRMLScene* mrmlScene = vtkMRMLScene::SafeDownCast(weakParent->GetPointer());
  //vtkMRMLScene* mrmlScene = vtkMRMLScene::SafeDownCast(v.ToVTKObject());
  Q_ASSERT(mrmlScene);
  return mrmlScene;
}

//------------------------------------------------------------------------------
void qMRMLVariantArrayItemHelper
::createProperties(vtkVariantArray& properties,
                   vtkObject* itemParent, 
                   const vtkStdString& title,
                   Qt::ItemFlags flags)
{
  properties.SetNumberOfValues(3);
  
  if (itemParent == 0)
    {// add itemParent as an integer
    properties.SetValue(0, vtkVariant(0));
    }
  else
    {// add itemParent as a void*
    // we don't want to increase the ref count of itemParent.
    // the following is not possible 
    // properties.SetValue(0, vtkVariant(itemParent));
    // because vtkVariant adds a ref automatically to the holded object. 
    // This is why we use a vtkWeakPointer wrapped into a vtkObject.
    vtkWeakObject* weakParent = vtkWeakObject::New();
    weakParent->WeakPointer = itemParent;
    properties.SetValue(0, vtkVariant(weakParent));
    weakParent->Delete();
    }
  properties.SetValue(1, vtkVariant(title));
  properties.SetValue(2, vtkVariant(static_cast<int>(flags)));
}

//------------------------------------------------------------------------------
// qMRMLProxyItemHelper

class qMRMLProxyItemHelperPrivate
{
public:
  QSharedPointer<qMRMLAbstractItemHelper> Proxy;
};

//------------------------------------------------------------------------------
qMRMLProxyItemHelper::qMRMLProxyItemHelper(qMRMLAbstractItemHelper* _proxy)
  :qMRMLAbstractItemHelper(_proxy->column(), _proxy->factory(), _proxy->row())
  , d_ptr(new qMRMLProxyItemHelperPrivate)
{
  Q_D(qMRMLProxyItemHelper);
  d->Proxy = QSharedPointer<qMRMLAbstractItemHelper>(_proxy);
}

//------------------------------------------------------------------------------
qMRMLProxyItemHelper::~qMRMLProxyItemHelper()
{
}

//------------------------------------------------------------------------------
bool qMRMLProxyItemHelper::canReparent(qMRMLAbstractItemHelper* newParent)const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->canReparent(newParent);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLProxyItemHelper::child(int row, int column) const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->child(row, column);
}

//------------------------------------------------------------------------------
int qMRMLProxyItemHelper::childCount() const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->childCount();
}

//------------------------------------------------------------------------------
int qMRMLProxyItemHelper::column() const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->column();
}

//------------------------------------------------------------------------------
QVariant qMRMLProxyItemHelper::data(int role) const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->data(role);
}
//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLProxyItemHelper::flags() const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->flags();
}
//------------------------------------------------------------------------------
bool qMRMLProxyItemHelper::hasChildren() const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->hasChildren();
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLProxyItemHelper::parent() const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->parent();
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLProxyItemHelper::proxy()const
{
  Q_D(const qMRMLProxyItemHelper);
  Q_ASSERT(!d->Proxy.isNull());
  return d->Proxy.data();
}

//------------------------------------------------------------------------------
bool qMRMLProxyItemHelper::reparent(qMRMLAbstractItemHelper* newParent)
{
  Q_D(qMRMLProxyItemHelper);
  return d->Proxy->reparent(newParent);
}

//------------------------------------------------------------------------------
int qMRMLProxyItemHelper::row() const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->row();
}

//------------------------------------------------------------------------------
bool qMRMLProxyItemHelper::setData(const QVariant &value, int role)
{
  Q_D(qMRMLProxyItemHelper);
  return d->Proxy->setData(value, role);
}

//------------------------------------------------------------------------------
vtkObject* qMRMLProxyItemHelper::object()const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->object();
}

//------------------------------------------------------------------------------
bool qMRMLProxyItemHelper::operator==(const qMRMLAbstractItemHelper& helper)const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->operator==(helper);
}

//------------------------------------------------------------------------------
int qMRMLProxyItemHelper::childIndex(const qMRMLAbstractItemHelper* child)const
{
  Q_D(const qMRMLProxyItemHelper);
  return d->Proxy->childIndex(child);
}

//------------------------------------------------------------------------------
// qMRMLExtraItemsHelper
class qMRMLExtraItemsHelperPrivate 
{
public: 
  qMRMLExtraItemsHelperPrivate();
  vtkCollection* PreItems;
  vtkCollection* PostItems;
  int preItemsCount()const;
  int postItemsCount()const;
};

//------------------------------------------------------------------------------
qMRMLExtraItemsHelperPrivate::qMRMLExtraItemsHelperPrivate()
{
  this->PreItems = 0;
  this->PostItems = 0;
}

//------------------------------------------------------------------------------
int qMRMLExtraItemsHelperPrivate::preItemsCount()const
{
  return this->PreItems ? this->PreItems->GetNumberOfItems() : 0;
}

//------------------------------------------------------------------------------
int qMRMLExtraItemsHelperPrivate::postItemsCount()const
{
  return this->PostItems ? this->PostItems->GetNumberOfItems() : 0;
}

//------------------------------------------------------------------------------
qMRMLExtraItemsHelper::qMRMLExtraItemsHelper(vtkCollection* preItems, 
                                             vtkCollection* postItems, 
                                             qMRMLAbstractItemHelper* proxy)
:qMRMLProxyItemHelper(proxy)
  , d_ptr(new qMRMLExtraItemsHelperPrivate)
{
  Q_D(qMRMLExtraItemsHelper);
  d->PreItems = preItems;
  d->PostItems = postItems;
}

//------------------------------------------------------------------------------
qMRMLExtraItemsHelper::~qMRMLExtraItemsHelper()
{

}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLExtraItemsHelper::child(int row, int column) const
{
  Q_D(const qMRMLExtraItemsHelper);
  int tmp;
  int relativeRow = row;
  if (relativeRow < (tmp=d->preItemsCount()))
    {
    return this->factory()->createItem(d->PreItems->GetItemAsObject(relativeRow), column, row);
    }
  relativeRow -= tmp;
  if (relativeRow < (tmp = this->proxy()->childCount()))
    {
    return this->qMRMLProxyItemHelper::child(relativeRow, column);
    }
  relativeRow -= tmp;
  if (relativeRow < (tmp=d->postItemsCount()))
    {
    return this->factory()->createItem(d->PostItems->GetItemAsObject(relativeRow), column, row);
    }
  Q_ASSERT(relativeRow < tmp);
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLExtraItemsHelper::childCount() const
{
  Q_D(const qMRMLExtraItemsHelper);
  if (this->column() != 0)
    {
    return 0;
    }
  //Q_ASSERT(d->preItemsCount() == 1);
  return d->preItemsCount() + this->proxy()->childCount() + d->postItemsCount();
}

//------------------------------------------------------------------------------
bool qMRMLExtraItemsHelper::hasChildren() const
{
  Q_D(const qMRMLExtraItemsHelper);
  if (this->column() != 0)
    {
    return false;
    }
  return d->preItemsCount()
    || d->postItemsCount()
    || this->qMRMLProxyItemHelper::hasChildren();
}

//------------------------------------------------------------------------------
int qMRMLExtraItemsHelper::childIndex(const qMRMLAbstractItemHelper* child)const
{
  Q_D(const qMRMLExtraItemsHelper);
  Q_ASSERT(child);
  int childRow = 0;
  vtkObject* childObject = 0;
  vtkCollectionSimpleIterator it;
  if (d->PreItems)
    {
    d->PreItems->InitTraversal(it);
    //pre item ?
    for (childObject = d->PreItems->GetNextItemAsObject(it); 
         childObject && childObject != child->object(); 
         childObject = d->PreItems->GetNextItemAsObject(it))
      {
      ++childRow;
      }
    if (childObject)
      {
      return childRow;
      }
    }
  // postItem ?
  if (d->PostItems)
    {
    childRow += this->proxy()->childCount();
    d->PostItems->InitTraversal(it);
    for (childObject = d->PostItems->GetNextItemAsObject(it); 
         childObject && childObject != child->object(); 
         childObject = d->PostItems->GetNextItemAsObject(it))
      {
      ++childRow;
      }
    if (childObject)
      {
      return childRow;
      }
    }
  // not a pre, nor a post, must be a child from the proxy
  childRow = d->preItemsCount();
  int index = this->qMRMLProxyItemHelper::childIndex(child);
  if (index < 0)
    {
    Q_ASSERT(this->qMRMLProxyItemHelper::childIndex(child) >= 0);
    }
  return childRow + index;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLExtraItemsHelper::preItems() const
{
  Q_D(const qMRMLExtraItemsHelper);
  return d->PreItems;
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLExtraItemsHelper::postItems() const
{
  Q_D(const qMRMLExtraItemsHelper);
  return d->PostItems;
}
