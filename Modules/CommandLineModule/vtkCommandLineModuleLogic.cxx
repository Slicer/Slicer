/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModule.h"

#include "vtkSlicerTask.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLROIListNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLModelHierarchyNode.h"

#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "itksys/RegularExpression.hxx"

#include <algorithm>
#include <set>

#ifdef _WIN32
#else
#include <sys/types.h>
#include <unistd.h>
#endif

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

typedef std::pair<vtkCommandLineModuleLogic *, vtkMRMLCommandLineModuleNode *> LogicNodePair;


vtkCommandLineModuleLogic* vtkCommandLineModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCommandLineModuleLogic");
  if(ret)
    {
      return (vtkCommandLineModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCommandLineModuleLogic;
}


//----------------------------------------------------------------------------
vtkCommandLineModuleLogic::vtkCommandLineModuleLogic()
{
  this->CommandLineModuleNode = NULL;
  this->DeleteTemporaryFiles = 1;
}

//----------------------------------------------------------------------------
vtkCommandLineModuleLogic::~vtkCommandLineModuleLogic()
{
  this->SetCommandLineModuleNode(NULL);
}

//----------------------------------------------------------------------------
void vtkCommandLineModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

std::string
vtkCommandLineModuleLogic
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
  fname = this->TemporaryDirectory + "/" + pid + "_" + fname + ".mrml";

  return fname;
}


//----------------------------------------------------------------------------
std::string
vtkCommandLineModuleLogic
::ConstructTemporaryFileName(const std::string& tag,
                             const std::string& type,
                             const std::string& name,
                             const std::vector<std::string>& extensions,
                             CommandLineModuleType commandType) const
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
  fname = this->TemporaryDirectory + "/" + pid + "_" + fname;

  if (tag == "image")
    {
    if ( ( commandType == CommandLineModule )
         || (type != "scalar" && type != "label"))
      {
      // If running an executable or running a shared memory module
      // but the image type is non-scalar...

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
      // If not a command line module and the image type is scalar,
      // then it is a shared object module for which we can
      // communicated directly with the MRML tree.

      // Redefine the filename to be a reference to a slicer node.
      
      // Must be large enough to hold slicer:, #, an ascii
      // representation of the scene pointer and the MRML node ID. 
      char *tname = new char[name.size() + 100];
      
      sprintf(tname, "slicer:%p#%s", this->MRMLScene, name.c_str());
      
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
  
    
  return fname;
}


void vtkCommandLineModuleLogic::Apply()
{
  this->Apply ( this->CommandLineModuleNode );
}

void vtkCommandLineModuleLogic::ApplyAndWait ( vtkMRMLCommandLineModuleNode* node )
{
  // Just execute and wait.
  node->Register(this);
  vtkCommandLineModuleLogic::ApplyTask ( node );
}

void vtkCommandLineModuleLogic::Apply ( vtkMRMLCommandLineModuleNode* node )
{
  bool ret;

  if ( node->GetModuleDescription().GetType() == "PythonModule" )
    {
    this->ApplyAndWait ( node );
    return;
    }


  vtkSlicerTask* task = vtkSlicerTask::New();

  // Pass the current node as client data to the task.  This allows
  // the user to switch to another parameter set after the task is
  // scheduled but before it starts to run. And when the scheduled
  // task does run, it will operate on the correct node.
  task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                        &vtkCommandLineModuleLogic::ApplyTask,
                        node);
  
  // Client data on the task is just a regular pointer, up the
  // reference count on the node, we'll decrease the reference count
  // once the task actually runs
  node->Register(this);
  
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

//
// This routine is called in a separate thread from the main thread.
// As such, this routine cannot directly or indirectly update the user
// interface.  In the Slicer architecture, the user interface can be
// updated whenever a node receives a Modified.  Since calls to
// Modified() can update the GUI, the ApplyTask must be careful not to
// modify a MRML node.
// 
void vtkCommandLineModuleLogic::ApplyTask(void *clientdata)
{
  // check if MRML node is present 
  if (clientdata == NULL)
    {
    vtkErrorMacro("No input CommandLineModuleNode found");
    return;
    }

  vtkMRMLCommandLineModuleNode *node = reinterpret_cast<vtkMRMLCommandLineModuleNode*>(clientdata);

  // Check to see if this node/task has been cancelled
  if (node->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelled)
    {
    // node was registered when the task was scheduled so unregister now
    node->UnRegister(this);

    return;
    }


  // Set the callback for progress.  This will only be used for the
  // scope of this function.
  LogicNodePair lnp( this, node );
  node->GetModuleDescription().GetProcessInformation()
    ->SetProgressCallback( vtkCommandLineModuleLogic::ProgressCallback,
                           &lnp );
  
  
  // Determine the type of the module: command line or shared object
  int (*entryPoint)(int argc, char* argv[]);
  entryPoint = NULL;
  CommandLineModuleType commandType = CommandLineModule;

  std::string target
    = node->GetModuleDescription().GetTarget();
  std::string::size_type pos = target.find("slicer:");
  if (pos != std::string::npos && pos == 0)
    {
    sscanf(target.c_str(), "slicer:%p", &entryPoint);
    }

  // Assume that the modules correctly report themselves
  if ( node->GetModuleDescription().GetType() == "CommandLineModule" )
    {
    vtkSlicerApplication::GetInstance()->InformationMessage( "Found CommandLine Module" );
    commandType = CommandLineModule;
    if ( entryPoint != NULL )
      {
      vtkWarningMacro("Module reports that it is a Command Line Module but has a shared object module target. " << target.c_str());
      }
    }
  else if ( node->GetModuleDescription().GetType() == "SharedObjectModule" )
    {
    vtkSlicerApplication::GetInstance()->InformationMessage( "Found SharedObject Module" );

    commandType = SharedObjectModule;
    if ( entryPoint == NULL )
      {
      vtkWarningMacro("Module reports that it is a Shared Object Module but does not have a shared object module target. " << target.c_str());
      }
    }
  else if ( node->GetModuleDescription().GetType() == "PythonModule" )
    {
    vtkSlicerApplication::GetInstance()->InformationMessage( "Found Python Module" );
    commandType = PythonModule;
    }
  vtkSlicerApplication::GetInstance()->InformationMessage( node->GetModuleDescription().GetType().c_str() );

  
  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap nodesToReload;
  MRMLIDToFileNameMap nodesToWrite;

  // map to keep track of the MRML Ids on the main scene to the MRML
  // Ids in the miniscene sent to the module
  typedef std::map<std::string, std::string> MRMLIDMap;
  MRMLIDMap sceneToMiniSceneMap;

  // Mini-scene used to communicate a subset of the main scene to the module
  vtkSmartPointer<vtkMRMLScene> miniscene = vtkMRMLScene::New();
  std::string minisceneFilename
    = this->ConstructTemporarySceneFileName(miniscene);
 
  // vector of files to delete
  std::set<std::string> filesToDelete;

  // iterators for parameter groups
  std::vector<ModuleParameterGroup>::iterator pgbeginit
    = node->GetModuleDescription().GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::iterator pgendit
    = node->GetModuleDescription().GetParameterGroups().end();
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
          || (*pit).GetTag() == "transform" || (*pit).GetTag() == "table")
        {
        std::string id = (*pit).GetDefault();

        // if the parameter is hidden, then deduce its value/id
        if ((*pit).GetHidden() == "true")
          {
          id = this->FindHiddenNodeID(node->GetModuleDescription(), *pit);

          // cache the id so we don't have to look for it later
          (*pit).SetDefault( id );
          }
        
        // only keep track of objects associated with real nodes
        if (!this->MRMLScene->GetNodeByID(id.c_str()) || id == "None")
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
  MRMLIDToFileNameMap::const_iterator id2fn;
    
  for (id2fn = nodesToWrite.begin();
       id2fn != nodesToWrite.end();
       ++id2fn)
    {
    vtkMRMLNode *nd
      = this->MRMLScene->GetNodeByID( (*id2fn).first.c_str() );
    
    vtkMRMLScalarVolumeNode *svnd
      = vtkMRMLScalarVolumeNode::SafeDownCast(nd);
    vtkMRMLVectorVolumeNode *vvnd
      = vtkMRMLVectorVolumeNode::SafeDownCast(nd);
    vtkMRMLDiffusionTensorVolumeNode *dtvnd
      = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(nd);
    vtkMRMLDiffusionWeightedVolumeNode *dwvnd
      = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(nd);
    vtkMRMLModelNode *mnd
      = vtkMRMLModelNode::SafeDownCast(nd);
    vtkMRMLFiberBundleNode *fbnd
      = vtkMRMLFiberBundleNode::SafeDownCast(nd);
    vtkMRMLTransformNode *tnd
      = vtkMRMLTransformNode::SafeDownCast(nd);
    vtkMRMLColorTableNode *ctnd
      = vtkMRMLColorTableNode::SafeDownCast(nd);
    vtkMRMLModelHierarchyNode *mhnd
      = vtkMRMLModelHierarchyNode::SafeDownCast(nd);

    
    vtkMRMLStorageNode *out = 0;

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
    if (((commandType == CommandLineModule) && svnd) || vvnd)
      {
      // only write out scalar image nodes if running an executable
      out = vtkMRMLVolumeArchetypeStorageNode::New();
      }
    else if (dtvnd || dwvnd)
      {
      // for now, always write out the diffusion tensor nodes
      out = vtkMRMLNRRDStorageNode::New();
      }
    else if (fbnd)
      {
      out = vtkMRMLFiberBundleStorageNode::New();
      }
    else if (mnd)
      {
      // always write out model nodes
      out = vtkMRMLModelStorageNode::New();
      }
    else if (tnd)
      {
      // always write out transform nodes

      // no storage node for transforms. put the transform in the mini-scene.
      vtkMRMLNode *cp = miniscene->CopyNode(nd);

      // Keep track what scene node corresponds to what miniscene node
      sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
      }
    else if (ctnd)
      {
      // always write out color table nodes
      out = vtkMRMLColorTableStorageNode::New();
      }
    else if (mhnd)
      {
      // model hierarchy nodes need to get put in a scene
      vtkMRMLNode *cp = miniscene->CopyNode(nd);

      // keep track of scene node corresponds to what the miniscene node
      sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
      }

    // if the file is to be written, then write it
    if (out)
      {
      out->SetFileName( (*id2fn).second.c_str() );
      if (!out->WriteData( nd ))
        {
        vtkErrorMacro("ERROR writing file " << out->GetFileName());
        }
      out->Delete();
      }
    }

  // Also need to run through any output nodes that will be
  // communicated through the miniscene and add them to the miniscene
  // 
  for (id2fn = nodesToReload.begin();
       id2fn != nodesToReload.end();
       ++id2fn)
    {
    vtkMRMLNode *nd
      = this->MRMLScene->GetNodeByID( (*id2fn).first.c_str() );
    
    vtkMRMLTransformNode *tnd
      = vtkMRMLTransformNode::SafeDownCast(nd);
    vtkMRMLModelHierarchyNode *mhnd
      = vtkMRMLModelHierarchyNode::SafeDownCast(nd);
  
    if (tnd || mhnd)
      {
      // always put transform and model hierarchy nodes in the miniscene
      vtkMRMLNode *cp = miniscene->CopyNode(nd);
      
      // Keep track what scene node corresponds to what miniscene node
      sceneToMiniSceneMap[nd->GetID()] = cp->GetID();
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
  commandLineAsString.push_back( node->GetModuleDescription().GetTarget() );


  // Add a command line flag for the process information structure
  if ( commandType == SharedObjectModule )
    {
    commandLineAsString.push_back( "--processinformationaddress" );

    char tname[256];
    sprintf(tname, "%p", node->GetModuleDescription().GetProcessInformation());
    
    commandLineAsString.push_back( tname );
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
            && (*pit).GetTag() != "table")
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
            || (*pit).GetTag() == "transform" || (*pit).GetTag() == "table")
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

          // Only put out the flag if the node is in out list
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
            = this->MRMLScene->GetNodeByID((*pit).GetDefault().c_str());
          vtkMRMLFiducialListNode *fiducials
            = vtkMRMLFiducialListNode::SafeDownCast(node);

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
                std::ostrstream ptAsString;

                if (fiducials->GetNthFiducialSelected(i))
                  {
                  pt = fiducials->GetNthFiducialXYZ(i);
                  ptAsString << pt[0] << "," << pt[1] << "," << pt[2]
                             << std::ends;
                  ptAsString.rdbuf()->freeze();
                  
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
          continue;
          }
        if ((*pit).GetTag() == "region")
          {
          // get the region node
          vtkMRMLNode *node
            = this->MRMLScene->GetNodeByID((*pit).GetDefault().c_str());
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
                float *pt;
                float *Radius;
                std::ostrstream roiAsString;

                if (regions->GetNthROISelected(i))
                  {
                  pt = regions->GetNthROIXYZ(i);
                  Radius = regions->GetNthROIRadiusXYZ(i);
                  roiAsString << pt[0] << "," << pt[1] << "," << pt[2] << ","
                             << Radius[0] << "," << Radius[1] << "," << Radius[2]
                             << std::ends;
                  roiAsString.rdbuf()->freeze();
                  
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

        node->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
        this->GetApplicationLogic()->RequestModified( node );
        return;
        }
      }
    else if ((*iit).second.GetTag() == "point"
             || (*iit).second.GetTag() == "region")
      {
      vtkErrorMacro("Fiducials and ROIs are not currently supported as index arguments to modules.");
      node->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
      this->GetApplicationLogic()->RequestModified( node );
      return;
      }
    else
      {
      // image or geometry or transform or table index parameter

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

        node->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
        this->GetApplicationLogic()->RequestModified( node );
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
  std::stringstream information;
  information << node->GetModuleDescription().GetTitle()
              << " command line: " << std::endl << std::endl;
  for (std::vector<std::string>::size_type i=0; i < commandLineAsString.size(); ++i)
    {
    information << command[i] << " ";
    }
  information << std::endl;
  vtkSlicerApplication::GetInstance()->InformationMessage( information.str().c_str() );
  

  // run the filter
  //
  //
  node->GetModuleDescription().GetProcessInformation()->Initialize();
  node->SetStatus(vtkMRMLCommandLineModuleNode::Running, false);
  this->GetApplicationLogic()->RequestModified( node );
  if (commandType == CommandLineModule)
    {
    // Run as a command line module
    //
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
      node->GetModuleDescription().GetProcessInformation()->ElapsedTime
        += (timeoutlimit - timeout);
      this->GetApplicationLogic()->RequestModified( node );
      
      // reset the timeout value 
      timeout = timeoutlimit;

      // Check to see if the plugin was cancelled
      if (node->GetModuleDescription().GetProcessInformation()->Abort)
        {
        itksysProcess_Kill(process);
        node->GetModuleDescription().GetProcessInformation()->Progress = 0;
        node->GetModuleDescription().GetProcessInformation()->StageProgress =0;
        this->GetApplicationLogic()->RequestModified( node ); 
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
              node->GetModuleDescription().GetProcessInformation()->Progress = atof(progressString.c_str());
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
              node->GetModuleDescription().GetProcessInformation()->StageProgress = atof(progressString.c_str());
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
              strncpy(node->GetModuleDescription().GetProcessInformation()->ProgressMessage, filterString.c_str(), 1023);
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
              std::string progressMessage(stdoutbuffer, tagstart+17,
                                         tagend-tagstart-17);
              strncpy (node->GetModuleDescription().GetProcessInformation()->ProgressMessage, progressMessage.c_str(), 1023);
              foundTag = true;
              }
            }
          if (foundTag)
            {
            this->GetApplicationLogic()->RequestModified( node );
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
      stdoutbuffer.insert(0, node->GetModuleDescription().GetTitle()+tmp);
      vtkSlicerApplication::GetInstance()->InformationMessage( stdoutbuffer.c_str() );
      }
    if (stderrbuffer.size() > 0)
      {
      std::string tmp(" standard error:\n\n");
      stderrbuffer.insert(0, node->GetModuleDescription().GetTitle()+tmp);
      vtkErrorMacro( << stderrbuffer.c_str() );
      }
    
    // check the exit state / error state of the process
    if (node->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelled)
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
          information << node->GetModuleDescription().GetTitle()
                      << " completed without errors" << std::endl;
          vtkSlicerApplication::GetInstance()->InformationMessage( information.str().c_str() );
          
          }
        else
          {
          std::stringstream information;
          information << node->GetModuleDescription().GetTitle()
                      << " completed with errors" << std::endl;
          vtkErrorMacro( << information.str().c_str() );
          node->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
          this->GetApplicationLogic()->RequestModified( node );
          }
        }
      else if (result == itksysProcess_State_Expired)
        {
        std::stringstream information;
        information << node->GetModuleDescription().GetTitle()
                    << " timed out" << std::endl;
        vtkErrorMacro( << information.str().c_str() );
        node->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified( node );
        }
      else
        {
        std::stringstream information;
        if (result == itksysProcess_State_Exception)
          {
          information << node->GetModuleDescription().GetTitle();
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
        else
          {
        information << node->GetModuleDescription().GetTitle()
                  << " unknown termination. " << result << std::endl;
          }
        vtkErrorMacro( << information.str().c_str() );
        node->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified( node );
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
    try
      {
      // redirect the streams
      std::cout.rdbuf( coutstringstream.rdbuf() );
      std::cerr.rdbuf( cerrstringstream.rdbuf() );

      // run the module
      if ( entryPoint != NULL ) {
        (*entryPoint)(commandLineAsString.size(), command);
      }

      // report the output
      if (coutstringstream.str().size() > 0)
        {
        std::string tmp(" standard output:\n\n");
        tmp = node->GetModuleDescription().GetTitle()+tmp;
        
        vtkSlicerApplication::GetInstance()->InformationMessage( (tmp + coutstringstream.str()).c_str() );
        }
      if (cerrstringstream.str().size() > 0)
        {
        std::string tmp(" standard error:\n\n");
        tmp = node->GetModuleDescription().GetTitle()+tmp;

        vtkErrorMacro( << (tmp + cerrstringstream.str()).c_str() );
        }

      // reset the streams
      std::cout.rdbuf( origcoutrdbuf );
      std::cerr.rdbuf( origcerrrdbuf );

      }
    catch (itk::ExceptionObject& exc)
      {
      std::stringstream information;
      if (node->GetStatus() == vtkMRMLCommandLineModuleNode::Cancelled)
        {
        information << node->GetModuleDescription().GetTitle()
                    << " cancelled.";
        vtkSlicerApplication::GetInstance()->InformationMessage( information.str().c_str() );
        }
      else
        {
        information << node->GetModuleDescription().GetTitle()
                    << " terminated with an exception: " << exc;
        vtkErrorMacro( << information.str().c_str() );
        node->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
        this->GetApplicationLogic()->RequestModified( node );
        }

      std::cout.rdbuf( origcoutrdbuf );
      std::cerr.rdbuf( origcerrrdbuf );
      }
    catch (...)
      {
      std::stringstream information;
      information << node->GetModuleDescription().GetTitle()
                << " terminated with an unknown exception." << std::endl;
      vtkErrorMacro( << information.str().c_str() );
      node->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
      this->GetApplicationLogic()->RequestModified( node );

      std::cout.rdbuf( origcoutrdbuf );
      std::cerr.rdbuf( origcerrrdbuf );
      }
    }
  else if ( commandType == PythonModule )
    {
    // For the moment, ignore the output and just run the module
    vtkSlicerApplication::GetInstance()->InformationMessage( "Preparing to execute Python Module" );

    // Now, call Python properly.  For the moment, make a big string...
    // ...later we'll want to do this through the Python API
    std::string ExecuteModuleString =
      "import sys;\n"
      "import Slicer;\n"
      "ModuleName = \"" + node->GetModuleDescription().GetTarget() + "\"\n"
      "ModuleArgs = []\n"
      "ArgTags = []\n";

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
        ExecuteModuleString += "ArgTags.append ( '" + (*pit).GetTag() + "' );\n";
        }
      }
    ExecuteModuleString +=
      "FlagArgs, PositionalArgs = Slicer.ParseArgs ( ModuleArgs, ArgTags )\n"
      "Module = __import__ ( ModuleName )\n"
      "reload ( Module )\n"
      "Module.Execute ( *PositionalArgs, **FlagArgs )\n";
#ifdef USE_PYTHON    
    PyObject* v;
      
    v = PyRun_String( ExecuteModuleString.c_str(),
                      Py_file_input,
                      vtkSlicerApplication::GetInstance()->GetPythonDictionary(),
                      vtkSlicerApplication::GetInstance()->GetPythonDictionary() );
    if (v == NULL)
      {
      node->SetStatus(vtkMRMLCommandLineModuleNode::CompletedWithErrors, false);
      PyErr_Print();
      }
    else
      {
      node->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
      if (Py_FlushLine())
        {
        PyErr_Clear();
        }
      }
#else
    vtkErrorMacro("Attempting to execute a Python Module without Python support enabled");
#endif

    this->GetApplicationLogic()->RequestModified( node );
    }
  if (node->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelled 
      && node->GetStatus() != vtkMRMLCommandLineModuleNode::CompletedWithErrors)
    {
    node->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
    this->GetApplicationLogic()->RequestModified( node );
    }
  // reset the progress to zero
  node->GetModuleDescription().GetProcessInformation()->Progress = 0;
  node->GetModuleDescription().GetProcessInformation()->StageProgress = 0;
  this->GetApplicationLogic()->RequestModified( node );
  
  // import the results if the plugin was allowed to complete
  //
  //
  if (node->GetStatus() != vtkMRMLCommandLineModuleNode::Cancelled
      && node->GetStatus() != vtkMRMLCommandLineModuleNode::CompletedWithErrors)
    {
    for (id2fn = nodesToReload.begin();
         id2fn != nodesToReload.end();
         ++id2fn)
      {
      // Is this node one that was put in the miniscene? Nodes in the
      // miniscene will be handled later 
      //
      MRMLIDMap::iterator mit = sceneToMiniSceneMap.find((*id2fn).first);
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
        bool displayData = false;
        bool deleteFile = this->GetDeleteTemporaryFiles();
        displayData = (node == this->GetCommandLineModuleNode());
        this->GetApplicationLogic()
          ->RequestReadData((*id2fn).first.c_str(), (*id2fn).second.c_str(),
                            displayData, deleteFile);
        
        // If we are reloading a file, then we know that it is a file
        // that needs to be removed.  It wouldn't make sense for two
        // outputs of a module to produce the same file to be reloaded.
        filesToDelete.erase( (*id2fn).second );
        }
      }
    }

  // if there was a miniscene that needs loading, request it
  if (miniscene->GetNumberOfNodes() > 0)
    {
    bool displayData = false;
    bool deleteFile = this->GetDeleteTemporaryFiles();
    displayData = (node == this->GetCommandLineModuleNode());

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

  // node was registered when the task was scheduled so unregister now
  node->UnRegister(this);
  miniscene->Delete();

}


void vtkCommandLineModuleLogic::ProgressCallback ( void *who )
{
  LogicNodePair *lnp = reinterpret_cast<LogicNodePair*>(who);

  // All we need to do is tell the node that it was Modified.  The
  // shared object plugin modifies fields in the ProcessInformation directly.
  lnp->first->GetApplicationLogic()->RequestModified(lnp->second);
}

std::string
vtkCommandLineModuleLogic::FindHiddenNodeID(const ModuleDescription& d,
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
              = vtkMRMLDisplayableNode::SafeDownCast(this->MRMLScene
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
      vtkErrorMacro(<< "Hidden parameter \"" << p.GetName().c_str()
                    << "\" but no reference parameter.");
      }
    }
  else
    {
    // not a hidden node, just return the default
    id = p.GetDefault();
    }

  return id;
}
