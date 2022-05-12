/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// Colors
#include "vtkSlicerColorLogic.h"

// Slicer
#include "vtkSlicerConfigure.h" // for Slicer_SHARE_DIR

// MRML
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLProceduralColorStorageNode.h"
#include "vtkMRMLColorLegendDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <cerrno>
#endif

#include <sstream>
#include <cstring>

vtkStandardNewMacro(vtkSlicerColorLogic);

//----------------------------------------------------------------------------
vtkSlicerColorLogic::vtkSlicerColorLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerColorLogic::~vtkSlicerColorLogic() = default;

//-----------------------------------------------------------------------------
void vtkSlicerColorLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
    }
  if (!scene->IsNodeClassRegistered("vtkMRMLColorLegendDisplayNode"))
    {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLColorLegendDisplayNode>::New());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkMRMLColorLogic::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultLabelMapColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericColors.txt");
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultEditorColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericAnatomyColors.txt");
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultChartColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("DarkBrightChartColors.txt");
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindDefaultColorFiles()
{
  // get the slicer home dir
  std::string slicerHome;
  if (vtksys::SystemTools::GetEnv("SLICER_HOME") == nullptr)
    {
    if (vtksys::SystemTools::GetEnv("PWD") != nullptr)
      {
      slicerHome =  std::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      slicerHome =  std::string("");
      }
    }
  else
    {
    slicerHome = std::string(vtksys::SystemTools::GetEnv("SLICER_HOME"));
    }
  // build up the vector
  std::vector<std::string> filesVector;
  filesVector.emplace_back(""); // for relative path
  filesVector.push_back(slicerHome);
  filesVector.push_back(std::string(Slicer_SHARE_DIR) + "/ColorFiles");
  std::string resourcesDirString = vtksys::SystemTools::JoinPath(filesVector);

  // now make up a vector to iterate through of dirs to look in
  std::vector<std::string> DirectoriesToCheck;

  DirectoriesToCheck.push_back(resourcesDirString);

  return this->FindColorFiles(DirectoriesToCheck);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindUserColorFiles()
{
  std::vector<std::string> DirectoriesToCheck;
  // add the list of dirs set from the application
  if (this->UserColorFilePaths != nullptr)
    {
    vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
    // parse out the list, breaking at delimiter strings
#ifdef WIN32
    const char *delim = ";";
#else
    const char *delim = ":";
#endif
    char *ptr = strtok(this->UserColorFilePaths, delim);
    while (ptr != nullptr)
      {
      std::string dir = std::string(ptr);
      vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
      DirectoriesToCheck.push_back(dir);
      ptr = strtok(nullptr, delim);
      }
    } else { vtkDebugMacro("\nFindColorFiles: oops, the user color file paths aren't set!"); }

  return this->FindColorFiles(DirectoriesToCheck);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindColorFiles(const std::vector<std::string>& directories)
{
  std::vector<std::string> filenames;

  // get the list of color files in these dir
  for (unsigned int d = 0; d < directories.size(); d++)
    {
    std::string dirString = directories[d];
    vtkDebugMacro("FindColorFiles: checking for color files in dir " << d << " = " << dirString.c_str());

    std::vector<std::string> filesVector;
    filesVector.push_back(dirString);
    filesVector.emplace_back("/");

#ifdef WIN32
    WIN32_FIND_DATA findData;
    HANDLE fileHandle;
    int flag = 1;
    std::string search ("*.*");
    dirString += "/";
    search = dirString + search;

    fileHandle = FindFirstFile(search.c_str(), &findData);
    if (fileHandle != INVALID_HANDLE_VALUE)
      {
      while (flag)
        {
        // add this file to the vector holding the base dir name so check the
        // file type using the full path
        filesVector.push_back(std::string(findData.cFileName));
#else
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dirString.c_str())) == nullptr)
      {
      vtkErrorMacro("\nError(" << errno << ") opening user specified color path: " << dirString.c_str() << ", no color files will be loaded from that directory\n(check Edit -> Application Settings -> Module Settings to adjust your User defined color file paths)");
      }
    else
      {
      while ((dirp = readdir(dp)) != nullptr)
        {
        // add this file to the vector holding the base dir name
        filesVector.emplace_back(dirp->d_name);
#endif

        std::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector);
        int fileType = vtksys::SystemTools::DetectFileType(fileToCheck.c_str());
        if (fileType == vtksys::SystemTools::FileTypeText)
          {
          // check that it's a supported file type
          // create storage nodes so can check for supported file types
          vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
          vtkNew<vtkMRMLProceduralColorStorageNode> procColorStorageNode;
          if (colorStorageNode->SupportedFileType(fileToCheck.c_str()) ||
              procColorStorageNode->SupportedFileType(fileToCheck.c_str()))
            {
            vtkDebugMacro("FindColorFiles: Adding " << fileToCheck.c_str() << " to list of potential color files. Type = " << fileType);
              // add it to the list
              this->AddColorFile(fileToCheck.c_str(), &filenames);
            }
          else
            {
            vtkWarningMacro("FindColorFiles: not a supported file type:\n"<< fileToCheck);
            }
          }
        else
          {
          vtkDebugMacro("FindColorFiles: Skipping potential color file " << fileToCheck.c_str() << ", not a text file (file type = " << fileType << ")");
          }
        // take this file off so that can build the next file name
        filesVector.pop_back();

#ifdef WIN32
        flag = FindNextFile(fileHandle, &findData);
        } // end of while flag
      FindClose(fileHandle);
      } // end of having a valid fileHandle
#else
        } // end of while loop over reading the directory entries
      closedir(dp);
      } // end of able to open dir
#endif
    } // end of looping over dirs
  return filenames;
}

//----------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode* vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode)
{
  vtkMRMLDisplayNode* displayNode = vtkSlicerColorLogic::GetFirstNonColorLegendDisplayNode(displayableNode);
  if (!displayNode)
    {
    vtkGenericWarningMacro("AddDefaultColorLegendDisplayNode failed: no valid display node is found.");
    return nullptr;
    }
  return vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
}

//----------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode* vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
    {
    vtkGenericWarningMacro("AddDefaultColorLegendDisplayNode: Display node is invalid");
    return nullptr;
    }
  vtkMRMLScene* mrmlScene = displayNode->GetScene();
  if (!mrmlScene)
    {
    vtkErrorWithObjectMacro(displayNode, "vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode: Invalid MRML scene");
    return nullptr;
    }

  vtkMRMLColorLegendDisplayNode* colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  if (colorLegendNode)
    {
    // Found already existing color legend node
    return colorLegendNode;
    }

  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  if (!displayNode)
    {
    vtkErrorWithObjectMacro(displayNode, "vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode: Displayable node is invalid");
    return nullptr;
    }

  std::string title = displayableNode->GetName() ? displayableNode->GetName() : "";

  // Create color legend and observe color legend by displayable node
  colorLegendNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(
    mrmlScene->AddNewNodeByClass("vtkMRMLColorLegendDisplayNode", title + " color legend"));
  if (!colorLegendNode)
    {
    vtkErrorWithObjectMacro(displayNode, "vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode: Failed to create vtkMRMLColorLegendDisplayNode");
    return nullptr;
    }
  colorLegendNode->SetTitleText(title);
  colorLegendNode->SetAndObservePrimaryDisplayNode(displayNode);

  // If the color node is a color table containing only a handful of colors then most likely it is a
  // list of named colors, tehrefore use the color names for labels.
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(displayNode->GetColorNode());
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(colorNode);
  if (colorTableNode && (colorTableNode->GetNumberOfColors() < 20))
    {
    // it looks like a discrete color table, use color names instead of labels by default
    colorLegendNode->SetMaxNumberOfColors(colorNode->GetNumberOfColors());
    colorLegendNode->SetUseColorNamesForLabels(true);
    colorLegendNode->SetLabelFormat(colorLegendNode->GetDefaultTextLabelFormat());
    }

  // Add color legend to displayable node
  // observe primary display node to get current scalar range
  displayableNode->AddAndObserveDisplayNodeID(colorLegendNode->GetID());
  // Adding as display node to displayableNode does not trigger an update in vtkMRMLColorLegendDisplayableManager.
  // We trigger an update manually as a workaround.
  colorLegendNode->Modified();

  return colorLegendNode;
}

//----------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode* vtkSlicerColorLogic::GetNthColorLegendDisplayNode(
  vtkMRMLDisplayableNode* displayableNode, int n)
{
  if (!displayableNode)
   {
   vtkGenericWarningMacro("vtkSlicerColorLogic::GetNthColorLegendDisplayNode: Displayable node is invalid");
   return nullptr;
   }

  int colorLegendIndex = 0;
  int numberOfDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; ++i)
    {
    vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(
      displayableNode->GetNthDisplayNode(i));
    if (!colorLegendDisplayNode)
      {
      continue;
      }
    if (colorLegendIndex == n)
      {
      return colorLegendDisplayNode;
      }
    colorLegendIndex++;
    }

  return nullptr;
}

//----------------------------------------------------------------------------
int vtkSlicerColorLogic::GetNumberOfColorLegendDisplayNodes(vtkMRMLDisplayableNode* displayableNode)
{
  if (!displayableNode)
   {
   vtkGenericWarningMacro("vtkSlicerColorLogic::GetNumberOfColorLegendDisplayNodes: Displayable node is invalid");
   return 0;
   }
  int numberOfColorLegendDIsplayNodes = 0;
  int numberOfDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; ++i)
    {
    vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(
      displayableNode->GetNthDisplayNode(i));
    if (colorLegendDisplayNode)
      {
      numberOfColorLegendDIsplayNodes++;
      }
    }
  return numberOfColorLegendDIsplayNodes;
}

//------------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode* vtkSlicerColorLogic::GetColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode)
{
  vtkMRMLDisplayNode* displayNode = vtkSlicerColorLogic::GetFirstNonColorLegendDisplayNode(displayableNode);
  return vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
}

//------------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode* vtkSlicerColorLogic::GetColorLegendDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return nullptr;
    }
  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  if (!displayableNode)
    {
    return nullptr;
    }
  vtkMRMLColorLegendDisplayNode* colorLegendNode = nullptr;
  int nofDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
  for (int i = 0; i < nofDisplayNodes; ++i)
    {
    vtkMRMLColorLegendDisplayNode* foundColorLegendNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(
      displayableNode->GetNthDisplayNode(i));
    if (!foundColorLegendNode)
      {
      continue;
      }
    if (foundColorLegendNode->GetPrimaryDisplayNode() == displayNode)
      {
      // found an exact match
      return foundColorLegendNode;
      }
    if (!colorLegendNode && !foundColorLegendNode->GetPrimaryDisplayNode())
      {
      // found a color legend node that is not bound to a specific display node,
      // use it if more specific match is not found
      colorLegendNode = foundColorLegendNode;
      }
    }

  return colorLegendNode;
}

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkSlicerColorLogic::GetFirstNonColorLegendDisplayNode(
  vtkMRMLDisplayableNode* displayableNode)
{
  if (!displayableNode)
    {
    return nullptr;
    }
  int numberOfDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; ++i)
    {
    vtkMRMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(i);
    vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(displayNode);
    if (colorLegendDisplayNode)
      {
      // it is a color legend display node, ignore it
      // (because color legend display node must refer to a non-color-legend display node)
      continue;
      }
    if (displayNode)
      {
      // first valid display node
      return displayNode;
      }
    }
  return nullptr;
}
