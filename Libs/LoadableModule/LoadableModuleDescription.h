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

  LoadableModuleDescription& operator=(const LoadableModuleDescription &md);
  
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

  void AddDependency(const std::string &depend)
  {
    this->Dependencies.push_back(depend);
  }

  const std::vector<std::string>& GetDependencies() const
  {
    return this->Dependencies;
  }

  void SetType(const std::string &type)
  {
    this->Type = type;
  }

  const std::string& GetType() const
  {
    return this->Type;
  }

  void SetTarget(const std::string &target)
  {
    this->Target = target;
  }

  const std::string& GetTarget() const
  {
    return this->Target;
  }

  void SetLocation(const std::string &location)
  {
    this->Location = location;
  }

  const std::string& GetLocation() const
  {
    return this->Location;
  }

  void SetAlternativeType(const std::string &type)
  {
    this->AlternativeType = type;
  }

  const std::string& GetAlternativeType() const
  {
    return this->AlternativeType;
  }

  const std::string& GetAlternativeTarget() const
  {
    return this->AlternativeTarget;
  }

  void SetAlternativeTarget(const std::string &target)
  {
    this->AlternativeTarget = target;
  }

  void SetAlternativeLocation(const std::string &location)
  {
    this->AlternativeLocation = location;
  }

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

  std::string Type;
  std::string Target;
  std::string Location;

  std::string AlternativeType;
  std::string AlternativeTarget;
  std::string AlternativeLocation;

};

LoadableModule_EXPORT std::ostream & operator<<(std::ostream &os, const LoadableModuleDescription &module);

#endif
