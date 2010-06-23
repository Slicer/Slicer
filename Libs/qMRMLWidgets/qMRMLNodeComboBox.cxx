
// Qt includes
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QStandardItemModel>

// qMRMLWidgets includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLTreeProxyModel.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// -----------------------------------------------------------------------------
class qMRMLNodeComboBoxPrivate: public ctkPrivate<qMRMLNodeComboBox>
{
  CTK_DECLARE_PUBLIC(qMRMLNodeComboBox);
public:
  qMRMLNodeComboBoxPrivate();
  void init();
  vtkMRMLNode* mrmlNode(int index)const;
  vtkMRMLNode* mrmlNodeFromIndex(const QModelIndex& index)const;
  void updateNoneItem();
  void updateActionItems();

  QComboBox*        ComboBox;
  qMRMLNodeFactory* MRMLNodeFactory;
  qMRMLSceneModel*  MRMLSceneModel;
  bool              SelectNodeUponCreation;
  bool              NoneEnabled;
  bool              AddEnabled;
  bool              RemoveEnabled;
  bool              EditEnabled;
private:
  void setModel(QAbstractItemModel* model);
};

// -----------------------------------------------------------------------------
qMRMLNodeComboBoxPrivate::qMRMLNodeComboBoxPrivate()
{
  this->ComboBox = 0;
  this->MRMLNodeFactory = 0;
  this->MRMLSceneModel = 0;
  this->SelectNodeUponCreation = true; 
  this->NoneEnabled = false;
  this->AddEnabled = true;
  this->RemoveEnabled = true;
  this->EditEnabled = false;
}

// -----------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::init()
{
  CTK_P(qMRMLNodeComboBox);
  Q_ASSERT(this->MRMLNodeFactory == 0);
  this->ComboBox = new QComboBox(p);
  p->setLayout(new QHBoxLayout);
  p->layout()->addWidget(this->ComboBox);
  p->setContentsMargins(0,0,0,0);

  this->MRMLNodeFactory = new qMRMLNodeFactory(p);

  QAbstractItemModel* model = p->createSceneModel();
  QAbstractItemModel* rootModel = model;
  while (qobject_cast<QAbstractProxyModel*>(rootModel) && 
         qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel())
    {
    rootModel = qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel();
    }
  this->MRMLSceneModel = qobject_cast<qMRMLSceneModel*>(rootModel);
  this->updateNoneItem();
  this->updateActionItems();

  qMRMLSortFilterProxyModel* sortFilterModel = new qMRMLSortFilterProxyModel(p);
  sortFilterModel->setSourceModel(model);
  this->setModel(sortFilterModel);

  p->connect(p, SIGNAL(currentIndexChanged(int)), p, SLOT(emitCurrentNodeChanged(int)));

  p->setEnabled(p->mrmlScene() != 0);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::setModel(QAbstractItemModel* model)
{
  CTK_P(qMRMLNodeComboBox);
  this->ComboBox->setModel(model);
  p->connect(model, SIGNAL(rowsInserted(const QModelIndex&, int,int)),
             p, SLOT(emitNodesAdded(const QModelIndex&, int, int)));
  p->connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int,int)),
             p, SLOT(emitNodesAboutToBeRemoved(const QModelIndex&, int, int)));
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBoxPrivate::mrmlNode(int index)const
{
  CTK_P(const qMRMLNodeComboBox);
  QString nodeId = 
    this->ComboBox->itemData(index, qMRML::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkMRMLScene* scene = p->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toLatin1().data()) : 0;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBoxPrivate::mrmlNodeFromIndex(const QModelIndex& index)const
{
  CTK_P(const qMRMLNodeComboBox);
  Q_ASSERT(p->model());
  QString nodeId = 
    p->model()->data(index, qMRML::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkMRMLScene* scene = p->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toLatin1().data()) : 0;
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::updateNoneItem()
{
  CTK_P(qMRMLNodeComboBox);
  QStringList noneItem;
  if (this->NoneEnabled)
    {
    noneItem.append("None");
    }
  this->MRMLSceneModel->setPreItems(p->mrmlScene(), noneItem);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::updateActionItems()
{
  CTK_P(qMRMLNodeComboBox);
  QStringList extraItems;

  if (p->mrmlScene() == 0)
    {
    this->MRMLSceneModel->setPostItems(p->mrmlScene(), extraItems);
    return;
    }
  
  if (this->AddEnabled || this->RemoveEnabled || this->EditEnabled)
    {
    extraItems.append("separator");
    }
  if (this->AddEnabled)
    {
    extraItems.append("Add Node");
    }
  if (this->RemoveEnabled)
    {
    extraItems.append("Remove Node");
    }
  if (this->EditEnabled)
    {
    extraItems.append("Edit Node");
    }
  this->MRMLSceneModel->setPostItems(p->mrmlScene(), extraItems);

  QObject::connect(this->ComboBox->view(), SIGNAL(clicked(const QModelIndex& )),
                   p, SLOT(activateExtraItem(const QModelIndex& )), 
                   Qt::UniqueConnection);
}

// --------------------------------------------------------------------------
// qMRMLNodeComboBox

// --------------------------------------------------------------------------
qMRMLNodeComboBox::qMRMLNodeComboBox(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLNodeComboBox);
  CTK_D(qMRMLNodeComboBox);
  d->init();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::activateExtraItem(const QModelIndex& index)
{
  CTK_D(qMRMLNodeComboBox);
  // FIXME: check the type of the item on a different role instead of the display role
  QString data = this->model()->data(index, Qt::DisplayRole).toString();
  if (data == "Add Node")
    {
    d->ComboBox->hidePopup();
    this->addNode();
    }
  else if (data == "Remove Node")
    {
    d->ComboBox->hidePopup();
    this->removeCurrentNode();
    }
  else if (data == "Edit Node")
    {
    d->ComboBox->hidePopup();
    this->editCurrentNode();
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox::addAttribute(const QString& nodeType, 
                                     const QString& attributeName,
                                     const QVariant& attributeValue)
{
  CTK_D(qMRMLNodeComboBox);
  d->MRMLNodeFactory->addAttribute(attributeName, attributeValue.toString());
  this->sortFilterProxyModel()->addAttribute(nodeType, attributeName, attributeValue);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::addNode()
{
  CTK_D(qMRMLNodeComboBox);
  // Create the MRML node via the MRML Scene
  // FIXME, for the moment we create only nodes of the first type, but we should
  // be able to add a node of any type in NodeTypes
  vtkMRMLNode * _node = 
    d->MRMLNodeFactory->createNode(this->nodeTypes()[0]);
  // The created node is appended at the bottom of the current list  
  Q_ASSERT(_node);
  if (_node && this->selectNodeUponCreation())
    {// select the created node.
    this->setCurrentNode(_node);
    }
  emit this->nodeAddedByUser(_node);
}

// --------------------------------------------------------------------------
QAbstractItemModel* qMRMLNodeComboBox::createSceneModel()
{
  return new qMRMLSceneModel(this);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox::currentNode()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->mrmlNode(d->ComboBox->currentIndex());
}

// --------------------------------------------------------------------------
QString qMRMLNodeComboBox::currentNodeId()const
{
  vtkMRMLNode* node = this->currentNode();
  return node ? node->GetID() : "";
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::editCurrentNode()
{
  //CTK_D(const qMRMLNodeComboBox);
  //FIXME
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::emitCurrentNodeChanged(int currentIndex)
{
  CTK_D(qMRMLNodeComboBox);
  vtkMRMLNode* node = d->mrmlNode(currentIndex);
  emit currentNodeChanged(node);
  emit currentNodeChanged(node != 0);
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeComboBox::mrmlScene()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->MRMLSceneModel->mrmlScene();
}

// --------------------------------------------------------------------------
int qMRMLNodeComboBox::nodeCount()const
{
  CTK_D(const qMRMLNodeComboBox);
  int extraItemsCount =
      d->MRMLSceneModel->preItems(this->mrmlScene()).count()
    + (d->MRMLSceneModel->postItems(this->mrmlScene()).count() ? d->MRMLSceneModel->postItems(this->mrmlScene()).count(): 0);
  return this->mrmlScene() ? d->ComboBox->count() - extraItemsCount : 0;
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::removeCurrentNode()
{
  qDebug() << __FUNCTION__ << this->currentNode();
  this->mrmlScene()->RemoveNode(this->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qMRMLNodeComboBox);
  
  //if (d->MRMLSceneModel->mrmlScene() == scene)
  //  { 
  //  return ;
  //  }
  
  // The Add button is valid only if the scene is non-empty
  //this->setAddEnabled(scene != 0);
  
  // Update factory
  d->MRMLNodeFactory->setMRMLScene(scene);
  d->MRMLSceneModel->setMRMLScene(scene);
  d->updateNoneItem();
  d->updateActionItems();
  d->ComboBox->setRootModelIndex(this->model()->index(0, 0));

  this->setEnabled(scene != 0);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(vtkMRMLNode* _node)
{
  CTK_D(qMRMLNodeComboBox);
  int index = _node ? d->ComboBox->findData(_node->GetID(), qMRML::UIDRole) : -1;
  d->ComboBox->setCurrentIndex(index);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(const QString& nodeID)
{
  vtkMRMLScene* scene = this->mrmlScene();
  if (scene == 0)
    {
    return;
    }
  this->setCurrentNode(scene->GetNodeByID(nodeID.toLatin1().data()));
}

//--------------------------------------------------------------------------
CTK_SET_CXX(qMRMLNodeComboBox, bool, setSelectNodeUponCreation, SelectNodeUponCreation);
CTK_GET_CXX(qMRMLNodeComboBox, bool, selectNodeUponCreation, SelectNodeUponCreation);

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setNoneEnabled(bool enable)
{
  CTK_D(qMRMLNodeComboBox);
  if (d->NoneEnabled == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  d->updateNoneItem();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::noneEnabled()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->NoneEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setAddEnabled(bool enable)
{
  CTK_D(qMRMLNodeComboBox);
  if (d->AddEnabled == enable)
    {
    return;
    }
  d->AddEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::addEnabled()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->AddEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setRemoveEnabled(bool enable)
{
  CTK_D(qMRMLNodeComboBox);
  if (d->RemoveEnabled == enable)
    {
    return;
    }
  d->RemoveEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::removeEnabled()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->RemoveEnabled;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::setEditEnabled(bool enable)
{
  CTK_D(qMRMLNodeComboBox);
  if (d->EditEnabled == enable)
    {
    return;
    }
  d->EditEnabled = enable;
  d->updateActionItems();
}

//--------------------------------------------------------------------------
bool qMRMLNodeComboBox::editEnabled()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->EditEnabled;
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLNodeComboBox::sortFilterProxyModel()const
{
  Q_ASSERT(qobject_cast<qMRMLSortFilterProxyModel*>(this->model()));
  return qobject_cast<qMRMLSortFilterProxyModel*>(this->model());
}

//----------------------------------=---------------------------------------
QAbstractItemModel* qMRMLNodeComboBox::model()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->ComboBox->model();
}

//----------------------------------=---------------------------------------
void qMRMLNodeComboBox::setComboBox(QComboBox* comboBox)
{
  CTK_D(qMRMLNodeComboBox);
  QAbstractItemModel* oldModel = this->model();
  this->layout()->addWidget(comboBox);
  d->ComboBox = comboBox;
  d->setModel(oldModel);
  this->layout()->removeWidget(d->ComboBox);
}

//----------------------------------=---------------------------------------
QComboBox* qMRMLNodeComboBox::comboBox()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->ComboBox;
}

//--------------------------------------------------------------------------
void qMRMLNodeComboBox::emitNodesAdded(const QModelIndex & parent, int start, int end)
{
  CTK_D(qMRMLNodeComboBox);
  Q_ASSERT(this->model());
  for(int i = start; i <= end; ++i)
    {
    vtkMRMLNode* node = d->mrmlNodeFromIndex(this->model()->index(start, 0, parent));
    Q_ASSERT(node);
    emit nodeAdded(node);
    }
}
 
//--------------------------------------------------------------------------
void qMRMLNodeComboBox::emitNodesAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  CTK_D(qMRMLNodeComboBox);
  Q_ASSERT(this->model());
  for(int i = start; i <= end; ++i)
    {
    vtkMRMLNode* node = d->mrmlNodeFromIndex(this->model()->index(start, 0, parent));
    Q_ASSERT(node);
    emit nodeAboutToBeRemoved(node);
    }
}
