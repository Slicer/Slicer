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
    cout << "*********MRMLCallback called recursively?" << endl;
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

    m->DeleteAllItems();

    if (this->NewNodeEnabled)
    {
      this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton("Create New");
      this->GetWidget()->GetWidget()->SetValue("Create New");
    }
    bool selected = false;
    vtkMRMLNode *node;
    this->MRMLScene->InitTraversal();
    while ( (node = this->MRMLScene->GetNextNodeByClass(this->NodeClass) ) != NULL)
      {
      this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(node->GetName());
      if (!selected)
      {
        this->GetWidget()->GetWidget()->SetValue(node->GetName());
        selected = true;
      }
    }


}

//----------------------------------------------------------------------------
vtkMRMLNode *vtkSlicerNodeSelectorWidget::GetSelected()
{
  vtkMRMLNode *n = NULL;
  vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();

  if (strcmp(mb->GetValue(), "Create New") == 0)
    {
    vtkMRMLNode* node = this->MRMLScene->CreateNodeByClass( this->NodeClass );
    node->SetScene(this->MRMLScene);
    node->SetID(this->MRMLScene->GetUniqueIDByClass(this->NodeClass));
    this->MRMLScene->AddNode(node);
    }
  else if (m != NULL ) 
    {  
      int nth_rank = m->GetIndexOfItem(mb->GetValue());
      n = this->MRMLScene->GetNthNodeByClass (nth_rank, this->NodeClass);
    }
  return n;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelected(vtkMRMLNode *node)
{
  if (node && this->NodeClass == node->GetClassName()) 
    {
    this->GetWidget()->GetWidget()->SetValue(node->GetName());
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

