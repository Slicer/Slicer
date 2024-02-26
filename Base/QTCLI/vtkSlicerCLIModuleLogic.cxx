/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "vtkSlicerCLIModuleLogic.h"

#include "vtkSlicerTask.h"

// SlicerExecutionModel includes
#include <ModuleDescription.h>

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLMarkupsStorageNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLROIListNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLModelStorageNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkIntArray.h>
#include <vtkMultiThreader.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

// ITKSYS includes
#include <itksys/Process.h>
#include <itksys/SystemTools.hxx>
#include <itksys/RegularExpression.hxx>

// STL includes
#include <algorithm>
#include <cassert>
#include <ctime>
#include <mutex>
#include <random>
#include <set>

#ifdef _WIN32
#else
# include <sys/types.h>
# include <unistd.h>
#endif

//----------------------------------------------------------------------------
struct DigitsToCharacters
{
  char operator()(char in)
  {
    if (in >= 48 && in <= 57)
    {
      return in + 17;
    }

    return in;
  }
};

typedef std::pair<vtkSlicerCLIModuleLogic*, vtkMRMLCommandLineModuleNode*> LogicNodePair;
class MRMLIDMap : public std::map<std::string, std::string>
{
};

//---------------------------------------------------------------------------
class vtkSlicerCLIRescheduleCallback : public vtkCallbackCommand
{
public:
  static vtkSlicerCLIRescheduleCallback* New() { return new vtkSlicerCLIRescheduleCallback; }
  void Execute(vtkObject* caller, unsigned long eid, void* callData) override
  {
    if (std::find(this->ThreadIDs.begin(), this->ThreadIDs.end(), vtkMultiThreader::GetCurrentThreadID())
        != this->ThreadIDs.end())
    {
      if (this->CLIModuleLogic)
      {
        vtkSlicerApplicationLogic* appLogic = this->CLIModuleLogic->GetApplicationLogic();
        appLogic->InvokeEventWithDelay(this->Delay, caller, eid, callData);
      }
      this->SetAbortFlag(1);
    }
  }

  void SetCLIModuleLogic(vtkSlicerCLIModuleLogic* logic) { this->CLIModuleLogic = logic; }
  vtkSlicerCLIModuleLogic* GetCLIModuleLogic() { return this->CLIModuleLogic; }

  void SetDelay(int delay) { this->Delay = delay; }
  int GetDelay() { return this->Delay; }

  void RescheduleEventsFromThreadID(vtkMultiThreaderIDType id, bool reschedule)
  {
    if (id == 0)
    {
      return;
    }
    if (reschedule)
    {
      this->ThreadIDs.push_back(id);
    }
    else
    {
      this->ThreadIDs.erase(std::remove(this->ThreadIDs.begin(), this->ThreadIDs.end(), id), this->ThreadIDs.end());
    }
  }

protected:
  vtkSlicerCLIRescheduleCallback()
  {
    this->CLIModuleLogic = nullptr;
    this->Delay = 0;
  }
  ~vtkSlicerCLIRescheduleCallback() override { this->SetCLIModuleLogic(nullptr); }

  vtkSlicerCLIModuleLogic* CLIModuleLogic;
  int Delay;
  std::vector<vtkMultiThreaderIDType> ThreadIDs;
};

//---------------------------------------------------------------------------
// A callback command that calls another callback command that was passed in callData.
// This is essentially passing a function object as calldata to a generic callback routine.
//
// After the callback that was passed as callData is called, it is deleted
//
class vtkSlicerCLIOneShotCallbackCallback : public vtkCallbackCommand
{
public:
  static vtkSlicerCLIOneShotCallbackCallback* New() { return new vtkSlicerCLIOneShotCallbackCallback; }
  void Execute(vtkObject* caller, unsigned long eid, void* callData) override
  {
    if (callData)
    {
      vtkObject* object = reinterpret_cast<vtkObject*>(callData);
      if (object)
      {
        vtkCallbackCommand* callback = vtkCallbackCommand::SafeDownCast(object);
        if (callback)
        {
          callback->Execute(caller, eid, nullptr);

          // delete the callback
          callback->Delete();

          this->SetAbortFlag(1);
        }
      }
    }
  }

protected:
  vtkSlicerCLIOneShotCallbackCallback() = default;
  ~vtkSlicerCLIOneShotCallbackCallback() override = default;
};

//----------------------------------------------------------------------------
class vtkSlicerCLIModuleLogic::vtkInternal
{
public:
  ModuleDescription DefaultModuleDescription;
  int DeleteTemporaryFiles;
  int AllowInMemoryTransfer;

  int RedirectModuleStreams;

  std::default_random_engine RandomGenerator;

  std::mutex ProcessesKillLock;
  std::vector<itksysProcess*> Processes;

  typedef std::vector<std::pair<vtkMTimeType, vtkMRMLCommandLineModuleNode*>> RequestType;
  struct FindRequest
  {
    FindRequest(vtkMRMLCommandLineModuleNode* node)
      : Node(node)
      , LastRequestUID(0)
    {
    }
    FindRequest(vtkMTimeType requestUID)
      : Node(nullptr)
      , LastRequestUID(requestUID)
    {
    }
    bool operator()(const std::pair<vtkMTimeType, vtkMRMLCommandLineModuleNode*>& p)
    {
      return (this->Node != nullptr && p.second == this->Node)
             || (this->LastRequestUID != 0 && p.first == this->LastRequestUID);
    }
    vtkMRMLCommandLineModuleNode* Node;
    vtkMTimeType LastRequestUID;
  };

  void SetLastRequest(vtkMRMLCommandLineModuleNode* node, vtkMTimeType requestUID)
  {
    RequestType::iterator it = std::find_if(this->LastRequests.begin(), this->LastRequests.end(), FindRequest(node));
    if (it == this->LastRequests.end())
    {
      this->LastRequests.push_back(std::make_pair(requestUID, node));
    }
    else
    {
      assert(it->first < requestUID);
      it->first = requestUID;
    }
  }
  vtkMTimeType GetLastRequest(vtkMRMLCommandLineModuleNode* node)
  {
    RequestType::iterator it = std::find_if(this->LastRequests.begin(), this->LastRequests.end(), FindRequest(node));
    return (it != this->LastRequests.end()) ? it->first : 0;
  }

  /// Install the reschedule callback on a node and its references
  /// \sa StopRescheduleNodeEvents()
  void StartRescheduleNodeEvents(vtkMRMLNode* node)
  {
    if (!node)
    {
      return;
    }
    node->AddObserver(vtkCommand::AnyEvent, this->RescheduleCallback, 100000000.f);
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
    if (displayableNode)
    {
      for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); ++i)
      {
        vtkMRMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(i);
        this->StartRescheduleNodeEvents(displayNode);
      }
    }
  }
  /// Remote the reschedule callback on a node and its references.
  /// \sa StartRescheduleNodeEvents()
  void StopRescheduleNodeEvents(vtkMRMLNode* node)
  {
    if (!node)
    {
      return;
    }
    node->RemoveObserver(this->RescheduleCallback);
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
    if (displayableNode)
    {
      for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); ++i)
      {
        vtkMRMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(i);
        this->StopRescheduleNodeEvents(displayNode);
      }
    }
  }

  /// List of read data/scene requests of the CLI nodes
  /// being executed with their.
  RequestType LastRequests;

  vtkSmartPointer<vtkSlicerCLIRescheduleCallback> RescheduleCallback;
  vtkSmartPointer<vtkSlicerCLIOneShotCallbackCallback> OneShotCallbackCallback;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCLIModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic::vtkSlicerCLIModuleLogic()
{
  this->Internal = new vtkInternal();

  this->Internal->RandomGenerator.seed(std::random_device{}());

  this->Internal->DeleteTemporaryFiles = 1;
  this->Internal->AllowInMemoryTransfer = 1;
  this->Internal->RedirectModuleStreams = 1;
  this->Internal->RescheduleCallback = vtkSmartPointer<vtkSlicerCLIRescheduleCallback>::New();
  this->Internal->RescheduleCallback->SetCLIModuleLogic(this);
  this->Internal->OneShotCallbackCallback = vtkSmartPointer<vtkSlicerCLIOneShotCallbackCallback>::New();

  this->AddObserver(
    vtkSlicerCLIModuleLogic::RequestHierarchyEditEvent, this->Internal->OneShotCallbackCallback, 100000000.f);
}

//----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic::~vtkSlicerCLIModuleLogic()
{
  this->RemoveObserver(this->Internal->OneShotCallbackCallback);

  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetDefaultModuleDescription(const ModuleDescription& description)
{
  this->Internal->DefaultModuleDescription = description;
}

//-----------------------------------------------------------------------------
const ModuleDescription& vtkSlicerCLIModuleLogic ::GetDefaultModuleDescription() const
{
  return this->Internal->DefaultModuleDescription;
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode* vtkSlicerCLIModuleLogic::CreateNode()
{
  vtkMRMLCommandLineModuleNode* node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLCommandLineModuleNode"));
  node->SetName(
    this->GetMRMLScene()->GetUniqueNameByString(this->Internal->DefaultModuleDescription.GetTitle().c_str()));
  node->SetModuleDescription(this->Internal->DefaultModuleDescription);
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode* vtkSlicerCLIModuleLogic::CreateNodeInScene()
{
  vtkMRMLCommandLineModuleNode* node = this->CreateNode();
  this->GetMRMLScene()->AddNode(node);
  node->Delete();
  return node;
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::DeleteTemporaryFilesOn()
{
  this->SetDeleteTemporaryFiles(static_cast<int>(1));
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::DeleteTemporaryFilesOff()
{
  this->SetDeleteTemporaryFiles(static_cast<int>(0));
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetDeleteTemporaryFiles(int value)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DeleteTemporaryFiles to " << value);
  if (this->Internal->DeleteTemporaryFiles != value)
  {
    this->Internal->DeleteTemporaryFiles = value;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
int vtkSlicerCLIModuleLogic::GetDeleteTemporaryFiles() const
{
  return this->Internal->DeleteTemporaryFiles;
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetAllowInMemoryTransfer(int value)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting AllowInMemoryTransfer to " << value);
  if (this->Internal->AllowInMemoryTransfer != value)
  {
    this->Internal->AllowInMemoryTransfer = value;
  }
}

//----------------------------------------------------------------------------
int vtkSlicerCLIModuleLogic::GetAllowInMemoryTransfer() const
{
  return this->Internal->AllowInMemoryTransfer;
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::RedirectModuleStreamsOn()
{
  this->SetRedirectModuleStreams(static_cast<int>(1));
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::RedirectModuleStreamsOff()
{
  this->SetRedirectModuleStreams(static_cast<int>(0));
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetRedirectModuleStreams(int value)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DeleteTemporaryFiles to " << value);
  if (this->Internal->RedirectModuleStreams != value)
  {
    this->Internal->RedirectModuleStreams = value;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
int vtkSlicerCLIModuleLogic::GetRedirectModuleStreams() const
{
  return this->Internal->RedirectModuleStreams;
}

//----------------------------------------------------------------------------
std::string vtkSlicerCLIModuleLogic ::ConstructTemporarySceneFileName(vtkMRMLScene* scene)
{
  std::string fname;
  std::ostringstream fnameString;
  std::string pid;
  std::ostringstream pidString;

  // Part of the filename will include an encoding of the scene
  // pointer for uniqueness
  char tname[256];

  sprintf(tname, "%p", scene);

  fname = tname;

  // To avoid confusing the Archetype readers, convert any
  // numbers in the filename to characters [0-9]->[A-J]
  std::transform(fname.begin(), fname.end(), fname.begin(), DigitsToCharacters());

  // Encode process id into a string.  To avoid confusing the
  // Archetype reader, convert the numbers in pid to characters [0-9]->[A-J]
#ifdef _WIN32
  pidString << GetCurrentProcessId();
#else
  pidString << getpid();
#endif
  pid = pidString.str();
  std::transform(pid.begin(), pid.end(), pid.begin(), DigitsToCharacters());

  // By default, the filename is based on the temporary directory and
  // the pid
  // by default use the current directory
  std::string temporaryDirectory = ".";
  vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
  if (appLogic)
  {
    temporaryDirectory = appLogic->GetTemporaryPath();
  }
  fname = temporaryDirectory + "/" + pid + "_" + fname + ".mrml";

  return fname;
}

//----------------------------------------------------------------------------
std::string vtkSlicerCLIModuleLogic ::ConstructTemporaryFileName(const std::string& tag,
                                                                 const std::string& type,
                                                                 const std::string& name,
                                                                 const std::vector<std::string>& extensions,
                                                                 CommandLineModuleType commandType)
{
  std::string fname = name;
  std::string pid;
  std::ostringstream pidString;

  // Constructing a temporary filename from a node involves:
  //
  // 1. If the consumer of the file can communicate directly with the
  // MRML scene, then the node is encoded as slicer:%p#%s where the
  // pointer is the address of the scene which contains the node
  // and the string is the MRML node ID.
  //
  // 2. If the consumer of the file is a Python module, it operates
  // in the process space of Slicer.  The Python module can be given
  // MRML node ID's directly.
  //
  // 3. If the consumer of the file cannot communicate directly with
  // the MRML scene, then a real temporary filename is constructed.
  // The filename will point to the Temporary directory defined for
  // Slicer. The filename will be unique to the process (multiple
  // running instances of slicer will not collide).  The filename
  // will be unique to the node in the process (the same node will be
  // encoded to the same filename every time within that running
  // instance of Slicer).  This last point is an optimization to
  // minimize the number of times a file is written when running a
  // module.  However, if we change the execution model such that more
  // than one module can run at the same time within the same Slicer
  // process, then this encoding will need to be changed to be unique
  // per module execution.
  //

  // Encode process id into a string.  To avoid confusing the
  // Archetype reader, convert the numbers in pid to characters [0-9]->[A-J]
#ifdef _WIN32
  pidString << GetCurrentProcessId();
#else
  pidString << getpid();
#endif
  pid = pidString.str();
  std::transform(pid.begin(), pid.end(), pid.begin(), DigitsToCharacters());

  // To avoid confusing the Archetype readers, convert any
  // numbers in the filename to characters [0-9]->[A-J]
  std::transform(fname.begin(), fname.end(), fname.begin(), DigitsToCharacters());

  // By default, the filename is based on the temporary directory and
  // the pid
  std::string temporaryDirectory = ".";
  vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
  if (appLogic)
  {
    temporaryDirectory = appLogic->GetTemporaryPath();
  }
  fname = temporaryDirectory + "/" + pid + "_" + fname;

  if (tag == "image")
  {
    if (commandType == CommandLineModule || type == "dynamic-contrast-enhanced"
        || this->GetAllowInMemoryTransfer() == 0)
    {
      // If running an executable

      // Use default fname construction, tack on extension
      std::string ext = ".nrrd";
      if (extensions.size() != 0)
      {
        ext = extensions[0];
      }
      fname = fname + ext;
    }
    else
    {
      // If not a command line module then it is a shared object
      // module for which we can communicated directly with the MRML
      // tree.

      // Redefine the filename to be a reference to a slicer node.

      // Must be large enough to hold slicer:, #, an ascii
      // representation of the scene pointer and the MRML node ID.
      char* tname = new char[name.size() + 100];

      sprintf(tname, "slicer:%p#%s", this->GetMRMLScene(), name.c_str());

      fname = tname;

      delete[] tname;
    }
  }

  if (tag == "geometry")
  {
    // geometry is currently always passed via files

    // Use default fname construction, tack on extension
    std::string ext = ".vtp";
    if (extensions.size() != 0)
    {
      ext = extensions[0];
    }
    fname = fname + ext;
  }

  if (tag == "transform")
  {
    // transform is currently always passed via files

    // Use default fname construction, tack on extension
    std::string ext = ".h5";
    if (extensions.size() != 0)
    {
      ext = extensions[0];
    }
    fname = fname + ext;
  }

  if (tag == "table")
  {
    // tables are currently always passed via files

    // Use default fname construction, tack on extension
    // In general, use tsv (tab-separated values) file format for tables
    std::string ext = ".tsv";
    if (type == "color")
    {
      // Special case: color tables use .ctbl files
      ext = ".ctbl";
    }

    if (extensions.size() != 0)
    {
      ext = extensions[0];
    }
    fname = fname + ext;
  }

  if (tag == "measurement")
  {
    // tables are currently always passed via files

    // Use default fname construction, tack on extension
    std::string ext = ".csv";
    if (extensions.size() != 0)
    {
      ext = extensions[0];
    }
    fname = fname + ext;
  }

  if (tag == "pointfile")
  {
    // fiducial files are always passed via files
    // use .json extension by default
    std::string ext = ".json";
    // for backward compatibility, use .fcsv by default for fiducial points
    if (type.empty() || type == "point")
    {
      ext = ".fcsv";
    }
    if (extensions.size() != 0)
    {
      // the module definition contains file formats,
      // use the first one as write format
      ext = extensions[0];
    }
    fname = fname + ext;
  }

  return fname;
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::ApplyAndWait(vtkMRMLCommandLineModuleNode* node, bool updateDisplay)
{
  // Just execute and wait.
  node->Register(this);
  node->SetAttribute("UpdateDisplay", updateDisplay ? "true" : "false");

  vtkSlicerCLIModuleLogic::ApplyTask(node);

  while (this->GetApplicationLogic()->GetReadDataQueueSize())
  {
    this->GetApplicationLogic()->ProcessReadData();
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::KillProcesses()
{
  this->Internal->ProcessesKillLock.lock();
  for (std::vector<itksysProcess*>::iterator it = this->Internal->Processes.begin();
       it != this->Internal->Processes.end();
       ++it)
  {
    itksysProcess* process = *it;
    itksysProcess_Kill(process);
  }
  this->Internal->ProcessesKillLock.unlock();
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::Apply(vtkMRMLCommandLineModuleNode* node, bool updateDisplay)
{
  bool ret;

  vtkNew<vtkSlicerTask> task;
  task->SetTypeToProcessing();

  // Pass the current node as client data to the task.  This allows
  // the user to switch to another parameter set after the task is
  // scheduled but before it starts to run. And when the scheduled
  // task does run, it will operate on the correct node.
  task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)&vtkSlicerCLIModuleLogic::ApplyTask, node);

  // Client data on the task is just a regular pointer, up the
  // reference count on the node, we'll decrease the reference count
  // once the task actually runs
  node->Register(this);
  node->SetAttribute("UpdateDisplay", updateDisplay ? "true" : "false");

  // Schedule the task
  ret = this->GetApplicationLogic()->ScheduleTask(task.GetPointer());

  if (!ret)
  {
    vtkWarningMacro(<< "Could not schedule task");
  }
  else
  {
    node->SetOutputText("", false);
    node->SetErrorText("", false);
    node->SetStatus(vtkMRMLCommandLineModuleNode::Scheduled);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic ::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* logic)
{
  vtkMRMLApplicationLogic* oldLogic = this->GetMRMLApplicationLogic();
  if (logic == oldLogic)
  {
    return;
  }

  bool wasModifying = this->StartModify();
  if (oldLogic)
  {
    vtkEventBroker::GetInstance()->RemoveObservations(
      oldLogic, vtkSlicerApplicationLogic::RequestProcessedEvent, this, this->GetMRMLLogicsCallbackCommand());
  }

  Superclass::SetMRMLApplicationLogic(logic);
  assert(logic == this->GetMRMLApplicationLogic());

  // Observe application logic to know when the CLI is completed and the
  // associated data loaded.
  if (logic)
  {
    vtkEventBroker::GetInstance()->AddObservation(
      logic, vtkSlicerApplicationLogic::RequestProcessedEvent, this, this->GetMRMLLogicsCallbackCommand());
  }
  this->EndModify(wasModifying);
}

//-----------------------------------------------------------------------------
// Static method for lazy evaluation of module target
// void vtkSlicerCLIModuleLogic::LazyEvaluateModuleTarget(ModuleDescription& moduleDescriptionObject)
// {
//   if (moduleDescriptionObject.GetTarget() == "Unknown")
//     {
//     if (moduleDescriptionObject.GetType() == "SharedObjectModule")
//       {
//       typedef int (*ModuleEntryPoint)(int argc, char* argv[]);
//
// #if defined(__APPLE__) && (MAC_OS_X_VERSION_MAX_ALLOWED >= 1030)
//       // macOS defaults to RTLD_GLOBAL and there is no way to
//       // override in itksys. So make the direct call to dlopen().
//       itksys::DynamicLoader::LibraryHandle lib
//         = dlopen(moduleDescriptionObject.GetLocation().c_str(), RTLD_LAZY | RTLD_LOCAL);
// #else
//       itksys::DynamicLoader::LibraryHandle lib
//         = itksys::DynamicLoader::OpenLibrary(moduleDescriptionObject.GetLocation().c_str());
// #endif
//       if ( lib )
//         {
//         ModuleEntryPoint entryPoint
//           = (ModuleEntryPoint)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleEntryPoint");
//
//         if (entryPoint)
//           {
//           char entryPointAsText[256];
//           std::string entryPointAsString;
//
//           sprintf(entryPointAsText, "%p", entryPoint);
//           entryPointAsString = std::string("slicer:") + entryPointAsText;
//
//           moduleDescriptionObject.SetTarget( entryPointAsString );
//           }
//         else
//           {
//           // can't find entry point, eject.
//           itksys::DynamicLoader::CloseLibrary(lib);
//
//           vtkErrorMacro(<< "Cannot find entry point for " << moduleDescriptionObject.GetLocation() << "\nCannot run
//           module." ); return;
//           }
//         }
//       }
//     }
// }

//-----------------------------------------------------------------------------
//
// This routine is called in a separate thread from the main thread.
// As such, this routine cannot directly or indirectly update the user
// interface.  In the Slicer architecture, the user interface can be
// updated whenever a node receives a Modified.  Since calls to
// Modified() can update the GUI, the ApplyTask must be careful not to
// modify a MRML node.
//
void vtkSlicerCLIModuleLogic::ApplyTask(void* clientdata)
{
  // check if MRML node is present
  if (clientdata == nullptr)
  {
    vtkErrorMacro("No input CommandLineModuleNode found");
    return;
  }

  vtkSmartPointer<vtkMRMLCommandLineModuleNode> node0;
  // node was registered when the task was scheduled so take reference to
  // release it when it goes out of scope
  node0.TakeReference(reinterpret_cast<vtkMRMLCommandLineModuleNode*>(clientdata));

  // Check to see if this node/task has been cancelled
  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling
      || node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelled)
  {
    node0->SetOutputText("", false);
    node0->SetErrorText("", false);
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
    this->GetApplicationLogic()->RequestModified(node0);
    return;
  }

  // Set the callback for progress.  This will only be used for the
  // scope of this function.
  LogicNodePair lnp(this, node0);
  node0->GetModuleDescription().GetProcessInformation()->SetProgressCallback(vtkSlicerCLIModuleLogic::ProgressCallback,
                                                                             &lnp);

  // Determine the type of the module: command line or shared object
  int (*entryPoint)(int argc, char* argv[]);
  entryPoint = nullptr;
  CommandLineModuleType commandType = CommandLineModule;

  std::string target = node0->GetModuleDescription().GetTarget();
  std::string::size_type pos = target.find("slicer:");
  if (pos == 0)
  {
    sscanf(target.c_str(), "slicer:%p", &entryPoint);
  }

  // Assume that the modules correctly report themselves
  if (node0->GetModuleDescription().GetType() == "CommandLineModule")
  {
    vtkInfoMacro("Found CommandLine Module, target is " << node0->GetModuleDescription().GetTarget());
    commandType = CommandLineModule;
    if (entryPoint != nullptr)
    {
      vtkWarningMacro("Module reports that it is a Command Line Module but has a shared object module target. "
                      << target);
    }
  }
  else if (node0->GetModuleDescription().GetType() == "SharedObjectModule")
  {
    vtkInfoMacro("Found SharedObject Module");

    commandType = SharedObjectModule;
    if (entryPoint == nullptr)
    {
      vtkWarningMacro(
        "Module reports that it is a Shared Object Module but does not have a shared object module target. " << target);
    }
  }

  vtkInfoMacro("ModuleType: " << node0->GetModuleDescription().GetType());

  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap nodesToReload;
  MRMLIDToFileNameMap nodesToWrite;

  // map to keep track of the MRML Ids on the main scene to the MRML
  // Ids in the miniscene sent to the module
  MRMLIDMap sceneToMiniSceneMap;

  // Mini-scene used to communicate a subset of the main scene to the module
  // Additional handling is necessary because we use SmartPointers
  // (see http://slicer.spl.harvard.edu/slicerWiki/index.php/Slicer3:Memory_Management#SmartPointers)
  vtkNew<vtkMRMLScene> miniscene;
  std::string minisceneFilename = this->ConstructTemporarySceneFileName(miniscene.GetPointer());
  miniscene->SetRootDirectory(vtksys::SystemTools::GetParentDirectory(minisceneFilename.c_str()).c_str());

  // vector of files to delete
  std::set<std::string> filesToDelete;

  // iterators for parameter groups
  std::vector<ModuleParameterGroup>::iterator pgbeginit = node0->GetModuleDescription().GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::iterator pgendit = node0->GetModuleDescription().GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::iterator pgit;

  // Make a pass over the parameters and establish which parameters
  // have images or geometry or transforms or tables or point files that need to be written
  // before execution or loaded upon completion.
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
  {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::iterator pbeginit = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::iterator pendit = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
    {
      if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry" || (*pit).GetTag() == "transform"
          || (*pit).GetTag() == "table" || (*pit).GetTag() == "measurement" || (*pit).GetTag() == "pointfile")
      {
        std::string id = (*pit).GetValue();

        // if the parameter is hidden, then deduce its value/id
        if ((*pit).GetHidden() == "true")
        {
          id = this->FindHiddenNodeID(node0->GetModuleDescription(), *pit);

          // cache the id so we don't have to look for it later
          (*pit).SetValue(id);
        }

        // only keep track of objects associated with real nodes
        if (!this->GetMRMLScene()->GetNodeByID(id.c_str()) || id == "None")
        {
          continue;
        }

        std::string fname = this->ConstructTemporaryFileName(
          (*pit).GetTag(), (*pit).GetType(), id, (*pit).GetFileExtensions(), commandType);

        filesToDelete.insert(fname);
        if ((*pit).GetChannel() == "input")
        {
          nodesToWrite[id] = fname;
        }
        else if ((*pit).GetChannel() == "output")
        {
          nodesToReload[id] = fname;
        }

        // if it's a point file, set an attribute on the node to pass along to the storage node
        if ((*pit).GetTag() == "pointfile")
        {
          std::string coordinateSystemStr = (*pit).GetCoordinateSystem();
          vtkMRMLNode* nodeToFlag = this->GetMRMLScene()->GetNodeByID(id.c_str());
          if (nodeToFlag)
          {
            // Disable modified event, because we would not want to emit a node modified event from this
            // worker thread.
            bool wasDisableModified = nodeToFlag->GetDisableModifiedEvent();
            nodeToFlag->SetDisableModifiedEvent(true);
            nodeToFlag->SetAttribute("Markups.CoordinateSystem", coordinateSystemStr.c_str());
            nodeToFlag->SetDisableModifiedEvent(wasDisableModified);
          }
        }
      }
    }
  }

  // Define a temporary directory for storing files
  // by default use the current directory for storing files
  std::string temporaryDirectory = ".";
  vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
  if (appLogic)
  {
    temporaryDirectory = appLogic->GetTemporaryPath();
  }

  // write out the input datasets
  //
  //

  std::set<std::string> MemoryTransferPossible;
  MemoryTransferPossible.insert("vtkMRMLScalarVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLLabelMapVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLVectorVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLDiffusionWeightedVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLDiffusionTensorVolumeNode");

  MRMLIDToFileNameMap::const_iterator id2fn0;

  for (id2fn0 = nodesToWrite.begin(); id2fn0 != nodesToWrite.end(); ++id2fn0)
  {
    vtkMRMLNode* nd = this->GetMRMLScene()->GetNodeByID((*id2fn0).first.c_str());

    vtkSmartPointer<vtkMRMLStorageNode> out = nullptr;
    vtkSmartPointer<vtkMRMLStorageNode> defaultOut = nullptr;

    vtkMRMLStorableNode* sn = dynamic_cast<vtkMRMLStorableNode*>(nd);
    if (sn)
    {

      if (!sn->IsA("vtkMRMLMarkupsNode"))
      {
        defaultOut.TakeReference(sn->CreateDefaultStorageNode());
      }
      else
      {
        // Markups storage node is a special case as we still support the old fcsv format
        // for backward compatibility, which uses a non-default storage node.
        std::string extension =
          vtksys::SystemTools::LowerCase(vtksys::SystemTools::GetFilenameLastExtension((*id2fn0).second));
        if (extension == ".fcsv")
        {
          // special case for backward compatibility
          defaultOut.TakeReference(vtkMRMLStorageNode::SafeDownCast(
            this->GetMRMLScene()->CreateNodeByClass("vtkMRMLMarkupsFiducialStorageNode")));
        }
        else
        {
          defaultOut.TakeReference(sn->CreateDefaultStorageNode());
        }
        // Set requested coordinate system
        vtkMRMLMarkupsStorageNode* markupsStorage = vtkMRMLMarkupsStorageNode::SafeDownCast(defaultOut);
        if (markupsStorage)
        {
          int coordinateSystem = this->GetCoordinateSystemFromString(nd->GetAttribute("Markups.CoordinateSystem"));
          markupsStorage->SetCoordinateSystem(coordinateSystem);
        }
      }

      if (defaultOut)
      {
        defaultOut->ConfigureForDataExchange();
      }
    }

    // Determine if and how a node is to be written.  If we update the
    // MRMLIDImageIO, then we can change these conditions for the
    // other image types so that we only write nodes to disk if we are
    // running as a command line executable (and all image types will
    // go through memory in shared object modules).
    if ((commandType == CommandLineModule) && defaultOut)
    {
      // Default case for CommandLineModule is to use a storage node
      out = defaultOut;
    }
    if ((commandType == SharedObjectModule) && defaultOut)
    {
      // std::cerr << nd->GetName() << " is " << nd->GetClassName() << std::endl;

      // Check if we can transfer the datatype using a direct memory transfer
      if ((this->GetAllowInMemoryTransfer() == 0)
          || std::find(MemoryTransferPossible.begin(), MemoryTransferPossible.end(), nd->GetClassName())
               == MemoryTransferPossible.end())
      {
        // Cannot use a memory transfer, use a StorageNode
        out = defaultOut;
      }
    }

    vtkMRMLTransformNode* tnd = vtkMRMLTransformNode::SafeDownCast(nd);
    if (tnd)
    {
      // Transform nodes will use either a storage node OR a miniscene

      std::string extension =
        vtksys::SystemTools::LowerCase(vtksys::SystemTools::GetFilenameLastExtension((*id2fn0).second));
      if (!extension.empty())
      {
        // if we start passing pointers to MRML transforms, then we'll
        // need an additional check/case
        if (extension == ".mrml")
        {
          // not using a storage node.  using a mini-scene to transfer
          // the node
          out = nullptr; // don't use the storage node

          vtkMRMLNode* cp = miniscene->CopyNode(nd);

          // Keep track what scene node corresponds to what miniscene node
          sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
        }
      }
    }

    // Add model hierarchies to the miniscene
    vtkMRMLModelHierarchyNode* mhnd = vtkMRMLModelHierarchyNode::SafeDownCast(nd);
    if (mhnd)
    {
      this->AddCompleteModelHierarchyToMiniScene(miniscene.GetPointer(), mhnd, &sceneToMiniSceneMap, filesToDelete);
    }

    // if the file is to be written, then write it
    if (out)
    {
      out->SetScene(this->GetMRMLScene());
      out->SetFileName((*id2fn0).second.c_str());
      if (!out->WriteData(nd))
      {
        vtkErrorMacro("ERROR writing file " << out->GetFileName());
      }
      out = nullptr;
    }
  }

  // Also need to run through any output nodes that will be
  // communicated through the miniscene and add them to the miniscene
  //
  for (id2fn0 = nodesToReload.begin(); id2fn0 != nodesToReload.end(); ++id2fn0)
  {
    vtkMRMLNode* nd = this->GetMRMLScene()->GetNodeByID((*id2fn0).first.c_str());

    vtkMRMLTransformNode* tnd = vtkMRMLTransformNode::SafeDownCast(nd);
    vtkMRMLModelHierarchyNode* mhnd = vtkMRMLModelHierarchyNode::SafeDownCast(nd);

    if (tnd || mhnd)
    {
      std::string extension =
        vtksys::SystemTools::LowerCase(vtksys::SystemTools::GetFilenameLastExtension((*id2fn0).second));
      if (!extension.empty())
      {
        // if we start passing pointers to MRML transforms, then we'll
        // need an additional check/case
        if (extension == ".mrml")
        {
          // put this transform node in the miniscene
          vtkMRMLNode* cp = miniscene->CopyNode(nd);

          // Keep track what scene node corresponds to what miniscene node
          sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
        }
      }
    }
    else if (mhnd)
    {
      // always put model hierarchy nodes in the miniscene
      vtkMRMLNode* cp = miniscene->CopyNode(nd);

      // Keep track what scene node corresponds to what miniscene node
      sceneToMiniSceneMap[nd->GetID()] = cp->GetID();

      // try casting to a DisplayableNode, if successful, add the
      // display node if there is one
      vtkMRMLDisplayableNode* dable = vtkMRMLDisplayableNode::SafeDownCast(nd);
      if (dable)
      {
        vtkMRMLDisplayNode* dnd = dable->GetDisplayNode();
        if (dnd)
        {
          vtkMRMLNode* dcp = miniscene->CopyNode(dnd);

          vtkMRMLDisplayableNode* dablecp = vtkMRMLDisplayableNode::SafeDownCast(cp);
          vtkMRMLDisplayNode* d = vtkMRMLDisplayNode::SafeDownCast(dcp);

          dablecp->SetAndObserveDisplayNodeID(d->GetID());
        }
      }
    }
    if (commandType == SharedObjectModule && sceneToMiniSceneMap.find(nd->GetID()) == sceneToMiniSceneMap.end())
    {
      // If the node is not in the mini-scene, then it means the filter will
      // modify the node directly (via itkMRMLIDImageIO). We don't want any
      // event to be fired from the thread, but from the main thread instead.
      this->Internal->StartRescheduleNodeEvents(nd);
    }
  }
  // Start rescheduling the output nodes events.
  if (commandType == SharedObjectModule)
  {
    this->Internal->RescheduleCallback->RescheduleEventsFromThreadID(vtkMultiThreader::GetCurrentThreadID(), true);
  }
  // write out the miniscene if needed
  if (miniscene->GetNumberOfNodes() > 0)
  {
    miniscene->Commit(minisceneFilename.c_str());

    // tell the storage nodes in the miniscene to write their data
    vtkCollection* nodes = miniscene->GetNodes();
    for (int n = 0; n < nodes->GetNumberOfItems(); n++)
    {
      vtkMRMLNode* node = (vtkMRMLNode*)nodes->GetItemAsObject(n);

      vtkMRMLStorableNode* storable = vtkMRMLStorableNode::SafeDownCast(node);
      if (storable)
      {
        if (storable->GetStorageNode())
        {
          storable->GetStorageNode()->WriteData(storable);
        }
      }
    }
  }

  // build the command line
  //
  //
  std::vector<std::string> commandLineAsString;

  // Command to execute
  if (node0->GetModuleDescription().GetLocation() != std::string("") && commandType == CommandLineModule
      && node0->GetModuleDescription().GetLocation() != node0->GetModuleDescription().GetTarget())
  {
    vtkDebugMacro("Setting a location for a command line module: " << node0->GetModuleDescription().GetLocation()
                                                                   << ", target is '"
                                                                   << node0->GetModuleDescription().GetTarget() << "'");
    // it's a command line module that has an executable that is used to run the target
    commandLineAsString.push_back(node0->GetModuleDescription().GetLocation());
  }
  commandLineAsString.push_back(node0->GetModuleDescription().GetTarget());

  // Add a command line flag for the process information structure
  if (commandType == SharedObjectModule)
  {
    commandLineAsString.emplace_back("--processinformationaddress");

    char tname[256];
    sprintf(tname, "%p", node0->GetModuleDescription().GetProcessInformation());

    commandLineAsString.emplace_back(tname);
  }

  // Add a command line flag for a file of return types
  if (node0->GetModuleDescription().HasReturnParameters())
  {
    commandLineAsString.emplace_back("--returnparameterfile");

    std::ostringstream pidString;
#ifdef _WIN32
    pidString << GetCurrentProcessId();
#else
    pidString << getpid();
#endif

    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";

    std::ostringstream code;
    for (int ii = 0; ii < 10; ii++)
    {
      code << alphanum[this->Internal->RandomGenerator() % (sizeof(alphanum) - 1)];
    }
    std::string returnFile = temporaryDirectory + "/" + pidString.str() + "_" + code.str() + ".params";

    commandLineAsString.push_back(returnFile);

    // We will need to load this results file back when module completes
    nodesToReload[node0->GetID()] = returnFile;

    // This is an extra file we will need to delete
    filesToDelete.insert(returnFile);
  }

  // Run over all the parameters with flags
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
  {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
    {
      std::string prefix;
      std::string flag;
      bool hasFlag = false;

      if ((*pit).GetLongFlag() != "")
      {
        prefix = "--";
        flag = (*pit).GetLongFlag();
        hasFlag = true;
      }
      else if ((*pit).GetFlag() != "")
      {
        prefix = "-";
        flag = (*pit).GetFlag();
        hasFlag = true;
      }

      if (hasFlag)
      {
        if ((*pit).GetTag() != "boolean" && (*pit).GetTag() != "file" && (*pit).GetTag() != "directory"
            && (*pit).GetTag() != "string" && (*pit).GetTag() != "integer-vector" && (*pit).GetTag() != "float-vector"
            && (*pit).GetTag() != "double-vector" && (*pit).GetTag() != "string-vector" && (*pit).GetTag() != "image"
            && (*pit).GetTag() != "point" && (*pit).GetTag() != "pointfile" && (*pit).GetTag() != "region"
            && (*pit).GetTag() != "transform" && (*pit).GetTag() != "geometry" && (*pit).GetTag() != "table"
            && (*pit).GetTag() != "measurement")
        {
          // simple parameter, write flag and value
          commandLineAsString.push_back(prefix + flag);
          commandLineAsString.push_back((*pit).GetValue());
          continue;
        }
        if ((*pit).GetTag() == "boolean")
        {
          if ((*pit).GetValue() == "true")
          {
            commandLineAsString.push_back(prefix + flag);
          }
          continue;
        }
        if ((*pit).GetTag() == "file" || (*pit).GetTag() == "directory" || (*pit).GetTag() == "string"
            || (*pit).GetTag() == "integer-vector" || (*pit).GetTag() == "float-vector"
            || (*pit).GetTag() == "double-vector" || (*pit).GetTag() == "string-vector")
        {
          // Only write out the flag if value is not empty
          if ((*pit).GetValue() != "")
          {
            commandLineAsString.push_back(prefix + flag);
            commandLineAsString.push_back((*pit).GetValue());
          }
          continue;
        }
        if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry" || (*pit).GetTag() == "transform"
            || (*pit).GetTag() == "table" || (*pit).GetTag() == "measurement" || (*pit).GetTag() == "pointfile")
        {
          std::string fname;

          // default to using filename in the read/write maps
          // established earlier
          MRMLIDToFileNameMap::const_iterator id2fn;

          id2fn = nodesToWrite.find((*pit).GetValue());
          if ((*pit).GetChannel() == "input" && id2fn != nodesToWrite.end())
          {
            fname = (*id2fn).second;
          }

          id2fn = nodesToReload.find((*pit).GetValue());
          if ((*pit).GetChannel() == "output" && id2fn != nodesToReload.end())
          {
            fname = (*id2fn).second;
          }

          // check to see if we need to remap to a scene file and node id
          MRMLIDMap::iterator mit = sceneToMiniSceneMap.find((*pit).GetValue());
          if (mit != sceneToMiniSceneMap.end())
          {
            // node is being sent inside of a scene, so use the scene
            // filename and the remapped id
            fname = minisceneFilename + "#" + (*mit).second;
          }

          // Only put out the flag if the node in nodesToWrite/Reload
          // or in the mini-scene
          if (fname.size() > 0)
          {
            commandLineAsString.push_back(prefix + flag);
            commandLineAsString.push_back(fname);
          }

          continue;
        }
        if ((*pit).GetTag() == "point")
        {
          // get coordinate system
          int coordinateSystem = this->GetCoordinateSystemFromString((*pit).GetCoordinateSystem().c_str());
          // get the fiducial list node
          vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID((*pit).GetValue().c_str());
          vtkMRMLDisplayableHierarchyNode* points = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
          vtkMRMLDisplayableNode* markups = vtkMRMLDisplayableNode::SafeDownCast(node);
          if (markups && markups->IsA("vtkMRMLMarkupsNode"))
          {
            int multipleFlag = 1;
            if ((*pit).GetMultiple() == "false")
            {
              multipleFlag = 0;
            }
            markups->WriteCLI(commandLineAsString, prefix + flag, coordinateSystem, multipleFlag);
          }
          else if (points)
          {
            // find the children of this hierarchy node
            vtkNew<vtkCollection> col;
            points->GetChildrenDisplayableNodes(col.GetPointer());
            vtkDebugMacro("Getting children displayable nodes from points " << points->GetID());
            unsigned int numChildren = col->GetNumberOfItems();
            vtkDebugMacro("Displayable hierarchy has " << numChildren << " child nodes");
            int multipleFlag = 1;
            if ((*pit).GetMultiple() == "false")
            {
              multipleFlag = 0;
            }
            for (unsigned int c = 0; c < numChildren; c++)
            {
              // the hierarchy nodes have a sorting index that's respected by
              // GetNthChildNode
              vtkMRMLHierarchyNode* nthHierarchyNode = points->GetNthChildNode(c);
              // then get the displayable node from that hierarchy node
              if (nthHierarchyNode)
              {
                vtkMRMLDisplayableHierarchyNode* nthDisplayableHierarchyNode =
                  vtkMRMLDisplayableHierarchyNode::SafeDownCast(nthHierarchyNode);
                vtkMRMLDisplayableNode* displayableNode = nullptr;
                if (nthDisplayableHierarchyNode)
                {
                  displayableNode = nthDisplayableHierarchyNode->GetDisplayableNode();
                }
                if (displayableNode)
                {
                  vtkDebugMacro("Found displayable node with id " << displayableNode->GetID());
                  displayableNode->WriteCLI(commandLineAsString, prefix + flag, coordinateSystem);
                  if (multipleFlag == 0)
                  {
                    // only write out the first child in the hierarchy
                    break;
                  }
                }
              }
            }
          }
          continue;
        }
        if ((*pit).GetTag() == "pointfile")
        {
          // get coordinate system
          int coordinateSystem = this->GetCoordinateSystemFromString((*pit).GetCoordinateSystem().c_str());
          // get the fiducial list node
          vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID((*pit).GetValue().c_str());
          vtkMRMLDisplayableNode* markups = vtkMRMLDisplayableNode::SafeDownCast(node);
          if (markups && markups->IsA("vtkMRMLMarkupsNode"))
          {
            vtkMRMLStorageNode* mrmlStorageNode = markups->GetStorageNode();
            if (mrmlStorageNode)
            {
              vtkMRMLMarkupsStorageNode* markupsStorageNode = vtkMRMLMarkupsStorageNode::SafeDownCast(mrmlStorageNode);
              if (markupsStorageNode)
              {
                markupsStorageNode->SetCoordinateSystem(coordinateSystem);
              }
            }
          }
        }
        if ((*pit).GetTag() == "region")
        {
          // get coordinate system
          int coordinateSystem = this->GetCoordinateSystemFromString((*pit).GetCoordinateSystem().c_str());
          // check multiple flag
          int multipleFlag = 1;
          if ((*pit).GetMultiple() == "false")
          {
            multipleFlag = 0;
          }

          // get the region node
          vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID((*pit).GetValue().c_str());
          vtkMRMLROIListNode* regions = vtkMRMLROIListNode::SafeDownCast(node);

          vtkMRMLDisplayableHierarchyNode* points = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
          vtkMRMLDisplayableNode* roi = vtkMRMLDisplayableNode::SafeDownCast(node);

          if (regions)
          {
            // check to see if module can handle more than one region
            long numberOfSelectedRegions = 0;
            for (int i = 0; i < regions->GetNumberOfROIs(); ++i)
            {
              if (regions->GetNthROISelected(i))
              {
                numberOfSelectedRegions++;
              }
            }

            if (numberOfSelectedRegions == 1 || (*pit).GetMultiple() == "true")
            {
              for (int i = 0; i < regions->GetNumberOfROIs(); ++i)
              {
                double* pt;
                double* Radius;
                std::ostringstream roiAsString;

                if (regions->GetNthROISelected(i))
                {
                  pt = regions->GetNthROIXYZ(i);
                  Radius = regions->GetNthROIRadiusXYZ(i);
                  roiAsString << pt[0] << "," << pt[1] << "," << pt[2] << "," << Radius[0] << "," << Radius[1] << ","
                              << Radius[2];

                  commandLineAsString.push_back(prefix + flag);
                  commandLineAsString.push_back(roiAsString.str());
                }
              }
            }
          }
          else if (roi && roi->IsA("vtkMRMLMarkupsROINode"))
          {
            roi->WriteCLI(commandLineAsString, prefix + flag, coordinateSystem, multipleFlag);
          }
          else if (points)
          {
            // find the children of this hierarchy node
            vtkNew<vtkCollection> col;
            points->GetChildrenDisplayableNodes(col.GetPointer());
            vtkDebugMacro("Getting children displayable nodes from points " << points->GetID());
            unsigned int numChildren = col->GetNumberOfItems();
            vtkDebugMacro("Displayable hierarchy has " << numChildren << " child nodes");
            for (unsigned int c = 0; c < numChildren; c++)
            {
              // the hierarchy nodes have a sorting index that's respected by
              // GetNthChildNode
              vtkMRMLHierarchyNode* nthHierarchyNode = points->GetNthChildNode(c);
              // then get the displayable node from that hierarchy node
              if (nthHierarchyNode)
              {
                vtkMRMLDisplayableHierarchyNode* nthDisplayableHierarchyNode =
                  vtkMRMLDisplayableHierarchyNode::SafeDownCast(nthHierarchyNode);
                vtkMRMLDisplayableNode* displayableNode = nullptr;
                if (nthDisplayableHierarchyNode)
                {
                  displayableNode = nthDisplayableHierarchyNode->GetDisplayableNode();
                }
                if (displayableNode)
                {
                  displayableNode->WriteCLI(commandLineAsString, prefix + flag, coordinateSystem, multipleFlag);
                }
              }
            }
          }
          continue;
        }
      }
    }
  }

  // now tack on any parameters that are based on indices
  //
  // build a list of indices to traverse in order
  std::map<int, ModuleParameter> indexmap;
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
  {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
    {
      if ((*pit).GetIndex() != "")
      {
        indexmap[atoi((*pit).GetIndex().c_str())] = (*pit);
      }
    }
  }

  // walk the index parameters in order
  std::map<int, ModuleParameter>::const_iterator iit;
  for (iit = indexmap.begin(); iit != indexmap.end(); ++iit)
  {
    // Most parameter types have a reasonable default. However,
    // parameters like image, geometry, file, and directory have no
    // defaults that are reasonable for index parameters
    if ((*iit).second.GetTag() != "image" && (*iit).second.GetTag() != "geometry"
        && (*iit).second.GetTag() != "transform" && (*iit).second.GetTag() != "table"
        && (*iit).second.GetTag() != "measurement" && (*iit).second.GetTag() != "file"
        && (*iit).second.GetTag() != "directory" && (*iit).second.GetTag() != "string"
        && (*iit).second.GetTag() != "point" && (*iit).second.GetTag() != "pointfile"
        && (*iit).second.GetTag() != "region" && (*iit).second.GetTag() != "integer-vector"
        && (*iit).second.GetTag() != "float-vector" && (*iit).second.GetTag() != "double-vector"
        && (*iit).second.GetTag() != "string-vector")
    {
      commandLineAsString.push_back((*iit).second.GetValue());
    }
    else if ((*iit).second.GetTag() == "file" || (*iit).second.GetTag() == "directory"
             || (*iit).second.GetTag() == "string" || (*iit).second.GetTag() == "integer-vector"
             || (*iit).second.GetTag() == "float-vector" || (*iit).second.GetTag() == "double-vector"
             || (*iit).second.GetTag() == "string-vector")
    {
      if ((*iit).second.GetValue() != "")
      {
        commandLineAsString.push_back((*iit).second.GetValue());
      }
      else
      {
        std::string errorText = "No value assigned to \"" + (*iit).second.GetLabel() + "\"";
        vtkErrorMacro(<< errorText.c_str());
        node0->SetOutputText("", false);
        node0->SetErrorText(errorText, false);
        node0->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
        this->GetApplicationLogic()->RequestModified(node0);
        return;
      }
    }
    else if ((*iit).second.GetTag() == "point" || (*iit).second.GetTag() == "region")
    {
      std::string errorText = "Fiducials and ROIs are not currently supported as index arguments to modules.";
      vtkErrorMacro(<< errorText.c_str());
      node0->SetOutputText("", false);
      node0->SetErrorText(errorText, false);
      node0->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
      this->GetApplicationLogic()->RequestModified(node0);
      return;
    }
    else
    {
      // image or geometry or transform or table or measurement index parameter or point file

      std::string fname;

      MRMLIDToFileNameMap::const_iterator id2fn;

      if ((*iit).second.GetChannel() == "input")
      {
        // Check to make sure the index parameter is set
        id2fn = nodesToWrite.find((*iit).second.GetValue());
        if (id2fn != nodesToWrite.end())
        {
          fname = (*id2fn).second;
        }
      }
      else if ((*iit).second.GetChannel() == "output")
      {
        // Check to make sure the index parameter is set
        id2fn = nodesToReload.find((*iit).second.GetValue());
        if (id2fn != nodesToReload.end())
        {
          fname = (*id2fn).second;
        }
      }

      // check to see if we need to remap to a scene file and node id
      MRMLIDMap::iterator mit = sceneToMiniSceneMap.find((*iit).second.GetValue());
      if (mit != sceneToMiniSceneMap.end())
      {
        // node is being sent inside of a scene, so use the scene
        // filename and the remapped id
        fname = minisceneFilename + "#" + (*mit).second;
      }

      if (fname.size() > 0)
      {
        commandLineAsString.push_back(fname);
      }
      else
      {
        std::string errorText =
          "No " + (*iit).second.GetChannel() + " data assigned to \"" + (*iit).second.GetLabel() + "\"";
        vtkErrorMacro(<< errorText.c_str());
        node0->SetOutputText("", false);
        node0->SetErrorText(errorText, false);
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified(node0);
        return;
      }
    }
  }

  // copy the command line arguments into an array of pointers to
  // chars
  char** command = new char*[commandLineAsString.size() + 1];
  for (std::vector<std::string>::size_type i = 0; i < commandLineAsString.size(); ++i)
  {
    command[i] = const_cast<char*>(commandLineAsString[i].c_str());
  }
  command[commandLineAsString.size()] = nullptr;

  // print the command line
  //
  std::stringstream information0;
  information0 << node0->GetModuleDescription().GetTitle() << " command line: " << std::endl << std::endl;
  for (std::vector<std::string>::size_type i = 0; i < commandLineAsString.size(); ++i)
  {
    information0 << command[i] << " ";
  }
  vtkInfoMacro(<< information0.str());

  // run the filter
  //
  //
  node0->GetModuleDescription().GetProcessInformation()->Initialize();
  node0->SetOutputText("", false);
  node0->SetErrorText("", false);
  node0->SetStatus(vtkMRMLCommandLineModuleNode::Running, false);
  this->GetApplicationLogic()->RequestModified(node0);
  if (commandType == CommandLineModule)
  {
    // Run as a command line module
    //
    //

    // Unset ITK_AUTOLOAD_PATH environment variable to prevent the CLI from
    // loading the itkMRMLIDIOPlugin plugin because executable CLIs read images
    // from file and not from shared memory. Worst the plugin in the CLI
    // could clash by loading libraries (ITK, VTK, MRML) other than the
    // statically linked to the executable.
    // Historically, there was an nvidia driver bug that causes the module
    // to fail on exit with undefined symbol.
    std::string saveITKAutoLoadPath;
    itksys::SystemTools::GetEnv("ITK_AUTOLOAD_PATH", saveITKAutoLoadPath);
    std::string emptyString("ITK_AUTOLOAD_PATH=");
    int putSuccess = itksys::SystemTools::PutEnv(const_cast<char*>(emptyString.c_str()));
    if (!putSuccess)
    {
      vtkErrorMacro("Unable to reset ITK_AUTOLOAD_PATH.");
    }
    //
    // now run the process
    //
    itksysProcess* process = itksysProcess_New();

    this->Internal->Processes.push_back(process);

    // setup the command
    itksysProcess_SetCommand(process, command);
    itksysProcess_SetOption(process, itksysProcess_Option_Detach, 0);
    itksysProcess_SetOption(process, itksysProcess_Option_HideWindow, 1);
    // itksysProcess_SetTimeout(process, 5.0); // 5 seconds

    // execute the command
    itksysProcess_Execute(process);

    // restore the load path
    std::string putEnvString = ("ITK_AUTOLOAD_PATH=");
    putEnvString = putEnvString + saveITKAutoLoadPath;
    putSuccess = itksys::SystemTools::PutEnv(const_cast<char*>(putEnvString.c_str()));
    if (!putSuccess)
    {
      vtkErrorMacro("Unable to restore ITK_AUTOLOAD_PATH. ");
    }

    // Wait for the command to finish
    char* tbuffer;
    int length;
    int pipe;
    const double timeoutlimit = 0.1; // tenth of a second
    double timeout = timeoutlimit;
    std::string stdoutbuffer;
    std::string stdoutbufferWithoutProgressInfo; // stdout, with progress information removed
    std::string stderrbuffer;
    std::string::size_type tagend;
    std::string::size_type tagstart;
    while ((pipe = itksysProcess_WaitForData(process, &tbuffer, &length, &timeout)) != 0)
    {
      // increment the elapsed time
      bool enableUpdateOutputDuringExecution = node0->IsContinuousOutputUpdate();
      node0->GetModuleDescription().GetProcessInformation()->ElapsedTime += (timeoutlimit - timeout);
      this->GetApplicationLogic()->RequestModified(node0);

      // reset the timeout value
      timeout = timeoutlimit;

      // Check to see if the plugin was cancelled
      if (node0->GetModuleDescription().GetProcessInformation()->Abort)
      {
        itksysProcess_Kill(process);
        this->Internal->Processes.erase(
          std::find(this->Internal->Processes.begin(), this->Internal->Processes.end(), process));
        node0->GetModuleDescription().GetProcessInformation()->Progress = 0;
        node0->GetModuleDescription().GetProcessInformation()->StageProgress = 0;
        this->GetApplicationLogic()->RequestModified(node0);
        break;
      }

      // Capture the output from the filter
      if (length != 0 && tbuffer != nullptr)
      {
        if (pipe == itksysProcess_Pipe_STDOUT)
        {
          // std::cout << "STDOUT: " << std::string(tbuffer, length) << std::endl;
          std::string stdoutNewContent(tbuffer, length);
          stdoutbuffer.append(stdoutNewContent);

          bool foundTag = false;
          // search for the last occurrence of </filter-progress>
          tagend = stdoutbuffer.rfind("</filter-progress>");
          if (tagend != std::string::npos)
          {
            tagstart = stdoutbuffer.rfind("<filter-progress>");
            if (tagstart != std::string::npos)
            {
              std::string progressString(stdoutbuffer, tagstart + 17, tagend - tagstart - 17);
              node0->GetModuleDescription().GetProcessInformation()->Progress = atof(progressString.c_str());
              foundTag = true;
            }
          }
          // search for the last occurrence of </filter-stage-progress>
          tagend = stdoutbuffer.rfind("</filter-stage-progress>");
          if (tagend != std::string::npos)
          {
            tagstart = stdoutbuffer.rfind("<filter-stage-progress>");
            if (tagstart != std::string::npos)
            {
              std::string progressString(stdoutbuffer, tagstart + 23, tagend - tagstart - 23);
              node0->GetModuleDescription().GetProcessInformation()->StageProgress = atof(progressString.c_str());
              foundTag = true;
            }
          }

          // search for the last occurrence of </filter-name>
          tagend = stdoutbuffer.rfind("</filter-name>");
          if (tagend != std::string::npos)
          {
            tagstart = stdoutbuffer.rfind("<filter-name>");
            if (tagstart != std::string::npos)
            {
              std::string filterString(stdoutbuffer, tagstart + 13, tagend - tagstart - 13);
              strncpy(
                node0->GetModuleDescription().GetProcessInformation()->ProgressMessage, filterString.c_str(), 1023);
              foundTag = true;
            }
          }

          // search for the last occurrence of </filter-comment>
          tagend = stdoutbuffer.rfind("</filter-comment>");
          if (tagend != std::string::npos)
          {
            tagstart = stdoutbuffer.rfind("<filter-comment>");
            if (tagstart != std::string::npos)
            {
              std::string progressMessage(stdoutbuffer, tagstart + 16, tagend - tagstart - 16);
              strncpy(
                node0->GetModuleDescription().GetProcessInformation()->ProgressMessage, progressMessage.c_str(), 1023);
              foundTag = true;
            }
          }
          if (enableUpdateOutputDuringExecution)
          {
            vtkSlicerCLIModuleLogic::RemoveProgressInfoFromProcessOutput(stdoutNewContent);
            stdoutbufferWithoutProgressInfo.append(stdoutNewContent);
            node0->SetOutputText(stdoutbufferWithoutProgressInfo, false);
          }
          if (foundTag || enableUpdateOutputDuringExecution)
          {
            this->GetApplicationLogic()->RequestModified(node0);
          }
        }
        else if (pipe == itksysProcess_Pipe_STDERR)
        {
          stderrbuffer = stderrbuffer.append(tbuffer, length);
          if (enableUpdateOutputDuringExecution)
          {
            node0->SetErrorText(stderrbuffer, false);
            this->GetApplicationLogic()->RequestModified(node0);
          }
        }
      }
    }
    this->Internal->ProcessesKillLock.lock();
    itksysProcess_WaitForExit(process, nullptr);
    this->Internal->ProcessesKillLock.unlock();

    vtkSlicerCLIModuleLogic::RemoveProgressInfoFromProcessOutput(stdoutbuffer);
    if (stdoutbuffer.size() > 0)
    {
      vtkInfoMacro(<< node0->GetModuleDescription().GetTitle() << " standard output:\n\n" << stdoutbuffer);
    }
    node0->SetOutputText(stdoutbuffer, false);

    if (stderrbuffer.size() > 0)
    {
      vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " standard error:\n\n" << stderrbuffer);
    }
    node0->SetErrorText(stderrbuffer, false);

    // check the exit state / error state of the process
    if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
    {
      node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
      this->GetApplicationLogic()->RequestModified(node0);
    }
    else
    {
      int result = itksysProcess_GetState(process);
      if (result == itksysProcess_State_Exited)
      {
        // executable exited cleanly and must of done
        // "something"
        if (itksysProcess_GetExitValue(process) == 0)
        {
          // executable exited without errors,
          vtkInfoMacro(<< node0->GetModuleDescription().GetTitle() << " completed without errors");
        }
        else
        {
          vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " completed with errors");
          node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
          this->GetApplicationLogic()->RequestModified(node0);
        }
      }
      else if (result == itksysProcess_State_Expired)
      {
        vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " timed out");
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified(node0);
      }
      else
      {
        std::string message;
        if (result == itksysProcess_State_Exception)
        {
          int excResult = itksysProcess_GetExitException(process);
          switch (excResult)
          {
            case itksysProcess_Exception_None:
              message = "terminated with no exceptions";
              break;
            case itksysProcess_Exception_Fault:
              message = "terminated with a fault";
              break;
            case itksysProcess_Exception_Illegal:
              message = "terminated with an illegal instruction";
              break;
            case itksysProcess_Exception_Interrupt:
              message = "terminated with an interrupt";
              break;
            case itksysProcess_Exception_Numerical:
              message = "terminated with a numerical fault";
              break;
            case itksysProcess_Exception_Other:
              message = "terminated with an unknown exception";
              break;
          }
        }
        else if (result == itksysProcess_State_Error)
        {
          message = "process was null when state was queried";
        }
        else
        {
          std::stringstream information;
          information << "unknown termination (result=" << result << ")";
          message = information.str();
        }
        vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " " << message);
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified(node0);
      }

      // clean up
      this->Internal->ProcessesKillLock.lock();
      this->Internal->Processes.erase(
        std::find(this->Internal->Processes.begin(), this->Internal->Processes.end(), process));
      itksysProcess_Delete(process);
      this->Internal->ProcessesKillLock.unlock();
    }
  }
  else if (commandType == SharedObjectModule)
  {
    // Run as a shared object module
    //
    //

    std::ostringstream coutstringstream;
    std::ostringstream cerrstringstream;
    std::streambuf* origcoutrdbuf = std::cout.rdbuf();
    std::streambuf* origcerrrdbuf = std::cerr.rdbuf();
    int returnValue = 0;
    try
    {
      if (this->Internal->RedirectModuleStreams)
      {
        // redirect the streams
        std::cout.rdbuf(coutstringstream.rdbuf());
        std::cerr.rdbuf(cerrstringstream.rdbuf());
      }

      // run the module
      if (entryPoint != nullptr)
      {
        returnValue = (*entryPoint)(commandLineAsString.size(), command);
      }

      // report the output
      if (coutstringstream.str().size() > 0)
      {
        vtkInfoMacro(<< node0->GetModuleDescription().GetTitle() + " standard output:\n\n" + coutstringstream.str());
      }
      node0->SetOutputText(coutstringstream.str(), false);
      if (cerrstringstream.str().size() > 0)
      {
        vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() + " standard error:\n\n" + cerrstringstream.str());
      }
      node0->SetErrorText(cerrstringstream.str(), false);

      if (this->Internal->RedirectModuleStreams)
      {
        // reset the streams
        std::cout.rdbuf(origcoutrdbuf);
        std::cerr.rdbuf(origcerrrdbuf);
      }
    }
    catch (itk::ExceptionObject& exc)
    {
      if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
      {
        vtkInfoMacro(<< node0->GetModuleDescription().GetTitle() << " cancelled.");
      }
      else
      {
        vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " terminated with an exception: " << exc);
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified(node0);
      }

      std::cout.rdbuf(origcoutrdbuf);
      std::cerr.rdbuf(origcerrrdbuf);
    }
    catch (...)
    {
      vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " terminated with an unknown exception.");
      node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
      this->GetApplicationLogic()->RequestModified(node0);

      std::cout.rdbuf(origcoutrdbuf);
      std::cerr.rdbuf(origcerrrdbuf);
    }
    if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
    {
      node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
      this->GetApplicationLogic()->RequestModified(node0);
    }
    // Check the return status of the module
    if (returnValue)
    {
      vtkErrorMacro(<< node0->GetModuleDescription().GetTitle() << " returned " << returnValue
                    << " which probably indicates an error.");
      node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
      this->GetApplicationLogic()->RequestModified(node0);
      std::cout.rdbuf(origcoutrdbuf);
      std::cerr.rdbuf(origcerrrdbuf);
    }
  }

  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
  {
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
    this->GetApplicationLogic()->RequestModified(node0);
  }
  else if (node0->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelled
           && node0->GetStatus() != vtkMRMLCommandLineModuleNode::CompletedWithErrors)
  {
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Completing, false);
    this->GetApplicationLogic()->RequestModified(node0);
  }
  // reset the progress to zero
  node0->GetModuleDescription().GetProcessInformation()->Progress = 0;
  node0->GetModuleDescription().GetProcessInformation()->StageProgress = 0;
  this->GetApplicationLogic()->RequestModified(node0);

  // Stop rescheduling the output nodes events.
  if (commandType == SharedObjectModule)
  {
    this->Internal->RescheduleCallback->RescheduleEventsFromThreadID(vtkMultiThreader::GetCurrentThreadID(), false);
  }

  // import the results if the plugin was allowed to complete
  //
  //
  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Completing)
  {
    // reload nodes
    for (id2fn0 = nodesToReload.begin(); id2fn0 != nodesToReload.end(); ++id2fn0)
    {
      // Is this node one that was put in the miniscene? Nodes in the
      // miniscene will be handled later
      //
      MRMLIDMap::iterator mit = sceneToMiniSceneMap.find((*id2fn0).first);
      if (mit == sceneToMiniSceneMap.end())
      {
        // Node is not being communicated in the miniscene, load via a file

        // Make request that data be reloaded. The data will loaded and
        // rendered in the main gui thread.  Data to be reloaded can be
        // safely deleted after the load. (It would not make sense for
        // two outputs of a module to produce the same file to be
        // reloaded.) We assume that if the user is looking at the node
        // now, he/she will still be looking at the node by the time the
        // data is reloaded by the main thread.
        bool displayData = this->IsCommandLineModuleNodeUpdatingDisplay(node0);

        // displayData causes resetting of slice views (the output volume is shown in the background,
        // the foreground volume is cleared, the middle slice is selected, the FOV is reset, etc.)
        // which is not desirable when AutoRun is enabled (because the user already set up the
        // slice viewers for optimal viewing)
        if (node0->GetAutoRun())
        {
          displayData = false;
        }

        bool deleteFile = this->GetDeleteTemporaryFiles();
        vtkMTimeType requestUID = this->GetApplicationLogic()->RequestReadFile(
          (*id2fn0).first.c_str(), (*id2fn0).second.c_str(), displayData, deleteFile);
        this->Internal->SetLastRequest(node0, requestUID);

        // If we are reloading a file, then we know that it is a file
        // that needs to be removed.  It wouldn't make sense for two
        // outputs of a module to produce the same file to be reloaded.
        filesToDelete.erase((*id2fn0).second);

        if (commandType == SharedObjectModule)
        {
          vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID((*id2fn0).first);
          this->Internal->StopRescheduleNodeEvents(node);
        }
      }
    }

    // if there was a miniscene that needs loading, request it
    if (miniscene->GetNumberOfNodes() > 0)
    {
      // don't load the mini scene if errors were found or the cli was cancelled.
      if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Completing)
      {
        bool displayData = this->IsCommandLineModuleNodeUpdatingDisplay(node0);
        bool deleteFile = this->GetDeleteTemporaryFiles();

        // Convert the index map to two vectors so that we can pass it to
        // a function in a different library (Win32 limitation)
        std::vector<std::string> keys, values;

        MRMLIDMap::iterator mit;
        for (mit = sceneToMiniSceneMap.begin(); mit != sceneToMiniSceneMap.end(); ++mit)
        {
          // only load the nodes that are needed back into the main scene
          MRMLIDToFileNameMap::iterator rit = nodesToReload.find((*mit).first);

          if (rit != nodesToReload.end())
          {
            keys.push_back((*mit).first);
            values.push_back((*mit).second);
          }
        }

        // Place a request to read the miniscene and map any ids as necessary
        vtkMTimeType requestUID =
          this->GetApplicationLogic()->RequestReadScene(minisceneFilename, keys, values, displayData, deleteFile);
        this->Internal->SetLastRequest(node0, requestUID);
      }
      else // but delete the temporary file.
      {
        filesToDelete.insert(minisceneFilename);
      }
    }

    // Rewire the MRML scene as directed
    //
    // 1. If an output transform has a reference, then that reference is placed under the transform.
    //        (subject, predicate, object) = (reference, setTransform, transform)
    //
    // 2. If an output volume/model has a reference, then that output volume/model is placed in the
    // same spot of the subject hierarchy as the reference.
    //        (subject, predicate, object) = (reference, setParent, volume/model)
    //
    // 3. If a node has a forward reference (reference as a child element not an attribute, which
    // specifies reference from the place where it was defined as opposed to 1. which points
    // backwards), then a node reference is added from that node to the specified node with a given
    // role.
    //        (subject, predicate, object) = (node, addNodeReference, referencedNode)
    //
    // Warning: Never make any call that results in a Modified event. Instead, do a request
    //          (see the solutions below)
    //
    for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
      // iterate over each parameter in this group
      std::vector<ModuleParameter>::const_iterator pbeginit = (*pgit).GetParameters().begin();
      std::vector<ModuleParameter>::const_iterator pendit = (*pgit).GetParameters().end();
      std::vector<ModuleParameter>::const_iterator pit;

      for (pit = pbeginit; pit != pendit; ++pit)
      {
        // does parameter have a reference attribute?
        if ((*pit).GetChannel() == "output" && (*pit).GetReference().size() > 0)
        {
          // does the reference parameter exist?
          if (node0->GetModuleDescription().HasParameter((*pit).GetReference()))
          {
            // get the id stored in the parameter referenced
            std::string reference = node0->GetModuleDescription().GetParameterValue((*pit).GetReference());
            if (reference.size() > 0)
            {
              // "reference" can mean different things based on the parameter type.
              // If  the parameter is a transform and the reference is transformable, then the transform hierarchy
              // of the reference is manipulated such that the reference is under the transform.
              // If the parameter is an image or a model, then the parameter is placed in subject hierarchy at the same
              // level as the reference.

              vtkMRMLNode* refNode = this->GetMRMLScene()->GetNodeByID(reference.c_str());
              if (refNode)
              {
                if ((*pit).GetTag() == "transform")
                {
                  std::string transformNodeID = (*pit).GetValue();
                  // is the reference a transformable node?
                  vtkMRMLTransformableNode* trefNode = vtkMRMLTransformableNode::SafeDownCast(refNode);
                  // Multiple transform nodes can refer to the same transformable node (e.g., linear or bspline
                  // transform can be computed, whichever is computed should transform the moving volume), we only want
                  // to use the transform that is not None.
                  if (trefNode != nullptr && !transformNodeID.empty())
                  {
                    // Place a request to update parent transform based of the referenced node
                    vtkMTimeType requestUID =
                      this->GetApplicationLogic()->RequestUpdateParentTransform(reference, transformNodeID);
                    this->Internal->SetLastRequest(node0, requestUID);
                  }
                }
                else if (((*pit).GetTag() == "image") || ((*pit).GetTag() == "geometry"))
                {
                  // Placing an image or model in the same position in a hierarchy as the reference
                  std::string updatedNodeID = (*pit).GetValue();
                  if (!updatedNodeID.empty())
                  {
                    // Place a request to update location in the subject hierarchy based of the referenced node
                    vtkMTimeType requestUID =
                      this->GetApplicationLogic()->RequestUpdateSubjectHierarchyLocation(updatedNodeID, reference);
                    this->Internal->SetLastRequest(node0, requestUID);
                  }
                }
              }
              else
              {
                vtkWarningMacro(<< "Cannot find referenced node " << (*pit).GetValue());
              }
            }
          }
          else
          {
            vtkWarningMacro(<< "Referenced parameter unknown: " << (*pit).GetReference());
          }
        }
        // does parameter have forward references?
        std::map<std::string, std::vector<std::string>> forwardReferences;
        (*pit).GetForwardReferences(forwardReferences);
        if (forwardReferences.size() > 0)
        {
          std::string referencingNodeID = (*pit).GetValue();
          if (referencingNodeID.size() > 0)
          {
            std::map<std::string, std::vector<std::string>>::const_iterator frit;
            for (frit = forwardReferences.begin(); frit != forwardReferences.end(); ++frit)
            {
              std::string role = frit->first;

              std::vector<std::string>::const_iterator frvit;
              for (frvit = frit->second.begin(); frvit != frit->second.end(); ++frvit)
              {
                // does the reference parameter exist?
                if (node0->GetModuleDescription().HasParameter(*frvit))
                {
                  // get the id stored in the parameter referenced
                  std::string referencedNodeID = node0->GetModuleDescription().GetParameterValue(*frvit);
                  if (referencedNodeID.size() > 0)
                  {
                    // Place a request to add node reference
                    vtkMTimeType requestUID =
                      this->GetApplicationLogic()->RequestAddNodeReference(referencingNodeID, referencedNodeID, role);
                    this->Internal->SetLastRequest(node0, requestUID);
                  }
                }
              } // for frvit (referenced nodes)
            }   // for frit (forward references)
          }
        }
      } // for pit
    }   // for pgit
  }     // if status == Completing

  // clean up
  //
  //
  delete[] command;

  // Remove any remaining temporary files.  At this point, these files
  // should be the files written as inputs to the module
  if (this->GetDeleteTemporaryFiles())
  {
    bool removed;
    std::set<std::string>::iterator fit;
    for (fit = filesToDelete.begin(); fit != filesToDelete.end(); ++fit)
    {
      if (itksys::SystemTools::FileExists((*fit).c_str()))
      {
        removed = static_cast<bool>(itksys::SystemTools::RemoveFile((*fit).c_str()));
        if (!removed)
        {
          vtkWarningMacro("Unable to delete temporary file " << *fit);
        }
      }
    }
  }

  // The CLI node is only completed if the outputs are loaded back into the
  // scene.
  // This is a special case where no output is needed to be read. Usually
  // the CLI node is set to Completed when the Application Logic has finished
  // to process all the requests.
  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Completing && this->Internal->GetLastRequest(node0) == 0)
  {
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
    this->GetApplicationLogic()->RequestModified(node0);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::ProgressCallback(void* who)
{
  LogicNodePair* lnp = reinterpret_cast<LogicNodePair*>(who);

  // All we need to do is tell the node that it was Modified.  The
  // shared object plugin modifies fields in the ProcessInformation directly.
  lnp->first->GetApplicationLogic()->RequestModified(lnp->second);
}

//-----------------------------------------------------------------------------
std::string vtkSlicerCLIModuleLogic::FindHiddenNodeID(const ModuleDescription& d, const ModuleParameter& p)
{
  std::string id = "None";

  if (p.GetHidden() == "true")
  {
    if (p.GetReference().size() > 0)
    {
      std::string reference;
      if (d.HasParameter(p.GetReference()))
      {
        reference = d.GetParameterValue(p.GetReference());

        if (p.GetTag() == "table")
        {
          if (p.GetType() == "color")
          {
            // go to the display node for the reference parameter and
            // get its color node
            vtkMRMLDisplayableNode* rn =
              vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(reference.c_str()));
            if (rn)
            {
              vtkMRMLDisplayNode* dn = rn->GetDisplayNode();
              if (dn)
              {
                // get the id of the color node
                if (dn->GetColorNode())
                {
                  id = dn->GetColorNode()->GetID();
                }
                else
                {
                  vtkErrorMacro(<< "Display node of the reference node does not have a color node. No value for "
                                   "\"table\" parameter.");
                }
              }
              else
              {
                vtkErrorMacro(<< "Reference node \"" << reference.c_str()
                              << "\" does not have a display node which is needed to find the color node.");
              }
            }
            else
            {
              vtkErrorMacro(<< "Reference node \"" << reference.c_str() << "\" does not exist in the scene.");
            }
          }
          else
          {
            vtkErrorMacro(<< "Hidden \"table\" parameters must be of type \"color\"");
          }
        }
        else if (p.GetTag() == "image")
        {
          // hidden parameters allowed for images to hide from GUI but
          // we don't currently do anything with them. This implies
          // that hidden parameters that are images must have flags
          // and not be index parameters.
        }
        else
        {
          vtkErrorMacro(<< "Hidden parameters not supported on \"" << p.GetType().c_str() << "\"");
        }
      }
      else
      {
        vtkErrorMacro(<< "Reference parameter \"" << p.GetReference().c_str() << "\" not found.");
      }
    }
    else
    {
      // no reference node
      if (this->GetDebug())
      {
        vtkErrorMacro(<< "Hidden parameter \"" << p.GetName().c_str() << "\" but no reference parameter.");
      }
    }
  }
  else
  {
    // not a hidden node, just return the default
    id = p.GetValue();
  }

  return id;
}

//----------------------------------------------------------------------------
bool vtkSlicerCLIModuleLogic ::IsCommandLineModuleNodeUpdatingDisplay(vtkMRMLCommandLineModuleNode* node) const
{
  // Update display except if the node has the updateDisplay attribute set to
  // "false".
  const char* updateDisplay = node->GetAttribute("UpdateDisplay");
  return !updateDisplay || (strcmp(updateDisplay, "false") != 0);
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node->IsA("vtkMRMLCommandLineModuleNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLCommandLineModuleNode::AutoRunEvent);
    vtkObserveMRMLNodeEventsMacro(node, events.GetPointer());
  }
  this->Superclass::OnMRMLSceneNodeAdded(node);
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::ProcessMRMLLogicsEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller->IsA("vtkSlicerApplicationLogic") && event == vtkSlicerApplicationLogic::RequestProcessedEvent)
  {
    vtkMTimeType uid = reinterpret_cast<vtkMTimeType>(callData);
    vtkInternal::RequestType::iterator it = std::find_if(
      this->Internal->LastRequests.begin(), this->Internal->LastRequests.end(), vtkInternal::FindRequest(uid));
    if (it != this->Internal->LastRequests.end())
    {
      vtkMRMLCommandLineModuleNode* node = it->second;
      // If the status is not Completing, then there should be no request made
      // on the application logic.
      assert(node->GetStatus() == vtkMRMLCommandLineModuleNode::Completing);
      this->Internal->LastRequests.erase(it);
      // we are not interested in any request anymore because the cli node is
      // Completed.

      node->SetStatus(vtkMRMLCommandLineModuleNode::Completed);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic ::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
  assert(node);
  // Observe only the CLI of the logic.
  vtkMRMLCommandLineModuleNode* cliNode = vtkMRMLCommandLineModuleNode::SafeDownCast(node);
  if (cliNode && cliNode->GetModuleTitle() == this->Internal->DefaultModuleDescription.GetTitle())
  {
    switch (event)
    {
      case vtkCommand::ModifiedEvent:
        break;
      case vtkMRMLCommandLineModuleNode::AutoRunEvent:
      {
        vtkMTimeType requestTime = reinterpret_cast<vtkMTimeType>(callData);
        // Make sure the CLI node has its AutoRun flag enabled and its mode is
        // valid.
        bool autoRun =
          cliNode->GetAutoRun() && cliNode->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunEnabledMask;
        // 0 is a special value that means the autorun request has just been
        // triggered, therefore the logic should wait to see if other requests
        // are made before calling Apply.
        if (requestTime != 0)
        {
          if (cliNode->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnAnyInputEvent)
          {
            autoRun = autoRun && (cliNode->GetInputMTime() <= requestTime);
          }
          if (cliNode->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter)
          {
            autoRun = autoRun && (cliNode->GetParameterMTime() <= requestTime);
          }
        }

        if (autoRun)
        {
          if ((requestTime == 0) || cliNode->IsBusy())
          {
            this->AutoRun(cliNode);
          }
          else
          {
            this->Apply(cliNode);
          }
        }
        else
        {
          vtkDebugMacro(<< "Skip AutoRun, it is not the last event");
        }
        break;
      }
      default:
        break;
    }
  }
  this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic ::AutoRun(vtkMRMLCommandLineModuleNode* node)
{
  if (!node || !node->GetAutoRun())
  {
    return;
  }
  unsigned int extraDelay = 0;

  if (node->IsBusy())
  {
    if (!(node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunCancelsRunningProcess))
    {
      return;
    }
    if (node->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelling)
    {
      // request the module execution to be cancelled.
      node->Cancel();
      // Wait until the module is stopped
      extraDelay = 100;
    }
  }
  vtkMTimeType requestTime = 1; // we don't want 0, so 1 works
  if (node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnChangedParameter)
  {
    requestTime = std::max(requestTime, node->GetParameterMTime());
  }
  if (node->GetAutoRunMode() & vtkMRMLCommandLineModuleNode::AutoRunOnAnyInputEvent)
  {
    requestTime = std::max(requestTime, node->GetInputMTime());
  }
  // Wait a bit (for potential other modifications) before re-running the module.
  this->GetApplicationLogic()->InvokeEventWithDelay(node->GetAutoRunDelay() + extraDelay,
                                                    node,
                                                    vtkMRMLCommandLineModuleNode::AutoRunEvent,
                                                    reinterpret_cast<void*>(requestTime));
}

//---------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::AddCompleteModelHierarchyToMiniScene(vtkMRMLScene* miniscene,
                                                                   vtkMRMLModelHierarchyNode* mhnd,
                                                                   MRMLIDMap* sceneToMiniSceneMap,
                                                                   std::set<std::string>& filesToDelete)
{
  if (!mhnd)
  {
    return;
  }

  // construct a list that includes this node and all its children
  std::vector<vtkMRMLHierarchyNode*> hnodes;
  mhnd->GetAllChildrenNodes(hnodes);
  hnodes.insert(hnodes.begin(), mhnd); // add the current node to the front of the vector

  // copy the entire hierarchy into the miniscene, we assume the nodes are ordered such that parents appear before
  // children
  for (std::vector<vtkMRMLHierarchyNode*>::iterator it = hnodes.begin(); it != hnodes.end(); ++it)
  {
    vtkMRMLNode* tnd = *it;
    vtkMRMLModelHierarchyNode* tmhnd = vtkMRMLModelHierarchyNode::SafeDownCast(tnd);

    if (!tmhnd)
    {
      std::cerr << "Child is not a model hierarchy node." << std::endl;
      continue;
    }

    // model hierarchy nodes need to get put in a scene
    vtkMRMLNode* cp = miniscene->CopyNode(tnd);
    vtkMRMLModelHierarchyNode* mhcp = vtkMRMLModelHierarchyNode::SafeDownCast(cp);

    // wire the parent relationship (again, we assume the parents appeared in the list before the children)
    vtkMRMLNode* p = tmhnd->GetParentNode();
    if (p)
    {
      // find parent in the sceneToMiniSceneMap
      MRMLIDMap::iterator mit = sceneToMiniSceneMap->find(p->GetID());
      if (mit != sceneToMiniSceneMap->end())
      {
        mhcp->SetParentNodeID((*mit).second.c_str());
      }
    }

    // keep track of what scene node corresponds to what miniscene node
    (*sceneToMiniSceneMap)[tnd->GetID()] = cp->GetID();

    // also add any display node
    vtkMRMLDisplayNode* dnd = tmhnd->GetDisplayNode();
    if (dnd)
    {
      vtkMRMLNode* dcp = miniscene->CopyNode(dnd);
      vtkMRMLDisplayNode* d = vtkMRMLDisplayNode::SafeDownCast(dcp);
      mhcp->SetAndObserveDisplayNodeID(d->GetID());
    }

    // add the actual model node
    vtkMRMLModelNode* mnd = vtkMRMLModelNode::SafeDownCast(tmhnd->GetDisplayableNode());
    if (mnd)
    {
      vtkMRMLNode* mcp = miniscene->CopyNode(mnd);

      vtkMRMLModelNode* tmcp = vtkMRMLModelNode::SafeDownCast(mcp);
      if (tmcp)
      {
        mhcp->SetAssociatedNodeID(tmcp->GetID());

        // add the display nodes for the model to the miniscene
        int ndn = mnd->GetNumberOfDisplayNodes();
        for (int i = 0; i < ndn; i++)
        {
          vtkMRMLDisplayNode* mdnd = mnd->GetNthDisplayNode(i);
          if (mdnd)
          {
            vtkMRMLDisplayNode* d = vtkMRMLDisplayNode::SafeDownCast(miniscene->CopyNode(mdnd));
            tmcp->AddAndObserveDisplayNodeID(d->GetID());
          }
        }

        // add the storage node for the model to the miniscene
        vtkMRMLStorageNode* msnd = mnd->GetStorageNode();
        if (msnd)
        {
          vtkMRMLModelStorageNode* s = vtkMRMLModelStorageNode::SafeDownCast(miniscene->CopyNode(msnd));
          std::string fname = this->ConstructTemporaryFileName(
            "geometry", "", tmcp->GetID(), std::vector<std::string>(), CommandLineModule);
          s->SetFileName(fname.c_str());
          filesToDelete.insert(fname);
          tmcp->SetAndObserveStorageNodeID(s->GetID());
        }
      }

      // keep track of the what scene node corresponds to what miniscene node
      (*sceneToMiniSceneMap)[mnd->GetID()] = mcp->GetID();
    }
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerCLIModuleLogic::SetDefaultParameterValue(const std::string& name, const std::string& value)
{
  return this->Internal->DefaultModuleDescription.SetParameterValue(name, value);
}

//---------------------------------------------------------------------------
std::string vtkSlicerCLIModuleLogic::GetDefaultParameterValue(const std::string& name) const
{
  return this->Internal->DefaultModuleDescription.GetParameterValue(name);
}

//---------------------------------------------------------------------------
int vtkSlicerCLIModuleLogic::GetCoordinateSystemFromString(const char* coordinateSystemStrPtr) const
{
  int coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS;
  if (!coordinateSystemStrPtr)
  {
    // empty, use default
    return coordinateSystem;
  }
  std::string coordinateSystemStr = coordinateSystemStrPtr;
  if (coordinateSystemStr.empty())
  {
    // empty, use default
    return coordinateSystem;
  }
  if (coordinateSystemStr.compare("ras") == 0)
  {
    coordinateSystem = vtkMRMLStorageNode::CoordinateSystemRAS;
  }
  else if (coordinateSystemStr.compare("lps") == 0)
  {
    coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS;
  }
  else
  {
    vtkErrorMacro("Invalid coordinateSystemFlag value: " << coordinateSystem << ", assuming lps");
  }
  return coordinateSystem;
}

//---------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::RemoveProgressInfoFromProcessOutput(std::string& text)
{
  // remove the embedded XML from the stdout stream
  //
  // Note that itksys::RegularExpression gives begin()/end() as
  // size_types not iterators. So we need to use the version of
  // erase that takes a position and length to erase.
  //
  itksys::RegularExpression filterProgressRegExp("<filter-progress>[^<]*</filter-progress>[ \t\n\r]*");
  while (filterProgressRegExp.find(text))
  {
    text.erase(filterProgressRegExp.start(), filterProgressRegExp.end() - filterProgressRegExp.start());
  }
  itksys::RegularExpression filterStageProgressRegExp("<filter-stage-progress>[^<]*</filter-stage-progress>[ \t\n\r]*");
  while (filterStageProgressRegExp.find(text))
  {
    text.erase(filterStageProgressRegExp.start(), filterStageProgressRegExp.end() - filterStageProgressRegExp.start());
  }
  itksys::RegularExpression filterNameRegExp("<filter-name>[^<]*</filter-name>[ \t\n\r]*");
  while (filterNameRegExp.find(text))
  {
    text.erase(filterNameRegExp.start(), filterNameRegExp.end() - filterNameRegExp.start());
  }
  itksys::RegularExpression filterCommentRegExp("<filter-comment>[^<]*</filter-comment>[ \t\n\r]*");
  while (filterCommentRegExp.find(text))
  {
    text.erase(filterCommentRegExp.start(), filterCommentRegExp.end() - filterCommentRegExp.start());
  }
  itksys::RegularExpression filterTimeRegExp("<filter-time>[^<]*</filter-time>[ \t\n\r]*");
  while (filterTimeRegExp.find(text))
  {
    text.erase(filterTimeRegExp.start(), filterTimeRegExp.end() - filterTimeRegExp.start());
  }
  itksys::RegularExpression filterStartRegExp("<filter-start>[^<]*</filter-start>[ \t\n\r]*");
  while (filterStartRegExp.find(text))
  {
    text.erase(filterStartRegExp.start(), filterStartRegExp.end() - filterStartRegExp.start());
  }
  itksys::RegularExpression filterEndRegExp("<filter-end>[^<]*</filter-end>[ \t\n\r]*");
  while (filterEndRegExp.find(text))
  {
    text.erase(filterEndRegExp.start(), filterEndRegExp.end() - filterEndRegExp.start());
  }
}
