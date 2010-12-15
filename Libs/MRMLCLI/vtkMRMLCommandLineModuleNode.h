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

/// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

/// VTK includes
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

/// Libs/ModuleDescriptionParser includes
#include "ModuleDescription.h"

#include "vtkMRMLCLIWin32Header.h"

//BTX
class ModuleDescriptionMap;
//ETX

class VTK_MRML_CLI_EXPORT vtkMRMLCommandLineModuleNode : public vtkMRMLNode
{
  public:
  static vtkMRMLCommandLineModuleNode *New();
  vtkTypeMacro(vtkMRMLCommandLineModuleNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "CommandLineModule";};

  /// 
  /// Get/Set the module description object. THe module description
  /// object is used to cache the current settings for the module.
  const ModuleDescription& GetModuleDescription() const
    { return ModuleDescriptionObject; }
  ModuleDescription& GetModuleDescription()
    { return ModuleDescriptionObject; }
  void SetModuleDescription(const ModuleDescription& description);


  typedef enum { Idle=0, Scheduled=1, Running=2, Completed=3, CompletedWithErrors=4, Cancelled=5 } StatusType;

  /// 
  /// Set the status of the node (Idle, Scheduled, Running,
  /// Completed).  The "modify" parameter indicates whether the object
  /// can be modified by the call.
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

  ///
  /// Read a parameter file. This will set any parameters that
  /// parameters in this ModuleDescription.
//BTX
  bool ReadParameterFile(const std::string& filename);
//ETX
  bool ReadParameterFile(const char *filename);

  ///
  /// Write a parameter file. This will output any parameters that
  /// parameters in this
  /// ModuleDescription. "withHandlesToBulkParameters" allows to
  /// control whether all parameters are written or just the
  /// parameters with simple IO mechanisms.
//BTX
  bool WriteParameterFile(const std::string& filename, bool withHandlesToBulkParameters = true);
//ETX
  bool WriteParameterFile(const char *filename, bool withHandlesToBulkParameters = true);

  /// 
  /// Get/Set a parameter for the module.
//BTX
  void SetParameterAsString(const std::string& name, const std::string& value);
  void SetParameterAsInt(const std::string& name, int value);
  void SetParameterAsBool(const std::string& name, bool value);
  void SetParameterAsDouble(const std::string& name, double value);
  void SetParameterAsFloat(const std::string& name, float value);

  std::string GetParameterAsString(const std::string &name) const;
//ETX

  /// Some functions to make CommandLineModuleNodes useful from Tcl and Python
  void SetParameterAsString(const char *name, const char *value)
    {this->SetParameterAsString(std::string(name), std::string(value));}
  void SetParameterAsInt(const char *name, const int value)
    {this->SetParameterAsInt(std::string(name), value);}
  void SetParameterAsBool(const char *name, const int value)
    {this->SetParameterAsBool(std::string(name), (value == 0 ? false : true));}
  void SetParameterAsDouble(const char *name, const double value)
    {this->SetParameterAsDouble(std::string(name), value);}
  void SetParameterAsFloat(const char *name, const float value)
    {this->SetParameterAsFloat(std::string(name), value);}
  const char* GetParameterAsString(const char* name) const
  {
    std::string stringName = name;
    return this->GetParameterAsString(stringName).c_str();
  }
  int GetNumberOfRegisteredModules ();
  const char* GetRegisteredModuleNameByIndex ( int idx );
  void SetModuleDescription ( const char *name ) { this->SetModuleDescription ( this->GetRegisteredModuleDescription ( name ) ); }
  const char* GetModuleVersion () { return this->GetModuleDescription().GetVersion().c_str(); };
  const char* GetModuleTitle () { return this->GetModuleDescription().GetTitle().c_str(); };
  const char* GetModuleTarget () { return this->GetModuleDescription().GetTarget().c_str(); };
  const char* GetModuleType () { return this->GetModuleDescription().GetType().c_str(); };
  unsigned int GetNumberOfParameterGroups () { return (int)this->GetModuleDescription().GetParameterGroups().size(); }
  unsigned int GetNumberOfParametersInGroup ( unsigned int group ) {
    if ( group >= this->GetModuleDescription().GetParameterGroups().size() ) { return 0; }
    return (unsigned int)this->GetModuleDescription().GetParameterGroups()[group].GetParameters().size();
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

  /// 
  /// Methods to manage the master list of module description prototypes
//BTX
  static void RegisterModuleDescription(ModuleDescription md);
  static bool HasRegisteredModule(const std::string& name);
  static ModuleDescription GetRegisteredModuleDescription(const std::string& name);
  static void ClearRegisteredModules();
/// ETX
protected:
  void AbortProcess ();

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

