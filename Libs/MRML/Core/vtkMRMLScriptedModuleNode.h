/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLScriptedModuleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLScriptedModuleNode_h
#define __vtkMRMLScriptedModuleNode_h

// MRML includes
#include "vtkMRMLNode.h"

// STD includes
#include <string>
#include <vector>

/// The scripted module node is simply a MRMLNode container for
/// an arbitrary keyword value pair map
class VTK_MRML_EXPORT vtkMRMLScriptedModuleNode : public vtkMRMLNode
{
public:
  static vtkMRMLScriptedModuleNode *New();
  vtkTypeMacro(vtkMRMLScriptedModuleNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLScriptedModuleNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override;

  /// The name of the Module - this is used to
  /// customize the node selectors and other things
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  /// Set module parameter
  void SetParameter(const std::string& name, const std::string& value);

  /// Unset the parameter identified by \a name
  void UnsetParameter(const std::string& name);

  /// Unset all parameters
  /// \sa UnsetParameter
  void UnsetAllParameters();

  /// Get module parameter identified by \a name
  std::string GetParameter(const std::string& name) const;

  /// Get number of parameters
  int GetParameterCount();

  /// Get list of parameter names separated by a comma
  /// \sa GetParameterNames
  std::string GetParameterNamesAsCommaSeparatedList();

  /// Get list of parameter names
  std::vector<std::string> GetParameterNames();

protected:
  vtkMRMLScriptedModuleNode();
  ~vtkMRMLScriptedModuleNode() override;

  vtkMRMLScriptedModuleNode(const vtkMRMLScriptedModuleNode&);
  void operator=(const vtkMRMLScriptedModuleNode&);

  typedef std::map<std::string, std::string> ParameterMap;
  ParameterMap Parameters;
  char *ModuleName;
};

#endif
