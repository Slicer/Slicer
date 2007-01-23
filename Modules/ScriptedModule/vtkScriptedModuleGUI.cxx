/*==========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: http://www.na-mic.org:8000/svn/Slicer3/trunk/Modules/ScriptedModule/vtkScriptedModuleGUI.cxx $
Date:      $Date: 2006-07-31 22:32:34 -0400 (Mon, 31 Jul 2006) $
Version:   $Revision: 979 $

==========================================================================*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkScriptedModuleGUI.h"

//------------------------------------------------------------------------------
vtkScriptedModuleGUI* vtkScriptedModuleGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkScriptedModuleGUI");
  if(ret)
    {
      return (vtkScriptedModuleGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkScriptedModuleGUI;
}


//----------------------------------------------------------------------------
vtkScriptedModuleGUI::vtkScriptedModuleGUI()
{
  this->Logic = NULL;
  this->ScriptedModuleNode = NULL;
  this->ModuleName = NULL;
}

//----------------------------------------------------------------------------
vtkScriptedModuleGUI::~vtkScriptedModuleGUI()
{

  this->RemoveMRMLNodeObservers();
  this->RemoveLogicObservers();

  this->SetLogic (NULL);
  this->SetScriptedModuleNode (NULL);
  this->SetModuleName (NULL);
}


void vtkScriptedModuleGUI::RemoveMRMLNodeObservers()
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sRemoveMRMLNodeObservers %s", 
      this->GetModuleName(), this->GetTclName());
    }
}


void vtkScriptedModuleGUI::RemoveLogicObservers()
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sRemoveLogicObservers %s", 
      this->GetModuleName(), this->GetTclName());
    }
}


//----------------------------------------------------------------------------
void vtkScriptedModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::AddGUIObservers ( ) 
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sAddGUIObservers %s", 
      this->GetModuleName(), this->GetTclName());
    }
}



//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::RemoveGUIObservers ( )
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sRemoveGUIObservers %s", 
      this->GetModuleName(), this->GetTclName());
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  vtkKWObject *kwObject = vtkKWObject::SafeDownCast(caller);

  if (kwObject != NULL)
    {
    this->GetApplication()->Script("%sProcessGUIEvents %s %s %ld", 
      this->GetModuleName(), this->GetTclName(), kwObject->GetTclName(), event);
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::UpdateMRML ()
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sUpdateMRML %s", 
      this->GetModuleName(), this->GetTclName());
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::UpdateGUI ()
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sUpdateGUI %s", 
      this->GetModuleName(), this->GetTclName());
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // TODO: map the object and event to strings for tcl
  
  //std::cout << "ProcessMRMLEvents()" << std::endl;
  // if parameter node has been changed externally, update GUI widgets
  // with new values 
  vtkMRMLScriptedModuleNode* node
    = vtkMRMLScriptedModuleNode::SafeDownCast(caller);
  if (node != NULL && this->GetScriptedModuleNode() == node) 
    {
    this->UpdateGUI();
    }
}




//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::BuildGUI ( ) 
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sBuildGUI %s", 
      this->GetModuleName(), this->GetTclName());
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::TearDownGUI ( ) 
{
  if (this->GetApplication())
    {
    this->GetApplication()->Script("%sTearDownGUI %s", 
      this->GetModuleName(), this->GetTclName());
    }
}


//---------------------------------------------------------------------------
unsigned long vtkScriptedModuleGUI::AddObserverByNumber ( vtkObject *observee, unsigned long event ) {

  return ( observee->AddObserver(event, (vtkCommand *)this->GUICallbackCommand) );
} 


