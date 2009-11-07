
#include "qMRMLNodeSelector.h"
#include "qMRMLUtils.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

#include <QDebug>
#include <QComboBox>

//-----------------------------------------------------------------------------
struct qMRMLNodeSelector::qInternal
{
  qInternal()
    {
    this->MRMLScene = 0; 
    this->MRMLCurrentNode = 0; 
    this->MRMLNodeModifiedEvent = vtkCommand::ModifiedEvent; 
    }
  vtkMRMLScene* MRMLScene; 
  vtkMRMLNode*  MRMLCurrentNode; 
  QString       NodeType; 
  unsigned long MRMLNodeModifiedEvent; 
};

// --------------------------------------------------------------------------
qMRMLNodeSelector::qMRMLNodeSelector(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  
  // Connect comboBox
  this->connect(this, SIGNAL(currentIndexChanged(int)),
                SLOT(onCurrentIndexChanged(int)));
  this->connect(this, SIGNAL(itemAdded(int)),SLOT(onItemAdded(int)));
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
void qMRMLNodeSelector::setMRMLScene(vtkMRMLScene* scene)
{
  if (this->Internal->MRMLScene == scene) 
    { 
    return; 
    }
  this->Internal->MRMLCurrentNode = 0;
  
  // Connect MRML scene NodeAdded event
  this->qvtkReConnect(this->Internal->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent, 
    this, SLOT(onMRMLNodeAdded(vtkObject*)));
  
  // Connect MRML scene NodeRemoved event
  this->qvtkReConnect(this->Internal->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent, 
    this, SLOT(onMRMLNodeRemoved(vtkObject*)));
  
  // Scan the scene here 
  
  // the Add button is valid only if the scene is non-empty
  this->setAddEnabled(scene != 0);
  
  this->Internal->MRMLScene = scene; 
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setCurrentNode(vtkMRMLNode* node)
{
  int index = node ? this->findData(node->GetID()) : -1;
  this->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLNodeAdded(vtkObject * node)
{
  vtkMRMLNode* mrmlNode = vtkMRMLNode::SafeDownCast(node);
  
  Q_ASSERT(mrmlNode);
  if (!mrmlNode) { return; }
  
  // Make sure the the node added to the scene matches the nodeType of the selector
  if (this->Internal->NodeType.compare(mrmlNode->GetClassName())!=0)
    {
    return; 
    }
    
  // Connect MRML Node
  // The widget is now aware of any updates regarding that node. 
  // And, if required, will be able to update the name of an item in the comboxbox
  this->qvtkConnect(node, 
    this->Internal->MRMLNodeModifiedEvent, 
    this, SLOT(onMRMLNodeModified(void*,vtkObject*)));
  std::cout<< __FUNCTION__ << " " << mrmlNode->GetID() << " " << QString::fromAscii(mrmlNode->GetID()).toStdString() << std::endl;
  // Add the node the combobox
  this->addItem(mrmlNode->GetName(), QString::fromAscii(mrmlNode->GetID())); 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLNodeRemoved(vtkObject * node)
{
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode) { return; }
  
   // Make sure the the node removed matches the nodeType of the selector
  if (this->Internal->NodeType.compare(mrmlNode->GetClassName())!=0)
    {
    return; 
    }
  
  // Remove item from combo box
  int index = this->findData(QString::fromAscii(mrmlNode->GetID()));
  this->removeItem(index); 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLNodeModified(void* call_data, vtkObject * caller)
{
  if (!caller) { return; }
  qDebug() << "qMRMLNodeSelector::onMRMLNodeModified:" << caller; 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onAdd()
{
  // Create the MRML node via the MRML Scene
  vtkMRMLNode * node = qMRMLUtils::createAndAddNodeToSceneByClass(
      this->Internal->MRMLScene, this->nodeType().toLatin1().data() ); 
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
  this->Internal->MRMLScene->RemoveNode(this->Internal->MRMLCurrentNode);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onEdit()
{
  qDebug() << "qMRMLNodeSelector::" << __FUNCTION__ << ":" << this->currentText() << endl; 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onCurrentIndexChanged(int index)
{
  QString id = this->itemData(index).toString();
  this->nodeIdSelected(id); 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onItemAdded(int index)
{
  this->setComboBoxEnabled(true);
  this->setRemoveEnabled(true);
  this->setEditEnabled(true);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onItemRemoved(int index)
{
  if (!this->count())
    {
    this->setComboBoxEnabled(true);
    this->setRemoveEnabled(false);
    this->setEditEnabled(false);
    }  
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::nodeIdSelected(const QString& id)
{
  if (id.isEmpty())
    {
    this->Internal->MRMLCurrentNode= 0;
    }
  else
    {
    Q_ASSERT(this->Internal->MRMLScene);
    this->Internal->MRMLCurrentNode = this->Internal->MRMLScene ? 
      this->Internal->MRMLScene->GetNodeByID(id.toStdString()) : 0;
    }
  //qDebug() << __FUNCTION__ << " " << id ;
  emit this->currentNodeChanged(this->Internal->MRMLCurrentNode);
  emit this->currentNodeChanged(this->Internal->MRMLCurrentNode != 0);
}
