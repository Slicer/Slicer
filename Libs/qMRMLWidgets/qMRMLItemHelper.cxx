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

//------------------------------------------------------------------------------
class qMRMLAbstractItemHelperPrivate: public qCTKPrivate<qMRMLAbstractItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLAbstractItemHelper);
  int Column;
  const qMRMLAbstractItemHelperFactory* ItemFactory;
};

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper::qMRMLAbstractItemHelper(int _column, const qMRMLAbstractItemHelperFactory* itemFactory)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractItemHelper);
  QCTK_D(qMRMLAbstractItemHelper);
  d->Column = _column;
  d->ItemFactory = itemFactory;
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
  return qctk_d()->Column;
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
  QCTK_D(const qMRMLAbstractItemHelper);
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
  QSharedPointer<const qMRMLAbstractItemHelper> _parent =
    QSharedPointer<const qMRMLAbstractItemHelper>(this->parent());
  if (_parent.isNull())
    {
    return 0;
    }
  return _parent->childIndex(this);
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
class qMRMLAbstractSceneItemHelperPrivate: public qCTKPrivate<qMRMLAbstractSceneItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLAbstractSceneItemHelper); 
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
qMRMLAbstractSceneItemHelper::qMRMLAbstractSceneItemHelper(vtkMRMLScene* scene, int _column, const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractItemHelper(_column, itemFactory)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractSceneItemHelper);
  qctk_d()->MRMLScene = scene;
}


//------------------------------------------------------------------------------
QVariant qMRMLAbstractSceneItemHelper::data(int role) const
{
  QCTK_D(const qMRMLAbstractSceneItemHelper);
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
  QCTK_D(const qMRMLAbstractSceneItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLAbstractSceneItemHelper::mrmlScene() const
{
  QCTK_D(const qMRMLAbstractSceneItemHelper);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
// qMRMLAbstractNodeItemHelper


//------------------------------------------------------------------------------
class qMRMLAbstractNodeItemHelperPrivate: public qCTKPrivate<qMRMLAbstractNodeItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLAbstractNodeItemHelper); 
  vtkMRMLNode* MRMLNode;
};

//------------------------------------------------------------------------------
qMRMLAbstractNodeItemHelper::qMRMLAbstractNodeItemHelper(vtkMRMLNode* _node, int _column, const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractItemHelper(_column, itemFactory)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractNodeItemHelper);
  Q_ASSERT(_node);
  qctk_d()->MRMLNode = _node;
}


//------------------------------------------------------------------------------
QVariant qMRMLAbstractNodeItemHelper::data(int role) const
{
  QCTK_D(const qMRMLAbstractNodeItemHelper);
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
  QCTK_D(const qMRMLAbstractNodeItemHelper);
  Qt::ItemFlags f;
  if (!d->MRMLNode->GetHideFromEditors())
    {
    f |= Qt::ItemIsEnabled;
    }
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
  QCTK_D(const qMRMLAbstractNodeItemHelper);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLAbstractNodeItemHelper::mrmlNode() const
{
  QCTK_D(const qMRMLAbstractNodeItemHelper);
  return d->MRMLNode;
}

//------------------------------------------------------------------------------
bool qMRMLAbstractNodeItemHelper::setData(const QVariant &value, int role)
{
  QCTK_D(qMRMLAbstractNodeItemHelper);
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
class qMRMLAbstractRootItemHelperPrivate: public qCTKPrivate<qMRMLAbstractRootItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLAbstractRootItemHelper);
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
qMRMLAbstractRootItemHelper::qMRMLAbstractRootItemHelper(vtkMRMLScene* scene, 
                                                         const qMRMLAbstractItemHelperFactory* itemFactory)
  :qMRMLAbstractItemHelper(-1, itemFactory)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractRootItemHelper);
  qctk_d()->MRMLScene = scene;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLAbstractRootItemHelper::child(int _row, int _column) const
{
  if (_row == 0)
    {
    //return qctk_d()->MRMLScene;
    return this->factory()->createItem(qctk_d()->MRMLScene, _column);
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractRootItemHelper::childCount() const
{
  return qctk_d()->MRMLScene ? 1 : 0;
}

//------------------------------------------------------------------------------
QVariant qMRMLAbstractRootItemHelper::data(int role) const
{
  Q_UNUSED(role);
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLAbstractRootItemHelper::flags() const
{
  Q_ASSERT(false);
  return 0;
}

//------------------------------------------------------------------------------
bool qMRMLAbstractRootItemHelper::hasChildren() const
{
  return qctk_d()->MRMLScene != 0 ? true : false;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLAbstractRootItemHelper::mrmlScene() const
{
  return qctk_d()->MRMLScene;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLAbstractRootItemHelper::object() const
{
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLAbstractRootItemHelper::parent() const
{
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLAbstractRootItemHelper::row() const
{
  //Q_ASSERT(false);
  return -1;
}

//------------------------------------------------------------------------------
int qMRMLAbstractRootItemHelper::childIndex(const qMRMLAbstractItemHelper* _child)const
{
  Q_UNUSED(_child);
  // we know for sure that child is a child of this, child is at index 0
  return 0;
}

//------------------------------------------------------------------------------
// qMRMLExtraItemHelper

//------------------------------------------------------------------------------
class qMRMLExtraItemHelperPrivate: public qCTKPrivate<qMRMLExtraItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLExtraItemHelper);
  vtkSmartPointer<vtkVariantArray> VariantArray;
  bool isSeparator()const;
};

//------------------------------------------------------------------------------
bool qMRMLExtraItemHelperPrivate::isSeparator()const
{
  const vtkVariant v = this->VariantArray->GetValue(1);
  return v.ToString() == "separator";
}

//------------------------------------------------------------------------------
qMRMLExtraItemHelper::qMRMLExtraItemHelper(vtkVariantArray* array, int column, const qMRMLAbstractItemHelperFactory* factory)
  :qMRMLAbstractItemHelper(column, factory)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractRootItemHelper);
  qctk_d()->VariantArray = array;
}

//------------------------------------------------------------------------------
QVariant qMRMLExtraItemHelper::data(int role) const
{
  QCTK_D(const qMRMLExtraItemHelper);
  switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      if (d->isSeparator())
        {
        break;
        }
    case qMRML::UIDRole:
      if (this->column() == 0)
        {
        const vtkVariant v = d->VariantArray->GetValue(1);
        return QString(v.ToString());
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
Qt::ItemFlags qMRMLExtraItemHelper::flags() const
{
  //QCTK_D(const qMRMLExtraItemHelper);
  return Qt::ItemIsEnabled;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLExtraItemHelper::parent() const
{
  //QCTK_D(const qMRMLExtraItemHelper);
  return this->factory()->createItem(this->mrmlScene(), 0);
}

//------------------------------------------------------------------------------
bool qMRMLExtraItemHelper::setData(const QVariant &value, int role)
{
  //QCTK_D(const qMRMLExtraItemHelper);
  return true;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLExtraItemHelper::object()const
{
  QCTK_D(const qMRMLExtraItemHelper);
  return d->VariantArray;
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLExtraItemHelper::mrmlScene()const
{
  QCTK_D(const qMRMLExtraItemHelper);
  const vtkVariant v = d->VariantArray->GetValue(0);
  vtkMRMLScene* mrmlScene = vtkMRMLScene::SafeDownCast(v.ToVTKObject());
  Q_ASSERT(mrmlScene);
  return mrmlScene;
}

//------------------------------------------------------------------------------
void qMRMLExtraItemHelper
::createExtraItemProperties(vtkVariantArray& properties,
                            vtkObject* itemParent, 
                            const vtkStdString& title)
{
  properties.SetNumberOfValues(2);
  
  if (itemParent == 0)
    {
    properties.SetValue(0, vtkVariant(0));
    }
  else
    {
    properties.SetValue(0, vtkVariant(itemParent));
    }
  properties.SetValue(1, vtkVariant(title));
}
