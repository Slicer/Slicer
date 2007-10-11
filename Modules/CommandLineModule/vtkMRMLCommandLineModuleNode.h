/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCommandLineModuleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLCommandLineModuleNode_h
#define __vtkMRMLCommandLineModuleNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

#include "vtkCommandLineModule.h"

#include "ModuleDescription.h"

//BTX
class ModuleDescriptionMap;
//ETX

class VTK_COMMANDLINEMODULE_EXPORT vtkMRMLCommandLineModuleNode : public vtkMRMLNode
{
  public:
  static vtkMRMLCommandLineModuleNode *New();
  vtkTypeMacro(vtkMRMLCommandLineModuleNode,vtkMRMLNode);
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
    {return "CommandLineModule";};

  // Description:
  // Get/Set the module description object. THe module description
  // object is used to cache the current settings for the module.
  const ModuleDescription& GetModuleDescription() const
    { return ModuleDescriptionObject; }
  ModuleDescription& GetModuleDescription()
    { return ModuleDescriptionObject; }
  void SetModuleDescription(const ModuleDescription& description);


  //BTX
  typedef enum { Idle=0, Scheduled, Running, Completed, CompletedWithErrors, Cancelled } StatusType;
  //ETX

  // Description:
  // Set the status of the node (Idle, Scheduled, Running,
  // Completed).  The "modify" parameter indicates whether the object
  // can be modified by the call.
  //BTX
  void SetStatus(StatusType status, bool modify=true);
  StatusType GetStatus();
  //ETX


  // Description:
  // Get/Set a parameter for the module.
//BTX
  void SetParameterAsString(const std::string& name, const std::string& value);
  void SetParameterAsInt(const std::string& name, int value);
  void SetParameterAsBool(const std::string& name, bool value);
  void SetParameterAsDouble(const std::string& name, double value);
  void SetParameterAsFloat(const std::string& name, float value);

  std::string GetParameterAsString(const std::string &name) const;
//ETX  
  void SetParameterAsString(const char *name, const char *value)
    {this->SetParameterAsString(std::string(name), std::string(value));}
  void SetParameterAsInt(const char *name, const int value)
    {this->SetParameterAsInt(std::string(name), value);}
  void SetParameterAsBool(const char *name, const bool value)
    {this->SetParameterAsBool(std::string(name), value);}
  void SetParameterAsDouble(const char *name, const double value)
    {this->SetParameterAsDouble(std::string(name), value);}
  void SetParameterAsFloat(const char *name, const float value)
    {this->SetParameterAsFloat(std::string(name), value);}
  
  // Description:
  // Methods to manage the master list of module description prototypes
//BTX
  static void RegisterModuleDescription(ModuleDescription md);
  static bool HasRegisteredModule(const std::string& name);
  static ModuleDescription GetRegisteredModuleDescription(const std::string& name);
  static void ClearRegisteredModules();
// ETX
  
private:
  vtkMRMLCommandLineModuleNode();
  ~vtkMRMLCommandLineModuleNode();
  vtkMRMLCommandLineModuleNode(const vtkMRMLCommandLineModuleNode&);
  void operator=(const vtkMRMLCommandLineModuleNode&);

  ModuleDescription ModuleDescriptionObject;

  static ModuleDescriptionMap *RegisteredModules;
  
  //BTX
  StatusType m_Status;
  //ETX
  
};

#endif

