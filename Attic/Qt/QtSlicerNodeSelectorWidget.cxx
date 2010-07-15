/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: QtSlicerNodeSelectorWidget.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "QtSlicerNodeSelectorWidget.h"


#include <sstream>

#include "vtkEventQtSlotConnect.h"



//----------------------------------------------------------------------------
QtSlicerNodeSelectorWidget::QtSlicerNodeSelectorWidget()
{
  this->NewNodeCount = 0;
  this->NewNodeEnabled = 0;
  this->NoneEnabled = 0;
  this->ShowHidden = 0;
  this->ChildClassesEnabled = 1;
  this->MRMLScene      = NULL;
  this->ContextMenuHelper      = NULL;
  this->InMRMLCallbackFlag = 0;
  connect(this, SIGNAL(activated (const QString &)), this, SLOT(Select(const QString &)));

}

//----------------------------------------------------------------------------
QtSlicerNodeSelectorWidget::~QtSlicerNodeSelectorWidget()
{
  this->SetMRMLScene ( NULL );
  if (this->ContextMenuHelper)
    {
    this->ContextMenuHelper->Delete();
    }
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::CreateWidget()
{
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::SetMRMLScene( vtkMRMLScene *aMRMLScene)
{
  if ( this->MRMLScene )
    {
    this->MRMLScene->Delete ( );
    this->MRMLScene = NULL;
    }

  this->MRMLScene = aMRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(NULL);

    vtkEventQtSlotConnect* Connections = vtkEventQtSlotConnect::New();

    Connections->Connect(this->MRMLScene, vtkMRMLScene::NodeAddedEvent,
                         this, SLOT(UpdateMenu()));
    Connections->Connect(this->MRMLScene, vtkMRMLScene::NodeRemovedEvent,
                         this, SLOT(UpdateMenu()));
    Connections->Connect(this->MRMLScene, vtkMRMLScene::SceneClosedEvent,
                         this, SLOT(UpdateMenu()));
    Connections->Connect(this->MRMLScene, vtkMRMLScene::SceneEditedEvent,
                         this, SLOT(UpdateMenu()));
/**   
  this->MRMLScene->AddObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
   this->MRMLScene->AddObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
   this->MRMLScene->AddObserver( vtkMRMLScene::NewSceneEvent, this->MRMLCallbackCommand );
   this->MRMLScene->AddObserver( vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand );
   this->MRMLScene->AddObserver( vtkMRMLScene::SceneEditedEvent, this->MRMLCallbackCommand );
 **/
   }

  this->UpdateMenu();
}

//----------------------------------------------------------------------------
bool QtSlicerNodeSelectorWidget::CheckNodeClass(vtkMRMLNode *node)
{
  for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
    {
    const char *className = this->GetNodeClass(c);
    if (this->GetChildClassesEnabled() && node->IsA(className))
      {
      return true;
      }

    if (!this->GetChildClassesEnabled() && !strcmp(node->GetClassName(), className))
      {
      return true;
      }
    }
  return false;
}


//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::SetNodeClass(const char *className,
                                               const char *attName,
                                               const char *attValue,
                                               const char *nodeName)
{
  NodeClasses.clear();
  NodeNames.clear();
  AttributeNames.clear();
  AttributeValues.clear();

  const char *str = NULL;

  if (className)
    {
    NodeClasses.push_back(std::string(className));

    str = nodeName == NULL ? "" : nodeName;
    NodeNames.push_back(std::string(str));

    str = attName == NULL ? "" : attName;
    AttributeNames.push_back(std::string(str));

    str = attValue == NULL ? "" : attValue;
    AttributeValues.push_back(std::string(str));
    }

}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::AddNodeClass(const char *className,
                                               const char *attName,
                                               const char *attValue,
                                               const char *nodeName)
{
  const char *str = NULL;

  if (className)
    {
    NodeClasses.push_back(std::string(className));

    str = nodeName == NULL ? "" : nodeName;
    NodeNames.push_back(std::string(str));

    str = attName == NULL ? "" : attName;
    AttributeNames.push_back(std::string(str));

    str = attValue == NULL ? "" : attValue;
    AttributeValues.push_back(std::string(str));
    }

}
//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::ClearMenu()
{
  this->clear();
  this->EntryName_to_NodeID.clear();
  this->ClassTag_to_ClassName.clear();
  this->ClassTag_to_Class.clear();
}

//----------------------------------------------------------------------------
std::string QtSlicerNodeSelectorWidget::MakeEntryName(vtkMRMLNode *node)
{
  std::string entryName("");
  std::string nodeName("");
  if (node == NULL || node->GetName() == NULL)
    {
    return entryName;
    }
  else
    {
    nodeName = node->GetName();
    }
  entryName = nodeName;
  std::map<std::string, std::string>::iterator iter;
  int count = 1;
  for (iter = this->NodeID_to_EntryName.begin();
       iter != this->NodeID_to_EntryName.end();
       iter++)
    {
    if (iter->second == entryName)
      {
      std::stringstream ss;
      ss << nodeName << "_" << count++;
      entryName = ss.str();
      }
    }
  return entryName;
}

//----------------------------------------------------------------------------
std::string QtSlicerNodeSelectorWidget::FindEntryName(vtkMRMLNode *node)
{
  std::string entryName("");
  if (node == NULL || node->GetName() == NULL)
    {
    return entryName;
    }

  std::map<std::string, std::string>::iterator iter = this->NodeID_to_EntryName.find(node->GetID());

  if (iter != this->NodeID_to_EntryName.end() )
    {
    entryName = iter->second;
    }
  return entryName;
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::UpdateMenu()
{

  this->UnconditionalUpdateMenu();
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::UnconditionalUpdateMenu()
{
  if ( !this || !this->MRMLScene )
    {
    return;
    }

  if (this->NodeClasses.size() == 0)
    {
    return;
    }

  NodeID_to_EntryName.clear();
  EntryName_to_NodeID.clear();

  vtkMRMLNode *oldSelectedNode = this->GetSelected();
  std::string oldSelectedName = this->FindEntryName(oldSelectedNode);

  this->ClearMenu();

  int count = 0;
  int c=0;

  if (this->NewNodeEnabled)
    {
    for (c=0; c < this->GetNumberOfNodeClasses(); c++)
      {
      const char *name = this->GetNodeName(c);
      if (name == NULL || !strcmp(name, "") )
        {
        name = this->MRMLScene->GetTagByClassName(this->GetNodeClass(c));
        }

      std::stringstream ss;
      const char *node_class = this->GetNodeClass(c);
      const char *tag = this->MRMLScene->GetTagByClassName(node_class);
      if (!tag)
        {
        }
      else
        {
        ss << "Create New " << tag;

        // Build the command.  Since node name can contain spaces, we
        // need to quote the node name in the constructed Tcl command
        std::stringstream sc;
        sc << "ProcessNewNodeCommand " << this->GetNodeClass(c) << " \"" << name << "\"";
        this->addItem(ss.str().c_str());
        this->ClassTag_to_ClassName[std::string(tag)] = std::string(name);
        this->ClassTag_to_Class[std::string(tag)] = std::string(this->GetNodeClass(c));
        count++;
        //this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str() );
        //this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
        }
      }
    }

  if (this->NoneEnabled)
    {
    this->addItem("None");
    count++;
    //this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, "ProcessCommand None");
    }

  if (count)
    {
    this->insertSeparator(count++);
    }

  vtkMRMLNode *node = NULL;
  vtkMRMLNode *selectedNode = NULL;
  std::string selectedName;

  bool selected = false;
  int resultAddAdditionalNodes=this->AddAditionalNodes();
  count +=resultAddAdditionalNodes;
  for (c=0; c < this->GetNumberOfNodeClasses(); c++)
  {
    const char *className = this->GetNodeClass(c);
    this->MRMLScene->InitTraversal();
    while ( (node = this->MRMLScene->GetNextNodeByClass(className) ) != NULL)
      {
      if (!node->GetSelectable())
        {
        continue;
        }
      if (!this->ShowHidden && node->GetHideFromEditors())
        {
        continue;
        }

      if (!this->GetChildClassesEnabled() && strcmp(node->GetClassName(), className) != 0)
        {
        continue;
        }

      // If there is a Attribute Name-Value  specified, then only include nodes that
      // match both the NodeClass and Attribute
      if ((this->GetNodeAttributeName(c)== NULL  ||
          this->GetNodeAttributeValue(c)== NULL ||
          (node->GetAttribute( this->GetNodeAttributeName(c)) != NULL &&
          strcmp( node->GetAttribute( this->GetNodeAttributeName(c) ), this->GetNodeAttributeValue(c) ) == 0) )&&this->CheckAdditionalConditions(node))
        {
          std::stringstream sc;
          sc << "ProcessCommand " << node->GetID();

          std::string entryName = this->MakeEntryName(node);
          this->NodeID_to_EntryName[node->GetID()] = entryName.c_str();
          this->EntryName_to_NodeID[QString(entryName.c_str())]= std::string(node->GetID());
          
          this->addItem(entryName.c_str());

          // do we need a column break?
          if (count != 0 && count % 30 == 0)
            {
            //this->GetWidget()->GetWidget()->GetMenu()->SetItemColumnBreak(count, 1);
            }
          count++;
          //this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str());
          if (oldSelectedNode == node)
          {
            selectedNode = node;
            selected = true;
            selectedName = entryName;
          }
          //Only choose first one wenn the Additional Nodes didn't take care about this
          else if (!selected && !this->NoneEnabled&&(resultAddAdditionalNodes==0))
          {
            selectedNode = node;
            selected = true;
            selectedName = entryName;
          }
        }
     }
  }

  //Node already selected in additonal Nodes
  if(resultAddAdditionalNodes!=0&&selectedNode==NULL)
    {
    selectedNode=this->GetSelected();
    selectedName = this->FindEntryName(selectedNode);
    }
  if (selectedNode != NULL)
    {
    this->setCurrentIndex(this->findText (selectedName.c_str()));
    //this->GetWidget()->GetWidget()->SetValue(selectedName.c_str());
    this->SelectedID = std::string(selectedNode->GetID());
    }
  else
    {
    const char *name = "";
    if (this->NoneEnabled)
      {
      name = "None";
      }
    this->setCurrentIndex(this->findText (name));
    //this->GetWidget()->GetWidget()->SetValue(name);
    this->SelectedID = std::string(name);
    }

  // Add Context menu for operations on selected node
  // - first create it if needed
  // - then populate it based on the currently selected node
  if ( !this->ContextMenuHelper )
    {
    //this->ContextMenuHelper = vtkSlicerContextMenuHelper::New();
    //this->ContextMenuHelper->SetMRMLScene( this->GetMRMLScene() );
    }

  // vtkKWMenu *menu = this->GetWidget()->GetWidget()->GetMenu();
  //menu->AddSeparator();
  //this->ContextMenuHelper->SetMRMLNode(selectedNode);
  //this->ContextMenuHelper->SetContextMenu(menu);
  //this->ContextMenuHelper->PopulateMenu();
  //this->ContextMenuHelper->UpdateMenuState();

  if (oldSelectedNode != selectedNode)
    {
    if (selectedNode)
      {
      this->setCurrentIndex(this->findText (selectedName.c_str()));
      emit NodeSelected(QString(selectedNode->GetID()));
      }
    //this->InvokeEvent(QtSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
  if ( oldSelectedName != selectedName )
    {
    //this->InvokeEvent(QtSlicerNodeSelectorWidget::NodeRenameEvent, NULL);
    }
}
//----------------------------------------------------------------------------
vtkMRMLNode *QtSlicerNodeSelectorWidget::GetSelected()
{
  vtkMRMLNode *node = this->MRMLScene->GetNodeByID (this->SelectedID.c_str());
  if(node==NULL)
    {
    node=this->GetSelectedInAdditional();
    }
  return node;
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::ProcessNewNodeCommand(const char *className, const char *nodeName)
{
  vtkMRMLNode *node = NULL;
  vtkMRMLNode *retNode = NULL;

  if (className)
    {
    node = this->MRMLScene->CreateNodeByClass( className );
    if (node == NULL)
      {
      return;
      }

    // Invoke a new node event giving an observer an opportunity to
    // configure the node
    //this->InvokeEvent(QtSlicerNodeSelectorWidget::NewNodeEvent, node);

    node->SetScene(this->MRMLScene);

    std::stringstream ss;
    const char *name;
    if (nodeName == NULL || !strcmp(nodeName,"") )
      {
      name = this->MRMLScene->GetTagByClassName(className);
      }
    else
      {
      name = nodeName;
      }
    NewNodeCount++;
    ss << this->MRMLScene->GetUniqueNameByString(name);
    node->SetName(ss.str().c_str());

    // If there is a Attribute Name-Value specified, then set that
    // attribute on the node
    for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
      {
      if (!strcmp(this->GetNodeClass(c), className))
        {
        if (this->GetNodeAttributeName(c) != NULL)
          {
          node->SetAttribute(this->GetNodeAttributeName(c),
                             this->GetNodeAttributeValue(c));
          }
        break;
        }
      }

    // the ID is set in the call to AddNode
    //node->SetID(this->MRMLScene->GetUniqueIDByClass(className));
    retNode = this->MRMLScene->AddNode(node);
    node->Delete();
    }

  this->SetSelected(retNode);
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::Select(const QString &id)
{

  std::map<QString, std::string>::iterator iter = this->EntryName_to_NodeID.find(id);
  if (iter != this->EntryName_to_NodeID.end())
    {
    this->SelectedID = iter->second;
    emit NodeSelected(QString(iter->second.c_str()));
    }
  else if (id.contains("Create New", Qt::CaseSensitive))
    {
    QString tag = id.section(' ', 2, 2);
    std::string className = "";
    std::string name = "";
    std::map<std::string, std::string>::iterator iter = this->ClassTag_to_Class.find(tag.toStdString());
    if (iter != this->ClassTag_to_Class.end())
      {
      className = iter->second;
      }
    iter = this->ClassTag_to_ClassName.find(tag.toStdString());
    if (iter != this->ClassTag_to_ClassName.end())
      {
      name = iter->second;
      }

    this->ProcessNewNodeCommand(className.c_str(), name.c_str());
    }
  else if (id.contains("None", Qt::CaseSensitive))
    {
    emit NodeSelected(QString(""));
    }
}


//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::SetSelected(vtkMRMLNode *node)
{

  if ( node != NULL)
    {
    std::string name = this->FindEntryName(node);
    QString qname = name.c_str();
    this->SelectedID = std::string(node->GetID());

    if (this->currentText () == qname) 
      {
      return; // no change, don't propogate events
      }
    this->setCurrentIndex(this->findText (qname));

    // new value, set it and notify observers
    emit NodeSelected(QString(node->GetID()));

    //this->InvokeEvent(QtSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
  else
    {
    this->SelectedID = std::string("");
    //m->SetValue("");
    }

  if (this->ContextMenuHelper)
    {
    //this->ContextMenuHelper->SetMRMLNode(node);
    //this->ContextMenuHelper->UpdateMenuState();
    }
}

//----------------------------------------------------------------------------
void QtSlicerNodeSelectorWidget::SetSelectedNew(const char *className)
{
  if (this->NewNodeEnabled)
    {
    const char *name = this->MRMLScene->GetTagByClassName(className);
    std::stringstream ss;
    ss << "Create New " << name;
    this->setCurrentIndex(this->findText (ss.str().c_str()));
    //this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
    }
}


