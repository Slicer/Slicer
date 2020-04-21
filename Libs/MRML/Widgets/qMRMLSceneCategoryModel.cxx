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
#include "qMRMLSceneCategoryModel.h"
#include "qMRMLSceneModel_p.h"

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes

//------------------------------------------------------------------------------
class qMRMLSceneCategoryModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneCategoryModel);
public:
  qMRMLSceneCategoryModelPrivate(qMRMLSceneCategoryModel& object);

};

//------------------------------------------------------------------------------
qMRMLSceneCategoryModelPrivate
::qMRMLSceneCategoryModelPrivate(qMRMLSceneCategoryModel& object)
  : qMRMLSceneModelPrivate(object)
{

}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneCategoryModel::qMRMLSceneCategoryModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneCategoryModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneCategoryModel::~qMRMLSceneCategoryModel() = default;

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneCategoryModel::itemFromCategory(const QString& category)const
{
  if (category.isEmpty())
    {
    return this->mrmlSceneItem();
    }
  // doesn't search category items recursively.
  // options to optimize, categories are continuous and are the first children
  // of the mrmlSceneItem
  int rowCount = this->mrmlSceneItem()->rowCount();
  for (int i = 0; i < rowCount; ++i)
    {
    QStandardItem* child = this->mrmlSceneItem()->child(i,0);
    if (child &&
        child->data(qMRMLSceneModel::UIDRole).toString() == "category" &&
        child->text() == category)
      {
      return child;
      }
    }
  return this->mrmlSceneItem();
}

//------------------------------------------------------------------------------
int qMRMLSceneCategoryModel::categoryCount()const
{
  return this->match(this->mrmlSceneIndex().child(0,0),
                     qMRMLSceneModel::UIDRole,
                     QString("category"),
                     -1,
                     Qt::MatchExactly)
    .size();
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneCategoryModel::insertCategory(const QString& category, int row)
{
  Q_ASSERT(!category.isEmpty());

  QList<QStandardItem*> categoryItems;
  categoryItems << new QStandardItem;
  this->updateItemFromCategory(categoryItems[0], category);
  categoryItems << new QStandardItem;
  categoryItems[1]->setFlags(nullptr);

  this->mrmlSceneItem()->insertRow(row, categoryItems);
  Q_ASSERT(this->mrmlSceneItem()->columnCount() == 2);
  return categoryItems[0];
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLSceneCategoryModel::insertNode(vtkMRMLNode* node)
{
  QStandardItem* nodeItem = this->itemFromNode(node);
  if (nodeItem)
    {
    return nodeItem;
    }
  // WARNING: works only if the nodes are in the scene in the correct order:
  // parents are before children
  QString category = QString(node->GetAttribute("Category"));
  QStandardItem* parentItem = this->itemFromCategory(category);
  Q_ASSERT(parentItem);
  if (!category.isEmpty() && parentItem == this->mrmlSceneItem())
    {
    parentItem = this->insertCategory(category,
                                      this->preItems(parentItem).count()
                                      + this->categoryCount());
    }
  //int min = this->preItems(parentItem).count();
  int max = parentItem->rowCount() - this->postItems(parentItem).count();
  nodeItem = this->insertNode(node, parentItem, max);
  return nodeItem;
}

//------------------------------------------------------------------------------
bool qMRMLSceneCategoryModel::isANode(const QStandardItem * item)const
{
  return this->qMRMLSceneModel::isANode(item)
    && item->data(qMRMLSceneModel::UIDRole).toString() != "category";
}

//------------------------------------------------------------------------------
void qMRMLSceneCategoryModel::updateItemFromCategory(QStandardItem* item, const QString& category)
{
  item->setData(QString("category"), qMRMLSceneModel::UIDRole);
  item->setFlags(Qt::ItemIsEnabled);
  item->setText(category);
}

//------------------------------------------------------------------------------
void qMRMLSceneCategoryModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  QStandardItem* parentItem = item->parent();
  QString category = QString(node->GetAttribute("Category"));
  QStandardItem* newParentItem = this->itemFromCategory(category);
  // if the item has no parent, then it means it hasn't been put into the scene yet.
  // and it will do it automatically.
  if (parentItem != nullptr && (parentItem != newParentItem))
    {
    QList<QStandardItem*> children = parentItem->takeRow(item->row());
    //int min = this->preItems(newParentItem).count();
    int max = newParentItem->rowCount() - this->postItems(newParentItem).count();
    int pos = max;
    newParentItem->insertRow(pos, children);
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneCategoryModel::updateNodeFromItem(vtkMRMLNode* node, QStandardItem* item)
{
  this->qMRMLSceneModel::updateNodeFromItem(node, item);
  Q_ASSERT(node != this->mrmlNodeFromItem(item->parent()));

  // Don't do the following if the row is not complete (reparenting an
  // incomplete row might lead to errors). updateNodeFromItem is typically
  // called for every item changed, so it should be
  QStandardItem* parentItem = item->parent();
  for (int i = 0; i < parentItem->columnCount(); ++i)
    {
    if (parentItem->child(item->row(), i) == nullptr)
      {
      return;
      }
    }
  QString category =
    (parentItem != this->mrmlSceneItem()) ? parentItem->text() : QString();
  // If the attribute has never been set, don't set it with an empty string.
  if (!(node->GetAttribute("Category") == nullptr &&
        category.isEmpty()))
    {
    node->SetAttribute("Category", category.toUtf8());
    }
}
