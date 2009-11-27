
#include "qMRMLNodeSelector.h"
#include "qMRMLNodeFactory.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

// QT includes
#include <QDebug>
#include <QComboBox>

//-----------------------------------------------------------------------------
struct qMRMLNodeSelector::qInternal
{
  qInternal()
    {
    this->MRMLScene = 0; 
    this->MRMLCurrentNode = 0; 
    this->MRMLNodeBeingRemoved = 0;
    this->MRMLNodeModifiedEvent = vtkCommand::ModifiedEvent;
    this->MRMLNodeFactory = 0;

    this->ShowHidden = false;
    }
  qMRMLNodeFactory* MRMLNodeFactory; 
  vtkMRMLScene*     MRMLScene; 
  vtkMRMLNode*      MRMLCurrentNode; 
  vtkMRMLNode*      MRMLNodeBeingRemoved;
  unsigned long     MRMLNodeModifiedEvent; 

  QString       NodeType; 
  bool          ShowHidden;
};

// --------------------------------------------------------------------------
qMRMLNodeSelector::qMRMLNodeSelector(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal;

  this->Internal->MRMLNodeFactory = new qMRMLNodeFactory(this); 
  
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
qMRMLNodeSelector::~qMRMLNodeSelector()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
QString qMRMLNodeSelector::nodeType()const
{
  return this->Internal->NodeType;
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setNodeType(const QString& nodeType)
{
  this->Internal->NodeType = nodeType;
}

// --------------------------------------------------------------------------
bool qMRMLNodeSelector::showHidden()const
{
  return this->Internal->ShowHidden;
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setShowHidden(bool showHidden)
{
  this->Internal->ShowHidden = showHidden;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeSelector::mrmlScene()const
{
  return this->Internal->MRMLScene;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeSelector::currentNode()const
{
  Q_ASSERT(this->count() > 0 ? (this->Internal->MRMLCurrentNode != 0) : true);
  return this->Internal->MRMLCurrentNode;
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setMRMLNodeFactory(qMRMLNodeFactory* factory)
{
  Q_ASSERT(factory);
  Q_ASSERT(this->mrmlScene() == factory->mrmlScene());
  this->Internal->MRMLNodeFactory = factory; 
}

qMRMLNodeFactory* qMRMLNodeSelector::factory()
{
  return this->Internal->MRMLNodeFactory;
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::addNode(vtkMRMLNode* mrmlNode)
{
  if (!mrmlNode) 
    { 
    Q_ASSERT(mrmlNode);
    return; 
    }
  
  // Make sure the the node added to the scene matches the nodeType of the selector
  if (!mrmlNode->IsA(this->Internal->NodeType.toAscii().data()))
    {// this test should have been done prior to calling the function
    Q_ASSERT(mrmlNode->IsA(this->Internal->NodeType.toAscii().data()));
    return;
    }
    
  if (!this->Internal->ShowHidden && mrmlNode->GetHideFromEditors())
    {
    //qDebug() << this->Internal->NodeType << " hide from editor: " << mrmlNode->GetClassName();
    return;
    }

  // Connect MRML Node
  // The widget is now aware of any updates regarding that node. 
  // And, if required, will be able to update the name of an item in the comboxbox
  this->qvtkConnect(mrmlNode, this->Internal->MRMLNodeModifiedEvent, 
                    this, SLOT(onMRMLNodeModified(void*,vtkObject*)));
  
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
  if (this->Internal->MRMLScene == scene) 
    { 
    return; 
    }
  this->Internal->MRMLCurrentNode = 0;
  
  // Connect MRML scene NodeAdded event
  this->qvtkReconnect(this->Internal->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent, 
    this, SLOT(onMRMLSceneNodeAdded(vtkObject*)));
  
  // Connect MRML scene NodeRemoved event
  this->qvtkReconnect(this->Internal->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent, 
    this, SLOT(onMRMLSceneNodeRemoved(vtkObject*)));
  
  // the Add button is valid only if the scene is non-empty
  this->setAddEnabled(scene != 0);
  
  this->Internal->MRMLScene = scene; 
  if (!scene)
    {
    this->setComboBoxEnabled(false);
    }

  // Update factory
  this->Internal->MRMLNodeFactory->setMRMLScene(scene);

  // Scan the scene and populate the nodes
  if (scene)
    {
    std::vector<vtkMRMLNode *> nodes;
    int numberOfNodes = scene->GetNodesByClass(this->Internal->NodeType.toAscii().data(), nodes);
    for (int i = 0; i < numberOfNodes; ++i)
      {
      this->addNode(nodes[i]);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setCurrentNode(vtkMRMLNode* node)
{
  int index = node ? this->findData(node->GetID()) : -1;
  this->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLSceneNodeAdded(vtkObject * node)
{
  vtkMRMLNode* mrmlNode = vtkMRMLNode::SafeDownCast(node);
  if (!mrmlNode || !mrmlNode->IsA(this->Internal->NodeType.toAscii().data()))
    {
    return;
    }

  this->addNode(mrmlNode);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLSceneNodeRemoved(vtkObject * node)
{
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode) 
    { 
    return; 
    }
  
   // Make sure the the node removed matches the nodeType of the selector
  if (!mrmlNode->IsA(this->Internal->NodeType.toAscii().data()))
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
  this->Internal->MRMLNodeBeingRemoved = mrmlNode;
  this->removeItem(index); 
  this->Internal->MRMLNodeBeingRemoved = 0;
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLNodeModified(void* call_data, vtkObject * caller)
{
  if (!caller) 
    { 
    return; 
    }
  //qDebug() << "qMRMLNodeSelector::onMRMLNodeModified:" << caller;
  // TODO Check if the name of node changed and update accordingly
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onAdd()
{
  // Create the MRML node via the MRML Scene
  vtkMRMLNode * node = this->Internal->MRMLNodeFactory->createNode(this->nodeType());
  // The created node is appended at the bottom of the current list  
  Q_ASSERT(node);
  if (node)
    {// select the created node. Maybe it should be a widget property
    this->setCurrentIndex(this->count() - 1);
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onRemove()
{
  // remove events shall not be called if the scene empty
  Q_ASSERT(this->Internal->MRMLScene);  
  if (!this->Internal->MRMLScene)
    {
    return;
    }
  Q_ASSERT(this->Internal->MRMLCurrentNode);
  // we must emit the signal here (instead of onItemAboutToBeRemoved) because 
  // the node is still in the scene here.
  emit nodeAboutToBeRemoved(this->Internal->MRMLCurrentNode);
  // ask the scene to remove the node. As we observe the scene, the item will be 
  // removed from the list later in onMRMLSceneNodeRemoved
  this->Internal->MRMLScene->RemoveNode(this->Internal->MRMLCurrentNode);
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
  // Here the node can't be found in the scene. The signal nodeAboutToBeRemoved
  // has already been called in onRemove(). this->Internal->MRMLNodeBeingRemoved
  // is the node getting removed. Be careful when using index.
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onItemRemoved(int index)
{
  if (this->count() == 0)
    {
    this->setComboBoxEnabled(true);
    this->setRemoveEnabled(false);
    this->setEditEnabled(false);
    }
  // Here this->node(index) doesn't work as the scene has already removed the
  // node. Warning, the node can't be found in the scene.
  Q_ASSERT(this->Internal->MRMLNodeBeingRemoved);
  emit nodeRemoved(this->Internal->MRMLNodeBeingRemoved);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::nodeIdSelected(int index)
{
  if (index >= 0 && this->count())
    {// a NULL scene is only a problem if the index is valid.
    Q_ASSERT(this->Internal->MRMLScene);
    }
  this->Internal->MRMLCurrentNode = this->node(index);
  //qDebug() << __FUNCTION__ << " " << id ;
  emit this->currentNodeChanged(this->Internal->MRMLCurrentNode);
  emit this->currentNodeChanged(this->Internal->MRMLCurrentNode != 0);
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
  return this->Internal->MRMLScene ? 
    this->Internal->MRMLScene->GetNodeByID(id.toLatin1().data()) : 0;
}
