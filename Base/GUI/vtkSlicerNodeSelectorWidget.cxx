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
#include "vtkKWMenuButton.h"
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
static void MRMLCallback(vtkObject *__mrmlscene, unsigned long eid, void *__clientData, void *callData)
{
  static int inMRMLCallback = 0;

  vtkSlicerNodeSelectorWidget *self = reinterpret_cast<vtkSlicerNodeSelectorWidget *>(__clientData);

  if (!strcmp(self->GetNodeClass(0), "vtkMRMLLinearTransformNode") )
  {
    int foo = 0;
  }

  if (inMRMLCallback)
    {
    vtkErrorWithObjectMacro (__mrmlscene, << "*********MRMLCallback called recursively?" << endl);
    return;
    }
  inMRMLCallback = 1;

  vtkMRMLScene *mrmlscene = static_cast<vtkMRMLScene *>(__mrmlscene); // Not used, since it is ivar

  self->UpdateMenu();

  inMRMLCallback = 0;
}


//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::vtkSlicerNodeSelectorWidget()
{
  this->NewNodeCount = 0;
  this->NewNodeEnabled = 0;
  this->NoneEnabled = 0;
  this->MRMLScene      = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);
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
    }

  this->UpdateMenu();
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
    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();

    vtkMRMLNode *oldSelectedNode = this->GetSelected();
    m->DeleteAllItems();
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
    this->MRMLScene->InitTraversal();
    bool selected = false;
    for (c=0; c < this->GetNumberOfNodeClasses(); c++)
    {
      const char *className = this->GetNodeClass(c);
      while ( (node = this->MRMLScene->GetNextNodeByClass(className) ) != NULL)
        {
        // If there is a Attribute Name-Value  specified, then only include nodes that
        // match both the NodeClass and Attribute
        if (this->GetNodeAttributeName(c)== NULL  ||
            this->GetNodeAttributeValue(c)== NULL ||
            (node->GetAttribute( this->GetNodeAttributeName(c)) != NULL &&
             strcmp( node->GetAttribute( this->GetNodeAttributeName(c) ), this->GetNodeAttributeValue(c) ) == 0) )
          {
            std::stringstream sc;
            sc << "ProcessCommand " << node->GetID();

            this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(node->GetName());
            this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, sc.str().c_str());
            if (oldSelectedNode == node)
            {
              selectedNode = node;
              selected = true;
            }
            else if (!selected && !this->NoneEnabled)
            {  
              selectedNode = node;
              selected = true;
            }
          }
       }
    }
    
    if (selectedNode != NULL)
      {
      this->GetWidget()->GetWidget()->SetValue(selectedNode->GetName());
      this->SelectedID = std::string(selectedNode->GetID());
      if (oldSelectedNode != selectedNode)
        {
        this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
        }
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
      this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
      }
      
}

//----------------------------------------------------------------------------
vtkMRMLNode *vtkSlicerNodeSelectorWidget::GetSelected()
{
  vtkMRMLNode *node = this->MRMLScene->GetNodeByID (this->SelectedID.c_str());
  return node;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ProcessNewNodeCommand(const char *className, const char *nodeName)
{
  vtkMRMLNode *node = NULL;
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
    ss << name << NewNodeCount++;
    node->SetName(ss.str().c_str());
    node->SetID(this->MRMLScene->GetUniqueIDByClass(className));
    this->MRMLScene->AddNode(node);
    node->Delete();
    }

  this->SetSelected(node);
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
  if ( node != NULL) 
    {
    vtkKWMenuButton *m = this->GetWidget()->GetWidget();
    if ( !strcmp ( m->GetValue(), node->GetName() ) )
      {
      return; // no change, don't propogate events
      }

    // new value, set it and notify observers
    m->SetValue(node->GetName());
    this->SetBalloonHelpString(node->GetName());
    this->SelectedID = std::string(node->GetID());
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
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
    this->SetBalloonHelpString("Create a new Node");
    }
}
//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

