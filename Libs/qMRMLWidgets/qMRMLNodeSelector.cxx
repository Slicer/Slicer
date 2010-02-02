// QT includes
#include <QDebug>
#include <QComboBox>

// qMRML includes
#include "qMRMLNodeFactory.h"
#include "qMRMLNodeSelector.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLTransformNode.h>

//-----------------------------------------------------------------------------
class qMRMLNodeSelectorPrivate: public qCTKPrivate<qMRMLNodeSelector>
{
public:
  qMRMLNodeSelectorPrivate()
    {
    this->MRMLScene = 0; 
    this->MRMLCurrentNode = 0; 
    this->MRMLNodeBeingRemoved = 0;
    this->MRMLNodeModifiedEvent = vtkCommand::ModifiedEvent;
    this->MRMLNodeFactory = 0;

    this->ShowHidden = false;
    this->SelectNodeUponCreation = true; 
    }
  bool isValidNode(vtkMRMLNode* node)const;
  
  qMRMLNodeFactory* MRMLNodeFactory; 
  vtkMRMLScene*     MRMLScene; 
  vtkMRMLNode*      MRMLCurrentNode; 
  vtkMRMLNode*      MRMLNodeBeingRemoved;
  unsigned long     MRMLNodeModifiedEvent; 

  QStringList   NodeTypes; 
  bool          ShowHidden;
  bool          SelectNodeUponCreation; 
};

// --------------------------------------------------------------------------
bool qMRMLNodeSelectorPrivate::isValidNode(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  foreach(const QString& nodeType, this->NodeTypes)
    {
    if (node->IsA(nodeType.toAscii().data()))
      {
      return true;
      }
    }
  return false;
}

// --------------------------------------------------------------------------
qMRMLNodeSelector::qMRMLNodeSelector(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLNodeSelector);
  QCTK_D(qMRMLNodeSelector);

  d->MRMLNodeFactory = new qMRMLNodeFactory(this);
  
  // Connect comboBox
  this->connect(this, SIGNAL(currentIndexChanged(int)),
                SLOT(onCurrentIndexChanged(int)));
  this->connect(this, SIGNAL(itemAdded(int)),SLOT(onItemAdded(int)));
  //this->connect(this, SIGNAL(itemAboutToBeRemoved(int)),SLOT(onItemAboutToBeRemoved(int)));
  this->connect(this, SIGNAL(itemRemoved(int)),SLOT(onItemRemoved(int)));

  // as the Scene is empty, disable the "Add Button"
  this->setComboBoxEnabled(false);
  this->setAddEnabled(false);
  this->setRemoveEnabled(false);
  this->setEditEnabled(false);
}

// --------------------------------------------------------------------------
QCTK_SET_CXX(qMRMLNodeSelector, const QStringList&, setNodeTypes, NodeTypes);
QCTK_GET_CXX(qMRMLNodeSelector, QStringList, nodeTypes, NodeTypes);

// --------------------------------------------------------------------------
QCTK_SET_CXX(qMRMLNodeSelector, bool, setShowHidden, ShowHidden);
QCTK_GET_CXX(qMRMLNodeSelector, bool, showHidden, ShowHidden);

// --------------------------------------------------------------------------
QCTK_SET_CXX(qMRMLNodeSelector, bool, setSelectNodeUponCreation, SelectNodeUponCreation);
QCTK_GET_CXX(qMRMLNodeSelector, bool, selectNodeUponCreation, SelectNodeUponCreation);
QCTK_GET_CXX(qMRMLNodeSelector, vtkMRMLScene*, mrmlScene, MRMLScene);

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeSelector::currentNode()const
{
  QCTK_D(const qMRMLNodeSelector);
  
  Q_ASSERT(this->count() > 0 ? (d->MRMLCurrentNode != 0) : true);
  return d->MRMLCurrentNode;
}

// --------------------------------------------------------------------------
QString qMRMLNodeSelector::currentNodeId() const
{
  vtkMRMLNode* _currentNode = this->currentNode();
  if (_currentNode)
    {
    return QString::fromLatin1(_currentNode->GetID());
    }
  return QString();
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setMRMLNodeFactory(qMRMLNodeFactory* _factory)
{
  QCTK_D(qMRMLNodeSelector);
  
  Q_ASSERT(_factory);
  Q_ASSERT(this->mrmlScene() == _factory->mrmlScene());
  d->MRMLNodeFactory = _factory;
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qMRMLNodeSelector, qMRMLNodeFactory*, factory, MRMLNodeFactory);

// --------------------------------------------------------------------------
void qMRMLNodeSelector::addNode(vtkMRMLNode* mrmlNode)
{
  QCTK_D(qMRMLNodeSelector);
  
  // Make sure the the node added to the scene matches the nodeTypes of the selector
  if (!d->isValidNode(mrmlNode))
    {
    Q_ASSERT(d->isValidNode(mrmlNode));
    return;
    }
  
  if (!d->ShowHidden && mrmlNode->GetHideFromEditors())
    {
    //qDebug() << d->NodeType << " hide from editor: " << mrmlNode->GetClassName();
    return;
    }

  // Connect MRML Node
  // The widget is now aware of any updates regarding that node. 
  // And, if required, will be able to update the name of an item in the comboxbox
  this->qvtkConnect(mrmlNode, d->MRMLNodeModifiedEvent,
                    this, SLOT(onMRMLNodeModified(vtkObject*)));
  
  this->addNodeInternal(mrmlNode);  // Add the node into the combobox
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::addNodeInternal(vtkMRMLNode* mrmlNode)
{
  // all the checks have been done already.
  this->addItem(mrmlNode->GetName(), QString::fromAscii(mrmlNode->GetID())); 
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setMRMLScene(vtkMRMLScene* scene)
{
  QCTK_D(qMRMLNodeSelector);
  
  if (d->MRMLScene == scene)
    { 
    return; 
    }
  d->MRMLCurrentNode = 0;
  
  // Connect MRML scene NodeAdded event
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent,
    this, SLOT(onMRMLSceneNodeAdded(vtkObject*, vtkObject*)));
  
  // Connect MRML scene NodeRemoved event
  // Note: doesn't handle the case when the scene is deleted
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent,
    this, SLOT(onMRMLSceneNodeRemoved(vtkObject*, vtkObject*)));
  // When the scene is deleted, NodeRemovedEvent is not called. 
  // onMRMLSceneDeleted should clear the items 
  this->qvtkReconnect(d->MRMLScene, scene, vtkCommand::DeleteEvent,
    this, SLOT(onMRMLSceneDeleted(vtkObject*)));
  
  // clear should be called before d->MRMLScene is changed
  // to handle signals/slots with the correct scene
  this->clear();

  // the Add button is valid only if the scene is non-empty
  this->setAddEnabled(scene != 0);
  
  d->MRMLScene = scene;
  if (!scene)
    {
    this->setComboBoxEnabled(false);
    }

  // Update factory
  d->MRMLNodeFactory->setMRMLScene(scene);

  // Scan the scene and populate the nodes
  if (scene)
    {
    std::vector<vtkMRMLNode *> nodes;
    foreach(const QString& nodeType, d->NodeTypes)
      {
      // here we expect vtkMRMLScene::GetNodesByClass to concatenate 
      // new nodes to existing nodes
      scene->GetNodesByClass(nodeType.toAscii().data(), nodes);
      }
    int numberOfNodes = nodes.size();
    for (int i = 0; i < numberOfNodes; ++i)
      {
      this->addNode(nodes[i]);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setCurrentNode(vtkMRMLNode* _node)
{
  int index = _node ? this->findData(_node->GetID()) : -1;
  this->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLSceneNodeAdded(vtkObject * scene, vtkObject * _node)
{
  Q_UNUSED(scene);
  QCTK_D(qMRMLNodeSelector);
  Q_ASSERT(scene == d->MRMLScene);
  
  vtkMRMLNode* mrmlNode = vtkMRMLNode::SafeDownCast(_node);
  // we check the type here because addNode expect "valid" nodes only.
  if (!d->isValidNode(mrmlNode))
    {
    return;
    }

  this->addNode(mrmlNode);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLSceneNodeRemoved(vtkObject * scene, vtkObject * _node)
{
  Q_UNUSED(scene);
  QCTK_D(qMRMLNodeSelector);
  Q_ASSERT(scene == d->MRMLScene);
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(_node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode) 
    { 
    return; 
    }
  
   // Make sure the the node removed matches the nodeType of the selector
  if (!d->isValidNode(mrmlNode))
    {
    return;
    }
  
  // Remove item from combo box
  int index = this->findData(QString::fromAscii(mrmlNode->GetID()));
  if (index < 0)
    {// the node may not exist in the list ( if it is hidden from Editors
    return;
    }
  // we need to find a way to pass the data to the methods 
  // onItemAboutToBeRemoved() and onItemRemoved(). Querying the scene
  // with the node ID doesn't work as the item has already been removed
  // from the scene. 
  d->MRMLNodeBeingRemoved = mrmlNode;
  this->removeItem(index); 
  d->MRMLNodeBeingRemoved = 0;
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLNodeModified(vtkObject * caller)
{
  if (!caller) 
    { 
    return; 
    }
  //qDebug() << "qMRMLNodeSelector::onMRMLNodeModified:" << caller;
  // TODO Check if the name of node changed and update accordingly
}


//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLSceneDeleted(vtkObject * scene)
{
  QCTK_D(qMRMLNodeSelector);
  Q_UNUSED(scene);
  Q_ASSERT(scene == d->MRMLScene);
  // FIXME:
  // It's a bit tricky here. The scene has been deleted, but the scene 
  // destructor has deleted the nodes as well without letting anyone know.
  // As a result qMRMLNodeSelector is unsynchronized with the scene. We can't
  // even send signals like nodeAboutToBeRemoved/nodeRemoved. 
  // As of now, let's block signals/slots and clear the scene. 
  this->blockSignals(true);
  this->setMRMLScene(0);
  this->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onAdd()
{
  QCTK_D(qMRMLNodeSelector);
  
  // Create the MRML node via the MRML Scene
  // FIXME, for the moment we create only nodes of the first type, but we should
  // be able to add a node of any type in NodeTypes
  vtkMRMLNode * _node = d->MRMLNodeFactory->createNode(d->NodeTypes[0]);
  // The created node is appended at the bottom of the current list  
  Q_ASSERT(_node);
  if (_node && this->selectNodeUponCreation())
    {// select the created node.
    this->setCurrentIndex(this->count() - 1);
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onRemove()
{
  QCTK_D(qMRMLNodeSelector);
  
  // remove events shall not be called if the scene empty
  Q_ASSERT(d->MRMLScene);  
  if (!d->MRMLScene)
    {
    return;
    }
  Q_ASSERT(d->MRMLCurrentNode);
  // we must emit the signal here (instead of onItemAboutToBeRemoved) because 
  // the node is still in the scene here.
  emit nodeAboutToBeRemoved(d->MRMLCurrentNode);
  // ask the scene to remove the node. As we observe the scene, the item will be 
  // removed from the list later in onMRMLSceneNodeRemoved
  d->MRMLScene->RemoveNode(d->MRMLCurrentNode);
  // the signal nodeRemoved will be send by the method onItemRemoved
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onEdit()
{
  qDebug() << "qMRMLNodeSelector::" << __FUNCTION__ << ":" << this->currentText() << endl; 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onCurrentIndexChanged(int index)
{
  //qDebug() << "qMRMLNodeSelector::" << __FUNCTION__ << ":" << index << endl; 
  this->nodeIdSelected(index); 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onItemAdded(int index)
{
  //qDebug() << this->Internal->NodeType << " on item added" 
  //         << this->itemText(index) << "" << this->itemData(index);
  this->setComboBoxEnabled(true);
  this->setRemoveEnabled(true);
  this->setEditEnabled(true);

  emit nodeAdded(this->node(index));
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onItemAboutToBeRemoved(int index)
{
  Q_UNUSED(index);
  // Here the node can't be found in the scene. The signal nodeAboutToBeRemoved
  // has already been called in onRemove(). this->Internal->MRMLNodeBeingRemoved
  // is the node getting removed. Be careful when using index.
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onItemRemoved(int index)
{
  Q_UNUSED(index);
  QCTK_D(qMRMLNodeSelector);
  
  if (this->count() == 0)
    {
    this->setComboBoxEnabled(true);
    this->setRemoveEnabled(false);
    this->setEditEnabled(false);
    }
  // Here this->node(index) doesn't work as the scene has already removed the
  // node. Warning, the node can't be found in the scene.
  Q_ASSERT(d->MRMLNodeBeingRemoved);
  emit nodeRemoved(d->MRMLNodeBeingRemoved);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::nodeIdSelected(int index)
{
  QCTK_D(qMRMLNodeSelector);
  
  if (index >= 0 && this->count())
    {// a NULL scene is only a problem if the index is valid.
    Q_ASSERT(d->MRMLScene);
    }
  d->MRMLCurrentNode = this->node(index);
  //qDebug() << __FUNCTION__ << " " << id ;
  emit currentNodeChanged(d->MRMLCurrentNode);
  emit currentNodeChanged(d->MRMLCurrentNode != 0);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeSelector::node(int index)const 
{
  QVariant itemId = this->itemData(index);
  //qDebug() << __FUNCTION__ << " " << itemId;
  return this->node(itemId.toString());
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeSelector::node(const QString& id)const 
{
  QCTK_D(const qMRMLNodeSelector);
  
  return d->MRMLScene ? 
    d->MRMLScene->GetNodeByID(id.toLatin1().data()) : 0;
}
