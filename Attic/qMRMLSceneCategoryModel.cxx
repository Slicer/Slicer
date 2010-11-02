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
#include "qMRMLSceneCategoryModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkTimeStamp.h>

class qMRMLCategoryItemHelperPrivate;

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLCategorySceneItemHelper : public qMRMLAbstractSceneItemHelper
{
public:
  virtual qMRMLAbstractItemHelper* child(int row, int column) const;
  virtual int childCount() const;
  virtual bool hasChildren() const;
  virtual qMRMLAbstractItemHelper* parent()const;

protected:
  friend class qMRMLCategoryItemHelperFactory;
  qMRMLCategorySceneItemHelper(vtkMRMLScene* scene, int column, const qMRMLAbstractItemHelperFactory* factory, int row);
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
  virtual QVariant data(int role)const;
  virtual Qt::ItemFlags flags() const;
  virtual bool hasChildren() const;
  virtual vtkObject* object()const;
  virtual qMRMLAbstractItemHelper* parent() const;

  vtkCategory* category()const;
protected:
  friend class qMRMLCategoryItemHelperFactory;
  qMRMLCategoryItemHelper(vtkCategory* category, int column,
                          const qMRMLAbstractItemHelperFactory* factory, int row);
  // here we know for sure that child is a child of this.
  virtual int childIndex(const qMRMLAbstractItemHelper* child)const;

  QScopedPointer<qMRMLCategoryItemHelperPrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qMRMLCategoryItemHelper);
  Q_DISABLE_COPY(qMRMLCategoryItemHelper);
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


// qMRMLCategoryItemHelperFactory
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLCategoryItemHelperFactory::qMRMLCategoryItemHelperFactory()
{
  this->Categories = vtkCollection::New();
  this->UpdateTime = vtkTimeStamp::New();
}

//------------------------------------------------------------------------------
qMRMLCategoryItemHelperFactory::~qMRMLCategoryItemHelperFactory()
{
  this->Categories->Delete();
  this->Categories = 0;
  this->UpdateTime->Delete();
  this->UpdateTime = 0;
}

//------------------------------------------------------------------------------
void qMRMLCategoryItemHelperFactory::updateCategories(vtkMRMLScene* scene)
{
  if (scene->GetMTime() <= this->UpdateTime->GetMTime())
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
    const char* category = node->GetAttribute("Category");
    if (category != 0 && this->categoryIndex(category) < 0)
      {
      vtkCategory* nodeCategory = vtkCategory::New();
      nodeCategory->Name = category;
      nodeCategory->MRMLScene = scene;
      this->Categories->AddItem(nodeCategory);
      nodeCategory->Delete();
      }
    }
  this->UpdateTime->Modified();
}

//------------------------------------------------------------------------------
vtkCategory* qMRMLCategoryItemHelperFactory::category(const QString& name)const
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
  Q_ASSERT(nodeCategory); // not found ?
  return nodeCategory;
}

//------------------------------------------------------------------------------
int qMRMLCategoryItemHelperFactory::categoryIndex(const QString& name)const
{
  // searching on the pointer is fast but updateCategories might create new
  // categories, search a similar content
  int index = 0;
  vtkCategory* category = 0;
  vtkCollectionSimpleIterator it;
  for (this->Categories->InitTraversal(it);
       (category = vtkCategory::SafeDownCast(this->Categories->GetNextItemAsObject(it)));
       ++index)
    {
    if (name == category->Name)
      {
      return index;
      }
    }
  return -1;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategoryItemHelperFactory
::createItem(vtkObject* object, int column, int row)const
{
  if (!object)
    {
    Q_ASSERT(object);
    return 0;
    }
  if (object->IsA("vtkMRMLScene"))
    {
     qMRMLAbstractItemHelper* scene =
       new qMRMLCategorySceneItemHelper(vtkMRMLScene::SafeDownCast(object), column, this, 0);
     return new qMRMLExtraItemsHelper(this->preItems(), this->postItems(), scene);
    }
  else if (object->IsA("vtkMRMLNode"))
    {
    return new qMRMLCategoryNodeItemHelper(vtkMRMLNode::SafeDownCast(object), column, this, row);
    }
  else if (object->IsA("vtkCategory"))
    {
    return new qMRMLCategoryItemHelper(vtkCategory::SafeDownCast(object), column, this, row);
    }
  else
    {
    return this->qMRMLSceneModelItemHelperFactory::createItem(object, column, row);
    }
  return 0;
}

// qMRMLCategorySceneItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

qMRMLCategorySceneItemHelper
::qMRMLCategorySceneItemHelper(vtkMRMLScene* scene, int itemColumn,
                               const qMRMLAbstractItemHelperFactory* factoryHelper, int itemRow)
  :qMRMLAbstractSceneItemHelper(scene, itemColumn, factoryHelper, itemRow)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategorySceneItemHelper::child(int childRow, int childColumn) const
{
  const qMRMLCategoryItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLCategoryItemHelperFactory*>(this->factory());
  Q_ASSERT(colorFactory);
  const_cast<qMRMLCategoryItemHelperFactory*>(colorFactory)
    ->updateCategories(this->mrmlScene());
  if (childRow < colorFactory->Categories->GetNumberOfItems())
    {// categories are first in the tree
    return colorFactory->createItem(
      colorFactory->Categories->GetItemAsObject(childRow),childColumn, childRow);
    }
  // check for nodes without categories
  int index = colorFactory->Categories->GetNumberOfItems() - 1;
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") == 0)
      {
      ++index;
      if (index == childRow)
        {
        break;
        }
      }
    }
  Q_ASSERT(node);
  return this->factory()->createItem(node,childColumn, childRow);
}

//------------------------------------------------------------------------------
int qMRMLCategorySceneItemHelper::childCount() const
{
  if (this->column() != 0)
    {
    return 0;
    }
  const qMRMLCategoryItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLCategoryItemHelperFactory*>(this->factory());
  Q_ASSERT(colorFactory);
  const_cast<qMRMLCategoryItemHelperFactory*>(colorFactory)
    ->updateCategories(this->mrmlScene());
  int count = colorFactory->Categories->GetNumberOfItems();
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") == 0)
      {
      ++count;
      }
    }
  return count;
}

//------------------------------------------------------------------------------
int qMRMLCategorySceneItemHelper::childIndex(const qMRMLAbstractItemHelper* childItem) const
{
  const qMRMLCategoryNodeItemHelper* nodeItem =
    dynamic_cast<const qMRMLCategoryNodeItemHelper*>(childItem);
  const qMRMLCategoryItemHelper* categoryItem =
    dynamic_cast<const qMRMLCategoryItemHelper*>(childItem);
  if (!nodeItem && !categoryItem)
    {
    return -1;
    }
  const qMRMLCategoryItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLCategoryItemHelperFactory*>(this->factory());
  const_cast<qMRMLCategoryItemHelperFactory*>(colorFactory)
    ->updateCategories(this->mrmlScene());
  int index = -1;
  if (categoryItem)
    {
    return colorFactory->categoryIndex(QString(categoryItem->category()->Name));
    }
  vtkMRMLNode* node = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    if (node->GetAttribute("Category") == 0)
      {
      ++index;
      if (node == nodeItem->mrmlNode())
        {
        break;
        }
      }
    }
  Q_ASSERT(index >= 0);
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
class qMRMLCategoryItemHelperPrivate
{
public:
  vtkCategory* Category;
};

//------------------------------------------------------------------------------
qMRMLCategoryItemHelper
::qMRMLCategoryItemHelper(vtkCategory* category,
                          int itemColumn,
                          const qMRMLAbstractItemHelperFactory* helperFactory, int itemRow)
  :qMRMLAbstractItemHelper(itemColumn, helperFactory, itemRow)
  , d_ptr(new qMRMLCategoryItemHelperPrivate)
{
  Q_ASSERT(category);
  Q_D(qMRMLCategoryItemHelper);
  d->Category = category;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategoryItemHelper
::child(int itemRow, int itemColumn) const
{
  Q_D(const qMRMLCategoryItemHelper);
  vtkMRMLNode* node = 0;
  int index = -1;
  vtkCollectionSimpleIterator it;
  vtkCollection* scene = d->Category->MRMLScene->GetCurrentScene();
  for (scene->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(scene->GetNextItemAsObject(it))) ;)
    {
    const char* category = node->GetAttribute("Category");
    if (category && !d->Category->Name.compare(category))
      {
      ++index;
      if (index == itemRow)
        {
        break;
        }
      }
    }
  Q_ASSERT(node);
  return this->factory()->createItem(node, itemColumn, itemRow);
}

//------------------------------------------------------------------------------
int qMRMLCategoryItemHelper::childCount() const
{
  Q_D(const qMRMLCategoryItemHelper);
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
    const char* category = node->GetAttribute("Category");
    if (category && !d->Category->Name.compare(category))
      {
      ++count;
      }
    }
  return count;
}

//------------------------------------------------------------------------------
int qMRMLCategoryItemHelper::childIndex(const qMRMLAbstractItemHelper* childItem) const
{
  Q_D(const qMRMLCategoryItemHelper);
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
    const char* category = node->GetAttribute("Category");
    if (category && !d->Category->Name.compare(category))
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
QVariant qMRMLCategoryItemHelper::data(int role) const
{
  Q_D(const qMRMLCategoryItemHelper);
  switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return QString(d->Category->Name);
      break;
    default:
      return QVariant();
    };
  return QVariant();
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLCategoryItemHelper::flags() const
{
  // we don't yet support drag/drop...
  return Qt::ItemIsEnabled;// | Qt::ItemIsDropEnabled;
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
  Q_D(const qMRMLCategoryItemHelper);
  return d->Category;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategoryItemHelper::parent() const
{
  Q_D(const qMRMLCategoryItemHelper);
  return this->factory()->createItem(d->Category->MRMLScene, 0);
}

//------------------------------------------------------------------------------
vtkCategory* qMRMLCategoryItemHelper::category() const
{
  Q_D(const qMRMLCategoryItemHelper);
  return d->Category;
}


// qMRMLCategoryNodeItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLCategoryNodeItemHelper
::qMRMLCategoryNodeItemHelper(vtkMRMLNode* node,
                              int itemColumn,
                              const qMRMLAbstractItemHelperFactory* _factory,
                              int itemRow)
  :qMRMLAbstractNodeItemHelper(node, itemColumn, _factory, itemRow)
{
  Q_ASSERT(node);
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLCategoryNodeItemHelper::parent() const
{
  QString categoryTitle(this->mrmlNode()->GetAttribute("Category"));
  if (categoryTitle.isEmpty())
    {
    return this->factory()->createItem(this->mrmlNode()->GetScene(), 0);
    }
  const qMRMLCategoryItemHelperFactory* colorFactory =
    dynamic_cast<const qMRMLCategoryItemHelperFactory*>(this->factory());
  return this->factory()->createItem(colorFactory->category(categoryTitle), 0);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneCategoryModel::qMRMLSceneCategoryModel(QObject *vparent)
  :qMRMLSceneTreeModel(new qMRMLCategoryItemHelperFactory, vparent)
{
}
