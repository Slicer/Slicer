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
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLFiducialListNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLROIListNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkStringArray.h>

// ITKSYS includes
#include <itksys/Process.h>
#include <itksys/SystemTools.hxx>
#include <itksys/RegularExpression.hxx>

// QT includes
#include <QDebug>

#if defined(__APPLE__) && (MAC_OS_X_VERSION_MAX_ALLOWED >= 1030)
// needed to hack around itksys to override defaults used by Mac OS X
#endif

// STL includes
#include <algorithm>
#include <cassert>
#include <ctime>
#include <set>

#ifdef _WIN32
#else
#include <sys/types.h>
#include <unistd.h>
#endif

//----------------------------------------------------------------------------
struct DigitsToCharacters
{
  char operator() (char in)
    {
      if (in >= 48 && in <= 57)
        {
        return in + 17;
        }

      return in;
    }
};

typedef std::pair<vtkSlicerCLIModuleLogic *, vtkMRMLCommandLineModuleNode *> LogicNodePair;


//----------------------------------------------------------------------------
class vtkSlicerCLIModuleLogic::vtkInternal
{
public:
  ModuleDescription DefaultModuleDescription;
  int DeleteTemporaryFiles;

  int RedirectModuleStreams;

  std::string TemporaryDirectory;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCLIModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic::vtkSlicerCLIModuleLogic()
{
  this->Internal = new vtkInternal();

  this->Internal->DeleteTemporaryFiles = 1;
  this->Internal->RedirectModuleStreams = 1;
}

//----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic::~vtkSlicerCLIModuleLogic()
{
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
const ModuleDescription& vtkSlicerCLIModuleLogic
::GetDefaultModuleDescription()const
{
  return this->Internal->DefaultModuleDescription;
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode* vtkSlicerCLIModuleLogic::CreateNode()
{
  vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::SafeDownCast(
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLCommandLineModuleNode"));
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
void vtkSlicerCLIModuleLogic::DeleteTemporaryFilesOn ()
{
  this->SetDeleteTemporaryFiles(static_cast<int>(1));
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::DeleteTemporaryFilesOff ()
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
std::string
vtkSlicerCLIModuleLogic
::ConstructTemporarySceneFileName(vtkMRMLScene *scene)
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
  std::transform(fname.begin(), fname.end(),
                 fname.begin(), DigitsToCharacters());

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
  fname = this->Internal->TemporaryDirectory + "/" + pid + "_" + fname + ".mrml";

  return fname;
}

//----------------------------------------------------------------------------
std::string
vtkSlicerCLIModuleLogic
::ConstructTemporaryFileName(const std::string& tag,
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

  // Because Python is responsible for looking up the MRML Object,
  // we can simply return the MRML Id.
  if ( commandType == PythonModule )
    {
    return fname;
    }
  
  // To avoid confusing the Archetype readers, convert any
  // numbers in the filename to characters [0-9]->[A-J]
  std::transform(fname.begin(), fname.end(),
                 fname.begin(), DigitsToCharacters());

  // By default, the filename is based on the temporary directory and
  // the pid
  fname = this->Internal->TemporaryDirectory + "/" + pid + "_" + fname;

  if (tag == "image")
    {
    if ( commandType == CommandLineModule || type == "dynamic-contrast-enhanced")
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
      char *tname = new char[name.size() + 100];
      
      sprintf(tname, "slicer:%p#%s", this->GetMRMLScene(), name.c_str());
      
      fname = tname;

      delete [] tname;
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
    std::string ext = ".mrml";
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
    std::string ext = ".ctbl";
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
  
    
  return fname;
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::ApplyAndWait ( vtkMRMLCommandLineModuleNode* node, bool updateDisplay )
{
  // Just execute and wait.
  node->Register(this);
  node->SetAttribute("UpdateDisplay", updateDisplay ? "true" : "false");
  vtkSlicerCLIModuleLogic::ApplyTask ( node );

  while (this->GetApplicationLogic()->GetReadDataQueueSize())
    {
    this->GetApplicationLogic()->ProcessReadData();
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetTemporaryDirectory(const char *tempdir)
{
  this->Internal->TemporaryDirectory = tempdir;
}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::Apply ( vtkMRMLCommandLineModuleNode* node, bool updateDisplay )
{
  bool ret;

  if ( node->GetModuleDescription().GetType() == "PythonModule" )
    {
    this->ApplyAndWait ( node );
    return;
    }


  vtkSlicerTask* task = vtkSlicerTask::New();
  task->SetTypeToProcessing();

  // Pass the current node as client data to the task.  This allows
  // the user to switch to another parameter set after the task is
  // scheduled but before it starts to run. And when the scheduled
  // task does run, it will operate on the correct node.
  task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                        &vtkSlicerCLIModuleLogic::ApplyTask,
                        node);
  
  // Client data on the task is just a regular pointer, up the
  // reference count on the node, we'll decrease the reference count
  // once the task actually runs
  node->Register(this);
  node->SetAttribute("UpdateDisplay", updateDisplay ? "true" : "false");
  // Schedule the task
  ret = this->GetApplicationLogic()->ScheduleTask( task );

  if (!ret)
    {
    vtkWarningMacro( << "Could not schedule task" );
    }
  else
    {
    node->SetStatus(vtkMRMLCommandLineModuleNode::Scheduled);
    }
  
  task->Delete();

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
//       // Mac OS X defaults to RTLD_GLOBAL and there is no way to
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
//           vtkErrorMacro(<< "Cannot find entry point for " << moduleDescriptionObject.GetLocation() << "\nCannot run module." );
//           return;
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
void vtkSlicerCLIModuleLogic::ApplyTask(void *clientdata)
{
  // check if MRML node is present 
  if (clientdata == NULL)
    {
    vtkErrorMacro("No input CommandLineModuleNode found");
    return;
    }

  vtkSmartPointer<vtkMRMLCommandLineModuleNode> node0;
  // node was registered when the task was scheduled so take reference to
  // release it when it goes out of scope
  node0.TakeReference(reinterpret_cast<vtkMRMLCommandLineModuleNode*>(clientdata));

  // Check to see if this node/task has been cancelled
  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling ||
      node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelled)
    {
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
    this->GetApplicationLogic()->RequestModified( node0 );
    return;
    }


  // Set the callback for progress.  This will only be used for the
  // scope of this function.
  LogicNodePair lnp( this, node0 );
  node0->GetModuleDescription().GetProcessInformation()
    ->SetProgressCallback( vtkSlicerCLIModuleLogic::ProgressCallback,
                           &lnp );
  
  
  // Determine the type of the module: command line or shared object
  int (*entryPoint)(int argc, char* argv[]);
  entryPoint = NULL;
  CommandLineModuleType commandType = CommandLineModule;

  std::string target
    = node0->GetModuleDescription().GetTarget();
  std::string::size_type pos = target.find("slicer:");
  if (pos != std::string::npos && pos == 0)
    {
    sscanf(target.c_str(), "slicer:%p", &entryPoint);
    }

  // Assume that the modules correctly report themselves
  if ( node0->GetModuleDescription().GetType() == "CommandLineModule" )
    {
    // vtkSlicerApplication::GetInstance()->InformationMessage
    qDebug() << "Found CommandLine Module, target is "
             << node0->GetModuleDescription().GetTarget().c_str();
    commandType = CommandLineModule;
    if ( entryPoint != NULL )
      {
      vtkWarningMacro("Module reports that it is a Command Line Module but has a shared object module target. " << target.c_str());
      }
    }
  else if ( node0->GetModuleDescription().GetType() == "SharedObjectModule" )
    {
    // vtkSlicerApplication::GetInstance()->InformationMessage
    qDebug() << "Found SharedObject Module";

    commandType = SharedObjectModule;
    if ( entryPoint == NULL )
      {
      vtkWarningMacro("Module reports that it is a Shared Object Module but does not have a shared object module target. " << target.c_str());
      }
    }
  else if ( node0->GetModuleDescription().GetType() == "PythonModule" )
    {
    // vtkSlicerApplication::GetInstance()->InformationMessage
    qDebug() << "Found Python Module";
    commandType = PythonModule;
    }
  // vtkSlicerApplication::GetInstance()->InformationMessage
  qDebug() << "ModuleType:" << node0->GetModuleDescription().GetType().c_str(); 

  
  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap nodesToReload;
  MRMLIDToFileNameMap nodesToWrite;

  // map to keep track of the MRML Ids on the main scene to the MRML
  // Ids in the miniscene sent to the module
  typedef std::map<std::string, std::string> MRMLIDMap;
  MRMLIDMap sceneToMiniSceneMap;

  // Mini-scene used to communicate a subset of the main scene to the module
  // Additional handling is necessary because we use SmartPointers
  // (see http://slicer.spl.harvard.edu/slicerWiki/index.php/Slicer3:Memory_Management#SmartPointers)
  vtkSmartPointer<vtkMRMLScene> miniscene = vtkSmartPointer<vtkMRMLScene>::New();
  std::string minisceneFilename
    = this->ConstructTemporarySceneFileName(miniscene);
 
  // vector of files to delete
  std::set<std::string> filesToDelete;

  // iterators for parameter groups
  std::vector<ModuleParameterGroup>::iterator pgbeginit
    = node0->GetModuleDescription().GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::iterator pgendit
    = node0->GetModuleDescription().GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::iterator pgit;

  
  // Make a pass over the parameters and establish which parameters
  // have images or geometry or transforms or tables that need to be written
  // before execution or loaded upon completion.
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry"
          || (*pit).GetTag() == "transform" || (*pit).GetTag() == "table"
          || (*pit).GetTag() == "measurement")
        {
        std::string id = (*pit).GetDefault();

        // if the parameter is hidden, then deduce its value/id
        if ((*pit).GetHidden() == "true")
          {
          id = this->FindHiddenNodeID(node0->GetModuleDescription(), *pit);

          // cache the id so we don't have to look for it later
          (*pit).SetDefault( id );
          }
        
        // only keep track of objects associated with real nodes
        if (!this->GetMRMLScene()->GetNodeByID(id.c_str()) || id == "None")
          {
          continue;
          }

        std::string fname
          = this->ConstructTemporaryFileName((*pit).GetTag(),
                                             (*pit).GetType(),
                                             id,
                                             (*pit).GetFileExtensions(),
                                             commandType);

        filesToDelete.insert(fname);

        if ((*pit).GetChannel() == "input")
          {
          nodesToWrite[id] = fname;
          }
        else if ((*pit).GetChannel() == "output")
          {
          nodesToReload[id] = fname;
          }
        }
      }
    }
  
  
  // write out the input datasets
  //
  //
  std::set<std::string> MemoryTransferPossible;
  MemoryTransferPossible.insert("vtkMRMLScalarVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLVectorVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLDiffusionWeightedVolumeNode");
  MemoryTransferPossible.insert("vtkMRMLDiffusionTensorVolumeNode");

  MRMLIDToFileNameMap::const_iterator id2fn0;
    
  for (id2fn0 = nodesToWrite.begin();
       id2fn0 != nodesToWrite.end();
       ++id2fn0)
    {
    vtkMRMLNode *nd
      = this->GetMRMLScene()->GetNodeByID( (*id2fn0).first.c_str() );
    
    vtkSmartPointer<vtkMRMLStorageNode> out = 0;
    vtkSmartPointer<vtkMRMLStorageNode> defaultOut = 0;
    
    vtkMRMLStorableNode *sn = dynamic_cast<vtkMRMLStorableNode *>(nd);
    if (sn)
      {
      defaultOut.TakeReference(sn->CreateDefaultStorageNode());
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
    if ( commandType == PythonModule )
      {
      // No need to write anything out with Python
      continue;
      }
    if ((commandType == CommandLineModule) && defaultOut)
      {
      // Default case for CommandLineModule is to use a storage node
      out = defaultOut;
      }
    if ((commandType == SharedObjectModule) && defaultOut)
      {
      //std::cerr << nd->GetName() << " is " << nd->GetClassName() << std::endl;

      // Check if we can transfer the datatype using a direct memory transfer
      if (std::find(MemoryTransferPossible.begin(), MemoryTransferPossible.end(), 
                    nd->GetClassName()) == MemoryTransferPossible.end())
        {
        // Cannot use a memory transfer, use a StorageNode
        out = defaultOut;
        }
      }

    vtkMRMLTransformNode *tnd = vtkMRMLTransformNode::SafeDownCast(nd);
    if (tnd)
      {
      // Transform nodes will use either a storage node OR a miniscene

      std::string::size_type loc = (*id2fn0).second.find_last_of(".");
      if (loc != std::string::npos)
        {
        // if we start passing pointers to MRML transforms, then we'll
        // need an additional check/case
        std::string extension = (*id2fn0).second.substr(loc);

        if (extension == ".mrml")
          {
          // not using a storage node.  using a mini-scene to transfer
          // the node
          out = 0;  // don't use the storage node

          vtkMRMLNode *cp = miniscene->CopyNode(nd);

          // Keep track what scene node corresponds to what miniscene node
          sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
          }
        }
      }

    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::SafeDownCast(nd);
    if (mhnd)
      {
      // model hierarchy nodes need to get put in a scene
      vtkMRMLNode *cp = miniscene->CopyNode(nd);

      // keep track of scene node corresponds to what the miniscene node
      sceneToMiniSceneMap[nd->GetID()] = cp->GetID();

      // also add any display node
      vtkMRMLDisplayNode *dnd = mhnd->GetDisplayNode();
      if (dnd)
        {
        vtkMRMLNode *dcp = miniscene->CopyNode(dnd);

        vtkMRMLModelHierarchyNode *mhcp
          = vtkMRMLModelHierarchyNode::SafeDownCast(cp);
        vtkMRMLDisplayNode *d = vtkMRMLDisplayNode::SafeDownCast(dcp);
        
        mhcp->SetAndObserveDisplayNodeID( d->GetID() );
        }
      }

    // if the file is to be written, then write it
    if (out)
      {
      out->SetScene(this->GetMRMLScene());
      out->SetFileName( (*id2fn0).second.c_str() );
      if (!out->WriteData( nd ))
        {
        vtkErrorMacro("ERROR writing file " << out->GetFileName());
        }
      out = 0;
      }
    }

  // Also need to run through any output nodes that will be
  // communicated through the miniscene and add them to the miniscene
  // 
  for (id2fn0 = nodesToReload.begin();
       id2fn0 != nodesToReload.end();
       ++id2fn0)
    {
    vtkMRMLNode *nd
      = this->GetMRMLScene()->GetNodeByID( (*id2fn0).first.c_str() );
    
    vtkMRMLTransformNode *tnd
      = vtkMRMLTransformNode::SafeDownCast(nd);
    vtkMRMLModelHierarchyNode *mhnd
      = vtkMRMLModelHierarchyNode::SafeDownCast(nd);
  
    if (tnd || mhnd)
      {
      std::string::size_type loc = (*id2fn0).second.find_last_of(".");
      if (loc != std::string::npos)
        {
        // if we start passing pointers to MRML transforms, then we'll
        // need an additional check/case
        std::string extension = (*id2fn0).second.substr(loc);

        if (extension == ".mrml")
          {
          // put this transform node in the miniscene
          vtkMRMLNode *cp = miniscene->CopyNode(nd);

          // Keep track what scene node corresponds to what miniscene node
          sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
          }
        }
      }
    else if (mhnd)
      {
      // always put model hierarchy nodes in the miniscene
      vtkMRMLNode *cp = miniscene->CopyNode(nd);
      
      // Keep track what scene node corresponds to what miniscene node
      sceneToMiniSceneMap[nd->GetID()] = cp->GetID();

      // try casting to a DisplayableNode, if successful, add the
      // display node if there is one
      vtkMRMLDisplayableNode *dable = vtkMRMLDisplayableNode::SafeDownCast(nd);
      if (dable)
        {
        vtkMRMLDisplayNode *dnd = dable->GetDisplayNode();
        if (dnd)
          {
          vtkMRMLNode *dcp = miniscene->CopyNode(dnd);

          vtkMRMLDisplayableNode *dablecp
            = vtkMRMLDisplayableNode::SafeDownCast(cp);
          vtkMRMLDisplayNode *d = vtkMRMLDisplayNode::SafeDownCast(dcp);

          dablecp->SetAndObserveDisplayNodeID( d->GetID() );
          }
        }
      }
    }
  
  // write out the miniscene if needed
  if (miniscene->GetNumberOfNodes() > 0)
    {
    miniscene->Commit( minisceneFilename.c_str() );
    }

  // build the command line
  //
  //
  std::vector<std::string> commandLineAsString;

  // Command to execute
  if (node0->GetModuleDescription().GetLocation() != std::string("") && 
      commandType == CommandLineModule &&
      node0->GetModuleDescription().GetLocation() != node0->GetModuleDescription().GetTarget())
    {
      vtkDebugMacro("Setting a location for a command line module: " << node0->GetModuleDescription().GetLocation().c_str() << ", target is '" << node0->GetModuleDescription().GetTarget().c_str() << "'");
      // it's a command line module that has an executable that is used to run the target
    commandLineAsString.push_back(node0->GetModuleDescription().GetLocation());
    }
  commandLineAsString.push_back( node0->GetModuleDescription().GetTarget() );


  // Add a command line flag for the process information structure
  if ( commandType == SharedObjectModule )
    {
    commandLineAsString.push_back( "--processinformationaddress" );

    char tname[256];
    sprintf(tname, "%p", node0->GetModuleDescription().GetProcessInformation());
    
    commandLineAsString.push_back( tname );
    }

  // Add a command line flag for a file of return types
  if (node0->GetModuleDescription().HasReturnParameters())
    {
    commandLineAsString.push_back( "--returnparameterfile" );
    
    std::ostringstream pidString;
#ifdef _WIN32
    pidString << GetCurrentProcessId();
#else
    pidString << getpid();
#endif

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    srand(time(0));

    std::ostringstream code;
    for (int ii = 0; ii < 10; ii++)
      {
      code << alphanum[rand() % (sizeof(alphanum)-1)];
      }

    std::string returnFile = this->Internal->TemporaryDirectory + "/" + pidString.str()
      + "_" + code.str() + ".params";

    commandLineAsString.push_back( returnFile );

    // We will need to load this results file back when module completes
    nodesToReload[node0->GetID()] = returnFile;

    // This is an extra file we will need to delete
    filesToDelete.insert( returnFile );
    }
  
  // Run over all the parameters with flags
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
        if ((*pit).GetTag() != "boolean"
            && (*pit).GetTag() != "file" && (*pit).GetTag() != "directory"
            && (*pit).GetTag() != "string"
            && (*pit).GetTag() != "integer-vector"
            && (*pit).GetTag() != "float-vector"
            && (*pit).GetTag() != "double-vector"
            && (*pit).GetTag() != "string-vector"
            && (*pit).GetTag() != "image"
            && (*pit).GetTag() != "point"
            && (*pit).GetTag() != "region"
            && (*pit).GetTag() != "transform"
            && (*pit).GetTag() != "geometry"
            && (*pit).GetTag() != "table"
            && (*pit).GetTag() != "measurement")
          {
          // simple parameter, write flag and value
          commandLineAsString.push_back(prefix + flag);
          commandLineAsString.push_back((*pit).GetDefault());
          continue;
          }
        if ((*pit).GetTag() == "boolean")
          {
          // booleans only have a flag (no value) in non-Python modules
          if ( commandType != PythonModule )
            {
            if ((*pit).GetDefault() == "true")
              {
              commandLineAsString.push_back(prefix + flag);
              }
            }
          else
            {
            // For Python, if the flag is true or false, specify that
            commandLineAsString.push_back ( prefix + flag );
            commandLineAsString.push_back ( (*pit).GetDefault() );
            }
          continue;
          }
        if ((*pit).GetTag() == "file" || (*pit).GetTag() == "directory"
            || (*pit).GetTag() == "string"
            || (*pit).GetTag() == "integer-vector"
            || (*pit).GetTag() == "float-vector"
            || (*pit).GetTag() == "double-vector"
            || (*pit).GetTag() == "string-vector")
          {
          // Only write out the flag if value is not empty
          if ((*pit).GetDefault() != "")
            {
            commandLineAsString.push_back(prefix + flag);
            commandLineAsString.push_back((*pit).GetDefault());
            }
          continue;
          }
        if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry"
            || (*pit).GetTag() == "transform" || (*pit).GetTag() == "table" 
            || (*pit).GetTag() == "measurement")
          {
          std::string fname;

          // default to using filename in the read/write maps
          // established earlier
          MRMLIDToFileNameMap::const_iterator id2fn;
          
          id2fn  = nodesToWrite.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "input" && id2fn != nodesToWrite.end())
            {
            fname = (*id2fn).second;
            }

          id2fn  = nodesToReload.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "output" && id2fn != nodesToReload.end())
            {
            fname = (*id2fn).second;
            }

          // check to see if we need to remap to a scene file and node id
          MRMLIDMap::iterator mit
            = sceneToMiniSceneMap.find((*pit).GetDefault());
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
            commandLineAsString.push_back( fname );
            }

          continue;
          }
        if ((*pit).GetTag() == "point")
          {
          // get the fiducial list node
          vtkMRMLNode *node
            = this->GetMRMLScene()->GetNodeByID((*pit).GetDefault().c_str());
          vtkMRMLFiducialListNode *fiducials
            = vtkMRMLFiducialListNode::SafeDownCast(node);
          vtkMRMLDisplayableHierarchyNode *points = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
          if (fiducials)
            {
            // check to see if module can handle more than one point
            long numberOfSelectedFiducials=0;
            for (int i=0; i < fiducials->GetNumberOfFiducials(); ++i)
              {
              if (fiducials->GetNthFiducialSelected(i))
                {
                numberOfSelectedFiducials++;
                }
              }
            
            if (numberOfSelectedFiducials == 1
                || (*pit).GetMultiple() == "true")
              {
              for (int i=0; i < fiducials->GetNumberOfFiducials(); ++i)
                {
                float *pt;
                std::ostringstream ptAsString;

                if (fiducials->GetNthFiducialSelected(i))
                  {
                  pt = fiducials->GetNthFiducialXYZ(i);
                  ptAsString << pt[0] << "," << pt[1] << "," << pt[2];
                  
                  commandLineAsString.push_back(prefix + flag);
                  commandLineAsString.push_back(ptAsString.str());
                  }
                }
              }
            else
              {
              // Can't support this command line with this fiducial
              // list
              vtkErrorMacro("Module does not support multiple fiducials.");
              }
            }
          else if (points)
            {
            // find the children of this hierarchy node
            vtkSmartPointer<vtkCollection> col = vtkSmartPointer<vtkCollection>::New();
            points->GetChildrenDisplayableNodes(col);
            vtkDebugMacro("Getting children displayable nodes from points " << points->GetID());
            unsigned int numChildren = 0;
            if (col)
              {
              numChildren = col->GetNumberOfItems();
              }
            vtkDebugMacro("Displayable hierarchy has " << numChildren << " child nodes");
            for (unsigned int c = 0; c < numChildren; c++)
              {
              // the hierarchy nodes have a sorting index that's respected by
              // GetNthChildNode
              vtkMRMLHierarchyNode *nthHierarchyNode = points->GetNthChildNode(c);              
              // then get the displayable node from that hierarchy node
              if (nthHierarchyNode)
                {
                vtkMRMLDisplayableHierarchyNode *nthDisplayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(nthHierarchyNode);
                vtkMRMLDisplayableNode *displayableNode = NULL;
                if (nthDisplayableHierarchyNode)
                  {
                  displayableNode = nthDisplayableHierarchyNode->GetDisplayableNode();
                  }
                if (displayableNode)
                  {
                  vtkDebugMacro("Found displayable node with id " << displayableNode->GetID());
                  std::ostringstream ss;
                  displayableNode->WriteCLI(ss, prefix+flag);
                  vtkDebugMacro("WriteCL output = " << ss.str());
                  commandLineAsString.push_back(ss.str());
                  }
                }
              }
            }
          continue;
          }
        if ((*pit).GetTag() == "region")
          {
          // get the region node
          vtkMRMLNode *node
            = this->GetMRMLScene()->GetNodeByID((*pit).GetDefault().c_str());
          vtkMRMLROIListNode *regions = vtkMRMLROIListNode::SafeDownCast(node);

          if (regions)
            {
            // check to see if module can handle more than one region
            long numberOfSelectedRegions=0;
            for (int i=0; i < regions->GetNumberOfROIs(); ++i)
              {
              if (regions->GetNthROISelected(i))
                {
                numberOfSelectedRegions++;
                }
              }
            
            if (numberOfSelectedRegions == 1
                || (*pit).GetMultiple() == "true")
              {
              for (int i=0; i < regions->GetNumberOfROIs(); ++i)
                {
                double *pt;
                double *Radius;
                std::ostringstream roiAsString;

                if (regions->GetNthROISelected(i))
                  {
                  pt = regions->GetNthROIXYZ(i);
                  Radius = regions->GetNthROIRadiusXYZ(i);
                  roiAsString << pt[0] << "," << pt[1] << "," << pt[2] << ","
                             << Radius[0] << "," << Radius[1] << "," << Radius[2];

                  commandLineAsString.push_back(prefix + flag);
                  commandLineAsString.push_back(roiAsString.str());
                  }
                }
              }
            else
              {
              // Can't support this command line with this region
              // list
              vtkErrorMacro("Module does not support multiple regions. Region list contains " << numberOfSelectedRegions << " selected regions.");
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
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
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
    if ((*iit).second.GetTag() != "image"
        && (*iit).second.GetTag() != "geometry"
        && (*iit).second.GetTag() != "transform"
        && (*iit).second.GetTag() != "table"
        && (*iit).second.GetTag() != "measurement"
        && (*iit).second.GetTag() != "file"
        && (*iit).second.GetTag() != "directory"
        && (*iit).second.GetTag() != "string"
        && (*iit).second.GetTag() != "point"
        && (*iit).second.GetTag() != "region"
        && (*iit).second.GetTag() != "integer-vector"
        && (*iit).second.GetTag() != "float-vector"
        && (*iit).second.GetTag() != "double-vector"
        && (*iit).second.GetTag() != "string-vector")
      {
      commandLineAsString.push_back((*iit).second.GetDefault());
      }
    else if ((*iit).second.GetTag() == "file"
             || (*iit).second.GetTag() == "directory"
             || (*iit).second.GetTag() == "string"
             || (*iit).second.GetTag() == "integer-vector"
             || (*iit).second.GetTag() == "float-vector"
             || (*iit).second.GetTag() == "double-vector"
             || (*iit).second.GetTag() == "string-vector")
      {
      if ((*iit).second.GetDefault() != "")
        {
        commandLineAsString.push_back((*iit).second.GetDefault());
        }
      else
        {
        vtkErrorMacro("No value assigned to \""
                      << (*iit).second.GetLabel().c_str() << "\"");

        node0->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
        this->GetApplicationLogic()->RequestModified( node0 );
        return;
        }
      }
    else if ((*iit).second.GetTag() == "point"
             || (*iit).second.GetTag() == "region")
      {
      vtkErrorMacro("Fiducials and ROIs are not currently supported as index arguments to modules.");
      node0->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
      this->GetApplicationLogic()->RequestModified( node0 );
      return;
      }
    else
      {
      // image or geometry or transform or table or measurement index parameter

      std::string fname;
      
      MRMLIDToFileNameMap::const_iterator id2fn;

      if ((*iit).second.GetChannel() == "input")
        {
        // Check to make sure the index parameter is set
        id2fn  = nodesToWrite.find( (*iit).second.GetDefault() );
        if (id2fn != nodesToWrite.end())
          {
          fname = (*id2fn).second;
          }
        }
      else if ((*iit).second.GetChannel() == "output")
        {
        // Check to make sure the index parameter is set
        id2fn  = nodesToReload.find( (*iit).second.GetDefault() );
        if (id2fn != nodesToReload.end())
          {
          fname = (*id2fn).second;
          }
        }

      // check to see if we need to remap to a scene file and node id
      MRMLIDMap::iterator mit
        = sceneToMiniSceneMap.find((*iit).second.GetDefault());
      if (mit != sceneToMiniSceneMap.end())
        {
        // node is being sent inside of a scene, so use the scene
        // filename and the remapped id
        fname = minisceneFilename + "#" + (*mit).second;
        }      

      if (fname.size() > 0)
        {
        commandLineAsString.push_back( fname );
        }
      else
        {
        vtkErrorMacro("No " << (*iit).second.GetChannel().c_str()
                      << " data assigned to \""
                      << (*iit).second.GetLabel().c_str() << "\"");

        node0->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
        this->GetApplicationLogic()->RequestModified( node0 );
        return;
        }
      }
    }


  // copy the command line arguments into an array of pointers to
  // chars
  char **command = new char*[commandLineAsString.size()+1];
  for (std::vector<std::string>::size_type i=0; i < commandLineAsString.size(); ++i)
    {
    command[i] = const_cast<char*>(commandLineAsString[i].c_str());
    }
  command[commandLineAsString.size()] = 0;

  // print the command line
  //
  std::stringstream information0;
  information0 << node0->GetModuleDescription().GetTitle()
              << " command line: " << std::endl << std::endl;
  for (std::vector<std::string>::size_type i=0; i < commandLineAsString.size(); ++i)
    {
    information0 << command[i] << " ";
    }
  information0 << std::endl;
  // vtkSlicerApplication::GetInstance()->InformationMessage
  qDebug() << information0.str().c_str();
  

  // run the filter
  //
  //
  node0->GetModuleDescription().GetProcessInformation()->Initialize();
  node0->SetStatus(vtkMRMLCommandLineModuleNode::Running, false);
  this->GetApplicationLogic()->RequestModified( node0 );
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
     int putSuccess =
       itksys::SystemTools::PutEnv(const_cast <char *> (emptyString.c_str()));
     if (!putSuccess)
       {
       vtkErrorMacro( "Unable to reset ITK_AUTOLOAD_PATH.");
       }
    //
    // now run the process
    //
    itksysProcess *process = itksysProcess_New();
    
    // setup the command
    itksysProcess_SetCommand(process, command);
    itksysProcess_SetOption(process,
                            itksysProcess_Option_Detach, 0);
    itksysProcess_SetOption(process,
                            itksysProcess_Option_HideWindow, 1);
    // itksysProcess_SetTimeout(process, 5.0); // 5 seconds
    
    // execute the command
    itksysProcess_Execute(process);

    // restore the load path
    std::string putEnvString = ("ITK_AUTOLOAD_PATH=");
    putEnvString = putEnvString + saveITKAutoLoadPath;
    putSuccess =
      itksys::SystemTools::PutEnv(const_cast <char *> (putEnvString.c_str()));
    if (!putSuccess)
      {
      vtkErrorMacro( "Unable to restore ITK_AUTOLOAD_PATH. ");
      }

    // Wait for the command to finish
    char *tbuffer;
    int length;
    int pipe;
    const double timeoutlimit = 0.1;    // tenth of a second
    double timeout = timeoutlimit;
    std::string stdoutbuffer;
    std::string stderrbuffer;
    std::string::size_type tagend;
    std::string::size_type tagstart;
    while ((pipe = itksysProcess_WaitForData(process ,&tbuffer,
                                             &length, &timeout)) != 0)
      {
      // increment the elapsed time
      node0->GetModuleDescription().GetProcessInformation()->ElapsedTime
        += (timeoutlimit - timeout);
      this->GetApplicationLogic()->RequestModified( node0 );
      
      // reset the timeout value 
      timeout = timeoutlimit;

      // Check to see if the plugin was cancelled
      if (node0->GetModuleDescription().GetProcessInformation()->Abort)
        {
        itksysProcess_Kill(process);
        node0->GetModuleDescription().GetProcessInformation()->Progress = 0;
        node0->GetModuleDescription().GetProcessInformation()->StageProgress =0;
        this->GetApplicationLogic()->RequestModified( node0 ); 
        break;
        }

      // Capture the output from the filter
      if (length != 0 && tbuffer != 0)
        {
        if (pipe == itksysProcess_Pipe_STDOUT)
          {
          //std::cout << "STDOUT: " << std::string(tbuffer, length) << std::endl;
          stdoutbuffer = stdoutbuffer.append(tbuffer, length);

          bool foundTag = false;
          // search for the last occurence of </filter-progress>
          tagend = stdoutbuffer.rfind("</filter-progress>");
          if (tagend != std::string::npos)
            {
            tagstart = stdoutbuffer.rfind("<filter-progress>");
            if (tagstart != std::string::npos)
              {
              std::string progressString(stdoutbuffer, tagstart+17,
                                         tagend-tagstart-17);
              node0->GetModuleDescription().GetProcessInformation()->Progress = atof(progressString.c_str());
              foundTag = true;
              }
            }
          // search for the last occurence of </filter-stage-progress>
          tagend = stdoutbuffer.rfind("</filter-stage-progress>");
          if (tagend != std::string::npos)
            {
            tagstart = stdoutbuffer.rfind("<filter-stage-progress>");
            if (tagstart != std::string::npos)
              {
              std::string progressString(stdoutbuffer, tagstart+23,
                                         tagend-tagstart-23);
              node0->GetModuleDescription().GetProcessInformation()->StageProgress = atof(progressString.c_str());
              foundTag = true;
              }
            }

          // search for the last occurence of </filter-name>
          tagend = stdoutbuffer.rfind("</filter-name>");
          if (tagend != std::string::npos)
            {
            tagstart = stdoutbuffer.rfind("<filter-name>");
            if (tagstart != std::string::npos)
              {
              std::string filterString(stdoutbuffer, tagstart+13,
                                       tagend-tagstart-13);
              strncpy(node0->GetModuleDescription().GetProcessInformation()->ProgressMessage, filterString.c_str(), 1023);
              foundTag = true;
              }
            }
          
          // search for the last occurence of </filter-comment>
          tagend = stdoutbuffer.rfind("</filter-comment>");
          if (tagend != std::string::npos)
            {
            tagstart = stdoutbuffer.rfind("<filter-comment>");
            if (tagstart != std::string::npos)
              {
              std::string progressMessage(stdoutbuffer, tagstart+16,
                                         tagend-tagstart-16);
              strncpy (node0->GetModuleDescription().GetProcessInformation()->ProgressMessage, progressMessage.c_str(), 1023);
              foundTag = true;
              }
            }
          if (foundTag)
            {
            this->GetApplicationLogic()->RequestModified( node0 );
            }
          }
        else if (pipe == itksysProcess_Pipe_STDERR)
          {
          stderrbuffer = stderrbuffer.append(tbuffer, length);
          }
        }
      }
    itksysProcess_WaitForExit(process, 0);


    // remove the embedded XML from the stdout stream
    //
    // Note that itksys::RegularExpression gives begin()/end() as
    // size_types not iterators. So we need to use the version of
    // erase that takes a position and length to erase.
    //
    itksys::RegularExpression filterProgressRegExp("<filter-progress>[^<]*</filter-progress>[ \t\n\r]*");
    while (filterProgressRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterProgressRegExp.start(),
                         filterProgressRegExp.end()
                         - filterProgressRegExp.start());
      }
    itksys::RegularExpression filterStageProgressRegExp("<filter-stage-progress>[^<]*</filter-stage-progress>[ \t\n\r]*");
    while (filterStageProgressRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterStageProgressRegExp.start(),
                         filterStageProgressRegExp.end()
                         - filterStageProgressRegExp.start());
      }
    itksys::RegularExpression filterNameRegExp("<filter-name>[^<]*</filter-name>[ \t\n\r]*");
    while (filterNameRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterNameRegExp.start(),
                         filterNameRegExp.end()
                         - filterNameRegExp.start());
      }
    itksys::RegularExpression filterCommentRegExp("<filter-comment>[^<]*</filter-comment>[ \t\n\r]*");
    while (filterCommentRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterCommentRegExp.start(),
                         filterCommentRegExp.end()
                         - filterCommentRegExp.start());
      }
    itksys::RegularExpression filterTimeRegExp("<filter-time>[^<]*</filter-time>[ \t\n\r]*");
    while (filterTimeRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterTimeRegExp.start(),
                         filterTimeRegExp.end()
                         - filterTimeRegExp.start());
      }
    itksys::RegularExpression filterStartRegExp("<filter-start>[^<]*</filter-start>[ \t\n\r]*");
    while (filterStartRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterStartRegExp.start(),
                         filterStartRegExp.end()
                         - filterStartRegExp.start());
      }
    itksys::RegularExpression filterEndRegExp("<filter-end>[^<]*</filter-end>[ \t\n\r]*");
    while (filterEndRegExp.find(stdoutbuffer))
      {
      stdoutbuffer.erase(filterEndRegExp.start(),
                         filterEndRegExp.end()
                         - filterEndRegExp.start());
      }
    
    
    if (stdoutbuffer.size() > 0)
      {
      std::string tmp(" standard output:\n\n");
      stdoutbuffer.insert(0, node0->GetModuleDescription().GetTitle()+tmp);
      // vtkSlicerApplication::GetInstance()->InformationMessage
      qDebug() << stdoutbuffer.c_str();
      }
    if (stderrbuffer.size() > 0)
      {
      std::string tmp(" standard error:\n\n");
      stderrbuffer.insert(0, node0->GetModuleDescription().GetTitle()+tmp);
      vtkErrorMacro( << stderrbuffer.c_str() );
      }
    
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
          std::stringstream information;
          information << node0->GetModuleDescription().GetTitle()
                      << " completed without errors" << std::endl;
          // vtkSlicerApplication::GetInstance()->InformationMessage
          qDebug() << information.str().c_str();
          
          }
        else
          {
          std::stringstream information;
          information << node0->GetModuleDescription().GetTitle()
                      << " completed with errors" << std::endl;
          vtkErrorMacro( << information.str().c_str() );
          node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
          this->GetApplicationLogic()->RequestModified( node0 );
          }
        }
      else if (result == itksysProcess_State_Expired)
        {
        std::stringstream information;
        information << node0->GetModuleDescription().GetTitle()
                    << " timed out" << std::endl;
        vtkErrorMacro( << information.str().c_str() );
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified( node0 );
        }
      else
        {
        std::stringstream information;
        if (result == itksysProcess_State_Exception)
          {
          information << node0->GetModuleDescription().GetTitle();
          int excResult = itksysProcess_GetExitException(process);
          switch (excResult)
            {
            case itksysProcess_Exception_None:
              information << " terminated with no exceptions." << std::endl;
              break;
            case itksysProcess_Exception_Fault:
              information << " terminated with a fault." << std::endl;
              break;
            case itksysProcess_Exception_Illegal:
              information << " terminated with an illegal instruction." << std::endl;
              break;
            case itksysProcess_Exception_Interrupt:
              information << " terminated with an interrupt." << std::endl;
              break;
            case itksysProcess_Exception_Numerical:
              information << " terminated with a numerical fault." << std::endl;
              break;
            case itksysProcess_Exception_Other:
              information << " terminated with an unknown exception." << std::endl;
              break;
            }
          }
        else if (result == itksysProcess_State_Error)
          {
          information << node0->GetModuleDescription().GetTitle()
                      << " process was null when state was queried, result (" << result << ") = Error." << std::endl;
          } 
        else
          {
        information << node0->GetModuleDescription().GetTitle()
                  << " unknown termination. Result = " << result << std::endl;
          }
        vtkErrorMacro( << information.str().c_str() );
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified( node0 );
        }

      // clean up
      itksysProcess_Delete(process);
      }
    }
  else if ( commandType == SharedObjectModule )
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
        std::cout.rdbuf( coutstringstream.rdbuf() );
        std::cerr.rdbuf( cerrstringstream.rdbuf() );
        }

      // run the module
      if ( entryPoint != NULL ) {
        returnValue = (*entryPoint)(commandLineAsString.size(), command);
      }

      // report the output
      if (coutstringstream.str().size() > 0)
        {
        std::string tmp(" standard output:\n\n");
        tmp = node0->GetModuleDescription().GetTitle()+tmp;

        // vtkSlicerApplication::GetInstance()->InformationMessage
        qDebug() << (tmp + coutstringstream.str()).c_str();
        }
      if (cerrstringstream.str().size() > 0)
        {
        std::string tmp(" standard error:\n\n");
        tmp = node0->GetModuleDescription().GetTitle()+tmp;

        vtkErrorMacro( << (tmp + cerrstringstream.str()).c_str() );
        }

      if (this->Internal->RedirectModuleStreams)
        {
        // reset the streams
        std::cout.rdbuf( origcoutrdbuf );
        std::cerr.rdbuf( origcerrrdbuf );
        }
      }
    catch (itk::ExceptionObject& exc)
      {
      std::stringstream information;
      if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
        {
        information << node0->GetModuleDescription().GetTitle()
                    << " cancelled.";
        // vtkSlicerApplication::GetInstance()->InformationMessage
        qDebug() << information.str().c_str();
        }
      else
        {
        information << node0->GetModuleDescription().GetTitle()
                    << " terminated with an exception: " << exc;
        vtkErrorMacro( << information.str().c_str() );
        node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified( node0 );
        }

      std::cout.rdbuf( origcoutrdbuf );
      std::cerr.rdbuf( origcerrrdbuf );
      }
    catch (...)
      {
      std::stringstream information;
      information << node0->GetModuleDescription().GetTitle()
                << " terminated with an unknown exception." << std::endl;
      vtkErrorMacro( << information.str().c_str() );
      node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
      this->GetApplicationLogic()->RequestModified( node0 );

      std::cout.rdbuf( origcoutrdbuf );
      std::cerr.rdbuf( origcerrrdbuf );
      }
    if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
      {
      node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
      this->GetApplicationLogic()->RequestModified( node0 );
      }
    // Check the return status of the module
    if (returnValue)
      {
      std::stringstream information;
      information << node0->GetModuleDescription().GetTitle()
                  << " returned " << returnValue << " which probably indicates an error." << std::endl;
      vtkErrorMacro( << information.str().c_str() );
      node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
      this->GetApplicationLogic()->RequestModified( node0 );
      std::cout.rdbuf( origcoutrdbuf );
      std::cerr.rdbuf( origcerrrdbuf );
      }
    }
  else if ( commandType == PythonModule )
    {
    // For the moment, ignore the output and just run the module
    // vtkSlicerApplication::GetInstance()->InformationMessage
    qDebug() << "Preparing to execute Python Module";

    // Now, call Python properly.  For the moment, make a big string...
    // ...later we'll want to do this through the Python API
    std::string ExecuteModuleString =
      "import sys\n"
      "import Slicer\n"
      "import inspect\n"
      "ModuleName = \"" + node0->GetModuleDescription().GetTarget() + "\"\n"
      "ModuleArgs = []\n"
      "ArgTags = []\n"
      "ArgFlags = []\n"
      "ArgMultiples = []\n";

    //cout<<"-----------------------Individial command line items---------------------"<<endl;
    // Now add the individual command line items
    for (std::vector<std::string>::size_type i=1; i < commandLineAsString.size(); ++i)
      {
      ExecuteModuleString += "ModuleArgs.append ( '" + commandLineAsString[i] + "' );\n";
      }
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
        ExecuteModuleString += "ArgTags.append ( '" + (*pit).GetTag() + "' )\n";
        ExecuteModuleString += "ArgFlags.append ( '" + (*pit).GetLongFlag() + "' )\n";
        ExecuteModuleString += "ArgMultiples.append ( '" + (*pit).GetMultiple() + "' )\n";
        }
      }
    // TODO: FlagArgs, PositionalArgs, Arguments are in global scope - potential name clash
    ExecuteModuleString +=
      "FlagArgs, PositionalArgs = Slicer.ParseArgs ( ModuleArgs, ArgTags , ArgFlags, ArgMultiples )\n"
      "Module = __import__ ( ModuleName )\n"
      "reload ( Module )\n"
      "Arguments = inspect.getargspec(Module.Execute)[0]\n"
      "if 'commandLineModuleNode' in Arguments:\n"
      "  FlagArgs['commandLineModuleNode'] = '";
    ExecuteModuleString += std::string(node0->GetID()) + "'\n";
    ExecuteModuleString += "Module.Execute ( *PositionalArgs, **FlagArgs )\n";

// #ifdef Slicer_USE_PYTHON    
//     PyObject* v;
//     v = PyRun_String(
//       ExecuteModuleString.c_str(),
//       Py_file_input,
//       (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
//       (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
// 
//     if (v == NULL || PyErr_Occurred())
//       {
//       node0->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
//       PyErr_Print();
//       }
//     else
//       {
//       node0->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
//       if (Py_FlushLine())
//         {
//         PyErr_Clear();
//         }
//       }
// #else
    vtkErrorMacro("Attempting to execute a Python Module without Python support enabled");
// #endif

    this->GetApplicationLogic()->RequestModified( node0 );
    }
  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelling)
    {
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled, false);
    this->GetApplicationLogic()->RequestModified( node0 );
    }
  else if (node0->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelled
           && node0->GetStatus() != vtkMRMLCommandLineModuleNode::CompletedWithErrors)
    {
    node0->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
    this->GetApplicationLogic()->RequestModified( node0 );
    }
  // reset the progress to zero
  node0->GetModuleDescription().GetProcessInformation()->Progress = 0;
  node0->GetModuleDescription().GetProcessInformation()->StageProgress = 0;
  this->GetApplicationLogic()->RequestModified( node0 );
  
  // import the results if the plugin was allowed to complete
  //
  //
  if (node0->GetStatus() == vtkMRMLCommandLineModuleNode::Completed)
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
        bool deleteFile = this->GetDeleteTemporaryFiles();
        this->GetApplicationLogic()
          ->RequestReadData((*id2fn0).first.c_str(), (*id2fn0).second.c_str(),
                            displayData, deleteFile);
        
        // If we are reloading a file, then we know that it is a file
        // that needs to be removed.  It wouldn't make sense for two
        // outputs of a module to produce the same file to be reloaded.
        filesToDelete.erase( (*id2fn0).second );
        }
      }


    // rewire the mrml scene as directed 
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
        if ((*pit).GetTag() == "transform"
            && (*pit).GetChannel() == "output"
            && (*pit).GetReference().size() > 0)
          {
          std::string reference;
          if (node0->GetModuleDescription().HasParameter((*pit).GetReference()))
            {
            reference
              = node0->GetModuleDescription()
                           .GetParameterDefaultValue((*pit).GetReference());
            if (reference.size() > 0)
              {
              vtkMRMLTransformableNode *t
                = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()
                           ->GetNodeByID(reference.c_str()));
              if (t)
                {
                if ( (*pit).GetDefault() != "" )
                  {
                  vtkSmartPointer<vtkStringArray> reqSTNID = vtkSmartPointer<vtkStringArray>::New();
                  vtkStdString areq;
                  areq = "[$::slicer3::MRMLScene GetNodeByID " + reference + "] "
                    + "SetAndObserveTransformNodeID "
                    + (*pit).GetDefault() + " ; "
                    + "$::slicer3::MRMLScene Edited";
                  reqSTNID->InsertNextValue( areq );
                  this->GetApplicationLogic()->RequestModified( reqSTNID );
                  }
                }
              else
                {
                vtkWarningMacro( << "Cannot find referenced node " << (*pit).GetDefault());
                }
              }
            }
          else
            {
            vtkWarningMacro( << "Referenced parameter unknown: " << (*pit).GetReference() );
            }
        
          }
        }
      }
    }

  // if there was a miniscene that needs loading, request it
  if (miniscene->GetNumberOfNodes() > 0)
    {
    bool displayData = this->IsCommandLineModuleNodeUpdatingDisplay(node0);
    bool deleteFile = this->GetDeleteTemporaryFiles();

    // Convert the index map to two vectors so that we can pass it to
    // a function in a different library (Win32 limitation)
    std::vector<std::string> keys, values;
    
    MRMLIDMap::iterator mit;
    for (mit = sceneToMiniSceneMap.begin(); mit != sceneToMiniSceneMap.end();
         ++mit)
      {
      // only load the nodes that are needed back into the main scene
      MRMLIDToFileNameMap::iterator rit = nodesToReload.find( (*mit).first );

      if (rit != nodesToReload.end())
        {
        keys.push_back( (*mit).first );
        values.push_back( (*mit).second );
        }
      }

    // Place a request to read the miniscene and map any ids as necessary
    this->GetApplicationLogic()
      ->RequestReadScene( minisceneFilename, keys, values,
                          displayData, deleteFile );
    }


  // clean up
  //
  //
  delete [] command;

  // Remove any remaining temporary files.  At this point, these files
  // should be the files written as inputs to the module
  if ( this->GetDeleteTemporaryFiles() )
    {
    bool removed;
    std::set<std::string>::iterator fit;
    for (fit = filesToDelete.begin(); fit != filesToDelete.end(); ++fit)
      {
      if (itksys::SystemTools::FileExists((*fit).c_str()))
        {
        removed = itksys::SystemTools::RemoveFile((*fit).c_str());
        if (!removed)
          {
          std::stringstream information;
          information << "Unable to delete temporary file " << *fit << std::endl;
          vtkWarningMacro( << information.str().c_str() );
          }
        }
      }
    }

}

//-----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::ProgressCallback ( void *who )
{
  LogicNodePair *lnp = reinterpret_cast<LogicNodePair*>(who);

  // All we need to do is tell the node that it was Modified.  The
  // shared object plugin modifies fields in the ProcessInformation directly.
  lnp->first->GetApplicationLogic()->RequestModified(lnp->second);
}

//-----------------------------------------------------------------------------
std::string
vtkSlicerCLIModuleLogic::FindHiddenNodeID(const ModuleDescription& d,
                                            const ModuleParameter& p)
{
  std::string id = "None";
  
  if (p.GetHidden() == "true")
    {
    if (p.GetReference().size() > 0)
      {
      std::string reference;
      if (d.HasParameter(p.GetReference()))
        {
        reference = d.GetParameterDefaultValue(p.GetReference());

        if (p.GetTag() == "table")
          {
          if (p.GetType() == "color")
            {
            // go to the display node for the reference parameter and
            // get its color node 
            vtkMRMLDisplayableNode *rn
              = vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()
                                            ->GetNodeByID(reference.c_str()));
            if (rn)
              {
              vtkMRMLDisplayNode *dn = rn->GetDisplayNode();
              if (dn)
                {
                // get the id of the color node
                if (dn->GetColorNode())
                  {
                  id = dn->GetColorNode()->GetID();
                  }
                else
                  {
                  vtkErrorMacro(<< "Display node of the reference node does not have a color node. No value for \"table\" parameter.");
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
              vtkErrorMacro(<< "Reference node \"" << reference.c_str()
                            << "\" does not exist in the scene.");
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
          vtkErrorMacro(<< "Hidden parameters not supported on \""
                        << p.GetType().c_str() << "\"");
          }
        }
      else
        {
        vtkErrorMacro(<< "Reference parameter \"" << p.GetReference().c_str()
                      << "\" not found.");
        }
      }
    else
      {
      // no reference node
      if (this->GetDebug())
        {
        vtkErrorMacro(<< "Hidden parameter \"" << p.GetName().c_str()
                      << "\" but no reference parameter.");
        }
      }
    }
  else
    {
    // not a hidden node, just return the default
    id = p.GetDefault();
    }

  return id;
}

//----------------------------------------------------------------------------
bool vtkSlicerCLIModuleLogic
::IsCommandLineModuleNodeUpdatingDisplay(vtkMRMLCommandLineModuleNode* node)const
{
  // Update display except if the node has the updateDisplay attribute set to
  // "false".
  const char* updateDisplay = node->GetAttribute("UpdateDisplay");
  return !updateDisplay || (strcmp(updateDisplay, "false") != 0);
}

//----------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
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
    events->InsertNextValue(
      vtkMRMLCommandLineModuleNode::ParameterChangedEvent);
    events->InsertNextValue(
      vtkMRMLCommandLineModuleNode::InputParameterModifiedEvent);
    events->InsertNextValue(
      vtkMRMLCommandLineModuleNode::AutoRunEvent);
    vtkObserveMRMLNodeEventsMacro(node, events.GetPointer());
    }
  this->Superclass::OnMRMLSceneNodeAdded(node);
}

//---------------------------------------------------------------------------
void vtkSlicerCLIModuleLogic
::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event,
                         void *callData)
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
  assert(node);
  // Observe only the CLI of the logic.
  vtkMRMLCommandLineModuleNode* cliNode =
    vtkMRMLCommandLineModuleNode::SafeDownCast(node);
  if (cliNode &&
      cliNode->GetModuleTitle() ==
        this->Internal->DefaultModuleDescription.GetTitle())
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        break;
      case vtkMRMLCommandLineModuleNode::ParameterChangedEvent:
        if (cliNode->GetAutoRun() &
            vtkMRMLCommandLineModuleNode::AutoRunWhenParameterChanged)
          {
          this->AutoRun(cliNode);
          }
        break;
      case vtkMRMLCommandLineModuleNode::InputParameterModifiedEvent:
        if (cliNode->GetAutoRun() &
            vtkMRMLCommandLineModuleNode::AutoRunWhenInputModified)
          {
          this->AutoRun(cliNode);
          }
        break;
      case vtkMRMLCommandLineModuleNode::AutoRunEvent:
        {
        unsigned long requestTime = reinterpret_cast<unsigned long>(callData);
        if (cliNode->IsAutoRunOn() &&
            ((cliNode->GetAutoRun() & vtkMRMLCommandLineModuleNode::AutoRunWhenInputModified &&
              cliNode->GetInputMTime() <= requestTime) ||
             (cliNode->GetAutoRun() & vtkMRMLCommandLineModuleNode::AutoRunWhenParameterChanged &&
              cliNode->GetParameterMTime() <= requestTime)))
          {
          if (cliNode->IsBusy())
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
          vtkDebugMacro(<<"Skip AutoRun, it is not the last event");
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
void vtkSlicerCLIModuleLogic
::AutoRun(vtkMRMLCommandLineModuleNode* node)
{
  if (!node || !node->IsAutoRunOn())
    {
    return;
    }
  unsigned int extraDelay = 0;

  if (node->IsBusy())
    {
    if (!(node->GetAutoRun()
          & vtkMRMLCommandLineModuleNode::AutoRunCancelsRunningProcess))
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
  unsigned long requestTime =
    (node->GetAutoRun() & vtkMRMLCommandLineModuleNode::AutoRunWhenInputModified) ?
    node->GetInputMTime() : node->GetParameterMTime();
  // Wait a bit (for potential other modifications) before re-running the module.
  this->GetApplicationLogic()->InvokeEventWithDelay(
    node->GetAutoRunDelay() + extraDelay,
    node, vtkMRMLCommandLineModuleNode::AutoRunEvent,
    reinterpret_cast<void*>(requestTime));
}
