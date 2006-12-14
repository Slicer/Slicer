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

#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModule.h"

#include "vtkSlicerTask.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
//#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelDisplayNode.h"

#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "itksys/RegularExpression.hxx"

#include <algorithm>
#include <set>

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

//----------------------------------------------------------------------------
std::string
vtkCommandLineModuleLogic
::ConstructTemporaryFileName(const std::string& tag,
                             const std::string& type,
                             const std::string& name,
                             bool isCommandLineModule) const
{
  std::string fname = name;

  if (tag == "image")
    {
    if (isCommandLineModule || type != "scalar")
      {
      // If running an executable or running a shared memory module
      // but the image type is non-scalar...
      
      // To avoid confusing the Archetype readers, convert any
      // numbers in the filename to characters [0-9]->[A-J]
      std::transform(fname.begin(), fname.end(),
                     fname.begin(), DigitsToCharacters());
      
      fname = this->TemporaryDirectory + "/" + fname + ".nrrd";
      }
    else
      {
      // If not a command line module and the image type is scalar,
      // then it is a shared object module for which we can
      // communicated directly with the MRML tree.

      
      // Must be large enough to hold slicer:, / and two copies of the
      // ascii representation of the pointer. 256 should be more than
      // enough.
      char tname[256];
      
      sprintf(tname, "slicer:%p/%p", this->MRMLScene,
              this->MRMLScene->GetNodeByID(name.c_str()));
      
      fname = tname;
      }
    }

  if (tag == "geometry")
    {
    // geometry is currently always passed via files

    // To avoid confusing the Archetype readers, convert any
    // numbers in the filename to characters [0-9]->[A-J]
    std::transform(fname.begin(), fname.end(),
                   fname.begin(), DigitsToCharacters());
    
    fname = this->TemporaryDirectory + "/" + fname + ".vtk";
    }

    
  return fname;
}


void vtkCommandLineModuleLogic::Apply()
{
  bool ret;
  vtkSlicerTask* task = vtkSlicerTask::New();

  // Pass the current node as client data to the task.  This allows
  // the user to switch to another parameter set after the task is
  // scheduled but before it starts to run. And when the scheduled
  // task does run, it will operate on the correct node.
  task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                        &vtkCommandLineModuleLogic::ApplyTask,
                        this->CommandLineModuleNode);
  
  // Client data on the task is just a regular pointer, up the
  // reference count on the node, we'll decrease the reference count
  // once the task actually runs
  this->CommandLineModuleNode->Register(this);
  
  // Schedule the task
  ret = this->GetApplicationLogic()->ScheduleTask( task );

  if (!ret)
    {
    vtkWarningMacro( << "Could not schedule task" );
    }
  else
    {
    this->CommandLineModuleNode
      ->SetStatus(vtkMRMLCommandLineModuleNode::Scheduled);
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
  bool isCommandLine = true;

  std::string target
    = node->GetModuleDescription().GetTarget();
  std::string::size_type pos
    = target.find("slicer:");

  if (pos != std::string::npos && pos == 0)
    {
    sscanf(target.c_str(), "slicer:%p", &entryPoint);
    isCommandLine = false;
    }
  
  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap nodesToReload;
  MRMLIDToFileNameMap nodesToWrite;

  // vector of files to delete
  std::set<std::string> filesToDelete;
  //std::cout << node->GetModuleDescription();  
  // iterators for parameter groups
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = node->GetModuleDescription().GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = node->GetModuleDescription().GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;

  
  // Make a pass over the parameters and establish which parameters
  // have images or geometry that need to be written before execution
  // or loaded upon completion.
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
      if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry")
        {
        // only keep track of images associated with real nodes
        if (!this->MRMLScene->GetNodeByID((*pit).GetDefault().c_str())
            || (*pit).GetDefault() == "None")
          {
          continue;
          }

        std::string fname
          = this->ConstructTemporaryFileName((*pit).GetTag(),
                                             (*pit).GetType(),
                                             (*pit).GetDefault(),
                                             isCommandLine);

        filesToDelete.insert(fname);

        if ((*pit).GetChannel() == "input")
          {
          nodesToWrite[(*pit).GetDefault()] = fname;
          }
        else if ((*pit).GetChannel() == "output")
          {
          nodesToReload[(*pit).GetDefault()] = fname;
          }
        }
      }
    }
  

  // build the command line
  //
  //
  std::vector<std::string> commandLineAsString;

  // Command to execute
  commandLineAsString.push_back( node->GetModuleDescription().GetTarget() );


  // Add a command line flag for the process information structure
  if ( !isCommandLine )
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
        if ((*pit).GetTag() != "boolean" && (*pit).GetTag() != "image"
            && (*pit).GetTag() != "point" && (*pit).GetTag() != "geometry")
          {
          // simple parameter, write flag and value
          commandLineAsString.push_back(prefix + flag);
          commandLineAsString.push_back((*pit).GetDefault());
          }
        if ((*pit).GetTag() == "boolean")
          {
          // booleans only have a flag (no value)
          if ((*pit).GetDefault() == "true")
            {
            commandLineAsString.push_back(prefix + flag);
            }
          }
        if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry")
          {
          MRMLIDToFileNameMap::const_iterator id2fn;
          
          id2fn  = nodesToWrite.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "input" && id2fn != nodesToWrite.end())
            {
            // Only put out the flag if the node is in out list
            commandLineAsString.push_back(prefix + flag);
            commandLineAsString.push_back( (*id2fn).second );
            }

          id2fn  = nodesToReload.find( (*pit).GetDefault() );
          if ((*pit).GetChannel() == "output" && id2fn != nodesToReload.end())
            {
            commandLineAsString.push_back(prefix + flag);
            commandLineAsString.push_back( (*id2fn).second );
            }
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
              std::cerr << "Module does not support multiple fiducials."
                        << std::endl;
              }
            }
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
    if ((*iit).second.GetTag() != "image"
        && (*iit).second.GetTag() != "geometry")
      {
      commandLineAsString.push_back((*iit).second.GetDefault());
      }
    else
      {
      MRMLIDToFileNameMap::const_iterator id2fn;

      if ((*iit).second.GetChannel() == "input")
        {
        // Check to make sure the index parameter is set
        id2fn  = nodesToWrite.find( (*iit).second.GetDefault() );
        if (id2fn != nodesToWrite.end())
          {
          commandLineAsString.push_back( (*id2fn).second );
          }
        else
          {
          vtkErrorMacro("No input data assigned to \""
                        << (*iit).second.GetLabel().c_str() << "\"");

          node->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
          this->GetApplicationLogic()->RequestModified( node );
          return;
          }
        }
      else if ((*iit).second.GetChannel() == "output")
        {
        // Check to make sure the index parameter is set
        id2fn  = nodesToReload.find( (*iit).second.GetDefault() );
        if (id2fn != nodesToReload.end())
          {
          commandLineAsString.push_back( (*id2fn).second );
          }
        else
          {
          vtkErrorMacro("No output data assigned to \""
                        << (*iit).second.GetLabel().c_str() << "\"");

          node->SetStatus(vtkMRMLCommandLineModuleNode::Idle, false);
          this->GetApplicationLogic()->RequestModified( node );
          return;
          }
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

    vtkMRMLStorageNode *out = 0;

    // Determine if and how a node is to be written.  If we update the
    // MRMLIDImageIO, then we can change these conditions for the
    // other image types so that we only write nodes to disk if we are
    // running as a command line executable (and all image types will
    // go through memory in shared object modules).
    if ((isCommandLine && svnd) || vvnd)
      {
      // only write out scalar image nodes if running an executable
      out = vtkMRMLVolumeArchetypeStorageNode::New();
      }
    else if (dtvnd || dwvnd)
      {
      // for now, always write out the diffusion tensor nodes
//      out = vtkMRMLNRRDStorageNode::New();
      }
    else if (mnd)
      {
      // always write out model nodes
      out = vtkMRMLModelStorageNode::New();
      }

    // if the file is to be written, then write it
    if (out)
      {
      out->SetFileName( (*id2fn).second.c_str() );
      out->WriteData( nd );
      out->Delete();
      }
    }
  

  // run the filter
  //
  //
  node->GetModuleDescription().GetProcessInformation()->Initialize();
  node->SetStatus(vtkMRMLCommandLineModuleNode::Running, false);
  this->GetApplicationLogic()->RequestModified( node );
  if (isCommandLine)
    {
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
          //std::cerr << "STDERR: " << std::string(tbuffer, length) << std::endl;
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
  else
    {
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
      (*entryPoint)(commandLineAsString.size(), command);

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

      //std::cout << "STDOUT: " << coutstringstream.str().c_str() << std::endl;
      //std::cout << "STDERR: " << cerrstringstream.str().c_str() << std::endl;
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
      // Make request that data be reloaded. The data will loaded and
      // rendered in the main gui thread.  Data to be reloaded can be
      // safely deleted after the load. (It would not make sense for
      // two outputs of a module to produce the same file to be
      // reloaded.) We assume that if the user is looking at the node
      // now, he/she will still be looking at the node by the time the
      // data is reloaded by the main thread.
      bool displayData = false;
      bool deleteFile = true;
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

  // clean up
  //
  //
  delete [] command;

  // Remove any remaining temporary files.  At this point, these files
  // should be the files written as inputs to the module
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

  // node was registered when the task was scheduled so unregister now
  node->UnRegister(this);

}


void vtkCommandLineModuleLogic::ProgressCallback ( void *who )
{
  LogicNodePair *lnp = reinterpret_cast<LogicNodePair*>(who);

  // All we need to do is tell the node that it was Modified.  The
  // shared object plugin modifies fields in the ProcessInformation directly.
  lnp->first->GetApplicationLogic()->RequestModified(lnp->second);
}
