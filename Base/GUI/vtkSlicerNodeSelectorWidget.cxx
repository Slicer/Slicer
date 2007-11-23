/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerNodeSelectorWidget.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkSlicerNodeSelectorWidget.h"


#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithSpinButtons.h"

#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerNodeSelectorWidget );
vtkCxxRevisionMacro(vtkSlicerNodeSelectorWidget, "$Revision: 1.33 $");


//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml scene back into the logic layer for further processing
// - this can also end up calling observers of the logic (i.e. in the GUI)
//
static void MRMLCallback(vtkObject *caller, unsigned long eid, void *__clientData, void *callData)
{
  vtkSlicerNodeSelectorWidget *self = reinterpret_cast<vtkSlicerNodeSelectorWidget *>(__clientData);

  /* don't clear menu on scene close since the singleton nodes are not recreated anymore
  if (vtkMRMLScene::SceneCloseEvent == eid)
    {
    self->ClearMenu();
    self->SetSelected(NULL);
    return;
    }
  */
  if (self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerNodeSelectorWidget *********MRMLCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerWidget MRMLCallback");

  vtkMRMLNode *node = reinterpret_cast<vtkMRMLNode *>(callData);

  self->SetInMRMLCallbackFlag(1);

  if (node == NULL || self->CheckNodeClass(node))
    {
    self->UpdateMenu();
    }

  self->SetInMRMLCallbackFlag(0);
}


//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::vtkSlicerNodeSelectorWidget()
{
  this->NewNodeCount = 0;
  this->NewNodeEnabled = 0;
  this->NoneEnabled = 0;
  this->ShowHidden = 0;
  this->ChildClassesEnabled = 1;
  this->MRMLScene      = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);
  this->InMRMLCallbackFlag = 0;
}

//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::~vtkSlicerNodeSelectorWidget()
{
  this->SetMRMLScene ( NULL );
  if (this->MRMLCallbackCommand)
    {
    this->MRMLCallbackCommand->Delete();
    this->MRMLCallbackCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetMRMLScene( vtkMRMLScene *MRMLScene)
{
  if ( this->MRMLScene )
    {
    this->MRMLScene->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene->Delete ( );
    this->MRMLScene = NULL;
    //    this->MRMLScene->Delete();
    }
  
  this->MRMLScene = MRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::NewSceneEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand );
    }

  this->UpdateMenu();
}

//----------------------------------------------------------------------------
bool vtkSlicerNodeSelectorWidget::CheckNodeClass(vtkMRMLNode *node)
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
void vtkSlicerNodeSelectorWidget::SetNodeClass(const char *className,
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
void vtkSlicerNodeSelectorWidget::AddNodeClass(const char *className,
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
void vtkSlicerNodeSelectorWidget::ClearMenu()
{
    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();
    m->DeleteAllItems();
}

//----------------------------------------------------------------------------
std::string vtkSlicerNodeSelectorWidget::MakeEntryName(vtkMRMLNode *node)
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
std::string vtkSlicerNodeSelectorWidget::FindEntryName(vtkMRMLNode *node)
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
void vtkSlicerNodeSelectorWidget::UpdateMenu()
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

  vtkMRMLNode *oldSelectedNode = this->GetSelected();
  std::string oldSelectedName = this->FindEntryName(oldSelectedNode);

  this->ClearMenu();

  vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();

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
      ss << "Create New " << this->MRMLScene->GetTagByClassName(this->GetNodeClass(c));

      // Build the command.  Since node name can contain spaces, we
      // need to quote the node name in the constructed Tcl command
      std::stringstream sc;
      sc << "ProcessNewNodeCommand " << this->GetNodeClass(c) << " \"" << name << "\"";

      this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(ss.str().c_str());
      this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str() );
      this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
    }
  }

  if (this->NoneEnabled) 
    {
    this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton("None");
    this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, "ProcessCommand None");
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
          this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(entryName.c_str());
          // do we need a column break?
          if (count != 0 && count % 30 == 0)
            {
            this->GetWidget()->GetWidget()->GetMenu()->SetItemColumnBreak(count, 1);
            }
          this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str());
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
    this->GetWidget()->GetWidget()->SetValue(selectedName.c_str());
    this->SelectedID = std::string(selectedNode->GetID());
    }
  else
    {
    char *name = "";
    if (this->NoneEnabled)
      {
      name = "None";
      }
    this->GetWidget()->GetWidget()->SetValue(name);
    this->SelectedID = std::string(name);
    }

  if (oldSelectedNode != selectedNode)
    {
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode *vtkSlicerNodeSelectorWidget::GetSelected()
{
  vtkMRMLNode *node = this->MRMLScene->GetNodeByID (this->SelectedID.c_str());
  if(node==NULL)
  {
     node=this->GetSelectedInAdditional();
  }
  return node;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ProcessNewNodeCommand(const char *className, const char *nodeName)
{
  vtkMRMLNode *node = NULL;
  vtkMRMLNode *retNode = NULL;
  vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();

  if (className)
    {
    node = this->MRMLScene->CreateNodeByClass( className );
    if (node == NULL)
    {
      return;
    }
    // Invoke a new node event giving an observer an opportunity to
    // configure the node
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NewNodeEvent, node);

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
//    ss << name << NewNodeCount;
    vtkDebugMacro("Node selector process new node, new node count is " << NewNodeCount);
    ss << this->MRMLScene->GetUniqueNameByString(name);
    node->SetName(ss.str().c_str());
    vtkDebugMacro("\tset the name to " << node->GetName() << endl);

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
void vtkSlicerNodeSelectorWidget::ProcessCommand(char *slectedId)
{
  this->SelectedID = std::string(slectedId);

  this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
}


//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelected(vtkMRMLNode *node)
{
 
  vtkKWMenuButton *m = this->GetWidget()->GetWidget();

  if ( node != NULL) 
    {
    std::string name = this->FindEntryName(node);
    if ( !strcmp ( m->GetValue(), name.c_str() ) )
      {
      return; // no change, don't propogate events
      }

    // new value, set it and notify observers
    this->SetBalloonHelpString(node->GetName());
    this->SelectedID = std::string(node->GetID());
    m->SetValue(name.c_str());
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
  else 
    {
    this->SelectedID = std::string("");
    m->SetValue("");
    this->SetBalloonHelpString("Select a node");
    }
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelectedNew(const char *className)
{
  if (this->NewNodeEnabled) 
    {
    const char *name = this->MRMLScene->GetTagByClassName(className);
    std::stringstream ss;
    ss << "Create New " << name;
    this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
    this->SetBalloonHelpString("Create a new node");
    }
}
//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

