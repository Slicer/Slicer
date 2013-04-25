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

/// \brief MRML node for representing the parameters allowing to run a command
/// line interface module (CLI).
/// The CLI parameters are defined with \a SetModuleDescription().
/// The parameters can be changed with SetParameterAsXXX().
class VTK_MRML_CLI_EXPORT vtkMRMLCommandLineModuleNode : public vtkMRMLNode
{
public:
  static vtkMRMLCommandLineModuleNode *New();
  vtkTypeMacro(vtkMRMLCommandLineModuleNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes(const char** atts);

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

  typedef enum {
    /// Initial state of the CLI.
    Idle=0x00,
    /// State when the CLI has been requested to be executed.
    Scheduled=0x01,
    /// State when the CLI is being executed.
    Running=0x02,
    /// State when the CLI has been requested to be cancelled.
    Cancelling=0x04,
    /// State when the CLI is no longer being executed because
    /// Cancelling has been requested.
    Cancelled=0x08,
    /// State when the CLI has been successfully executed.
    Completed=0x10,
    /// Mask applied when the CLI has been executed with errors
    ErrorsMask=0x20,
    /// State when the CLI has been executed with errors
    CompletedWithErrors= Completed | ErrorsMask,
    /// Mask used to know if the CLI is in pending mode.
    BusyMask = Scheduled | Running | Cancelling
  } StatusType;

  /// Set the status of the node (Idle, Scheduled, Running,
  /// Completed).  The "modify" parameter indicates whether the object
  /// can be modified by the call. Having modify = false is used when a separate
  /// thread updates the node status but does not want to invoke modified
  /// events because it would refresh the GUI from the thread.
  void SetStatus(StatusType status, bool modify=true);
  StatusType GetStatus() const;

  /// Return current status as a string for display.
  const char* GetStatusString() const;

  /// Return true if the module is in a busy state: Scheduled, Running or
  /// Cancelling.
  /// \sa SetStatus(), GetStatus(), BusyMask
  bool IsBusy()const;
  /// Read a parameter file. This will set any parameters that
  /// parameters in this ModuleDescription.
  bool ReadParameterFile(const std::string& filename);

  /// Write a parameter file. This will output any parameters that
  /// parameters in this
  /// ModuleDescription. "withHandlesToBulkParameters" allows to
  /// control whether all parameters are written or just the
  /// parameters with simple IO mechanisms.
  bool WriteParameterFile(const std::string& filename, bool withHandlesToBulkParameters = true);

  /// Set the parameter \a name to the node \a value.
  /// If the parameter is not in the output channel, InputParameterModifiedEvent
  /// is invoked anytime the node is modified (ModifiedEvent is invoked).
  /// \sa SetParameterAsString(), SetParameterAsBool(), SetParameterAsDouble(),
  /// SetParameterAsFloat(), SetParameterAsInt()
  bool SetParameterAsNode(const char* name, vtkMRMLNode* value);
  /// Set the parameter \a name to the string \a value.
  /// \sa SetParameterAsInt(), SetParameterAsBool(), SetParameterAsDouble(),
  /// SetParameterAsFloat(), SetParameterAsNode()
  bool SetParameterAsString(const char* name, const std::string& value);
  /// Set the parameter \a name to the int \a value.
  /// \sa SetParameterAsString(), SetParameterAsBool(), SetParameterAsDouble(),
  /// SetParameterAsFloat(), SetParameterAsNode()
  bool SetParameterAsInt(const char* name, int value);
  /// Set the parameter \a name to the bool \a value.
  /// \sa SetParameterAsString(), SetParameterAsInt(), SetParameterAsDouble(),
  /// SetParameterAsFloat(), SetParameterAsNode()
  bool SetParameterAsBool(const char* name, bool value);
  /// Set the parameter \a name to the double \a value.
  /// \sa SetParameterAsString(), SetParameterAsInt(), SetParameterAsBool(),
  /// SetParameterAsFloat(), SetParameterAsNode()
  bool SetParameterAsDouble(const char* name, double value);
  /// Set the parameter \a name to the float \a value.
  /// \sa SetParameterAsString(), SetParameterAsInt(), SetParameterAsDouble(),
  /// SetParameterAsBool(), SetParameterAsNode()
  bool SetParameterAsFloat(const char* name, float value);

  std::string GetParameterAsString(const char* name) const;

  void SetModuleDescription(const char *name);
  std::string GetModuleVersion() const;
  std::string GetModuleTitle() const;
  std::string GetModuleTarget() const;
  std::string GetModuleType() const;
  bool IsValidGroupId(unsigned int group) const;
  bool IsValidParamId(unsigned int group, unsigned int param) const;
  unsigned int GetNumberOfParameterGroups() const;
  unsigned int GetNumberOfParametersInGroup(unsigned int group) const;
  std::string GetParameterGroupLabel(unsigned int group) const;
  std::string GetParameterGroupDescription(unsigned int group) const;
  std::string GetParameterGroupAdvanced(unsigned int group) const;
  std::string GetParameterTag(unsigned int group, unsigned int param) const;
  std::string GetParameterType(unsigned int group, unsigned int param) const;
  std::string GetParameterArgType(unsigned int group, unsigned int param) const;
  std::string GetParameterName(unsigned int group, unsigned int param) const;
  std::string GetParameterLongFlag(unsigned int group, unsigned int param) const;
  std::string GetParameterLabel(unsigned int group, unsigned int param) const;
  std::string GetParameterConstraints(unsigned int group, unsigned int param) const;
  std::string GetParameterMaximum(unsigned int group, unsigned int param) const;
  std::string GetParameterMinimum(unsigned int group, unsigned int param) const;
  std::string GetParameterDescription(unsigned int group, unsigned int param) const;
  std::string GetParameterChannel(unsigned int group, unsigned int param) const;
  std::string GetParameterIndex(unsigned int group, unsigned int param) const;
  std::string GetParameterDefault(unsigned int group, unsigned int param) const;
  std::string GetParameterFlag(unsigned int group, unsigned int param) const;
  std::string GetParameterMultiple(unsigned int group, unsigned int param) const;
  std::string GetParameterFileExtensions(unsigned int group, unsigned int param) const;
  std::string GetParameterCoordinateSystem(unsigned int group, unsigned int param) const;

  /// Methods to manage the master list of module description prototypes
  static int GetNumberOfRegisteredModules();
  static const char* GetRegisteredModuleNameByIndex(int idx);
  static void RegisterModuleDescription(ModuleDescription md);
  static bool HasRegisteredModule(const std::string& name);
  static ModuleDescription GetRegisteredModuleDescription(const std::string& name);

protected:
  void AbortProcess();

private:
  vtkMRMLCommandLineModuleNode();
  ~vtkMRMLCommandLineModuleNode();
  vtkMRMLCommandLineModuleNode(const vtkMRMLCommandLineModuleNode&);
  void operator=(const vtkMRMLCommandLineModuleNode&);

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
