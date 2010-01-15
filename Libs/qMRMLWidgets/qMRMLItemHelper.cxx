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
qMRMLAbstractSceneItemHelper::qMRMLAbstractSceneItemHelper(vtkMRMLScene* scene, int column)
  :qMRMLAbstractItemHelper(column)
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
qMRMLAbstractItemHelper* qMRMLAbstractSceneItemHelper::parent() const
{
  QCTK_D(const qMRMLAbstractSceneItemHelper);
  return new qMRMLRootItemHelper(d->MRMLScene);
}




// qMRMLSceneItemHelper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
qMRMLSceneItemHelper::qMRMLSceneItemHelper(vtkMRMLScene* scene, int column)
  :qMRMLAbstractSceneItemHelper(scene, column)
{
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLSceneItemHelper::child(int row, int column) const
{
  vtkMRMLNode* childNode = qMRMLUtils::topLevelNthNode(this->mrmlScene(), row);
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
  return qMRMLUtils::childCount(this->mrmlScene());
}

//------------------------------------------------------------------------------
int qMRMLSceneItemHelper::childIndex(const qMRMLAbstractItemHelper* child) const
{
  const qMRMLAbstractNodeItemHelper* nodeItemHelper = 
    dynamic_cast<const qMRMLAbstractNodeItemHelper*>(child);
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







// qMRMLAbstractNodeItemHelper
//------------------------------------------------------------------------------
class qMRMLAbstractNodeItemHelperPrivate: public qCTKPrivate<qMRMLAbstractNodeItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLAbstractNodeItemHelper); 
  vtkMRMLNode* MRMLNode;
};

//------------------------------------------------------------------------------
qMRMLAbstractNodeItemHelper::qMRMLAbstractNodeItemHelper(vtkMRMLNode* node, int column)
  :qMRMLAbstractItemHelper(column)
{
  QCTK_INIT_PRIVATE(qMRMLAbstractNodeItemHelper);
  Q_ASSERT(node);
  qctk_d()->MRMLNode = node;
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
#include <iostream>
#include <fstream>

//------------------------------------------------------------------------------
Qt::ItemFlags qMRMLAbstractNodeItemHelper::flags() const
{
  QCTK_D(const qMRMLAbstractNodeItemHelper);
  Qt::ItemFlags f;
  if (!d->MRMLNode->GetHideFromEditors())
    {
    f |= Qt::ItemIsEnabled;
    std::ofstream toto ("flags.txt", std::ios_base::app);
    toto << d->MRMLNode->GetName() << "don't hide from editors" << std::endl;
    toto << std::endl;
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
qMRMLAbstractItemHelper* qMRMLAbstractNodeItemHelper::parent() const
{
  QCTK_D(const qMRMLAbstractNodeItemHelper);
  return new qMRMLSceneItemHelper(d->MRMLNode->GetScene(), 0);
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



// qMRMLNodeItemHelper
//------------------------------------------------------------------------------
class qMRMLNodeItemHelperPrivate: public qCTKPrivate<qMRMLNodeItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLNodeItemHelper);
};

//------------------------------------------------------------------------------
qMRMLNodeItemHelper::qMRMLNodeItemHelper(vtkMRMLNode* node, int column)
  :qMRMLAbstractNodeItemHelper(node, column)
{
  QCTK_INIT_PRIVATE(qMRMLNodeItemHelper);
  Q_ASSERT(node);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::canReparent(qMRMLAbstractItemHelper* newParent)const
{  
  //QCTK_D(const qMRMLNodeItemHelper);
  if (this->qMRMLAbstractNodeItemHelper::canReparent(newParent))
    {
    return true;
    }
  bool canReparent = false;
  if (dynamic_cast<qMRMLAbstractSceneItemHelper*>(newParent))
    {
    canReparent = qMRMLUtils::canReparent(this->mrmlNode(), 0);
    }
  else if (dynamic_cast<qMRMLNodeItemHelper*>(newParent))
    {
    vtkMRMLNode*  newParentNode = dynamic_cast<qMRMLNodeItemHelper*>(newParent)->mrmlNode();
    canReparent = qMRMLUtils::canReparent(this->mrmlNode(), newParentNode);
    }
  return canReparent;
}

//------------------------------------------------------------------------------
qMRMLAbstractItemHelper* qMRMLNodeItemHelper::child(int row, int column) const
{
  vtkMRMLNode* childNode = qMRMLUtils::childNode(this->mrmlNode(), row);
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
  return qMRMLUtils::childCount(this->mrmlNode());
}

//------------------------------------------------------------------------------
int qMRMLNodeItemHelper::childIndex(const qMRMLAbstractItemHelper* child) const
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
        std::ofstream toto ("flags.txt", std::ios_base::app);
    toto << this->mrmlNode()->GetName() << "use nodeitemhelper..." << std::endl;
    toto << std::endl;
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
  //QCTK_D(const qMRMLNodeItemHelper);
  vtkMRMLNode* parentNode = qMRMLUtils::parentNode(this->mrmlNode());
  if (parentNode == 0)
    {
    return this->qMRMLAbstractNodeItemHelper::parent();
    }
  return new qMRMLNodeItemHelper(parentNode, 0);
}

//------------------------------------------------------------------------------
bool qMRMLNodeItemHelper::reparent(qMRMLAbstractItemHelper* newParent)
{ 
  vtkMRMLNode*  newParentNode = 
    newParent ? vtkMRMLNode::SafeDownCast(newParent->object()) : 0;
  return qMRMLUtils::reparent(this->mrmlNode(), newParentNode);
}





//------------------------------------------------------------------------------
// qMRMLRootItemHelper
//------------------------------------------------------------------------------
class qMRMLRootItemHelperPrivate: public qCTKPrivate<qMRMLRootItemHelper>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLRootItemHelper);
  bool TopLevelScene;
};

//------------------------------------------------------------------------------
qMRMLRootItemHelper::qMRMLRootItemHelper(vtkMRMLScene* scene, bool topLevelScene)
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
  //Q_ASSERT(false);
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
