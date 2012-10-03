/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCommandLineModuleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLCommandLineModuleNode_h
#define __vtkMRMLCommandLineModuleNode_h

/// MRML includes
#include <vtkMRMLNode.h>

#include "vtkMRMLCLIWin32Header.h"

class ModuleDescription;

/// \brief MRML node for representing the parameters allowing to run a command line module
class VTK_MRML_CLI_EXPORT vtkMRMLCommandLineModuleNode : public vtkMRMLNode
{
  public:
  static vtkMRMLCommandLineModuleNode *New();
  vtkTypeMacro(vtkMRMLCommandLineModuleNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "CommandLineModule";}

  /// Get/Set the module description object. THe module description
  /// object is used to cache the current settings for the module.
  const ModuleDescription& GetModuleDescription() const;
  ModuleDescription& GetModuleDescription();
  void SetModuleDescription(const ModuleDescription& description);

  typedef enum { Idle=0, Scheduled=1, Running=2, Completed=3, CompletedWithErrors=4, Cancelled=5 } StatusType;

  /// Set the status of the node (Idle, Scheduled, Running,
  /// Completed).  The "modify" parameter indicates whether the object
  /// can be modified by the call.
  void SetStatus(StatusType status, bool modify=true);
  StatusType GetStatus();
  const char* GetStatusString();

  /// Read a parameter file. This will set any parameters that
  /// parameters in this ModuleDescription.
  bool ReadParameterFile(const std::string& filename);
  bool ReadParameterFile(const char *filename);

  /// Write a parameter file. This will output any parameters that
  /// parameters in this
  /// ModuleDescription. "withHandlesToBulkParameters" allows to
  /// control whether all parameters are written or just the
  /// parameters with simple IO mechanisms.
  bool WriteParameterFile(const std::string& filename, bool withHandlesToBulkParameters = true);
  bool WriteParameterFile(const char *filename, bool withHandlesToBulkParameters = true);

  /// Get/Set a parameter for the module.
  void SetParameterAsString(const std::string& name, const std::string& value);
  void SetParameterAsInt(const std::string& name, int value);
  void SetParameterAsBool(const std::string& name, bool value);
  void SetParameterAsDouble(const std::string& name, double value);
  void SetParameterAsFloat(const std::string& name, float value);

  std::string GetParameterAsString(const std::string &name) const;

  /// Some functions to make CommandLineModuleNodes useful from Tcl and Python
  void SetParameterAsString(const char *name, const char *value);
  void SetParameterAsInt(const char *name, const int value);
  void SetParameterAsBool(const char *name, const int value);
  void SetParameterAsDouble(const char *name, const double value);
  void SetParameterAsFloat(const char *name, const float value);

  const char* GetParameterAsString(const char* name) const;
  int GetNumberOfRegisteredModules ();
  const char* GetRegisteredModuleNameByIndex ( int idx );
  void SetModuleDescription ( const char *name );
  const char* GetModuleVersion ();
  const char* GetModuleTitle ();
  const char* GetModuleTarget ();
  const char* GetModuleType ();
  unsigned int GetNumberOfParameterGroups ();
  unsigned int GetNumberOfParametersInGroup ( unsigned int group );
  const char* GetParameterGroupLabel ( unsigned int group );
  const char* GetParameterGroupDescription ( unsigned int group );
  const char* GetParameterGroupAdvanced ( unsigned int group );
  const char* GetParameterTag ( unsigned int group, unsigned int param );
  const char* GetParameterType ( unsigned int group, unsigned int param );
  const char* GetParameterArgType ( unsigned int group, unsigned int param );
  const char* GetParameterName ( unsigned int group, unsigned int param );
  const char* GetParameterLongFlag ( unsigned int group, unsigned int param );
  const char* GetParameterLabel ( unsigned int group, unsigned int param );
  const char* GetParameterConstraints ( unsigned int group, unsigned int param );
  const char* GetParameterMaximum ( unsigned int group, unsigned int param );
  const char* GetParameterMinimum ( unsigned int group, unsigned int param );
  const char* GetParameterDescription ( unsigned int group, unsigned int param );
  const char* GetParameterChannel ( unsigned int group, unsigned int param );
  const char* GetParameterIndex ( unsigned int group, unsigned int param );
  const char* GetParameterDefault ( unsigned int group, unsigned int param );
  const char* GetParameterFlag ( unsigned int group, unsigned int param );
  const char* GetParameterMultiple ( unsigned int group, unsigned int param );
  const char* GetParameterFileExtensions ( unsigned int group, unsigned int param );
  const char* GetParameterCoordinateSystem ( unsigned int group, unsigned int param );

  /// Methods to manage the master list of module description prototypes
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

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
