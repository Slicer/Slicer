
// Qt includes
#include <QStandardItemModel>

// CTK includes
#include <ctkTreeComboBox.h>

// qMRML includes
#include "qMRMLNodeTreeSelector.h"

// MRML includes
#include <vtkMRMLNode.h>

//-----------------------------------------------------------------------------
class qMRMLNodeTreeSelectorPrivate: public ctkPrivate<qMRMLNodeTreeSelector>
{
public:
  void addItemInternal(int index, const QIcon &icon, 
                       const QString &text, const QVariant &userData);
};

// --------------------------------------------------------------------------
qMRMLNodeTreeSelector::qMRMLNodeTreeSelector(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLNodeTreeSelector);
  
  ctkTreeComboBox* comboBox = new ctkTreeComboBox;
  this->setComboBox(comboBox);
}

// --------------------------------------------------------------------------
void qMRMLNodeTreeSelector::addNodeInternal(vtkMRMLNode* mrmlNode)
{
  CTK_D(qMRMLNodeTreeSelector);

  QString categoryName = QString::fromAscii(mrmlNode->GetAttribute("Category"));
  if (categoryName.isEmpty())
    {// normal node, add it as a top-level node
    this->qMRMLNodeSelector::addNodeInternal(mrmlNode);
    return;
    }
  QString categoryUID = QString("Category:") + categoryName;
  QModelIndex _rootModelIndex = this->rootModelIndex();
  // the node belongs to a category.
  // does the category exists or do we have to create it ?
  int categoryIndex = this->findData(categoryUID);
  QModelIndex categoryModelIndex;
  if (categoryIndex < 0)
    {// create a top-level category
    this->addItem(categoryName, categoryUID);
    categoryIndex = this->findData(categoryUID);
    categoryModelIndex = this->model()->index(categoryIndex, this->modelColumn(), _rootModelIndex);
    this->model()->insertColumns(0, 2, categoryModelIndex);
    QStandardItemModel* standardModel = qobject_cast<QStandardItemModel*>(this->model());
    if (standardModel)
      {
      QStandardItem* item = standardModel->itemFromIndex(categoryModelIndex);
      item->setSelectable(false);
      }
    }
  else
    {
    categoryModelIndex = this->model()->index(categoryIndex, this->modelColumn(), _rootModelIndex);
    }

  this->setRootModelIndex(categoryModelIndex);
  int index = this->count();
  // inserting the node through the combobox doesn't work as it doesn't
  // take into account the rootModeIndex. We have to do it manually
  //this->qMRMLNodeSelector::addNodeInternal(mrmlNode);
  d->addItemInternal(index, QIcon(),
                     QString::fromAscii(mrmlNode->GetName()), QString::fromAscii(mrmlNode->GetID()));
  this->setRootModelIndex(_rootModelIndex);
}

// --------------------------------------------------------------------------
// qMRMLNodeTreeSelectorPrivate methods

// --------------------------------------------------------------------------
void qMRMLNodeTreeSelectorPrivate::addItemInternal(int index, const QIcon &icon, 
                                                   const QString &text, const QVariant &userData)
{
  CTK_P(qMRMLNodeTreeSelector);
  
  int itemCount = p->count();
  index = qBound(0, index, itemCount);
  
  // if (index >= d->maxCount)
  //   {
  //   return;
  //   }
  
  //d->inserting = true;

  if (p->model()->insertRows(index, 1, p->rootModelIndex()))
    {
    QModelIndex item = p->model()->index(index, p->modelColumn(), p->rootModelIndex());
    if (icon.isNull() && !userData.isValid()) 
      {
      p->model()->setData(item, text, Qt::EditRole);
      } 
    else 
      {
      QMap<int, QVariant> values;
      if (!text.isNull()) values.insert(Qt::EditRole, text);
      if (!icon.isNull()) values.insert(Qt::DecorationRole, icon);
      if (userData.isValid()) values.insert(Qt::UserRole, userData);
      if (!values.isEmpty()) p->model()->setItemData(item, values);
      }
    //d->inserting = false;
    //d->_q_rowsInserted(d->root, index, index);
    ++itemCount;
    } 
  else 
    {
    //d->inserting = false;
    }
    
  // if (itemCount > d->maxCount)
  //   {
  //   d->model->removeRows(itemCount - 1, itemCount - d->maxCount, d->root);
  //   }
}
