/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __LoadableModuleDescription_h
#define __LoadableModuleDescription_h

#include "LoadableModuleWin32Header.h"

#include "vtkTclUtil.h"

#include <string>
#include <vector>

class vtkSlicerModuleGUI;
class vtkSlicerModuleLogic;

typedef void * (*TclInit)(Tcl_Interp*);

class LoadableModule_EXPORT LoadableModuleDescription
{
public:
  LoadableModuleDescription();
  LoadableModuleDescription(const LoadableModuleDescription &md);

  void operator=(const LoadableModuleDescription &md);

  void SetCategory(const std::string &cat)
  {
    this->Category = cat;
  }

  const std::string&  GetCategory() const
  {
    return this->Category;
  }
  
  void SetName(const std::string &name)
  {
    this->Name = name;
  }

  const std::string& GetName() const
  {
    return this->Name;
  }

  void SetShortName(const std::string &name)
  {
    this->ShortName = name;
  }

  const std::string& GetShortName() const
  {
    return this->ShortName;
  }

  void SetGUIName(const std::string &name)
  {
    this->GUIName = name;
  }

  const std::string& GetGUIName() const
  {
    return this->GUIName;
  }

  void SetTclInitName(const std::string &name)
  {
    this->TclInitName = name;
  }

  const std::string& GetTclInitName() const
  {
    return this->TclInitName;
  }

  void SetMessage(const std::string &message)
  {
    this->Message = message;
  }

  const std::string& GetMessage() const
  {
    return this->Message;
  }

  void SetGUIPtr(vtkSlicerModuleGUI* guiPtr)
  {
    this->GUIPtr = guiPtr;
  }

  vtkSlicerModuleGUI* GetGUIPtr() const
  {
    return this->GUIPtr;
  }
 
  void SetLogicPtr(vtkSlicerModuleLogic* logicPtr)
  {
    this->LogicPtr = logicPtr;
  }

  vtkSlicerModuleLogic* GetLogicPtr() const
  {
    return this->LogicPtr;
  }

  void SetTclInitFunction(TclInit functionPtr)
  {
    this->TclInitFunction = functionPtr;
  }

  TclInit GetTclInitFunction() const
  {
    return this->TclInitFunction;
  }

  void AddDependency(const std::string& dependency)
  {
    this->Dependencies.push_back(dependency);
  }

  std::vector<std::string> GetDependencies() const
  {
    return this->Dependencies;
  }

  void SetDescription(const std::string &description)
  {
    this->Description = description;
  }

  const std::string& GetDescription() const
  {
    return this->Description;
  }

  void SetVersion(const std::string &version)
  {
    this->Version = version;
  }

  const std::string& GetVersion() const
  {
    return this->Version;
  }

  void SetDocumentationURL(const std::string &documentationURL)
  {
    this->DocumentationURL = documentationURL;
  }

  const std::string& GetDocumentationURL() const
  {
    return this->DocumentationURL;
  }

  void SetLicense(const std::string &license)
  {
    this->License = license;
  }

  const std::string& GetLicense() const
  {
    return this->License;
  }

  void SetAcknowledgements(const std::string &acknowledgements)
  {
    this->Acknowledgements = acknowledgements;
  }

  const std::string& GetAcknowledgements() const
  {
    return this->Acknowledgements;
  }

  void SetContributor(const std::string &contributor)
  {
    this->Contributor = contributor;
  }

  const std::string& GetContributor() const
  {
    return this->Contributor;
  }

  // Set the type of module: Unknown, SharedObjectModule, CommandLineModule
  void SetType(const std::string &type)
  {
    if (type == "SharedObjectModule" 
        || type == "CommandLineModule"
        || type == "PythonModule")
      {
      this->Type = type;
      }
    else
      {
      this->Type = "Unknown";
      }
  }

  
  // Get the type of the module: Unknown, SharedObjectModule, CommandLineModule
  const std::string& GetType() const
  {
    return this->Type;
  }

  // Set the type of an alternative version of the module: Unknown,
  // SharedObjectModule, CommandLineModule. The alternative version is
  // usually a different type than the primary version.
  void SetAlternativeType(const std::string &type)
  {
    if (type == "SharedObjectModule" 
        || type == "CommandLineModule"
        || type == "PythonModule")
      {
      this->AlternativeType = type;
      }
    else
      {
      this->AlternativeType = "Unknown";
      }
  }

  
  // Get the type of an alternative version of the module: Unknown,
  // SharedObjectModule, CommandLineModule. The alternative version is
  // usually a different type than the primary version.
  const std::string& GetAlternativeType() const
  {
    return this->AlternativeType;
  }
  
  // Set the target for the module.  This is the entry point for a
  // shared object module and the full command (with path) for an executable.
  void SetTarget(const std::string &target)
  {
    this->Target = target;
  }

  // Get the target for the module.  This is the entry point for a
  // shared object module and the full command (with path) for an executable.
  const std::string& GetTarget() const
  {
    return this->Target;
  }

  // Set the alternative target for the module.  This is the entry
  // point for a shared object module and the full command (with path)
  // for an executable. The alternative target is used for a second version
  // of a module (whose type differs from the primary target,
  // executable verses shared object).
  void SetAlternativeTarget(const std::string &target)
  {
    this->AlternativeTarget = target;
  }

  // Get the alternative target for the module.  This is the entry for a
  // shared object module and the full command (with path) for an
  // executable. The alternative target is used for a second version
  // of a module (whose type differs from the primary target,
  // executable verses shared object).
  const std::string& GetAlternativeTarget() const
  {
    return this->AlternativeTarget;
  }

  // Set the location for the module.  This is path to the file (shared
  // object or executable) for the module.
  void SetLocation(const std::string &target)
  {
    this->Location = target;
  }

  // Get the location for the module.  This is path to the file (shared
  // object or executable) for the module.
  const std::string& GetLocation() const
  {
    return this->Location;
  }

  // Set the alternative location for the module.  This is path to the
  // file (shared object or executable) for a second version of the
  // module (usually a different type from the primary).
  void SetAlternativeLocation(const std::string &target)
  {
    this->AlternativeLocation = target;
  }

  // Get the alternative location for the module.  This is path to the
  // file (shared object or executable) for a second version of the
  // module (usually a different type from the primary).
  const std::string& GetAlternativeLocation() const
  {
    return this->AlternativeLocation;
  }
  private:
  std::string Name;
  std::string ShortName;
  std::string GUIName;
  std::string TclInitName;

  std::string Message;

  vtkSlicerModuleGUI* GUIPtr;
  vtkSlicerModuleLogic* LogicPtr;

  TclInit TclInitFunction;

  std::vector<std::string> Dependencies;

  std::string Category;
  std::string Description;
  std::string Version;
  std::string DocumentationURL;
  std::string License;
  std::string Acknowledgements;
  std::string Contributor;
  std::string Type;
  std::string Target;
  std::string Location;
  std::string AlternativeType;
  std::string AlternativeTarget;
  std::string AlternativeLocation;
};

LoadableModule_EXPORT std::ostream & operator<<(std::ostream &os, const LoadableModuleDescription &module);

#endif
