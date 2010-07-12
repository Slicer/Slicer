// Qt includes
#include <QDebug>
#include <QIcon>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVariant>
#include <QVector>

// qMRML includes
#include "qMRMLItemHelper.h"
#include "qMRMLColorTableProxyModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

class qMRMLCategoryItemHelperPrivate;
class vtkCategory;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorTableItemHelperFactory : public qMRMLAbstractItemHelperFactory
{
public:
  qMRMLColorTableItemHelperFactory();
  virtual ~qMRMLColorTableItemHelperFactory();
  virtual qMRMLAbstractItemHelper* createItem(vtkObject* object, int column)const;

  void updateCategories(vtkMRMLScene* scene);
  vtkCategory* category(const QString& categoryName)const;
  vtkCollection* Categories;
protected:
  unsigned long MTime;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLCategorySceneItemHelper : public qMRMLAbstractSceneItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent()const;

protected:
  friend class qMRMLColorTableItemHelperFactory;
  qMRMLCategorySceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory);
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLCategoryItemHelper
  :public qMRMLAbstractItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual qMRMLAbstractItemHelper* parent() const;

protected:
  friend class qMRMLColorTableItemHelperFactory;
  qMRMLCategoryItemHelper(vtkCategory* category, int column,
                          const qMRMLAbstractItemHelperFactory* factory);
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;
  CTK_DECLARE_PRIVATE(qMRMLCategoryItemHelper);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorTableNodeItemHelper
  :public qMRMLAbstractNodeItemHelper
{
public:
  virtual QVariant data(int role)const;
  virtual qMRMLAbstractItemHelper* parent() const;
protected:
  friend class qMRMLColorTableItemHelperFactory;
  qMRMLColorTableNodeItemHelper(vtkMRMLNode* node, int column,
                                const qMRMLAbstractItemHelperFactory* factory);
};

//------------------------------------------------------------------------------
class vtkCategory: public vtkObject
{
public:
  static vtkCategory* New();
  vtkTypeMacro(vtkCategory, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkStdString Name;
  vtkMRMLScene* MRMLScene;
};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCategory);

//-----------------------------------------------------------------------------
void vtkCategory::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Name: " << this->Name;
}


// qMRMLColorTableItemHelperFactory
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLColorTableItemHelperFactory::qMRMLColorTableItemHelperFactory()
{
  this->Categories = vtkCollection::New();
  this->MTime = 0;
}

//------------------------------------------------------------------------------
qMRMLColorTableItemHelperFactory::~qMRMLColorTableItemHelperFactory()
{
  this->Categories->Delete();
  this->Categories = 0;
}

//------------------------------------------------------------------------------
void qMRMLColorTableItemHelperFactory::updateCategories(vtkMRMLScene* scene)
{
  if (scene->GetMTime() < this->MTime)
    {
    return;
    }
  this->Categories->RemoveAllItems();
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* nodeCollection = scene->GetCurrentScene();
  for (nodeCollection->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(nodeCollection->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") != 0)
      {
      vtkCategory* nodeCategory = vtkCategory::New();
      nodeCategory->Name = node->GetAttribute("Category");
      nodeCategory->MRMLScene = scene;
      this->Categories->AddItem(nodeCategory);
      nodeCategory->Delete();
      }
    }
  this->MTime = scene->GetMTime();
}

//------------------------------------------------------------------------------
vtkCategory* qMRMLColorTableItemHelperFactory::category(const QString& name)const
{
  vtkCollectionSimpleIterator it;
  vtkCategory* nodeCategory = 0;
  for(this->Categories->InitTraversal(it);
       (nodeCategory = vtkCategory::SafeDownCast(this->Categories->GetNextItemAsObject(it))) ;)
    {
    if (name == nodeCategory->Name)
      {
      break;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLColorTableItemHelperFactory
::createItem(vtkObject* object, int column)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLScene"))
    {
    return new qMRMLCategorySceneItemHelper(vtkMRMLScene::SafeDownCast(object), column, this);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLColorTableNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this);
    }
  else if (object->IsA("vtkCategory"))
    {
    return new qMRMLCategoryItemHelper(vtkCategory::SafeDownCast(object), column, this);
    }
  else
    {
    Q_ASSERT( false);
    }
  return 0;
}

// qMRMLCategorySceneItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

qMRMLCategorySceneItemHelper
::qMRMLCategorySceneItemHelper(vtkMRMLScene* scene, int itemColumn,
                               const qMRMLAbstractItemHelperFactory* factoryHelper)
  :qMRMLAbstractSceneItemHelper(scene, itemColumn, factoryHelper)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategorySceneItemHelper::child(int childRow, int childColumn) const
{
  const qMRMLColorTableItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLColorTableItemHelperFactory*>(this->factory());
  Q_ASSERT(colorFactory);
  const_cast<qMRMLColorTableItemHelperFactory*>(colorFactory)
    ->updateCategories(this->mrmlScene());
  if (childRow < colorFactory->Categories->GetNumberOfItems())
    {// categories are first in the tree
    return colorFactory->createItem(
      colorFactory->Categories->GetItemAsObject(childRow),childColumn);
    }
  // check for nodes without categories
  int index = -1;
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") != 0)
      {
      ++index;
      if (index == childRow)
        {
        break;
        }
      }
    }
  Q_ASSERT(node);
  return this->factory()->createItem(node,childColumn);
}

//------------------------------------------------------------------------------
int qMRMLCategorySceneItemHelper::childCount() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  const qMRMLColorTableItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLColorTableItemHelperFactory*>(this->factory());
  Q_ASSERT(colorFactory);
  const_cast<qMRMLColorTableItemHelperFactory*>(colorFactory)
    ->updateCategories(this->mrmlScene());
  int count = colorFactory->Categories->GetNumberOfItems();
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") != 0)
      {
      ++count;
      }
    }
  return count;
}

//------------------------------------------------------------------------------
int qMRMLCategorySceneItemHelper::childIndex(const qMRMLAbstractItemHelper* childItem) const
{
  const qMRMLColorTableNodeItemHelper* nodeItem =
    dynamic_cast<const qMRMLColorTableNodeItemHelper*>(childItem);
  const qMRMLCategoryItemHelper* categoryItem =
    dynamic_cast<const qMRMLCategoryItemHelper*>(childItem);
  if (!nodeItem && !categoryItem)
    {
    return -1;
    }
  const qMRMLColorTableItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLColorTableItemHelperFactory*>(this->factory());
  const_cast<qMRMLColorTableItemHelperFactory*>(colorFactory)
    ->updateCategories(this->mrmlScene());
  if (categoryItem)
    {
    Q_ASSERT(colorFactory->Categories->IsItemPresent(categoryItem->object()));
    return colorFactory->Categories->IsItemPresent(categoryItem->object()) - 1;
    }
  int index = -1;
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") != 0)
      {
      ++index;
      if (node == nodeItem->mrmlNode())
        {
        break;
        }
      }
    }
  return colorFactory->Categories->GetNumberOfItems() + index;
}

//------------------------------------------------------------------------------
bool qMRMLCategorySceneItemHelper::hasChildren() const
{
  return this->mrmlScene() ? this->mrmlScene()->GetNumberOfNodes() > 0 : false;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategorySceneItemHelper::parent() const
{
  return this->factory()->createRootItem(this->mrmlScene());
}

// qMRMLCategoryItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class qMRMLCategoryItemHelperPrivate: public ctkPrivate<qMRMLCategoryItemHelper>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLCategoryItemHelper);
  vtkCategory* Category;
};

//------------------------------------------------------------------------------
qMRMLCategoryItemHelper
::qMRMLCategoryItemHelper(vtkCategory* category,
                          int itemColumn,
                          const qMRMLAbstractItemHelperFactory* helperFactory)
  :qMRMLAbstractItemHelper(itemColumn, helperFactory)
{
  CTK_INIT_PRIVATE(qMRMLCategoryItemHelper);
  Q_ASSERT(category);
  CTK_D(qMRMLCategoryItemHelper);
  d->Category = category;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategoryItemHelper
::child(int itemRow, int itemColumn) const
{
  CTK_D(const qMRMLCategoryItemHelper);
  vtkMRMLNode* node = 0;
  int index = -1;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = d->Category->MRMLScene->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (!d->Category->Name.compare(node->GetAttribute("Category")))
      {
      ++index;
      if (index == itemRow)
        {
        break;
        }
      }
    }
  Q_ASSERT(node);
  return this->factory()->createItem(node, itemColumn);
}

//------------------------------------------------------------------------------
int qMRMLCategoryItemHelper::childCount() const
{
  CTK_D(const qMRMLCategoryItemHelper);
  if (this->column() != 0)
    {
    return 0;
    }
  vtkMRMLNode* node = 0;
  int count = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = d->Category->MRMLScene->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (!d->Category->Name.compare(node->GetAttribute("Category")))
      {
      ++count;
      }
    }
  return count;
}

//------------------------------------------------------------------------------
int qMRMLCategoryItemHelper::childIndex(const qMRMLAbstractItemHelper* childItem) const
{
  CTK_D(const qMRMLCategoryItemHelper);
  const qMRMLAbstractNodeItemHelper* nodeItemHelper =
    dynamic_cast<const qMRMLAbstractNodeItemHelper*>(childItem);
  Q_ASSERT(nodeItemHelper);
  if (nodeItemHelper == 0)
    {
    return -1;
    }
  vtkMRMLNode* childNode = nodeItemHelper->mrmlNode();
  Q_ASSERT(childNode);
  vtkMRMLNode* node = 0;
  int index = -1;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = d->Category->MRMLScene->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (!d->Category->Name.compare(node->GetAttribute("Category")))
      {
      ++index;
      if (childNode == node)
        {
        break;
        }
      }
    }
  return index;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLCategoryItemHelper::flags() const
{
  return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

//------------------------------------------------------------------------------
bool qMRMLCategoryItemHelper::hasChildren() const
{
  // typically if a category is created, that means it has children.
  return true;
}

//------------------------------------------------------------------------------
vtkObject* qMRMLCategoryItemHelper::object() const
{
  CTK_D(const qMRMLCategoryItemHelper);
  return d->Category;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategoryItemHelper::parent() const
{
  CTK_D(const qMRMLCategoryItemHelper);
  return this->factory()->createItem(d->Category->MRMLScene, 0);
}

// qMRMLColorTableNodeItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLColorTableNodeItemHelper
::qMRMLColorTableNodeItemHelper(vtkMRMLNode* node,
                                int itemColumn,
                                const qMRMLAbstractItemHelperFactory* _factory)
  :qMRMLAbstractNodeItemHelper(node, itemColumn, _factory)
{
  Q_ASSERT(node);
}

//------------------------------------------------------------------------------
QVariant qMRMLColorTableNodeItemHelper::data(int role)const
{
  if (role == Qt::DecorationRole)
    {
    return QIcon("VisibleOn.png");
    }
  return qMRMLAbstractNodeItemHelper::data(role);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLColorTableNodeItemHelper::parent() const
{
  QString categoryTitle(this->mrmlNode()->GetAttribute("Category"));
  if (categoryTitle.isEmpty())
    {
    return this->factory()->createItem(this->mrmlNode()->GetScene(), 0);
    }
  const qMRMLColorTableItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLColorTableItemHelperFactory*>(this->factory());
  return this->factory()->createItem(colorFactory->category(categoryTitle), 0);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class qMRMLColorTableProxyModelPrivate: public ctkPrivate<qMRMLColorTableProxyModel>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLColorTableProxyModel);
  qMRMLColorTableProxyModelPrivate();
  virtual ~qMRMLColorTableProxyModelPrivate();
  qMRMLColorTableItemHelperFactory* ItemFactory;
};

//------------------------------------------------------------------------------
qMRMLColorTableProxyModelPrivate::qMRMLColorTableProxyModelPrivate()
{
  this->ItemFactory = new qMRMLColorTableItemHelperFactory;
}

//------------------------------------------------------------------------------
qMRMLColorTableProxyModelPrivate::~qMRMLColorTableProxyModelPrivate()
{
  delete this->ItemFactory;
  this->ItemFactory = 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLColorTableProxyModel::qMRMLColorTableProxyModel(QObject *vparent)
  :qMRMLTreeProxyModel(vparent)
{
  CTK_INIT_PRIVATE(qMRMLColorTableProxyModel);
}

//------------------------------------------------------------------------------
qMRMLColorTableProxyModel::~qMRMLColorTableProxyModel()
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelperFactory* qMRMLColorTableProxyModel::itemFactory()const
{
  CTK_D(const qMRMLColorTableProxyModel);
  return d->ItemFactory;
}
