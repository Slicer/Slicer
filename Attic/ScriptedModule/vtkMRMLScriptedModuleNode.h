/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLScriptedModuleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLScriptedModuleNode_h
#define __vtkMRMLScriptedModuleNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkScriptedModule.h"

// Description: 
// The scripted module node is simply a MRMLNode container for 
// an arbitrary keyword value pair map

class VTK_SCRIPTED_EXPORT vtkMRMLScriptedModuleNode : public vtkMRMLNode
{
  public:
  static vtkMRMLScriptedModuleNode *New();
  vtkTypeMacro(vtkMRMLScriptedModuleNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "ScriptedModule";};

  // Description: The name of the Module - this is used to 
  // customize the node selectors and other things
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  // Description:
  // Get/Set a parameter for the module.
//BTX
  void SetParameter(const std::string& name, const std::string& value);
  const std::string* GetParameter(const std::string &name) const;
//ETX  

  void SetParameter(const char *name, const char *value);
  void RequestParameter(const char *name);
  const char *GetParameter(const char *name);
  vtkSetStringMacro(Value);
  vtkGetStringMacro(Value);
  // Description:
  // Parameter List is a space separated keyword/value pair string.
  // Note: you must call RequestParameterList first to update the value
  void RequestParameterList();
  vtkGetStringMacro(ParameterList);


protected:
  // to be used internally
  vtkSetStringMacro(ParameterList);

//BTX
  std::map<std::string, std::string> Parameters;
//ETX
  char *ParameterList;
  char *Value;

  char *ModuleName;
  
private:
  vtkMRMLScriptedModuleNode();
  ~vtkMRMLScriptedModuleNode();
  vtkMRMLScriptedModuleNode(const vtkMRMLScriptedModuleNode&);
  void operator=(const vtkMRMLScriptedModuleNode&);

};

#endif

