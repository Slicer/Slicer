
// Qt includes
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QAction>
#include <QDebug>
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
class qMRMLNodeComboBoxPrivate: public qCTKPrivate<qMRMLNodeComboBox>
{
public:
  qMRMLNodeComboBoxPrivate();
  void init();
  qMRMLNodeFactory* MRMLNodeFactory;
  qMRMLSceneModel*  MRMLSceneModel;
  bool              SelectNodeUponCreation;

};

// -----------------------------------------------------------------------------
qMRMLNodeComboBoxPrivate::qMRMLNodeComboBoxPrivate()
{
  this->MRMLNodeFactory = 0;
  this->MRMLSceneModel = 0;
  this->SelectNodeUponCreation = true; 
}

// -----------------------------------------------------------------------------
void qMRMLNodeComboBoxPrivate::init()
{
  QCTK_P(qMRMLNodeComboBox);
  Q_ASSERT(this->MRMLNodeFactory == 0);
  this->MRMLNodeFactory = new qMRMLNodeFactory(p);

  QAbstractItemModel* model = p->createSceneModel();
  QAbstractItemModel* rootModel = model;
  while (qobject_cast<QAbstractProxyModel*>(rootModel) && 
         qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel())
    {
    rootModel = qobject_cast<QAbstractProxyModel*>(rootModel)->sourceModel();
    }
  this->MRMLSceneModel = qobject_cast<qMRMLSceneModel*>(rootModel);
  QStringList extraItems;
  extraItems.append("Add Node");
  extraItems.append("Remove Node");
  extraItems.append("Edit Node");
  this->MRMLSceneModel->setExtraItems(p->mrmlScene(), extraItems);

  QObject::connect(p->view(), SIGNAL(clicked(const QModelIndex& )),
                   p, SLOT(activateExtraItem(const QModelIndex& )));

  qMRMLSortFilterProxyModel* sortFilterModel = new qMRMLSortFilterProxyModel(p);
  sortFilterModel->setSourceModel(model);
  p->setModel(sortFilterModel);
}

// --------------------------------------------------------------------------
// qMRMLNodeComboBox

// --------------------------------------------------------------------------
qMRMLNodeComboBox::qMRMLNodeComboBox(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLNodeComboBox);
  QCTK_D(qMRMLNodeComboBox);
  d->init();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::activateExtraItem(const QModelIndex& index)
{
  // FIXME: check the type of the item on a different role instead of the display role
  QString data = this->model()->data(index, Qt::DisplayRole).toString();
  if (data == "Add Node")
    {
    this->hidePopup();
    this->addNode();
    }
  else if (data == "Remove Node")
    {
    this->hidePopup();
    this->removeCurrentNode();
    }
  else if (data == "Edit Node")
    {
    this->hidePopup();
    this->editCurrentNode();
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::addNode()
{
  QCTK_D(qMRMLNodeComboBox);
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
}

// --------------------------------------------------------------------------
QAbstractItemModel* qMRMLNodeComboBox::createSceneModel()
{
  return new qMRMLSceneModel(this);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeComboBox::currentNode()const
{
  QString nodeId = 
    this->itemData(this->currentIndex(), qMRML::UIDRole).toString();
  if (nodeId.isEmpty())
    {
    return 0;
    }
  vtkMRMLScene* scene = this->mrmlScene();
  return scene ? scene->GetNodeByID(nodeId.toLatin1().data()) : 0;
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::editCurrentNode()
{
  //QCTK_D(const qMRMLNodeComboBox);
  //FIXME
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeComboBox::mrmlScene()const
{
  QCTK_D(const qMRMLNodeComboBox);
  return d->MRMLSceneModel->mrmlScene();
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
  QCTK_D(qMRMLNodeComboBox);
  
  //if (d->MRMLSceneModel->mrmlScene() == scene)
  //  { 
  //  return ;
  //  }
  
  // The Add button is valid only if the scene is non-empty
  //this->setAddEnabled(scene != 0);
  
  // Update factory
  d->MRMLNodeFactory->setMRMLScene(scene);
  d->MRMLSceneModel->setMRMLScene(scene);
  this->setRootModelIndex(this->model()->index(0, 0));

  this->setEnabled(scene != 0);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setCurrentNode(vtkMRMLNode* _node)
{
  int index = _node ? this->findData(_node->GetID(), qMRML::UIDRole) : -1;
  this->setCurrentIndex(index);
}

// --------------------------------------------------------------------------
QCTK_SET_CXX(qMRMLNodeComboBox, bool, setSelectNodeUponCreation, SelectNodeUponCreation);
QCTK_GET_CXX(qMRMLNodeComboBox, bool, selectNodeUponCreation, SelectNodeUponCreation);

// --------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLNodeComboBox::sortFilterProxyModel()const
{
  Q_ASSERT(qobject_cast<qMRMLSortFilterProxyModel*>(this->model()));
  return qobject_cast<qMRMLSortFilterProxyModel*>(this->model());
}
