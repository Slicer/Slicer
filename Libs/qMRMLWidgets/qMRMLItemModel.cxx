#include "qMRMLItemModel.h"
#include "qMRMLUtils.h"
#include <vtkMRMLScene.h>
#include <QDebug>
#include <QMimeData>
#include <QStringList>

//------------------------------------------------------------------------------
class qMRMLItemModelPrivate: public qCTKPrivate<qMRMLItemModel>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLItemModel);
  void init();
  
  vtkMRMLScene* MRMLScene;
};

//------------------------------------------------------------------------------
void qMRMLItemModelPrivate::init()
{
  this->MRMLScene = 0;
}

//------------------------------------------------------------------------------
qMRMLItemModel::qMRMLItemModel(QObject *parent)
  :QAbstractItemModel(parent)
{
  QCTK_INIT_PRIVATE(qMRMLItemModel);
  qctk_d()->init();
}

//------------------------------------------------------------------------------
qMRMLItemModel::~qMRMLItemModel()
{
}

//------------------------------------------------------------------------------
void qMRMLItemModel::setMRMLScene(vtkMRMLScene* scene)
{
  qctk_d()->MRMLScene = scene;
  this->reset();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLItemModel::mrmlScene()const
{
  return qctk_d()->MRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLItemModel::mrmlNodeFromIndex(const QModelIndex &index)const
{
  if (!index.isValid())
    {
    return 0;
    }
  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(index.internalPointer());
  Q_ASSERT(node);
  return node;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLItemModel::indexFromMRMLNode(vtkMRMLNode* node, int column)const
{
  if (!node)
    {
    return QModelIndex();
    }
  return this->createIndex(qMRMLUtils::nodeIndex(node), column, node);
}

//------------------------------------------------------------------------------
int qMRMLItemModel::columnCount(const QModelIndex &parent)const
{
  return 2;
}

//------------------------------------------------------------------------------
QVariant qMRMLItemModel::data(const QModelIndex &index, int role)const
{
  Q_ASSERT(qctk_d()->MRMLScene);
  vtkMRMLNode* node = this->mrmlNodeFromIndex(index);
  Q_ASSERT(node);
  QVariant data;
  switch (role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
      switch(index.column())
        {
        default:
        case 0:
          data = QString(node->GetName());
          break;
        case 1:
          data = QString(node->GetID());
          break;
        }
      break;
    default:
      break;
    }
  return data;
}

//------------------------------------------------------------------------------
bool qMRMLItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if (action == Qt::IgnoreAction)
    {
    return true;
    }
  if (action != Qt::MoveAction)
    {
    return false;
    }
  
  if (!data->hasFormat("application/vnd.text.list"))
    {
    return false;
    }

  if (column > 0)
    {
    return false;
    }
   int beginRow;
   if (row != -1)
     {
     beginRow = row;
     } 
   else
     {
     beginRow = this->rowCount(parent);
     }

   QByteArray encodedData = data->data("application/vnd.text.list");
   QDataStream stream(&encodedData, QIODevice::ReadOnly);
   QStringList newItems;
   
   while (!stream.atEnd()) 
     {
     QString text;
     stream >> text;
     newItems << text;
     }
   emit layoutAboutToBeChanged();
   bool res = false;
   foreach (QString id, newItems)
     {
     vtkMRMLNode* parentNode = this->mrmlNodeFromIndex(parent);
     vtkMRMLNode* node = qctk_d()->MRMLScene->GetNodeByID(id.toLatin1().data());
     QModelIndex oldIdx = this->indexFromMRMLNode(node);
     res = qMRMLUtils::reparent(node, parentNode) || res;
     beginRow++;
     }
   
   emit layoutChanged();
   return res;
}

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLItemModel::flags(const QModelIndex &index)const
{
  vtkMRMLNode* node = this->mrmlNodeFromIndex(index);
  if (node == 0)
    {
    return 0;
    }
  Qt::ItemFlags f;
  if (!node->GetHideFromEditors())
    {
    f |= Qt::ItemIsEnabled;
    }
  if (node->GetSelectable())
    {
    f |= Qt::ItemIsSelectable;
    }
  if (qMRMLUtils::canBeAChild(node))
    {
    f |= Qt::ItemIsDragEnabled;
    }
  if (index.column() == 0 && qMRMLUtils::canBeAParent(node))
    {
    f |= Qt::ItemIsDropEnabled;
    }
  if (index.column() == 0)
    {
    f |= Qt::ItemIsEditable;
    }
  return f;
}

// Has to be reimplemented for speed issues
//------------------------------------------------------------------------------
bool qMRMLItemModel::hasChildren(const QModelIndex &parent)const
{
  vtkMRMLNode* node = this->mrmlNodeFromIndex(parent);
  if (!node)
    {
    QCTK_D(const qMRMLItemModel);
    return d->MRMLScene ? d->MRMLScene->GetNumberOfNodes() > 0 : false;
    }
  // canBeAParent() is faster to compute than childNode().
  return parent.column() == 0 && 
    qMRMLUtils::canBeAParent(node) && 
    (qMRMLUtils::childNode(node) != 0);
}

//------------------------------------------------------------------------------
QVariant qMRMLItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical || role != Qt::DisplayRole)
    {
    return QVariant();
    }
  switch (section)
    {
    case 0:
      return tr("Name");
      break;
    case 1:
      return tr("Id");
      break;
    default:
      break;
    };

  return QVariant();
}

//------------------------------------------------------------------------------
QModelIndex qMRMLItemModel::index(int row, int column, const QModelIndex &parent)const
{
  QCTK_D(const qMRMLItemModel);
  vtkMRMLNode* parentNode = this->mrmlNodeFromIndex(parent);
  vtkMRMLNode* node = 0;
  if (parentNode == 0)
    {
    node = qMRMLUtils::topLevelNthNode(d->MRMLScene, row);
    }
  else
    {
    node = qMRMLUtils::childNode(parentNode, row);
    }
  if (node == 0)
    {
    return QModelIndex();
    }
  return this->createIndex(row, column, node);
}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//}

//------------------------------------------------------------------------------
//QMap<int, QVariant> itemData(const QModelIndex &index)const
//{
//}

//------------------------------------------------------------------------------
QMimeData *qMRMLItemModel::mimeData(const QModelIndexList &indexes)const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  foreach (QModelIndex index, indexes) 
    {
    vtkMRMLNode* node = this->mrmlNodeFromIndex(index);
    if (node && index.column() == 0)
      {
      QString text = node->GetID();
      stream << text;
      }
    }
  mimeData->setData("application/vnd.text.list", encodedData);
  return mimeData;
}

//------------------------------------------------------------------------------
QStringList qMRMLItemModel::mimeTypes()const
{
  // @todo: use a different mimetype. I think it exists one for qabstractitemmodel...
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

//------------------------------------------------------------------------------
QModelIndex qMRMLItemModel::parent(const QModelIndex &index)const
{
  if (!index.isValid())
    {
    return QModelIndex();
    }
  vtkMRMLNode* node = this->mrmlNodeFromIndex(index);
  Q_ASSERT(node);
  vtkMRMLNode* parentNode = qMRMLUtils::parentNode(node);
  QModelIndex parent =  this->indexFromMRMLNode(parentNode, 0);
  return parent;
}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::removeColumns(int column, int count, const QModelIndex &parent=QModelIndex())
//{
//}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//}

//------------------------------------------------------------------------------
int qMRMLItemModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    {
    return qMRMLUtils::childCount(qctk_d()->MRMLScene);
    }
  if (parent.column() != 0)
    {
    return 0;
    }
  return qMRMLUtils::childCount(this->mrmlNodeFromIndex(parent));
}

//------------------------------------------------------------------------------
bool qMRMLItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Q_ASSERT(qctk_d()->MRMLScene);
  vtkMRMLNode* node = this->mrmlNodeFromIndex(index);
  if (!node || index.column() != 0)
    {
    return false;
    }
  bool changed = false;
  switch (role)
    {
    case Qt::EditRole:
      node->SetName(value.toString().toAscii().data());
      changed = true;
      break;
    case Qt::DisplayRole:
      break;
    default:
      break;
    }
  if (changed)
    {
    emit dataChanged(index, index);
    }
  return changed;
}

//------------------------------------------------------------------------------
//bool qMRMLItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
//{
//}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLItemModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
