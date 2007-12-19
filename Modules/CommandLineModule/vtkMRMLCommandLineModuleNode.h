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
  typedef enum { Idle=0, Scheduled=1, Running=2, Completed=3, CompletedWithErrors=4, Cancelled=5 } StatusType;
  //ETX

  // Description:
  // Set the status of the node (Idle, Scheduled, Running,
  // Completed).  The "modify" parameter indicates whether the object
  // can be modified by the call.
  //BTX
  void SetStatus(StatusType status, bool modify=true);
  StatusType GetStatus();
  //ETX
  const char* GetStatusString() {
    switch (this->m_Status)
      {
      case Idle: return "Idle";
      case Scheduled: return "Scheduled";
      case Running: return "Running";
      case Completed: return "Completed";
      case CompletedWithErrors: return "CompletedWithErrors";
      case Cancelled: return "Cancelled";
      }
    return "Unknown";
  }
    


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

  // Some functions to make CommandLineModuleNodes useful from Tcl and Python
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
  int GetNumberOfRegisteredModules ();
  void AbortProcess () { 
    this->GetModuleDescription().GetProcessInformation()->Abort = 1; 
  }
  const char* GetRegisteredModuleNameByIndex ( int idx );
  void SetModuleDescription ( const char *name ) { this->SetModuleDescription ( this->GetRegisteredModuleDescription ( name ) ); }
  const char* GetModuleVersion () { return this->GetModuleDescription().GetVersion().c_str(); };
  const char* GetModuleTitle () { return this->GetModuleDescription().GetTitle().c_str(); };
  unsigned int GetNumberOfParameterGroups () { return this->GetModuleDescription().GetParameterGroups().size(); }
  unsigned int GetNumberOfParametersInGroup ( unsigned int group ) { 
    if ( group >= this->GetModuleDescription().GetParameterGroups().size() ) { return -1; }
    return this->GetModuleDescription().GetParameterGroups()[group].GetParameters().size();
  }
  const char* GetParameterGroupLabel ( unsigned int group ) { return this->GetModuleDescription().GetParameterGroups()[group].GetLabel().c_str(); }
  const char* GetParameterGroupDescription ( unsigned int group ) { return this->GetModuleDescription().GetParameterGroups()[group].GetDescription().c_str(); }
  const char* GetParameterGroupAdvanced ( unsigned int group ) { return this->GetModuleDescription().GetParameterGroups()[group].GetAdvanced().c_str(); }

  const char* GetParameterTag ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetTag().c_str(); }
  const char* GetParameterType ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetType().c_str(); }
  const char* GetParameterArgType ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetArgType().c_str(); }
  const char* GetParameterName ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetName().c_str(); }
  const char* GetParameterLongFlag ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetLongFlag().c_str(); }
  const char* GetParameterLabel ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetLabel().c_str(); }
  const char* GetParameterConstraints ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetConstraints().c_str(); }
  const char* GetParameterMaximum ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetMaximum().c_str(); }
  const char* GetParameterMinimum ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetMinimum().c_str(); }
  const char* GetParameterDescription ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetDescription().c_str(); }
  const char* GetParameterChannel ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetChannel().c_str(); }
  const char* GetParameterIndex ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetIndex().c_str(); }
  const char* GetParameterDefault ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetDefault().c_str(); }
  const char* GetParameterFlag ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetFlag().c_str(); }
  const char* GetParameterMultiple ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetMultiple().c_str(); }
  const char* GetParameterFileExtensions ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetFileExtensionsAsString().c_str(); }
  const char* GetParameterCoordinateSystem ( unsigned int group, unsigned int param ) { return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetCoordinateSystem().c_str(); }
  
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

