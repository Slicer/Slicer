/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRMLCLI includes
#include "vtkMRMLCommandLineModuleNode.h"

// MRML includes

/// SlicerExecutionModel includes
#include <ModuleDescription.h>
#include <ModuleParameter.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>


//------------------------------------------------------------------------------
// Private implementaton of an std::map
class ModuleDescriptionMap : public std::map<std::string, ModuleDescription> {};

//------------------------------------------------------------------------------
class vtkMRMLCommandLineModuleNode::vtkInternal
{
public:

  ModuleDescription ModuleDescriptionObject;

  static ModuleDescriptionMap RegisteredModules;

  int Status;

  /// AutoRun state (On/Off)
  bool AutoRun;
  /// AutoRunMode flags
  int AutoRunMode;
  /// Delay in msecs to wait before the module is auto run.
  unsigned int AutoRunDelay;

  /// Last time the module was started.
  vtkTimeStamp LastRunTime;
  /// Last time a parameter was modified.
  vtkTimeStamp ParameterMTime;
  /// Last time an input parameter was modified.
  vtkTimeStamp InputMTime;

  /// Flag to trigger or not the StatusModifiedEvent
  mutable bool InvokeStatusModifiedEvent;

  /// Output messages of last execution (printed to stdout)
  std::string OutputText;
  /// Error messages of last execution (printed to stderr)
  std::string ErrorText;
};

ModuleDescriptionMap vtkMRMLCommandLineModuleNode::vtkInternal::RegisteredModules;

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCommandLineModuleNode);

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLCommandLineModuleNode::CreateNodeInstance()
{
  return vtkMRMLCommandLineModuleNode::New();
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::vtkMRMLCommandLineModuleNode()
{
  this->Internal = new vtkInternal();
  this->HideFromEditors = true;
  this->Internal->Status = vtkMRMLCommandLineModuleNode::Idle;
  this->Internal->AutoRun = false;
  this->Internal->AutoRunMode =
    vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter
    | vtkMRMLCommandLineModuleNode::AutoRunCancelsRunningProcess;
  this->Internal->AutoRunDelay = 1000;
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::~vtkMRMLCommandLineModuleNode()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::WriteXML(ostream& of, int nIndent)
{
  // Serialize a CommandLineModule node.
  //
  // Only need to write out enough information from the
  // ModuleDescription such that we can recognize the node type.  When
  // we reconstitute a node, we will start with a copy of the
  // prototype node for that module and then overwrite individual
  // parameter values using the parameter values indicated here.


  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  const ModuleDescription& module = this->GetModuleDescription();

  // Need to write out module description and parameters as
  // attributes.  Only need to write out the module title and version
  // in order to be able recognize the node type.  Then we just need
  // to write out each parameter name and default.  Note that any
  // references to other nodes are already stored as IDs. So we write
  // out those IDs.
  //
  of << " title=\"" << this->URLEncodeString ( module.GetTitle().c_str() ) << "\"";
  of << " version=\"" << this->URLEncodeString ( module.GetVersion().c_str() ) << "\"";
  of << " autorunmode=\"" << this->Internal->AutoRunMode << "\"";
  of << " autorun=\"" << this->Internal->AutoRun << "\"";

  // Loop over the parameter groups, writing each parameter.  Note
  // that the parameter names are unique.
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = module.GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = module.GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;


  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      // two calls, as the mrml node method saves the new string in a member
      // variable and it was getting over written when used twice before the
      // buffer was flushed.
      of << " " << this->URLEncodeString ( (*pit).GetName().c_str() );
      of  << "=\"" << this->URLEncodeString ( (*pit).GetValue().c_str() ) << "\"";
      }
    }

}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::ReadXMLAttributes(const char** atts)
{
  int wasModifying = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  // To reconstitute a CommandLineModule node:
  //
  // 1. Find the prototype node from the "title" and "version".
  // 2. Copy the prototype node into the current node.
  // 3. Override parameter values with the attributes (attributes not
  // consumed by the superclass or known attributes from the prototype
  // node).
  //
  // Referenced nodes are stored as IDs.  Do we need to remap them at all?

  // first look for the title which we need to find the prototype node
  std::string moduleTitle;
  std::string moduleVersion;

  const char **tatts = atts;
  const char *attName = nullptr;
  const char *attValue;
  while (*tatts)
    {
    attName = *(tatts++);
    attValue = *(tatts++);

    if (!strcmp(attName, "title"))
      {
      moduleTitle = this->URLDecodeString(attValue);
      }
    else if (!strcmp(attName, "version"))
      {
      moduleVersion = this->URLDecodeString(attValue);
      }
    else if (!strcmp(attName, "autorunmode"))
      {
      int autoRunMode = 0;
      std::stringstream ss;
      ss << attValue;
      ss >> autoRunMode;
      this->SetAutoRunMode(autoRunMode);
      }
    else if (!strcmp(attName, "autorun"))
      {
      bool autoRun = false;
      std::stringstream ss;
      ss << attValue;
      ss >> autoRun;
      this->SetAutoRun(autoRun);
      }
    }

  // Set an attribute on the node based on the module title so that
  // the node selectors can filter on it.
  this->SetAttribute("CommandLineModule", moduleTitle.c_str());

  // look up the module description from the library
  if (vtkMRMLCommandLineModuleNode::HasRegisteredModule( moduleTitle ))
    {
    this->Internal->ModuleDescriptionObject =
     vtkMRMLCommandLineModuleNode::GetRegisteredModuleDescription(moduleTitle);
    }
  else
    {
    // can't locate the module, return;
    return;
    }

  // Verify the version
  if (moduleVersion != this->Internal->ModuleDescriptionObject.GetVersion())
    {
    std::string msg = "Command line module " + moduleTitle + " is version \""
      + this->Internal->ModuleDescriptionObject.GetVersion()
      + "\" but parameter set from MRML file is version \""
      + moduleVersion
      + "\". Parameter set may not load properly,";

    vtkWarningMacro(<< msg.c_str());
    }

  // run through the attributes and pull out any attributes for this
  // module
  tatts = atts;
  while (*tatts)
    {
    std::string sattName = std::string(this->URLDecodeString(*(tatts++)));
    std::string sattValue = std::string(this->URLDecodeString(*(tatts++)));

    if (this->Internal->ModuleDescriptionObject.HasParameter(attName))
      {
      this->Internal->ModuleDescriptionObject.SetParameterValue(sattName.c_str(),sattValue.c_str());
      }
    }
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->SetModuleDescription(node->GetModuleDescription());
  this->SetStatus(static_cast<StatusType>(node->GetStatus()));
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Status: " << this->GetStatusString() << "\n";
  os << indent << "AutoRun:" << this->GetAutoRun() << "\n";
  os << indent << "AutoRunMode:" << this->GetAutoRunMode() << "\n";

  os << indent << "Parameter values:\n";
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit = this->GetModuleDescription().GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit = this->GetModuleDescription().GetParameterGroups().end();
  for (std::vector<ModuleParameterGroup>::const_iterator pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    std::vector<ModuleParameter>::const_iterator pbeginit = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit = (*pgit).GetParameters().end();
    for (std::vector<ModuleParameter>::const_iterator pit = pbeginit; pit != pendit; ++pit)
      {
      os << indent << " " << (*pit).GetName() << " = " << (*pit).GetValue() << "\n";
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetModuleDescriptionAsString() const
{
  std::ostringstream s;
  s << this->GetModuleDescription();
  return s.str();
}

//----------------------------------------------------------------------------
const ModuleDescription& vtkMRMLCommandLineModuleNode::GetModuleDescription() const
{
  return this->Internal->ModuleDescriptionObject;
}

//----------------------------------------------------------------------------
ModuleDescription& vtkMRMLCommandLineModuleNode::GetModuleDescription()
{
  return this->Internal->ModuleDescriptionObject;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetModuleDescription(const ModuleDescription& description)
{
  // Copy the module description
  this->Internal->ModuleDescriptionObject = description;

  // Set an attribute on the node so that we can select nodes that
  // have the same command line module (program)
  this->SetAttribute("CommandLineModule", description.GetTitle().c_str());

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
::ProcessMRMLEvents(vtkObject *caller, unsigned long event,
                    void *vtkNotUsed(callData) )
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
  if (!node)
    {
    return;
    }
  // Make sure the caller is an input node.
  bool isInput = this->IsInputDefaultValue(node->GetID());
  // Let the CLI logic know that an input parameter has been modified.
  if (isInput)
    {
    int eventType = 0;
    if (event == vtkCommand::ModifiedEvent)
      {
      eventType |= vtkMRMLCommandLineModuleNode::AutoRunOnModifiedInputEvent;
      }
    if (event != vtkCommand::ModifiedEvent)
      {
      eventType |= vtkMRMLCommandLineModuleNode::AutoRunOnOtherInputEvents;
      }
    bool autoRun = false;
    if (eventType & this->GetAutoRunMode())
      {
      this->Internal->InputMTime.Modified();
      autoRun = true;
      }
    this->InvokeEvent(vtkMRMLCommandLineModuleNode::InputParameterEvent,
                      reinterpret_cast<void*>(event));
    if (autoRun)
      {
      this->InvokeEvent(vtkMRMLCommandLineModuleNode::AutoRunEvent,
                        reinterpret_cast<void*>(0));
      }
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::IsInputDefaultValue(const std::string& value)const
{
  bool isInput = false;
  std::vector<ModuleParameter> parameters =
    this->Internal->ModuleDescriptionObject.FindParametersWithValue(
      value);
  // It is an input if it is not an output.
  std::vector<ModuleParameter>::const_iterator it;
  for (it = parameters.begin(); it != parameters.end(); ++it)
    {
    static std::string output = std::string("output");
    if (it->GetChannel() == output)
      {
      // A parameter that is output can't trigger InputParameterModifiedEvent
      // even if it is an input too. It would generate infinite autorun loops
      // otherwise.
      isInput = false;
      break;
      }
    if (it->GetChannel() != output)
      {
      isInput = true;
      }
    }
  return isInput;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::SetParameterAsNode(const char* name, vtkMRMLNode* node)
{
  std::string value = node ? node->GetID() : "";
  std::string oldValue = this->GetParameterAsString(name);
  // Set the default value of the named parameter with the value
  // specified
  if (value != oldValue)
    {
    if (!this->Internal->ModuleDescriptionObject.SetParameterValue(name, value))
      {
#ifndef NDEBUG
      if (!this->Internal->ModuleDescriptionObject.HasParameter(name))
        {
        vtkErrorMacro(<< "There is no parameter named " << name );
        }
#endif
      return false;
      }
    // Observe all the nodes, even if we are not in autorun mode because we
    // have no way a posteriory to know which parameter is a node.
    if (node)
      {
      // Observe if not already observed
      if (this->MRMLObserverManager->GetObservationsCount(node) == 0 &&
          this->IsInputDefaultValue(value))
        {
        vtkNew<vtkIntArray> events;
        events->InsertNextValue(vtkCommand::AnyEvent);
        vtkObserveMRMLObjectEventsMacro(node, events.GetPointer());
        }
      // if the old node is no longer an input parameter
      //if (!this->IsInputDefaultValue(oldValue))
      //  {
      //  // no need to observe it.
      //  vtkMRMLNode* oldNode = (this->GetScene() ?
      //                          this->GetScene()->GetNodeByID(oldValue.c_str()) : 0);
      //  if (oldNode)
      //    {
      //    vtkUnObserveMRMLObjectMacro(oldNode);
      //    }
      //  }
      }
    this->Internal->ParameterMTime.Modified();
    this->InvokeEvent(vtkMRMLCommandLineModuleNode::ParameterChangedEvent);
    if (this->GetAutoRunMode()
        & vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter)
      {
      this->InvokeEvent(vtkMRMLCommandLineModuleNode::AutoRunEvent,
                        reinterpret_cast<void*>(0));
      }
    this->Modified();
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::SetParameterAsString(const char* name, const std::string& value)
{
  // Set the default value of the named parameter with the value
  // specified
  if (value != this->GetParameterAsString(name))
    {
    if (!this->Internal->ModuleDescriptionObject.SetParameterValue(name, value))
      {
#ifndef NDEBUG
      if (!this->Internal->ModuleDescriptionObject.HasParameter(name))
        {
        vtkErrorMacro(<< "There is no parameter named " << name );
        }
#endif
      return false;
      }
    this->Internal->ParameterMTime.Modified();
    this->InvokeEvent(vtkMRMLCommandLineModuleNode::ParameterChangedEvent);
    if (this->GetAutoRunMode()
        & vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter)
      {
      this->InvokeEvent(vtkMRMLCommandLineModuleNode::AutoRunEvent, nullptr);
      }
    this->Modified();
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::SetParameterAsDouble(const char* name, double value)
{
  std::ostringstream strvalue;
  strvalue << value;
  return this->SetParameterAsString(name, strvalue.str());
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::SetParameterAsFloat(const char *name, float value)
{
  std::ostringstream strvalue;
  strvalue << value;
  return this->SetParameterAsString(name, strvalue.str());
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::SetParameterAsInt(const char* name, int value)
{
  std::ostringstream strvalue;
  strvalue << value;
  return this->SetParameterAsString(name, strvalue.str());
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::SetParameterAsBool(const char* name, bool value)
{
  std::string valueAsString = (value ? "true" : "false");
  return this->SetParameterAsString(name, valueAsString);
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterAsString(const char *name) const
{
  return this->Internal->ModuleDescriptionObject.GetParameterValue(name);
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetStatus(int status, bool modify)
{
  if (this->Internal->Status != status)
    {
    this->Internal->Status = status;
    switch (this->Internal->Status)
      {
      case vtkMRMLCommandLineModuleNode::Running:
        this->Internal->LastRunTime.Modified();
        break;
      case vtkMRMLCommandLineModuleNode::Cancelling:
        this->AbortProcess();
        break;
      default:
        break;
      }
    // StatusModifiedEvent will be invoked next time Modified() is called.
    this->Internal->InvokeStatusModifiedEvent = true;
    if (modify)
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
int vtkMRMLCommandLineModuleNode
::GetStatus() const
{
  return this->Internal->Status;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::IsBusy() const
{
  return this->Internal->Status & vtkMRMLCommandLineModuleNode::BusyMask;
}

//----------------------------------------------------------------------------
int vtkMRMLCommandLineModuleNode::GetProgress() const
{
  return vtkMath::Round(this->GetModuleDescription().GetProcessInformation()->Progress * 100.0);
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetAutoRun(bool autoRun)
{
  if (this->Internal->AutoRun == autoRun)
    {
    return;
    }
  this->Internal->AutoRun = autoRun;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::GetAutoRun() const
{
  return this->Internal->AutoRun;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetAutoRunMode(int autoRunMode)
{
  if (this->Internal->AutoRunMode == autoRunMode)
    {
    return;
    }
  this->Internal->AutoRunMode = autoRunMode;
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkMRMLCommandLineModuleNode::GetAutoRunMode() const
{
  return this->Internal->AutoRunMode;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetAutoRunDelay(unsigned int delayInMs)
{
  if (this->Internal->AutoRunDelay == delayInMs)
    {
    return;
    }
  this->Internal->AutoRunDelay = delayInMs;
  this->Modified();
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLCommandLineModuleNode::GetAutoRunDelay() const
{
  return this->Internal->AutoRunDelay;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLCommandLineModuleNode::GetLastRunTime() const
{
  return this->Internal->LastRunTime.GetMTime();
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLCommandLineModuleNode::GetParameterMTime() const
{
  return this->Internal->ParameterMTime.GetMTime();
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLCommandLineModuleNode::GetInputMTime() const
{
  return this->Internal->InputMTime.GetMTime();
}


//----------------------------------------------------------------------------
const char* vtkMRMLCommandLineModuleNode::GetStatusString() const
{
  switch (this->Internal->Status)
    {
    case Idle: return "Idle";
    case Scheduled: return "Scheduled";
    case Running: return "Running";
    case Cancelling: return "Cancelling";
    case Cancelled: return "Cancelled";
    case Completing: return "Completing";
    case Completed: return "Completed";
    case CompletedWithErrors: return "Completed with errors";
    default:
      break;
    }
  return "Unknown";
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::Cancel()
{
  if (this->IsBusy())
    {
    this->SetStatus(vtkMRMLCommandLineModuleNode::Cancelling);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::AbortProcess()
{
  this->GetModuleDescription().GetProcessInformation()->Abort = 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::HasRegisteredModule(const std::string& name)
{
  ModuleDescriptionMap::iterator mit =
      vtkMRMLCommandLineModuleNode::vtkInternal::RegisteredModules.find(name);

  return mit != vtkMRMLCommandLineModuleNode::vtkInternal::RegisteredModules.end();
}

//----------------------------------------------------------------------------
int vtkMRMLCommandLineModuleNode::GetNumberOfRegisteredModules()
{
  return (int)vtkInternal::RegisteredModules.size();
}

//----------------------------------------------------------------------------
const char* vtkMRMLCommandLineModuleNode::GetRegisteredModuleNameByIndex( int idx )
{
  ModuleDescriptionMap::iterator mit = vtkInternal::RegisteredModules.begin();
  int count = 0;
  while ( mit != vtkInternal::RegisteredModules.end() )
    {
    if ( count == idx ) { return (*mit).first.c_str(); }
    ++mit;
    ++count;
    }
  return "";
}

//----------------------------------------------------------------------------
ModuleDescription vtkMRMLCommandLineModuleNode
::GetRegisteredModuleDescription(const std::string& name)
{
  ModuleDescriptionMap::iterator mit =
      vtkMRMLCommandLineModuleNode::vtkInternal::RegisteredModules.find(name);

  if (mit != vtkMRMLCommandLineModuleNode::vtkInternal::RegisteredModules.end())
    {
    return (*mit).second;
    }

  return ModuleDescription();
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
::RegisterModuleDescription(ModuleDescription md)
{
  vtkMRMLCommandLineModuleNode::vtkInternal::RegisteredModules[md.GetTitle()] = md;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetModuleDescription ( const char *name )
{
  this->SetModuleDescription ( this->GetRegisteredModuleDescription ( name ) );
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetModuleVersion () const
{
  return this->GetModuleDescription().GetVersion();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetModuleTitle () const
{
  return this->GetModuleDescription().GetTitle();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetModuleTarget () const
{
  return this->GetModuleDescription().GetTarget();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetModuleType () const
{
  return this->GetModuleDescription().GetType();
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::IsValidGroupId(unsigned int group) const
{
  if (group < this->GetNumberOfParameterGroups())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::IsValidParamId(unsigned int group, unsigned int param) const
{
  if (param < this->GetNumberOfParametersInGroup(group))
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLCommandLineModuleNode::GetNumberOfParameterGroups () const
{
  return this->GetModuleDescription().GetParameterGroups().size();
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLCommandLineModuleNode::GetNumberOfParametersInGroup ( unsigned int group ) const
{
  if (!this->IsValidGroupId(group))
    {
    return 0;
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters().size();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterGroupLabel( unsigned int group ) const
{
  if (!this->IsValidGroupId(group))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetLabel();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterGroupDescription ( unsigned int group ) const
{
  if (!this->IsValidGroupId(group))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetDescription();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterGroupAdvanced ( unsigned int group ) const
{
  if (!this->IsValidGroupId(group))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetAdvanced();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterTag ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetTag();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterType ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetType();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterArgType ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetArgType();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterName ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetName();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterLongFlag ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetLongFlag();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterLabel ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetLabel();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterConstraints ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetConstraints();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterMaximum ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetMaximum();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterMinimum ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetMinimum();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterDescription ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetDescription();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterChannel ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetChannel();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterIndex ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetIndex();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterDefault ( unsigned int group, unsigned int param ) const
{
  return this->GetParameterValue(group, param);
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterValue ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetValue();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterFlag ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetFlag();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterMultiple ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetMultiple();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterFileExtensions ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetFileExtensionsAsString();
}

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterCoordinateSystem ( unsigned int group, unsigned int param ) const
{
  if (!this->IsValidParamId(group, param))
    {
    return std::string();
    }
  return this->GetModuleDescription().GetParameterGroups()[group].GetParameters()[param].GetCoordinateSystem();
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::ReadParameterFile(const std::string& filename)
{
  bool modified = this->Internal->ModuleDescriptionObject.ReadParameterFile(filename);

  if (modified)
    {
    this->Modified();
    }

  return modified;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::WriteParameterFile(const std::string& filename, bool withHandlesToBulkParameters)
{
  bool modified
    = this->Internal->ModuleDescriptionObject.WriteParameterFile(filename, withHandlesToBulkParameters);

  if (modified)
    {
    this->Modified();
    }

  return modified;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::Modified()
{
  bool invokeStatusModifiedEvent = this->Internal->InvokeStatusModifiedEvent;
  this->Internal->InvokeStatusModifiedEvent = false;

  this->Superclass::Modified();

  if (invokeStatusModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLCommandLineModuleNode::StatusModifiedEvent);
    }
}

//----------------------------------------------------------------------------
const std::string vtkMRMLCommandLineModuleNode::GetErrorText() const
{
  return this->Internal->ErrorText;
}

//----------------------------------------------------------------------------
const std::string vtkMRMLCommandLineModuleNode::GetOutputText() const
{
  return this->Internal->OutputText;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetErrorText(const std::string& text, bool modify)
{
  if (this->Internal->ErrorText == text)
    {
    return;
    }
  this->Internal->ErrorText = text;
  if (modify)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetOutputText(const std::string& text, bool modify)
{
  if (this->Internal->OutputText == text)
    {
    return;
    }
  this->Internal->OutputText = text;
  if (modify)
    {
    this->Modified();
    }
}
