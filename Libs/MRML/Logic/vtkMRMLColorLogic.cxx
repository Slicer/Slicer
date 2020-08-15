/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLColorLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLColorLogic.h"

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLdGEMRICProceduralColorNode.h"
#include "vtkMRMLPETProceduralColorNode.h"
#include "vtkMRMLProceduralColorStorageNode.h"
#include "vtkMRMLScene.h"

// VTK sys includes
#include <vtkLookupTable.h>
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <ctype.h> // For isspace
#include <functional>
#include <random>
#include <sstream>

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::TempColorNodeID;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLColorLogic);

//----------------------------------------------------------------------------
vtkMRMLColorLogic::vtkMRMLColorLogic()
{
  this->UserColorFilePaths = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLColorLogic::~vtkMRMLColorLogic()
{
  // remove the default color nodes
  this->RemoveDefaultColorNodes();

  // clear out the lists of files
  this->ColorFiles.clear();
  this->UserColorFiles.clear();

  if (this->UserColorFilePaths)
    {
    delete [] this->UserColorFilePaths;
    this->UserColorFilePaths = nullptr;
    }
}

//------------------------------------------------------------------------------
void vtkMRMLColorLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  // We are solely interested in vtkMRMLScene::NewSceneEvent,
  // we don't want to listen to any other events.
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());

  if (newScene)
    {
    this->OnMRMLSceneNewEvent();
    }
}

//------------------------------------------------------------------------------
void vtkMRMLColorLogic::OnMRMLSceneNewEvent()
{
  this->AddDefaultColorNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkMRMLColorLogic:             " << this->GetClassName() << "\n";

  os << indent << "UserColorFilePaths: " << this->GetUserColorFilePaths() << "\n";
  os << indent << "Color Files:\n";
  for (size_t i = 0; i < this->ColorFiles.size(); i++)
    {
    os << indent.GetNextIndent() << i << " " << this->ColorFiles[i].c_str() << "\n";
    }
  os << indent << "User Color Files:\n";
  for (size_t i = 0; i < this->UserColorFiles.size(); i++)
    {
    os << indent.GetNextIndent() << i << " " << this->UserColorFiles[i].c_str() << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultColorNodes()
{
  // create the default color nodes, they don't get saved with the scenes as
  // they'll be created on start up, and when a new
  // scene is opened
  if (this->GetMRMLScene() == nullptr)
    {
    vtkWarningMacro("vtkMRMLColorLogic::AddDefaultColorNodes: no scene to which to add nodes\n");
    return;
    }

  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // add the labels first
  this->AddLabelsNode();

  // add the rest of the default color table nodes
  this->AddDefaultTableNodes();

  // add default procedural nodes, including a random one
  this->AddDefaultProceduralNodes();

  // add the PET nodes
  this->AddPETNodes();

  // add the dGEMRIC nodes
  this->AddDGEMRICNodes();

  // file based labels
  // first check for any new ones

  // load the one from the default resources directory
  this->AddDefaultFileNodes();

  // now add ones in files that the user pointed to, these ones are not hidden
  // from the editors
  this->AddUserFileNodes();

  vtkDebugMacro("Done adding default color nodes");
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::RemoveDefaultColorNodes()
{
  // try to find any of the default colour nodes that are still in the scene
  if (this->GetMRMLScene() == nullptr)
    {
    // nothing can do, it's gone
    return;
    }

  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  vtkMRMLColorTableNode *node;
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't have a File node...
    if (i != vtkMRMLColorTableNode::File
        && i != vtkMRMLColorTableNode::Obsolete)
      {
      //std::string id = std::string(this->GetColorTableNodeID(i));
      const char* id = this->GetColorTableNodeID(i);
      vtkDebugMacro("vtkMRMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
      node = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
      if (node != nullptr)
        {
        this->GetMRMLScene()->RemoveNode(node);
        }
      }
    }
  basicNode->Delete();

   // remove the procedural color nodes (after the fs proc nodes as
   // getting them by class)
  std::vector<vtkMRMLNode *> procNodes;
  int numProcNodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLProceduralColorNode", procNodes);
  for (int i = 0; i < numProcNodes; i++)
    {
    vtkMRMLProceduralColorNode* procNode = vtkMRMLProceduralColorNode::SafeDownCast(procNodes[i]);
    if (procNode != nullptr &&
        strcmp(procNode->GetID(), this->GetProceduralColorNodeID(procNode->GetName())) == 0)
      {
      // it's one we added
      this->GetMRMLScene()->RemoveNode(procNode);
      }
    }

  // remove the PET nodes
  vtkMRMLPETProceduralColorNode *basicPETNode = vtkMRMLPETProceduralColorNode::New();
  vtkMRMLPETProceduralColorNode *PETnode;
  for (int i = basicPETNode->GetFirstType(); i <= basicPETNode->GetLastType(); i++)
    {
    basicPETNode->SetType(i);
    const char* id = this->GetPETColorNodeID(i);
    vtkDebugMacro("vtkMRMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
    PETnode =  vtkMRMLPETProceduralColorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
    if (PETnode != nullptr)
      {
      this->GetMRMLScene()->RemoveNode(PETnode);
      }
    }
  basicPETNode->Delete();

  // remove the dGEMRIC nodes
  vtkMRMLdGEMRICProceduralColorNode *basicdGEMRICNode = vtkMRMLdGEMRICProceduralColorNode::New();
  vtkMRMLdGEMRICProceduralColorNode *dGEMRICnode;
  for (int i = basicdGEMRICNode->GetFirstType(); i <= basicdGEMRICNode->GetLastType(); i++)
    {
    basicdGEMRICNode->SetType(i);
    const char* id = this->GetdGEMRICColorNodeID(i);
    vtkDebugMacro("vtkMRMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
    dGEMRICnode =  vtkMRMLdGEMRICProceduralColorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
    if (dGEMRICnode != nullptr)
      {
      this->GetMRMLScene()->RemoveNode(dGEMRICnode);
      }
    }
  basicdGEMRICNode->Delete();

  // remove the file based labels node
  for (unsigned int i = 0; i < this->ColorFiles.size(); i++)
    {
    node =  vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetFileColorNodeID(this->ColorFiles[i].c_str())));
    if (node != nullptr)
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
    }
  for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    node =  vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetFileColorNodeID(this->UserColorFiles[i].c_str())));
    if (node != nullptr)
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
    }
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetColorTableNodeID(int type)
{
  vtkNew<vtkMRMLColorTableNode> basicNode;
  basicNode->SetType(type);
  return vtkMRMLColorLogic::GetColorNodeID(basicNode.GetPointer());
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetPETColorNodeID (int type )
{
  vtkNew<vtkMRMLPETProceduralColorNode> basicNode;
  basicNode->SetType(type);
  return vtkMRMLColorLogic::GetColorNodeID(basicNode.GetPointer());
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetdGEMRICColorNodeID(int type)
{
  vtkNew<vtkMRMLdGEMRICProceduralColorNode> basicNode;
  basicNode->SetType(type);
  return vtkMRMLColorLogic::GetColorNodeID(basicNode.GetPointer());
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetColorNodeID(vtkMRMLColorNode* colorNode)
{
  assert(colorNode);
  std::string id = std::string(colorNode->GetClassName()) +
                   std::string(colorNode->GetTypeAsString());
  vtkMRMLColorLogic::TempColorNodeID = id;
  return vtkMRMLColorLogic::TempColorNodeID.c_str();
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetProceduralColorNodeID(const char *name)
{
  std::string id = std::string("vtkMRMLProceduralColorNode") + std::string(name);
  vtkMRMLColorLogic::TempColorNodeID = id;
  return vtkMRMLColorLogic::TempColorNodeID.c_str();
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetFileColorNodeSingletonTag(const char * fileName)
{
  std::string singleton = std::string("File") +
    vtksys::SystemTools::GetFilenameName(fileName);
  return singleton;
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetFileColorNodeID(const char * fileName)
{
  std::string id = std::string("vtkMRMLColorTableNode") +
                   vtkMRMLColorLogic::GetFileColorNodeSingletonTag(fileName);
  vtkMRMLColorLogic::TempColorNodeID = id;
  return vtkMRMLColorLogic::TempColorNodeID.c_str();
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultVolumeColorNodeID()
{
  return vtkMRMLColorLogic::GetColorTableNodeID(vtkMRMLColorTableNode::Grey);
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultLabelMapColorNodeID()
{
  return vtkMRMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultEditorColorNodeID()
{
  return vtkMRMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultModelColorNodeID()
{
  return vtkMRMLColorLogic::GetProceduralColorNodeID("RedGreenBlue");
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultChartColorNodeID()
{
  return vtkMRMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultPlotColorNodeID()
{
  return vtkMRMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::AddColorFile(const char *fileName, std::vector<std::string> *Files)
{
  if (fileName == nullptr)
    {
    vtkErrorMacro("AddColorFile: can't add a null color file name");
    return;
    }
  if (Files == nullptr)
    {
    vtkErrorMacro("AddColorFile: no array to which to add color file to!");
    return;
    }
  // check if it's in the vector already
  std::string fileNameStr = std::string(fileName);
  for (unsigned int i = 0; i <  Files->size(); i++)
    {
    std::string fileToCheck;
    try
      {
      fileToCheck = Files->at(i);
      }
    catch (...)
      {
      // an out_of_range exception can be thrown.
      }
    if (fileToCheck.compare(fileNameStr) == 0)
      {
      vtkDebugMacro("AddColorFile: already have this file at index " << i << ", not adding it again: " << fileNameStr.c_str());
      return;
      }
    }
  vtkDebugMacro("AddColorFile: adding file name to Files: " << fileNameStr.c_str());
  Files->push_back(fileNameStr);
}

//----------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLColorLogic::LoadColorFile(const char *fileName, const char *nodeName)
{
  // try loading it as a color table node first
  vtkMRMLColorTableNode* node = this->CreateFileNode(fileName);
  vtkMRMLColorNode * addedNode = nullptr;

  if (node)
    {
    node->SetAttribute("Category", "File");
    node->SaveWithSceneOn();
    node->GetStorageNode()->SaveWithSceneOn();
    node->HideFromEditorsOff();
    node->SetSingletonTag(nullptr);

    if (nodeName != nullptr)
      {
      std::string uname( this->GetMRMLScene()->GetUniqueNameByString(nodeName));
      node->SetName(uname.c_str());
      }
    addedNode =
      vtkMRMLColorNode::SafeDownCast(this->GetMRMLScene()->AddNode(node));
    vtkDebugMacro("LoadColorFile: Done: Read and added file node: " <<  fileName);
    node->Delete();
    }
  else
    {
    // try loading it as a procedural node
    vtkWarningMacro("Trying to read color file as a procedural color node");
    vtkMRMLProceduralColorNode *procNode = this->CreateProceduralFileNode(fileName);
    if (procNode)
      {
      procNode->SetAttribute("Category", "File");
      procNode->SaveWithSceneOn();
      procNode->GetStorageNode()->SaveWithSceneOn();
      procNode->HideFromEditorsOff();
      procNode->SetSingletonTag(nullptr);

      if (nodeName != nullptr)
        {
        std::string uname( this->GetMRMLScene()->GetUniqueNameByString(nodeName));
        procNode->SetName(uname.c_str());
        }
      addedNode =
        vtkMRMLColorNode::SafeDownCast(this->GetMRMLScene()->AddNode(procNode));
      vtkDebugMacro("LoadColorFile: Done: Read and added file procNode: " <<  fileName);
      procNode->Delete();
      }
    }
  return addedNode;
}

//------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CreateLabelsNode()
{
  vtkMRMLColorTableNode *labelsNode = vtkMRMLColorTableNode::New();
  labelsNode->SetTypeToLabels();
  labelsNode->SetAttribute("Category", "Discrete");
  labelsNode->SaveWithSceneOff();
  labelsNode->SetName(labelsNode->GetTypeAsString());
  labelsNode->SetSingletonTag(labelsNode->GetTypeAsString());
  return labelsNode;
}

//------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CreateDefaultTableNode(int type)
{
  vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::New();
  node->SetType(type);
  const char* typeName = node->GetTypeAsString();
  if (strstr(typeName, "Tint") != nullptr)
    {
    node->SetAttribute("Category", "Tint");
    }
  else if (strstr(typeName, "Shade") != nullptr)
    {
    node->SetAttribute("Category", "Shade");
    }
  else
    {
    node->SetAttribute("Category", "Discrete");
    }
  if (strcmp(typeName, "(unknown)") == 0)
    {
    return node;
    }
  node->SaveWithSceneOff();
  node->SetName(node->GetTypeAsString());
  node->SetSingletonTag(node->GetTypeAsString());
  return node;
}

//------------------------------------------------------------------------------
vtkMRMLProceduralColorNode* vtkMRMLColorLogic::CreateRandomNode()
{
  vtkDebugMacro("vtkMRMLColorLogic::CreateRandomNode: making a random  mrml proc color node");
  vtkMRMLProceduralColorNode *procNode = vtkMRMLProceduralColorNode::New();
  procNode->SetName("RandomIntegers");
  procNode->SetAttribute("Category", "Discrete");
  procNode->SaveWithSceneOff();
  procNode->SetSingletonTag(procNode->GetTypeAsString());

  std::default_random_engine randomGenerator(std::random_device{}());

  vtkColorTransferFunction *func = procNode->GetColorTransferFunction();
  const int dimension = 1000;
  double table[3*dimension];
  double* tablePtr = table;
  for (int i = 0; i < dimension; ++i)
    {
    *tablePtr++ = static_cast<double>(randomGenerator()) / randomGenerator.max();
    *tablePtr++ = static_cast<double>(randomGenerator()) / randomGenerator.max();
    *tablePtr++ = static_cast<double>(randomGenerator()) / randomGenerator.max();
    }
  func->BuildFunctionFromTable(VTK_INT_MIN, VTK_INT_MAX, dimension, table);
  func->Build();
  procNode->SetNamesFromColors();

  return procNode;
}

//------------------------------------------------------------------------------
vtkMRMLProceduralColorNode* vtkMRMLColorLogic::CreateRedGreenBlueNode()
{
  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: making a red - green - blue mrml proc color node");
  vtkMRMLProceduralColorNode *procNode = vtkMRMLProceduralColorNode::New();
  procNode->SetName("RedGreenBlue");
  procNode->SetAttribute("Category", "Continuous");
  procNode->SaveWithSceneOff();
  procNode->SetSingletonTag(procNode->GetTypeAsString());
  procNode->SetDescription("A color transfer function that maps from -6 to 6, red through green to blue");
  vtkColorTransferFunction *func = procNode->GetColorTransferFunction();
  func->SetColorSpaceToRGB();
  func->AddRGBPoint(-6.0, 1.0, 0.0, 0.0);
  func->AddRGBPoint(0.0, 0.0, 1.0, 0.0);
  func->AddRGBPoint(6.0, 0.0, 0.0, 1.0);

  procNode->SetNamesFromColors();

  return procNode;
}

//--------------------------------------------------------------------------------
vtkMRMLPETProceduralColorNode* vtkMRMLColorLogic::CreatePETColorNode(int type)
{
  vtkMRMLPETProceduralColorNode *nodepcn = vtkMRMLPETProceduralColorNode::New();
  nodepcn->SetType(type);
  nodepcn->SetAttribute("Category", "PET");
  nodepcn->SaveWithSceneOff();

  if (nodepcn->GetTypeAsString() == nullptr)
    {
    vtkWarningMacro("Node type as string is null");
    nodepcn->SetName("NoName");
    }
  else
    {
    vtkDebugMacro("Got node type as string " << nodepcn->GetTypeAsString());
    nodepcn->SetName(nodepcn->GetTypeAsString());
    }

  nodepcn->SetSingletonTag(nodepcn->GetTypeAsString());

  return nodepcn;
}

//---------------------------------------------------------------------------------
vtkMRMLdGEMRICProceduralColorNode* vtkMRMLColorLogic::CreatedGEMRICColorNode(int type)
{
  vtkMRMLdGEMRICProceduralColorNode *pcnode = vtkMRMLdGEMRICProceduralColorNode::New();
  pcnode->SetType(type);
  pcnode->SetAttribute("Category", "Cartilage MRI");
  pcnode->SaveWithSceneOff();
  if (pcnode->GetTypeAsString() == nullptr)
    {
    vtkWarningMacro("Node type as string is null");
    pcnode->SetName("NoName");
    }
  else
    {
    vtkDebugMacro("Got node type as string " << pcnode->GetTypeAsString());
    pcnode->SetName(pcnode->GetTypeAsString());
    }

  pcnode->SetSingletonTag(pcnode->GetTypeAsString());

  return pcnode;
}

//---------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CreateDefaultFileNode(const std::string& colorFileName)
{
  vtkMRMLColorTableNode* ctnode = this->CreateFileNode(colorFileName.c_str());

  if (!ctnode)
    {
    return nullptr;
    }

  if (strcmp(ctnode->GetName(),"GenericColors") == 0 ||
      strcmp(ctnode->GetName(),"GenericAnatomyColors") == 0)
    {
    vtkDebugMacro("Found default lut node");
    // No category to float to the top of the node
    // can't unset an attribute, so just don't set it at all
    }
  else
    {
    ctnode->SetAttribute("Category", "Default Labels from File");
    }

  return ctnode;
}

//---------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CreateUserFileNode(const std::string& colorFileName)
{
  vtkMRMLColorTableNode * ctnode = this->CreateFileNode(colorFileName.c_str());
  if (ctnode == nullptr)
    {
    return nullptr;
    }
  ctnode->SetAttribute("Category", "Auto Loaded User Color Files");
  ctnode->SaveWithSceneOn();
  ctnode->HideFromEditorsOff();

  return ctnode;
}

//--------------------------------------------------------------------------------
std::vector<std::string> vtkMRMLColorLogic::FindDefaultColorFiles()
{
  return std::vector<std::string>();
}

//--------------------------------------------------------------------------------
std::vector<std::string> vtkMRMLColorLogic::FindUserColorFiles()
{
  return std::vector<std::string>();
}

//--------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CreateFileNode(const char* fileName)
{
  vtkMRMLColorTableNode * ctnode =  vtkMRMLColorTableNode::New();
  ctnode->SetTypeToFile();
  ctnode->SaveWithSceneOff();
  ctnode->HideFromEditorsOn();
  ctnode->SetScene(this->GetMRMLScene());

  // make a storage node
  vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
  colorStorageNode->SaveWithSceneOff();
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->AddNode(colorStorageNode.GetPointer());
    ctnode->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    }

  ctnode->GetStorageNode()->SetFileName(fileName);
  std::string basename = ctnode->GetStorageNode()->GetFileNameWithoutExtension(fileName);
  if (this->GetMRMLScene())
    {
    std::string uname(this->GetMRMLScene()->GetUniqueNameByString(basename.c_str()));
    ctnode->SetName(uname.c_str());
    }
  else
    {
    ctnode->SetName(basename.c_str());
    }
  vtkDebugMacro("CreateFileNode: About to read user file " << fileName);

  if (ctnode->GetStorageNode()->ReadData(ctnode) == 0)
    {
    vtkErrorMacro("Unable to read file as color table " << (ctnode->GetFileName() ? ctnode->GetFileName() : ""));

    if (this->GetMRMLScene())
      {
      ctnode->SetAndObserveStorageNodeID(nullptr);
      ctnode->SetScene(nullptr);
      this->GetMRMLScene()->RemoveNode(colorStorageNode.GetPointer());
      }

      ctnode->Delete();
      return nullptr;
    }
  vtkDebugMacro("CreateFileNode: finished reading user file " << fileName);
  ctnode->SetSingletonTag(
    this->GetFileColorNodeSingletonTag(fileName).c_str());

  return ctnode;
}

//--------------------------------------------------------------------------------
vtkMRMLProceduralColorNode* vtkMRMLColorLogic::CreateProceduralFileNode(const char* fileName)
{
  vtkMRMLProceduralColorNode * cpnode =  vtkMRMLProceduralColorNode::New();
  cpnode->SetTypeToFile();
  cpnode->SaveWithSceneOff();
  cpnode->HideFromEditorsOn();
  cpnode->SetScene(this->GetMRMLScene());

  // make a storage node
  vtkMRMLProceduralColorStorageNode *colorStorageNode = vtkMRMLProceduralColorStorageNode::New();
  colorStorageNode->SaveWithSceneOff();
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->AddNode(colorStorageNode);
    cpnode->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    }
  colorStorageNode->Delete();

  cpnode->GetStorageNode()->SetFileName(fileName);
  std::string basename = cpnode->GetStorageNode()->GetFileNameWithoutExtension(fileName);
  if (this->GetMRMLScene())
    {
    std::string uname(this->GetMRMLScene()->GetUniqueNameByString(basename.c_str()));
    cpnode->SetName(uname.c_str());
    }
  else
    {
    cpnode->SetName(basename.c_str());
    }

  vtkDebugMacro("CreateProceduralFileNode: About to read user file " << fileName);

  if (cpnode->GetStorageNode()->ReadData(cpnode) == 0)
    {
    vtkErrorMacro("Unable to read procedural colour file " << (cpnode->GetFileName() ? cpnode->GetFileName() : ""));
    if (this->GetMRMLScene())
      {
      cpnode->SetAndObserveStorageNodeID(nullptr);
      cpnode->SetScene(nullptr);
      this->GetMRMLScene()->RemoveNode(colorStorageNode);
      }
      cpnode->Delete();
      return nullptr;
    }
  vtkDebugMacro("CreateProceduralFileNode: finished reading user procedural color file " << fileName);
  cpnode->SetSingletonTag(
    this->GetFileColorNodeSingletonTag(fileName).c_str());

  return cpnode;
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddLabelsNode()
{
  vtkMRMLColorTableNode* labelsNode = this->CreateLabelsNode();
  //if (this->GetMRMLScene()->GetNodeByID(labelsNode->GetSingletonTag()) == nullptr)
    {
    //this->GetMRMLScene()->RequestNodeID(labelsNode, labelsNode->GetSingletonTag());
    this->GetMRMLScene()->AddNode(labelsNode);
    }
  labelsNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultTableNode(int i)
{
  vtkMRMLColorTableNode* node = this->CreateDefaultTableNode(i);
  //if (node->GetSingletonTag())
    {
    //if (this->GetMRMLScene()->GetNodeByID(node->GetSingletonTag()) == nullptr)
      {
      vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: requesting id " << node->GetSingletonTag() << endl);
      //this->GetMRMLScene()->RequestNodeID(node, node->GetSingletonTag());
      this->GetMRMLScene()->AddNode(node);
      vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << node->GetSingletonTag() << ", type = " << node->GetTypeAsString() << endl);
      }
    //else
    //  {
    //  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
    //  }
    }
  node->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultProceduralNodes()
{
  // random one
  vtkMRMLProceduralColorNode* randomNode = this->CreateRandomNode();
  this->GetMRMLScene()->AddNode(randomNode);
  randomNode->Delete();

  // red green blue one
  vtkMRMLProceduralColorNode* rgbNode = this->CreateRedGreenBlueNode();
  this->GetMRMLScene()->AddNode(rgbNode);
  rgbNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddPETNode(int type)
{
  vtkDebugMacro("AddDefaultColorNodes: adding PET nodes");
  vtkMRMLPETProceduralColorNode *nodepcn = this->CreatePETColorNode(type);
  //if (this->GetMRMLScene()->GetNodeByID( nodepcn->GetSingletonTag() ) == nullptr)
    {
    //this->GetMRMLScene()->RequestNodeID(nodepcn, nodepcn->GetSingletonTag() );
    this->GetMRMLScene()->AddNode(nodepcn);
    }
  nodepcn->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDGEMRICNode(int type)
{
  vtkDebugMacro("AddDefaultColorNodes: adding dGEMRIC nodes");
  vtkMRMLdGEMRICProceduralColorNode *pcnode = this->CreatedGEMRICColorNode(type);
  //if (this->GetMRMLScene()->GetNodeByID(pcnode->GetSingletonTag()) == nullptr)
    {
    //this->GetMRMLScene()->RequestNodeID(pcnode, pcnode->GetSingletonTag());
    this->GetMRMLScene()->AddNode(pcnode);
    }
  pcnode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultFileNode(int i)
{
  vtkMRMLColorTableNode* ctnode =  this->CreateDefaultFileNode(this->ColorFiles[i]);
  if (ctnode)
    {
    //if (this->GetMRMLScene()->GetNodeByID(ctnode->GetSingletonTag()) == nullptr)
      {
        //this->GetMRMLScene()->RequestNodeID(ctnode, ctnode->GetSingletonTag());
        this->GetMRMLScene()->AddNode(ctnode);
        ctnode->Delete();
        vtkDebugMacro("AddDefaultColorFiles: Read and added file node: " <<  this->ColorFiles[i].c_str());
      }
    //else
    //  {
    //  vtkDebugMacro("AddDefaultColorFiles: node " << ctnode->GetSingletonTag() << " already in scene");
    //  }
    }
  else
    {
    vtkWarningMacro("Unable to read color file " << this->ColorFiles[i].c_str());
    }
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddUserFileNode(int i)
{
  vtkMRMLColorTableNode* ctnode = this->CreateUserFileNode(this->UserColorFiles[i]);
  if (ctnode)
    {
    //if (this->GetMRMLScene()->GetNodeByID(ctnode->GetSingletonTag()) == nullptr)
      {
      //this->GetMRMLScene()->RequestNodeID(ctnode, ctnode->GetSingletonTag());
      this->GetMRMLScene()->AddNode(ctnode);
      vtkDebugMacro("AddDefaultColorFiles: Read and added user file node: " <<  this->UserColorFiles[i].c_str());
      }
    //else
    //  {
    //  vtkDebugMacro("AddDefaultColorFiles: node " << ctnode->GetSingletonTag() << " already in scene");
    //  }
    }
  else
    {
    vtkWarningMacro("Unable to read user color file " << this->UserColorFiles[i].c_str());
    }
  ctnode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultTableNodes()
{
  vtkMRMLColorTableNode* basicNode = vtkMRMLColorTableNode::New();
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't add a second Labels node, File node or the old atlas node
    if (i != vtkMRMLColorTableNode::Labels &&
        i != vtkMRMLColorTableNode::File &&
        i != vtkMRMLColorTableNode::Obsolete &&
        i != vtkMRMLColorTableNode::User)
      {
      this->AddDefaultTableNode(i);
      }
    }
  basicNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddPETNodes()
{
  vtkMRMLPETProceduralColorNode* basicPETNode = vtkMRMLPETProceduralColorNode::New();
  for (int type = basicPETNode->GetFirstType(); type <= basicPETNode->GetLastType(); ++type)
    {
    this->AddPETNode(type);
    }
  basicPETNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDGEMRICNodes()
{
  vtkMRMLdGEMRICProceduralColorNode* basicdGEMRICNode = vtkMRMLdGEMRICProceduralColorNode::New();
  for (int type = basicdGEMRICNode->GetFirstType(); type <= basicdGEMRICNode->GetLastType(); ++type)
    {
    this->AddDGEMRICNode(type);
    }
  basicdGEMRICNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultFileNodes()
{
  this->ColorFiles = this->FindDefaultColorFiles();
  vtkDebugMacro("AddDefaultColorNodes: found " <<  this->ColorFiles.size() << " default color files");
  for (unsigned int i = 0; i < this->ColorFiles.size(); i++)
    {
    this->AddDefaultFileNode(i);
    }
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddUserFileNodes()
{
  this->UserColorFiles = this->FindUserColorFiles();
  vtkDebugMacro("AddDefaultColorNodes: found " <<  this->UserColorFiles.size() << " user color files");
  for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    this->AddUserFileNode(i);
    }

}

//----------------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CopyNode(vtkMRMLColorNode* nodeToCopy, const char* copyName)
{
  vtkMRMLColorTableNode *colorNode = vtkMRMLColorTableNode::New();
  colorNode->SetName(copyName);
  colorNode->SetTypeToUser();
  colorNode->SetAttribute("Category", "User Generated");
  colorNode->SetHideFromEditors(false);
  colorNode->SetNamesInitialised(nodeToCopy->GetNamesInitialised());
  if (nodeToCopy->GetLookupTable())
    {
    double* range = nodeToCopy->GetLookupTable()->GetRange();
    colorNode->GetLookupTable()->SetRange(range[0], range[1]);
    }
  colorNode->SetNumberOfColors(nodeToCopy->GetNumberOfColors());
  for (int i = 0; i < nodeToCopy->GetNumberOfColors(); ++i)
    {
    double color[4];
    nodeToCopy->GetColor(i, color);
    colorNode->SetColor(i, nodeToCopy->GetColorName(i), color[0], color[1], color[2], color[3]);
    }
  return colorNode;
}

//----------------------------------------------------------------------------------------
vtkMRMLProceduralColorNode* vtkMRMLColorLogic::CopyProceduralNode(vtkMRMLColorNode* nodeToCopy, const char* copyName)
{
  vtkMRMLProceduralColorNode *colorNode = vtkMRMLProceduralColorNode::New();
  if (nodeToCopy->IsA("vtkMRMLProceduralColorNode"))
    {
    colorNode->Copy(nodeToCopy);
    // the copy will copy any singleton tag, make sure it's unset
    colorNode->SetSingletonTag(nullptr);
    }

  colorNode->SetName(copyName);
  colorNode->SetTypeToUser();
  colorNode->SetAttribute("Category", "User Generated");
  colorNode->SetHideFromEditors(false);

  return colorNode;
}
