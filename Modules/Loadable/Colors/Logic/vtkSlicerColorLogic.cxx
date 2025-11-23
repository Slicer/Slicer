/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// Colors
#include "vtkSlicerColorLogic.h"

// MRML
#include "vtkMRMLApplicationLogic.h"
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
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

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
const char* vtkSlicerColorLogic::GetDefaultLabelMapColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericColors.txt");
}

//----------------------------------------------------------------------------
const char* vtkSlicerColorLogic::GetDefaultEditorColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericAnatomyColors.txt");
}

//----------------------------------------------------------------------------
const char* vtkSlicerColorLogic::GetDefaultChartColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("DarkBrightChartColors.txt");
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindDefaultColorFiles()
{
  // get the slicer home dir
  vtkMRMLApplicationLogic* appLogic = this->GetMRMLApplicationLogic();
  if (!appLogic)
  {
    vtkErrorMacro("FindDefaultColorFiles failed: Unable to get application logic");
    return {};
  }

  const std::string& homeDir = appLogic->GetHomeDirectory();
  if (homeDir.empty())
  {
    vtkErrorMacro("FindDefaultColorFiles failed: HomeDirectory is not set. "
                  "Default color files will not be loaded.");
    return {};
  }

  const std::string& shareDir = appLogic->GetShareDirectory();
  if (shareDir.empty())
  {
    vtkErrorMacro("FindDefaultColorFiles failed: ShareDirectory is not set. "
                  "Default color files will not be loaded.");
    return {};
  }

  // Find color files in the application share folder
  std::vector<std::string> DirectoriesToCheck;
  DirectoriesToCheck.push_back(appLogic->GetShareFilePath("ColorFiles"));
  return this->FindColorFiles(DirectoriesToCheck);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindUserColorFiles()
{
  std::vector<std::string> directoriesToCheck;
  // add the list of dirs set from the application
  if (this->UserColorFilePaths != nullptr)
  {
    vtkDebugMacro("FindColorFiles: got user color file paths = " << this->UserColorFilePaths);
    // parse out the list, breaking at delimiter strings
#ifdef _WIN32
    const char* delim = ";";
#else
    const char* delim = ":";
#endif
    char* ptr = strtok(this->UserColorFilePaths, delim);
    while (ptr != nullptr)
    {
      std::string dir = std::string(ptr);
      vtkDebugMacro("FindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
      directoriesToCheck.push_back(dir);
      ptr = strtok(nullptr, delim);
    }
  }
  else
  {
    vtkDebugMacro("FindColorFiles: User color file paths are not set");
  }

  return this->FindColorFiles(directoriesToCheck);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindColorFiles(const std::vector<std::string>& directories)
{
  std::vector<std::string> filenames;

  // Gather all the parameter set files: all text files from the provided directories
  std::vector<std::string> filesToLoad = vtkMRMLApplicationLogic::FindTextFiles(directories);
  for (const std::string& fileToLoad : filesToLoad)
  {
    // Check if Slicer supports this color file
    vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
    vtkNew<vtkMRMLProceduralColorStorageNode> procColorStorageNode;
    if (colorStorageNode->SupportedFileType(fileToLoad.c_str()) || procColorStorageNode->SupportedFileType(fileToLoad.c_str()))
    {
      vtkDebugMacro("FindColorFiles: Adding " << fileToLoad);
      this->AddColorFile(fileToLoad.c_str(), &filenames);
    }
    else
    {
      vtkWarningMacro("FindColorFiles: Not a supported color file: " << fileToLoad);
    }
  }

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
  colorLegendNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(mrmlScene->AddNewNodeByClass("vtkMRMLColorLegendDisplayNode", title + " color legend"));
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
vtkMRMLColorLegendDisplayNode* vtkSlicerColorLogic::GetNthColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode, int n)
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
    vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(displayableNode->GetNthDisplayNode(i));
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
    vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(displayableNode->GetNthDisplayNode(i));
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
    vtkMRMLColorLegendDisplayNode* foundColorLegendNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(displayableNode->GetNthDisplayNode(i));
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
vtkMRMLDisplayNode* vtkSlicerColorLogic::GetFirstNonColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode)
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
