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

#include "vtkSlicerApplication.h"

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
  this->Language = vtkScriptedModuleGUI::Tcl;
}

//----------------------------------------------------------------------------
vtkScriptedModuleGUI::~vtkScriptedModuleGUI()
{

  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sDestructor %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].Destructor('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }

  this->RemoveMRMLNodeObservers();
  this->RemoveLogicObservers();

  this->SetLogic (NULL);
  this->SetScriptedModuleNode (NULL);
  this->SetModuleName (NULL);
}


void vtkScriptedModuleGUI::RemoveMRMLNodeObservers()
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sRemoveMRMLNodeObservers %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].RemoveMRMLNodeObservers('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}


void vtkScriptedModuleGUI::RemoveLogicObservers()
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sRemoveLogicObservers %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].RemoveLogicObservers('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}


//----------------------------------------------------------------------------
void vtkScriptedModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ModuleName: " << (this->ModuleName ? this->ModuleName : "null") << endl;
  os << indent << "Logic: " << endl;
  if (this->Logic)
    {
    this->Logic->PrintSelf(os, indent.GetNextIndent());
    }

  os << indent << "ScriptedModuleNode: " << endl;
  if (this->ScriptedModuleNode)
    {
    this->ScriptedModuleNode->PrintSelf(os, indent.GetNextIndent());
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::AddGUIObservers ( ) 
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sAddGUIObservers %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].AddGUIObservers('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}



//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::RemoveGUIObservers ( )
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sRemoveGUIObservers %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].RemoveGUIObservers('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  vtkKWObject *kwObject = vtkKWObject::SafeDownCast(caller);

  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (kwObject != NULL)
      {
      this->GetApplication()->Script("%sProcessGUIEvents %s %s %ld", 
        this->GetModuleName(), this->GetTclName(), kwObject->GetTclName(), event);
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].ProcessGUIEvents('" << this->GetTclName() << "','" << kwObject->GetTclName() << "'," << event << ")\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::UpdateMRML ()
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sUpdateMRML %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].UpdateMRML('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::UpdateGUI ()
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sUpdateGUI %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].UpdateGUI('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  vtkDebugMacro("ProcessMRMLEvents()");
  vtkMRMLNode *mrmlNode = vtkMRMLNode::SafeDownCast(caller);

  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (mrmlNode != NULL)
      {
      vtkDebugMacro("vtkScriptedModuleGUI::ProcessMRMLEvents: calling script " <<  this->GetModuleName() << "ProcessMRMLEvents with event " << event);
      this->GetApplication()->Script("%sProcessMRMLEvents %s %s %ld", 
                                     this->GetModuleName(), this->GetTclName(), mrmlNode->GetID(), event);
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].ProcessMRMLEvents('" << this->GetTclName() << "','" << mrmlNode->GetID() << "'," << event << ")\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
  
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
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sBuildGUI %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].BuildGUI('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    }
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::TearDownGUI ( ) 
{
  if (this->Language == vtkScriptedModuleGUI::Tcl)
    {
    if (this->GetApplication())
      {
      this->GetApplication()->Script("%sTearDownGUI %s", 
        this->GetModuleName(), this->GetTclName());
      }
    }
  else if (this->Language == vtkScriptedModuleGUI::Python)
    {
#ifdef USE_PYTHON
    std::stringstream pythonCommand;
    pythonCommand << "PythonScriptedModuleDict['" << this->GetModuleName() << "'].TearDownGUI('" << this->GetTclName() << "')\n";
    if (PyRun_SimpleString( pythonCommand.str().c_str() ) != 0)
      {
      PyErr_Print();
      }
#endif
    } 
}


//---------------------------------------------------------------------------
unsigned long vtkScriptedModuleGUI::AddObserverByNumber ( vtkObject *observee, unsigned long event ) {

  return ( observee->AddObserver(event, (vtkCommand *)this->GUICallbackCommand) );
} 


//---------------------------------------------------------------------------
unsigned long vtkScriptedModuleGUI::AddMRMLObserverByNumber ( vtkObject *observee, unsigned long event )
{
  if (observee == NULL)
    {
    vtkErrorMacro("AddMRMLObserverByNumber: observee is null, returning invalid event tag");
    return VTK_UNSIGNED_LONG_MAX;
    }
  if (observee->HasObserver(event, (vtkCommand *)this->MRMLCallbackCommand) )
    {
    return event;
    }
  return ( observee->AddObserver(event, (vtkCommand *)this->MRMLCallbackCommand) );
}

//---------------------------------------------------------------------------
void vtkScriptedModuleGUI::RemoveMRMLObserverByNumber ( vtkObject *observee, unsigned long event )
{
  if (observee == NULL)
    {
    vtkErrorMacro("RemoveMRMLObserverByNumber: observee is null");
    return;
    }
  vtkDebugMacro("RemoveMRMLObserverByNumber: event = " << event);
  if (observee->HasObserver(event, (vtkCommand *)this->MRMLCallbackCommand))
    {
    observee->RemoveObservers(event, (vtkCommand *)this->MRMLCallbackCommand);
    }
}
