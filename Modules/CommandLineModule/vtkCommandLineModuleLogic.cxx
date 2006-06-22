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

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"

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

void vtkCommandLineModuleLogic::Apply()
{
  // check if MRML node is present 
  if (this->CommandLineModuleNode == NULL)
    {
    vtkErrorMacro("No input CommandLineModuleNode found");
    return;
    }

//   // copy RASToIJK matrix, and other attributes from input to output
//   std::string name (outVolume->GetName());
//   std::string id (outVolume->GetID());

//   outVolume->Copy(inVolume);

//   outVolume->SetName(name.c_str());
//   outVolume->SetID(id.c_str());


  
  // build the command line
  //
  //
  std::vector<std::string> commandLineAsString;

  commandLineAsString.push_back( this->CommandLineModuleNode->GetModuleDescription().GetTarget() );


  // map to keep track of MRML Ids and filenames
  typedef std::map<std::string, std::string> MRMLIDToFileNameMap;
  MRMLIDToFileNameMap mrmlIDToFileName;

  // vector of files to delete
  std::set<std::string> filesToDelete;
  
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->CommandLineModuleNode->GetModuleDescription()
    .GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->CommandLineModuleNode->GetModuleDescription()
    .GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    // make a pass for any parameters that have flags
    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetLongFlag() != "")
        {
        commandLineAsString.push_back(std::string("--")+(*pit).GetLongFlag());
        if ((*pit).GetTag() != "boolean")
          {
          commandLineAsString.push_back((*pit).GetDefault());
          }
        }
      else if ((*pit).GetFlag() != "")
        {
        commandLineAsString.push_back(std::string("-")+(*pit).GetFlag());
        if ((*pit).GetTag() != "boolean")
          {
          commandLineAsString.push_back((*pit).GetDefault());
          }
        }
      }

    // now tack on any parameters that are based on indices
    // (still need to do images associated with flags)
    std::map<int, ModuleParameter> indexmap;
    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetIndex() != "")
        {
        indexmap[atoi((*pit).GetIndex().c_str())] = (*pit);
        }
      }

    std::map<int, ModuleParameter>::const_iterator iit;
    for (iit = indexmap.begin(); iit != indexmap.end(); ++iit)
      {
      if ((*iit).second.GetTag() != "image")
        {
        commandLineAsString.push_back((*iit).second.GetDefault());
        }
      else
        {
        // Need temporary files
        if ((*iit).second.GetChannel() == "input")
          {
          // Check to make sure a node was selected
          if (!this->MRMLScene
              ->GetNodeByID((*iit).second.GetDefault().c_str())
              || (*iit).second.GetDefault() == "None")
            {
            vtkErrorMacro("No input volume assigned to \""
                          << (*iit).second.GetLabel().c_str() << "\"");
            return;
            }
          
          // Need to write out the data to a temporary file and push
          // the name of this file on the command line
          vtkMRMLVolumeArchetypeStorageNode *image
            = vtkMRMLVolumeArchetypeStorageNode::New();

          // To avoid confusing the Archetype readers, convert any
          // numbers in the filename to characters [0-9]->[A-J]
          std::string name = (*iit).second.GetDefault();
          std::transform(name.begin(), name.end(),
                         name.begin(), DigitsToCharacters());
          
          std::string fname = this->TemporaryDirectory + "/"
            + name + ".nrrd";
          
          image->SetFileArchetype( fname.c_str() );
          image->WriteData(this->MRMLScene->GetNodeByID((*iit).second.GetDefault().c_str()));

          commandLineAsString.push_back( image->GetFileArchetype() );
          image->Delete();

          filesToDelete.insert(fname);
          }
        else if ((*iit).second.GetChannel() == "output")
          {
          // Check to make sure a node was selected
          if (!this->MRMLScene
              ->GetNodeByID((*iit).second.GetDefault().c_str())
              || (*iit).second.GetDefault() == "None")
            {
            vtkErrorMacro("No output volume assigned to \""
                          << (*iit).second.GetLabel().c_str() << "\"");
            return;
            }

          // Need to build a filename and push it on the command line.
          // we'll read the data back in later

          // To avoid confusing the Archetype readers, convert any
          // numbers in the filename to characters [0-9]->[A-J]
          std::string name = (*iit).second.GetDefault();
          std::transform(name.begin(), name.end(),
                         name.begin(), DigitsToCharacters());
          

          std::string fname = this->TemporaryDirectory + "/"
            + name + ".nrrd";

          mrmlIDToFileName[(*iit).second.GetDefault()] = fname;
          commandLineAsString.push_back( fname );

          filesToDelete.insert(fname);
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
  for (std::vector<std::string>::size_type i=0; i < commandLineAsString.size(); ++i)
    {
    std::cout << command[i] << " ";
    }
  std::cout << std::endl;
 
  // run the filter
  //
  //
  itksysProcess *process = itksysProcess_New();

  // setup the command
  itksysProcess_SetCommand(process, command);
  itksysProcess_SetOption(process,
                          itksysProcess_Option_Detach, 0);
  itksysProcess_SetOption(process,
                          itksysProcess_Option_HideWindow, 0);
  // itksysProcess_SetTimeout(process, 5.0); // 5 seconds

  // execute the command
  itksysProcess_Execute(process);

  // Wait for the command to finish
  char *tbuffer;
  int length;
  int pipe;
  std::string stdoutbuffer;
  std::string stderrbuffer;
  while ((pipe = itksysProcess_WaitForData(process ,&tbuffer,
                                           &length, 0)) != 0)
    {
    if (length != 0 && tbuffer != 0)
      {
      if (pipe == itksysProcess_Pipe_STDOUT)
        {
        stdoutbuffer = stdoutbuffer.append(tbuffer, length);
        }
      else if (pipe == itksysProcess_Pipe_STDERR)
        {
        stderrbuffer = stderrbuffer.append(tbuffer, length);
        }
      }
    }
  itksysProcess_WaitForExit(process, 0);
  
  // check the exit state / error state of the process
  int result = itksysProcess_GetState(process);
  if (result == itksysProcess_State_Exited)
    {
    // executable exited cleanly and must of done
    // "something" 
    if (itksysProcess_GetExitValue(process) == 0)
      {
      // executable exited without errors,
      std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
                << " completed without errors" << std::endl;
      }
    else
      {
      std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
                << " completed with errors" << std::endl;
      }
    }
  else if (result == itksysProcess_State_Expired)
    {
    std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
              << " timed out" << std::endl;
    }
  else
    {
    std::cout << this->CommandLineModuleNode->GetModuleDescription().GetTitle()
              << " unknown termination. " << result << std::endl;
    }
  
  // clean up
  itksysProcess_Delete(process);
  
  
  // import the results
  //
  //
  MRMLIDToFileNameMap::const_iterator id2fn;

  for (id2fn = mrmlIDToFileName.begin();
       id2fn != mrmlIDToFileName.end();
       ++id2fn)
    {
    vtkMRMLVolumeArchetypeStorageNode *in
      = vtkMRMLVolumeArchetypeStorageNode::New();
    in->SetFileArchetype( (*id2fn).second.c_str() );

    in->ReadData( this->MRMLScene->GetNodeByID( (*id2fn).first.c_str() ) );
    
    in->Delete();
    }

  // clean up
  //
  //
  delete [] command;

  bool removed;
  std::set<std::string>::iterator fit;
  for (fit = filesToDelete.begin(); fit != filesToDelete.end(); ++fit)
    {
    removed = itksys::SystemTools::RemoveFile((*fit).c_str());
    if (!removed)
      {
      std::cout << "Unable to delete temporary file " << *fit << std::endl;
      }
    }
}
