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

  if (inMRMLCallback)
    {
    vtkErrorWithObjectMacro (__mrmlscene, << "*********MRMLCallback called recursively?" << endl);
    return;
    }
  inMRMLCallback = 1;

  vtkMRMLScene *mrmlscene = static_cast<vtkMRMLScene *>(__mrmlscene); // Not used, since it is ivar

  vtkSlicerNodeSelectorWidget *self = reinterpret_cast<vtkSlicerNodeSelectorWidget *>(__clientData);

  self->UpdateMenu();

  inMRMLCallback = 0;
}


//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::vtkSlicerNodeSelectorWidget()
{
  this->NodeClass = NULL;
  this->NewNodeName = NULL;
  this->NewNodeCount = 0;
  this->NewNodeEnabled = 0;
  this->MRMLScene      = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);
}

//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::~vtkSlicerNodeSelectorWidget()
{
  this->SetNodeClass(NULL);
  this->SetNewNodeName(NULL);
  if (this->MRMLScene)
    {
    this->MRMLScene->Delete();
    this->MRMLScene = NULL;
    }
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
    this->MRMLScene->Delete();
    }
  
  this->MRMLScene = MRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  this->UpdateMenu();
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::UpdateMenu()
{
    if (this->NodeClass == NULL)
      {
      return;
      }
    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();

    vtkMRMLNode *oldSelectedNode = this->GetSelected();
    m->DeleteAllItems();
    int count = 0;
    
    if (this->NewNodeEnabled)
    {
      this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton("Create New");
      this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, "ProcessNewNodeCommand");
      this->GetWidget()->GetWidget()->SetValue("Create New");
    }

    vtkMRMLNode *node = NULL;
    vtkMRMLNode *selectedNode = NULL;
    this->MRMLScene->InitTraversal();
    bool selected = false;
    while ( (node = this->MRMLScene->GetNextNodeByClass(this->NodeClass) ) != NULL)
      {
      this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(node->GetName());
      this->GetWidget()->GetWidget()->GetMenu()->SetItemCommand(count++, this, "ProcessCommand");
      if (oldSelectedNode == node)
      {
        selectedNode = node;
        selected = true;
      }
      else if (!selected)
      {  
        selectedNode = node;
        selected = true;
      }
    }

    if (selectedNode != NULL)
    {
    this->GetWidget()->GetWidget()->SetValue(selectedNode->GetName());
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode *vtkSlicerNodeSelectorWidget::GetSelected()
{
  vtkMRMLNode *node = NULL;
  vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();

 if (m != NULL ) 
    {  
      int nth_rank = m->GetIndexOfItem(mb->GetValue());
      if (this->NewNodeEnabled) nth_rank--;
      node = this->MRMLScene->GetNthNodeByClass (nth_rank, this->NodeClass);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ProcessNewNodeCommand()
{
  vtkMRMLNode *node = NULL;
  vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();

  if (strcmp(mb->GetValue(), "Create New") == 0)
    {
    node = this->MRMLScene->CreateNodeByClass( this->NodeClass );
    if (node == NULL)
    {
      return;
    }
    node->SetScene(this->MRMLScene);
    std::stringstream ss;
    if (this->NewNodeName)
      {
      ss << this->NewNodeName << NewNodeCount++;
      }
    else
      {
      ss << this->NodeClass << NewNodeCount++;
      }
    
      node->SetName(ss.str().c_str());
      node->SetID(this->MRMLScene->GetUniqueIDByClass(this->NodeClass));
      this->MRMLScene->AddNode(node);
    }
    this->SetSelected(node);
}


//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ProcessCommand()
{
  this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
}


//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelected(vtkMRMLNode *node)
{
  if ( node != NULL && node->IsA( this->NodeClass ) ) 
    {
    this->GetWidget()->GetWidget()->SetValue(node->GetName());
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelectedNew()
{
  if (this->NewNodeEnabled) 
    {
    this->GetWidget()->GetWidget()->SetValue("Create New");
    }
}
//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MRMLScene: " << this->MRMLScene << endl;
  os << indent << "Node Class: " << 
      ((this->NodeClass) ? this->NodeClass : "NULL") << endl;
}

