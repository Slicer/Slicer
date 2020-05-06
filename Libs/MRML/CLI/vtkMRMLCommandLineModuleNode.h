/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLCommandLineModuleNode_h
#define __vtkMRMLCommandLineModuleNode_h

/// MRML includes
#include <vtkMRMLNode.h>

#include "vtkMRMLCLIExport.h"

class ModuleDescription;

/// \brief MRML node for representing the parameters allowing to run a command
/// line interface module (CLI).
/// The CLI parameters are defined with \a SetModuleDescription().
/// The parameters can be changed with SetParameterAsXXX().
/// It is possible to automatically run the CLI each time the parameters are
/// changed, see \a SetAutoRun().
class VTK_MRML_CLI_EXPORT vtkMRMLCommandLineModuleNode : public vtkMRMLNode
{
public:
  static vtkMRMLCommandLineModuleNode *New();
  vtkTypeMacro(vtkMRMLCommandLineModuleNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLCommandLineModuleNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override
    {return "CommandLineModule";}

  /// List of events that can be fired on or by the node.
  enum CLINodeEvent{
    /// Event invoked anytime a parameter value is changed.
    /// \sa InputParameterModifiedEvent, SetParameterAsString(),
    /// SetParameterAsNode(), SetParameterAsInt(), SetParameterAsBool(),
    /// SetParameterAsDouble(), SetParameterAsFloat()
    ParameterChangedEvent = 17000,
    /// Event invoked anytime a node set as input parameter triggers an event.
    /// The type of event is passed as callData. (e.g. ModifiedEvent,
    /// PolyDataModifiedEvent).
    /// \sa SetParameterAsNode(), ParameterChangedEvent
    InputParameterEvent,
    /// Event invoked when the AutoRun is triggered. It takes a request time as
    /// call data. 0 when called from CLI node.
    /// \sa SetAutoRun()
    AutoRunEvent,
    /// Event invoked when the CLI changes of status
    StatusModifiedEvent
  };

  /// Get/Set the module description object. THe module description
  /// object is used to cache the current settings for the module.
  const ModuleDescription& GetModuleDescription() const;
  ModuleDescription& GetModuleDescription();
  std::string GetModuleDescriptionAsString() const;
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
    /// Do not set manually, use Cancel() instead.
    Cancelled=0x08,
    /// State when the CLI has been successfully executed and is in a finishing
    /// state that loads the outputs into the scene.
    Completing=0x10,
    /// State when the CLI has been successfully executed and outputs are
    /// loaded in the scene.
    Completed=0x20,
    /// Mask applied when the CLI has been executed with errors
    ErrorsMask=0x40,
    /// State when the CLI has been executed with errors
    CompletedWithErrors= Completed | ErrorsMask,
    /// Mask used to know if the CLI is in pending mode.
    BusyMask = Scheduled | Running | Cancelling | Completing
  } StatusType;

  /// Set the status of the node (Idle, Scheduled, Running,
  /// Completed).  The "modify" parameter indicates whether the object
  /// can be modified by the call. Having modify = false is used when a separate
  /// thread updates the node status but does not want to invoke modified
  /// events because it would refresh the GUI from the thread.
  /// Do not call manually, only the logic should change the status of the node.
  /// \sa GetStatus(), GetStatusString(), IsBusy(), Cancel()
  void SetStatus(int status, bool modify=true);
  /// \sa SetStatus(), GetStatusString(), IsBusy()
  int GetStatus() const;

  /// Return current status as a string for display.
  /// \sa GetStatus(), IsBusy()
  const char* GetStatusString() const;

  /// Set output messages generated during latest execution.
  void SetOutputText(const std::string& text, bool modify = true);
  /// Set output messages generated during latest execution.
  const std::string GetOutputText() const;

  /// Set error messages generated during latest execution.
  void SetErrorText(const std::string& text, bool modify = true);
  /// Get error messages generated during latest execution.
  const std::string GetErrorText() const;

  /// Return true if the module is in a busy state: Scheduled, Running,
  /// Cancelling, Completing.
  /// \sa SetStatus(), GetStatus(), BusyMask, Cancel()
  bool IsBusy()const;

  int GetProgress()const;

  /// Set a request to stop the processing of the CLI.
  /// Do nothing if the module is not "busy".
  /// \sa IsBusy(), Cancelling, Cancelled
  void Cancel();

  /// This enum type controls when the CLI should be run automatically.
  /// \sa SetAutoRun(), GetAutoRun()
  enum AutoRunMode
  {
    /// Triggering a new autorun cancels the processing of the current CLI if
    /// any.
    /// \sa SetStatus(), Cancelling
    AutoRunCancelsRunningProcess = 0x01,
    /// When set, it triggers autorun requests when a parameter is modified
    /// when calling SetParameterAsXXX().
    /// \sa AutoRunOnModifiedInputEvent
    AutoRunOnChangedParameter = 0x10,
    /// When set, it triggers autorun requests when an input node (i.e. a
    /// parameter not in the output channel) is modified (
    /// vtkCommand::ModifiedEvent is invoked). As of now, a parameter in both
    /// input and output channels does not trigger an autorun (infinite loop).
    AutoRunOnModifiedInputEvent = 0x20,
    /// Trigger an auto run when an input parameter fires an event other than
    /// vtkCommand::ModifiedEvent
    AutoRunOnOtherInputEvents = 0x40,
    /// Trigger an auto run when an input parameter fires any event.
    AutoRunOnAnyInputEvent = AutoRunOnModifiedInputEvent | AutoRunOnOtherInputEvents,
    // <- add here new options
    /// Mask used to know if auto run is configured with valid parameters
    AutoRunEnabledMask = AutoRunOnChangedParameter | AutoRunOnAnyInputEvent
  };

  /// Enable/Disable the AutoRun for the CLI node.
  /// The behavior is ensured by the CLI logic.
  /// AutoRun is disabled (false) by default.
  /// \sa GetAutoRun(), AutoRunMode, SetAutoRunDelay()
  void SetAutoRun(bool enable);

  /// Return true if the AutoRun is enabled, false otherwise.
  /// \sa SetAutoRun(), AutoRunMode, GetAutoRunDelay()
  bool GetAutoRun()const;

  /// Set the auto running flags for the node.
  /// The behavior is ensured by the CLI logic. AutoRun is not enabled until
  /// \a SetAutoRun(true) is called.
  /// \a AutoRunWhenParameterChanged | AutoRunCancelsRunningProcess by default.
  /// \sa AutoRunMode, GetAutoRunMode(), SetAutoRun(), SetAutoRunDelay()
  void SetAutoRunMode(int autoRunMode);

  /// Return the AutoRun mode flags.
  /// \sa AutoRunMode, SetAutoRunMode(), GetAutoRun(), GetAutoRunDelay()
  int GetAutoRunMode()const;

  /// Set the number of msecs to wait before automatically running
  /// the module. 1000msecs by default.
  /// \sa GetAutoRunDelay(), SetAutoRun(), SetAutoRunMode()
  void SetAutoRunDelay(unsigned int delayInMs);

  /// Return the number of msecs to wait before automatically running
  /// the module.
  /// \sa SetAutoRunDelay(), GetAutoRun(), GetAutoRunMode()
  unsigned int GetAutoRunDelay()const;

  /// Return the last time the module was ran.
  /// \sa GetParameterMTime(), GetInputMTime(), GetMTime()
  vtkMTimeType GetLastRunTime()const;

  /// Return the last time a parameter was modified
  /// \sa GetInputMTime(), GetMTime()
  vtkMTimeType GetParameterMTime()const;

  /// Return the last time an input parameter was modified.
  /// \sa GetParameterMTime(), GetMTime()
  vtkMTimeType GetInputMTime()const;

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
  /// \deprecated Consider using GetParameterValue()
  std::string GetParameterDefault(unsigned int group, unsigned int param) const;
  std::string GetParameterValue(unsigned int group, unsigned int param) const;
  std::string GetParameterFlag(unsigned int group, unsigned int param) const;
  std::string GetParameterMultiple(unsigned int group, unsigned int param) const;
  std::string GetParameterFileExtensions(unsigned int group, unsigned int param) const;
  std::string GetParameterCoordinateSystem(unsigned int group, unsigned int param) const;

  /// Returns true if the value is a default value for a parameter that is not
  /// an output parameter.
  /// \sa SetAutoRun
  bool IsInputDefaultValue(const std::string& value)const;

  /// Methods to manage the master list of module description prototypes
  static int GetNumberOfRegisteredModules();
  static const char* GetRegisteredModuleNameByIndex(int idx);
  static void RegisterModuleDescription(ModuleDescription md);
  static bool HasRegisteredModule(const std::string& name);
  static ModuleDescription GetRegisteredModuleDescription(const std::string& name);

  /// Reimplemented for internal reasons.
  void Modified() override;
protected:
  void AbortProcess();
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event,
                                 void *callData) override;

private:
  vtkMRMLCommandLineModuleNode();
  ~vtkMRMLCommandLineModuleNode() override;
  vtkMRMLCommandLineModuleNode(const vtkMRMLCommandLineModuleNode&) = delete;
  void operator=(const vtkMRMLCommandLineModuleNode&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
