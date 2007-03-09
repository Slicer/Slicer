/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLWFEngineModuleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLWFEngineModuleNode_h
#define __vtkMRMLWFEngineModuleNode_h

#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkWFEngineModule.h"

#include <string>

class vtkImageData;
//VTK_WFENGINEMODULE_EXPORT
class VTK_WFENGINEMODULE_EXPORT vtkMRMLWFEngineModuleNode : public vtkMRMLNode
{
public:
  static vtkMRMLWFEngineModuleNode *New();
  vtkTypeMacro(vtkMRMLWFEngineModuleNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "WFEngineModuleParameters";};

  // Description:
  // Set new workflow-instance
  int SetNewWorkflowInstance();
  
//  void SetAttribute(const char* name, const char* value);
  // Description:
  // Get/Set a parameter for the workflow.
//BTX
  void SetParameterAsString(const std::string &stepID, const std::string& name, const std::string& value);
  void SetParameterAsInt(const std::string &stepID, const std::string& name, int value);
  void SetParameterAsBool(const std::string &stepID, const std::string& name, bool value);
  void SetParameterAsDouble(const std::string &stepID, const std::string& name, double value);
  void SetParameterAsFloat(const std::string &stepID, const std::string& name, float value);

  void GetMRMLNodeByID(std::string stepID);
  
  std::string GetParameterAsString(const std::string &name);
  const char *GetValueByName(std::string paramName);
//ETX
  
protected:
  vtkMRMLWFEngineModuleNode();
  ~vtkMRMLWFEngineModuleNode();
  vtkMRMLWFEngineModuleNode(const vtkMRMLWFEngineModuleNode&);
  void operator=(const vtkMRMLWFEngineModuleNode&);

private:
//BTX
    std::string m_curStepID;
//ETX
};

#endif

