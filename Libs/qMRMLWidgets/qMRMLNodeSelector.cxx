
#include "qMRMLNodeSelector.h"
#include "qMRMLUtils.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qMRMLNodeSelector::qInternal
{
  qInternal()
    {
    this->MRMLScene = 0; 
    this->MRMLNodeSelected = 0; 
    this->MRMLNodeModifiedEvent = vtkCommand::ModifiedEvent; 
    }
  vtkMRMLScene* MRMLScene; 
  vtkMRMLNode*  MRMLNodeSelected; 
  QString       NodeType; 
  unsigned long MRMLNodeModifiedEvent; 
};

// --------------------------------------------------------------------------
qMRMLNodeSelector::qMRMLNodeSelector(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  
  // Connect add button
  this->connect(this, SIGNAL(addPushButtonPressed()), 
    SLOT(onAddButtonPressed()));
  
  // Connect remove button
  this->connect(this, SIGNAL(removePushButtonPressed(const QString &)), 
    SLOT(onRemoveButtonPressed(const QString &)));
    
  // Connect edit button
  this->connect(this, SIGNAL(itemEditRequested(const QString &)), 
    SLOT(onSelectorItemEditRequested(const QString &))); 
    
  // Connect comboBox
  this->connect(this, SIGNAL(itemSelected(const QString &)), 
    SLOT(onSelectorItemSelected(const QString &))); 
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
vtkMRMLNode* qMRMLNodeSelector::getSelectedNode()const
{
  Q_ASSERT(this->count() > 0 ? (this->Internal->MRMLNodeSelected != 0) : true);
  return this->Internal->MRMLNodeSelected;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeSelector::mrmlScene()const
{
  return this->Internal->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLNodeSelector::setMRMLScene(vtkMRMLScene* scene)
{
  if (this->Internal->MRMLScene == scene) 
    { 
    return; 
    }
  
  // Connect MRML scene NodeAdded event
  this->qvtkReConnect(this->Internal->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent, 
    this, SLOT(onMRMLNodeAdded(vtkObject*)));
  
  // Connect MRML scene NodeRemoved event
  this->qvtkReConnect(this->Internal->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent, 
    this, SLOT(onMRMLNodeRemoved(vtkObject*)));
   
  this->Internal->MRMLScene = scene; 
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
  
  // Add the node the combobox
  this->addItemNoNotify(mrmlNode->GetID());
  
  this->setSelected(this->selectedItemName()); 
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
  this->removeItemNoNotify(QString::fromAscii(mrmlNode->GetID())); 
  
  this->setSelected(this->selectedItemName());
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onMRMLNodeModified(void* call_data, vtkObject * caller)
{
  if (!caller) { return; }
  qDebug() << "qMRMLNodeSelector::onMRMLNodeModified:" << caller; 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onAddButtonPressed()
{ 
  // Update MRML Scene
  vtkMRMLNode * node = qMRMLUtils::createAndAddNodeToSceneByClass(
      this->Internal->MRMLScene, this->nodeType().toAscii().data() ); 
  
  Q_ASSERT(node);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onRemoveButtonPressed(const QString & selectedItemName)
{
  vtkMRMLNode * node = this->Internal->MRMLScene->GetNodeByID(selectedItemName.toStdString());
  Q_ASSERT(node);
  
  // Update MRML Scene
  this->Internal->MRMLScene->RemoveNode(node);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onSelectorItemSelected(const QString & itemName)
{ 
  this->setSelected(itemName); 
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::setSelected(const QString& name)
{
  vtkMRMLNode * nodeSelected = this->Internal->MRMLScene->GetNodeByID(name.toStdString());
  
  this->Internal->MRMLNodeSelected = nodeSelected; 
  emit this->nodeSelected(nodeSelected);
}

//-----------------------------------------------------------------------------
void qMRMLNodeSelector::onSelectorItemEditRequested(const QString & itemName)
{ 
  qDebug() << "qMRMLNodeSelector::onSelectorItemEditRequested:" << itemName << endl; 
}
