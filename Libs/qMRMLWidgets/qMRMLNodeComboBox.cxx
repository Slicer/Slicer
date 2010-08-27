
// Qt includes
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QStandardItemModel>

// CTK includes
#include <ctkComboBox.h>

// qMRMLWidgets includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// -----------------------------------------------------------------------------
class qMRMLNodeComboBoxPrivate: public ctkPrivate<qMRMLNodeComboBox>
{
  CTK_DECLARE_PUBLIC(qMRMLNodeComboBox);
public:
  qMRMLNodeComboBoxPrivate();
  void init(QAbstractItemModel* model);
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
void qMRMLNodeComboBoxPrivate::init(QAbstractItemModel* model)
{
  CTK_P(qMRMLNodeComboBox);
  Q_ASSERT(this->MRMLNodeFactory == 0);
  ctkComboBox* comboBox = new ctkComboBox(p);
  comboBox->setDefaultText(QObject::tr("Select a node..."));
  comboBox->setElideMode(Qt::ElideMiddle);
  this->ComboBox = comboBox;
  p->setLayout(new QHBoxLayout);
  p->layout()->addWidget(this->ComboBox);
  p->layout()->setContentsMargins(0,0,0,0);
  p->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::ComboBox));
  this->ComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType));

  this->MRMLNodeFactory = new qMRMLNodeFactory(p);

  QAbstractItemModel* rootModel = model;
  while (qobject_cast<QAbstractProxyModel*>(rootModel) && 
         qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel())
    {
    rootModel = qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel();
    }
  this->MRMLSceneModel = qobject_cast<qMRMLSceneModel*>(rootModel);
  Q_ASSERT(this->MRMLSceneModel);
  this->updateNoneItem();
  this->updateActionItems();

  qMRMLSortFilterProxyModel* sortFilterModel = new qMRMLSortFilterProxyModel(p);
  sortFilterModel->setSourceModel(model);
  this->setModel(sortFilterModel);

  p->connect(this->ComboBox, SIGNAL(currentIndexChanged(int)), 
             p, SLOT(emitCurrentNodeChanged(int)));

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
qMRMLNodeComboBox::qMRMLNodeComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
{
  CTK_INIT_PRIVATE(qMRMLNodeComboBox);
  CTK_D(qMRMLNodeComboBox);
  d->init(new qMRMLSceneModel(this));
}

// --------------------------------------------------------------------------
qMRMLNodeComboBox::qMRMLNodeComboBox(QAbstractItemModel* sceneModel, QWidget* parentWidget)
  : Superclass(parentWidget)
{
  CTK_INIT_PRIVATE(qMRMLNodeComboBox);
  CTK_D(qMRMLNodeComboBox);
  d->init(sceneModel);
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

//-----------------------------------------------------------------------------
void qMRMLNodeComboBox::setBaseName(const QString& baseName)
{
  CTK_D(qMRMLNodeComboBox);
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
    {
    return;
    }
  d->MRMLNodeFactory->setBaseName(nodeClasses[0], baseName);
}

//-----------------------------------------------------------------------------
QString qMRMLNodeComboBox::baseName()const
{
  CTK_D(const qMRMLNodeComboBox);
  QStringList nodeClasses = this->nodeTypes();
  if (nodeClasses.isEmpty())
    {
    return QString();
    }
  return d->MRMLNodeFactory->baseName(nodeClasses[0]);
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
  qDebug() << "ExtraItems: " << extraItemsCount << d->ComboBox->count();
  Q_ASSERT(!this->mrmlScene() || d->ComboBox->count() >= extraItemsCount);
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
  QVariant currentNode = d->ComboBox->itemData(d->ComboBox->currentIndex(), qMRML::UIDRole);
  
  // Update factory
  d->MRMLNodeFactory->setMRMLScene(scene);
  d->MRMLSceneModel->setMRMLScene(scene);
  d->updateNoneItem();
  d->updateActionItems();
  // setting the rootmodel index looses the current item
  d->ComboBox->setRootModelIndex(this->model()->index(0, 0));
  // try to set the current item back
  this->setCurrentNode(currentNode.toString());

  this->setEnabled(scene != 0);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(vtkMRMLNode* newCurrentNode)
{
  this->setCurrentNode(newCurrentNode ? newCurrentNode->GetID() : "");
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(const QString& nodeID)
{
  CTK_D(qMRMLNodeComboBox);
  int index = !nodeID.isEmpty() ? d->ComboBox->findData(nodeID, qMRML::UIDRole) : -1;
  if (index == -1 && d->NoneEnabled)
    {
    index = 0; // select None
    }
  d->ComboBox->setCurrentIndex(index);
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
QAbstractItemModel* qMRMLNodeComboBox::rootModel()const
{
  CTK_D(const qMRMLNodeComboBox);
  return d->MRMLSceneModel;
}

//----------------------------------=---------------------------------------
void qMRMLNodeComboBox::setComboBox(QComboBox* comboBox)
{
  CTK_D(qMRMLNodeComboBox);
  QAbstractItemModel* oldModel = this->model();
  QComboBox* oldComboBox = d->ComboBox;
  this->layout()->addWidget(comboBox);
  d->ComboBox = comboBox;
  d->setModel(oldModel);
  delete oldComboBox;
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
