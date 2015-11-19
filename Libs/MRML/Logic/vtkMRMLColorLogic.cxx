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
#include "vtkMRMLFreeSurferProceduralColorNode.h"
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
#include <sstream>

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::TempColorNodeID;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLColorLogic);

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::DEFAULT_TERMINOLOGY_NAME = "GenericAnatomyColors";

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::StandardTerm::Print(std::ostream& os)
{
  vtkIndent indent;
  this->PrintSelf(os, indent.GetNextIndent());
}

//----------------------------------------------------------------------------
std::ostream& vtkMRMLColorLogic::StandardTerm::operator<<(std::ostream& os)
{
  this->Print(os);
  return os;
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::StandardTerm::PrintSelf(std::ostream &os, vtkIndent indent)
{
  os << indent << "Code value: " << CodeValue.c_str() << std::endl
     << indent << "Code scheme designator: " << CodingSchemeDesignator.c_str() << std::endl
     << indent << "Code meaning: " << CodeMeaning.c_str()
     << std::endl;
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::ColorLabelCategorization::Print(std::ostream& os)
{
  vtkIndent indent;
  this->PrintSelf(os, indent.GetNextIndent());
}

//----------------------------------------------------------------------------
std::ostream& vtkMRMLColorLogic::ColorLabelCategorization::operator<<(std::ostream& os)
{
  this->Print(os);
  return os;
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::ColorLabelCategorization::PrintSelf(ostream &os, vtkIndent indent)
{
  os << "Label: " << LabelValue << std::endl;
  os << "Segmented property category:\n";
  SegmentedPropertyCategory.PrintSelf(os, indent);
  os << "Segmented property type:\n";
  SegmentedPropertyType.PrintSelf(os, indent);
  os << "Segmented property type modifier:\n";
  SegmentedPropertyTypeModifier.PrintSelf(os, indent);
  os << "Anatomic region:\n";
  AnatomicRegion.PrintSelf(os, indent);
  os << "Antatomic region modifier:\n";
  AnatomicRegionModifier.PrintSelf(os, indent);
  os << std::endl;
}

//----------------------------------------------------------------------------
vtkMRMLColorLogic::vtkMRMLColorLogic()
{
  this->UserColorFilePaths = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLColorLogic::~vtkMRMLColorLogic()
{
  // remove the default color nodes
  this->RemoveDefaultColorNodes();

  // clear out the lists of files
  this->ColorFiles.clear();
  this->TerminologyColorFiles.clear();
  this->UserColorFiles.clear();

  if (this->UserColorFilePaths)
    {
    delete [] this->UserColorFilePaths;
    this->UserColorFilePaths = NULL;
    }
}

//------------------------------------------------------------------------------
void vtkMRMLColorLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  // We are solely interested in vtkMRMLScene::NewSceneEvent,
  // we don't want to listen to any other events.
  vtkIntArray* sceneEvents = vtkIntArray::New();
  sceneEvents->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
  sceneEvents->Delete();

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

  os << indent << "Terminology Color Files:\n";
  for (size_t i = 0; i < this->TerminologyColorFiles.size(); i++)
    {
    os << indent.GetNextIndent() << i << " " << this->TerminologyColorFiles[i].c_str() << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultColorNodes()
{
  // create the default color nodes, they don't get saved with the scenes as
  // they'll be created on start up, and when a new
  // scene is opened
  if (this->GetMRMLScene() == NULL)
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

  // add freesurfer nodes
  this->AddFreeSurferNodes();

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

  // now load in the terminology color files and associate them with the
  // color nodes
  this->AddDefaultTerminologyColors();

  vtkDebugMacro("Done adding default color nodes");
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::RemoveDefaultColorNodes()
{
  // try to find any of the default colour nodes that are still in the scene
  if (this->GetMRMLScene() == NULL)
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
      if (node != NULL)
        {
        this->GetMRMLScene()->RemoveNode(node);
        }
      }
    }
  basicNode->Delete();

  // remove freesurfer nodes
  vtkMRMLFreeSurferProceduralColorNode *basicFSNode = vtkMRMLFreeSurferProceduralColorNode::New();
  vtkMRMLFreeSurferProceduralColorNode *fsnode;
  for (int i = basicFSNode->GetFirstType(); i <= basicFSNode->GetLastType(); i++)
    {
    basicFSNode->SetType(i);
    const char* id = this->GetFreeSurferColorNodeID(i);
    vtkDebugMacro("vtkMRMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
    fsnode =  vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
    if (fsnode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(fsnode);
      }
    }
  basicFSNode->Delete();

   // remove the procedural color nodes (after the fs proc nodes as
   // getting them by class)
  std::vector<vtkMRMLNode *> procNodes;
  int numProcNodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLProceduralColorNode", procNodes);
  for (int i = 0; i < numProcNodes; i++)
    {
    vtkMRMLProceduralColorNode* procNode = vtkMRMLProceduralColorNode::SafeDownCast(procNodes[i]);
    if (procNode != NULL &&
        strcmp(procNode->GetID(), this->GetProceduralColorNodeID(procNode->GetName())) == 0)
      {
      // it's one we added
      this->GetMRMLScene()->RemoveNode(procNode);
      }
    }

  // remove the fs lookup table node
  node = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetDefaultFreeSurferLabelMapColorNodeID()));
  if (node != NULL)
    {
    this->GetMRMLScene()->RemoveNode(node);
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
    if (PETnode != NULL)
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
    if (dGEMRICnode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(dGEMRICnode);
      }
    }
  basicdGEMRICNode->Delete();

  // remove the file based labels node
  for (unsigned int i = 0; i < this->ColorFiles.size(); i++)
    {
    node =  vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetFileColorNodeID(this->ColorFiles[i].c_str())));
    if (node != NULL)
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
    }
  for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    node =  vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetFileColorNodeID(this->UserColorFiles[i].c_str())));
    if (node != NULL)
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
const char * vtkMRMLColorLogic::GetFreeSurferColorNodeID(int type)
{
  vtkNew<vtkMRMLFreeSurferProceduralColorNode> basicNode;
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
  return vtkMRMLColorLogic::GetFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::Heat);
}

//----------------------------------------------------------------------------
const char *vtkMRMLColorLogic::GetDefaultChartColorNodeID()
{
  return vtkMRMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetDefaultFreeSurferLabelMapColorNodeID()
{
  return vtkMRMLColorLogic::GetFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::Labels);
}

//----------------------------------------------------------------------------
void vtkMRMLColorLogic::AddColorFile(const char *fileName, std::vector<std::string> *Files)
{
  if (fileName == NULL)
    {
    vtkErrorMacro("AddColorFile: can't add a null color file name");
    return;
    }
  if (Files == NULL)
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
  vtkMRMLColorNode * addedNode = NULL;

  if (node)
    {
    node->SetAttribute("Category", "File");
    node->SaveWithSceneOn();
    node->GetStorageNode()->SaveWithSceneOn();
    node->HideFromEditorsOff();
    node->SetSingletonTag(NULL);

    if (nodeName != NULL)
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
      procNode->SetSingletonTag(NULL);

      if (nodeName != NULL)
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
  if (strstr(typeName, "Tint") != NULL)
    {
    node->SetAttribute("Category", "Tint");
    }
  else if (strstr(typeName, "Shade") != NULL)
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

  vtkColorTransferFunction *func = procNode->GetColorTransferFunction();
  const int dimension = 1000;
  double table[3*dimension];
  double* tablePtr = table;
  for (int i = 0; i < dimension; ++i)
    {
    *tablePtr++ = static_cast<double>(rand())/RAND_MAX;
    *tablePtr++ = static_cast<double>(rand())/RAND_MAX;
    *tablePtr++ = static_cast<double>(rand())/RAND_MAX;
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

//------------------------------------------------------------------------------
vtkMRMLFreeSurferProceduralColorNode* vtkMRMLColorLogic::CreateFreeSurferNode(int type)
{
  vtkMRMLFreeSurferProceduralColorNode *node = vtkMRMLFreeSurferProceduralColorNode::New();
  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: setting free surfer proc color node type to " << type);
  node->SetType(type);
  node->SetAttribute("Category", "FreeSurfer");
  node->SaveWithSceneOff();
  if (node->GetTypeAsString() == NULL)
    {
    vtkWarningMacro("Node type as string is null");
    node->SetName("NoName");
    }
  else
    {
    node->SetName(node->GetTypeAsString());
    }
  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: set proc node name to " << node->GetName());
  /*
  if (this->GetFreeSurferColorNodeID(i) == NULL)
    {
    vtkDebugMacro("Error getting default node id for freesurfer node " << i);
    }
  */
  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: Getting default fs color node id for type " << type);
  node->SetSingletonTag(node->GetTypeAsString());
  return node;
}

//------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorLogic::CreateFreeSurferFileNode(const char* fileName)
{
  if (fileName == NULL)
    {
    vtkErrorMacro("Unable to get the labels file name, not adding");
    return 0;
    }

  vtkMRMLColorTableNode* node = this->CreateFileNode(fileName);

  if (!node)
    {
    return 0;
    }
  node->SetAttribute("Category", "FreeSurfer");
  node->SetName("FreeSurferLabels");
  node->SetSingletonTag(node->GetTypeAsString());

  return node;
}

//--------------------------------------------------------------------------------
vtkMRMLPETProceduralColorNode* vtkMRMLColorLogic::CreatePETColorNode(int type)
{
  vtkMRMLPETProceduralColorNode *nodepcn = vtkMRMLPETProceduralColorNode::New();
  nodepcn->SetType(type);
  nodepcn->SetAttribute("Category", "PET");
  nodepcn->SaveWithSceneOff();

  if (nodepcn->GetTypeAsString() == NULL)
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
  if (pcnode->GetTypeAsString() == NULL)
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
    return 0;
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
  if (ctnode == 0)
    {
    return 0;
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
std::vector<std::string> vtkMRMLColorLogic::FindDefaultTerminologyColorFiles()
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
  std::string basename = vtksys::SystemTools::GetFilenameWithoutExtension(fileName);
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(basename.c_str()));
  ctnode->SetName(uname.c_str());

  vtkDebugMacro("CreateFileNode: About to read user file " << fileName);

  if (ctnode->GetStorageNode()->ReadData(ctnode) == 0)
    {
    vtkErrorMacro("Unable to read file as color table " << (ctnode->GetFileName() ? ctnode->GetFileName() : ""));

    if (this->GetMRMLScene())
      {
      ctnode->SetAndObserveStorageNodeID(NULL);
      ctnode->SetScene(NULL);
      this->GetMRMLScene()->RemoveNode(colorStorageNode.GetPointer());
      }

      ctnode->Delete();
      return 0;
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
  std::string basename = vtksys::SystemTools::GetFilenameWithoutExtension(fileName);
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(basename.c_str()));
  cpnode->SetName(uname.c_str());

  vtkDebugMacro("CreateProceduralFileNode: About to read user file " << fileName);

  if (cpnode->GetStorageNode()->ReadData(cpnode) == 0)
    {
    vtkErrorMacro("Unable to read procedural colour file " << (cpnode->GetFileName() ? cpnode->GetFileName() : ""));
    if (this->GetMRMLScene())
      {
      cpnode->SetAndObserveStorageNodeID(NULL);
      cpnode->SetScene(NULL);
      this->GetMRMLScene()->RemoveNode(colorStorageNode);
      }
      cpnode->Delete();
      return 0;
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
  //if (this->GetMRMLScene()->GetNodeByID(labelsNode->GetSingletonTag()) == NULL)
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
    //if (this->GetMRMLScene()->GetNodeByID(node->GetSingletonTag()) == NULL)
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
void vtkMRMLColorLogic::AddFreeSurferNode(int type)
{
  vtkMRMLFreeSurferProceduralColorNode* node = this->CreateFreeSurferNode(type);
  //if (this->GetMRMLScene()->GetNodeByID(node->GetSingletonTag()) == NULL)
    {
    //this->GetMRMLScene()->RequestNodeID(node, node->GetSingletonTag());
    this->GetMRMLScene()->AddNode(node);
    vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << node->GetSingletonTag()<< ", type = " << node->GetTypeAsString() << endl);
    }
  //else
  //  {
  //  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
  //  }
  node->Delete();
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddFreeSurferFileNode(vtkMRMLFreeSurferProceduralColorNode* basicFSNode)
{
  vtkMRMLColorTableNode* node = this->CreateFreeSurferFileNode(basicFSNode->GetLabelsFileName());
  if (node)
    {
    //if (this->GetMRMLScene()->GetNodeByID(node->GetSingletonTag()) == NULL)
      {
      //this->GetMRMLScene()->RequestNodeID(node, node->GetSingletonTag());
      this->GetMRMLScene()->AddNode(node);
      }
    //else
    //  {
    //  vtkDebugMacro("Unable to add a new colour node " << node->GetSingletonTag()
    //                << " with freesurfer colours, from file: "
    //                << node->GetStorageNode()->GetFileName()
    //                << " as there is already a node with this id in the scene");
    //  }
    node->Delete();
    }
}

//----------------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddPETNode(int type)
{
  vtkDebugMacro("AddDefaultColorNodes: adding PET nodes");
  vtkMRMLPETProceduralColorNode *nodepcn = this->CreatePETColorNode(type);
  //if (this->GetMRMLScene()->GetNodeByID( nodepcn->GetSingletonTag() ) == NULL)
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
  //if (this->GetMRMLScene()->GetNodeByID(pcnode->GetSingletonTag()) == NULL)
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
    //if (this->GetMRMLScene()->GetNodeByID(ctnode->GetSingletonTag()) == NULL)
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
    //if (this->GetMRMLScene()->GetNodeByID(ctnode->GetSingletonTag()) == NULL)
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
    // don't add a second Lables node, File node or the old atlas node
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
void vtkMRMLColorLogic::AddFreeSurferNodes()
{
  vtkDebugMacro("AddDefaultColorNodes: Adding Freesurfer nodes");
  vtkMRMLFreeSurferProceduralColorNode* basicFSNode = vtkMRMLFreeSurferProceduralColorNode::New();
  vtkDebugMacro("AddDefaultColorNodes: first type = " <<  basicFSNode->GetFirstType() << ", last type = " <<  basicFSNode->GetLastType());
  for (int type = basicFSNode->GetFirstType(); type <= basicFSNode->GetLastType(); ++type)
    {
    this->AddFreeSurferNode(type);
    }

  // add a regular colour tables holding the freesurfer volume file colours and
  // surface colours
  this->AddFreeSurferFileNode(basicFSNode);
  basicFSNode->Delete();
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
    colorNode->SetSingletonTag(NULL);
    }

  colorNode->SetName(copyName);
  colorNode->SetTypeToUser();
  colorNode->SetAttribute("Category", "User Generated");
  colorNode->SetHideFromEditors(false);

  return colorNode;
}

//------------------------------------------------------------------------------
void vtkMRMLColorLogic::AddDefaultTerminologyColors()
{
  this->TerminologyColorFiles = this->FindDefaultTerminologyColorFiles();
  vtkDebugMacro("AddDefaultTerminologyColorNodes: found " <<  this->TerminologyColorFiles.size() << " default terminology color files");
  for (unsigned int i = 0; i < this->TerminologyColorFiles.size(); i++)
    {
    this->InitializeTerminologyMappingFromFile(this->TerminologyColorFiles[i]);
    }
}

//------------------------------------------------------------------------------
bool vtkMRMLColorLogic::CreateNewTerminology(std::string lutName)
{
  if (lutName.length() == 0)
    {
    return false;
    }
  if (this->TerminologyExists(lutName))
    {
    vtkWarningMacro("Create new terminology: one already exists with name " << lutName);
    return false;
    }
  this->ColorCategorizationMaps[lutName] = ColorCategorizationMapType();
  return this->AssociateTerminologyWithColorNode(lutName);
}

//------------------------------------------------------------------------------
bool vtkMRMLColorLogic::TerminologyExists(std::string lutName)
{
  if (lutName.length() == 0)
    {
    return false;
    }

  if (this->ColorCategorizationMaps.find(lutName) !=
      this->ColorCategorizationMaps.end())
    {
    return true;
    }
  else
    {
    return false;
    }
}

//------------------------------------------------------------------------------
bool vtkMRMLColorLogic
::AddTermToTerminology(std::string lutName, int labelValue,
                       std::string categoryValue,
                       std::string categorySchemeDesignator,
                       std::string categoryMeaning,
                       std::string typeValue,
                       std::string typeSchemeDesignator,
                       std::string typeMeaning,
                       std::string modifierValue,
                       std::string modifierSchemeDesignator,
                       std::string modifierMeaning,
                       std::string regionValue,
                       std::string regionSchemeDesignator,
                       std::string regionMeaning,
                       std::string regionModifierValue,
                       std::string regionModifierSchemeDesignator,
                       std::string regionModifierMeaning)
{
  StandardTerm category(categoryValue, categorySchemeDesignator, categoryMeaning);
  StandardTerm type(typeValue, typeSchemeDesignator, typeMeaning);
  StandardTerm modifier(modifierValue, modifierSchemeDesignator, modifierMeaning);
  StandardTerm region(regionValue, regionSchemeDesignator, regionMeaning);
  StandardTerm regionModifier(regionModifierValue, regionModifierSchemeDesignator, regionModifierMeaning);
  return this->AddTermToTerminologyMapping(lutName, labelValue, category, type, modifier, region, regionModifier);
}

//------------------------------------------------------------------------------
bool vtkMRMLColorLogic::AddTermToTerminologyMapping(std::string lutName, int labelValue,
                                                    StandardTerm category, StandardTerm type, StandardTerm modifier,
                                                    StandardTerm region, StandardTerm regionModifier)
{
  if (lutName.length() == 0)
    {
    return false;
    }

 // check if the terminology mapping exists already, if not, create it
 if (!this->TerminologyExists(lutName))
   {
   vtkDebugMacro("Adding a new terminology for " << lutName);
   bool createFlag = this->CreateNewTerminology(lutName);
   if (!createFlag)
     {
     vtkWarningMacro("Unable to create new terminology for " << lutName.c_str() << " or associate it with a color node.");
     return false;
     }
   }

 ColorLabelCategorization termMapping;
 termMapping.LabelValue = labelValue;
 termMapping.SegmentedPropertyCategory = category;
 termMapping.SegmentedPropertyType = type;
 termMapping.SegmentedPropertyTypeModifier = modifier;
 termMapping.AnatomicRegion = region;
 termMapping.AnatomicRegionModifier = regionModifier;

 this->ColorCategorizationMaps[lutName][termMapping.LabelValue] = termMapping;

 return true;
}

//------------------------------------------------------------------------------
bool vtkMRMLColorLogic::AssociateTerminologyWithColorNode(std::string lutName)
{
  if (lutName.length() == 0)
    {
    return false;
    }

  vtkMRMLNode *colorNode = this->GetMRMLScene()->GetFirstNodeByName(lutName.c_str());
  if (!colorNode)
    {
    vtkWarningMacro("Unable to associate terminology with named color node: " << lutName);
    return false;
    }
  colorNode->SetAttribute("TerminologyName", lutName.c_str());

  return true;
}
//------------------------------------------------------------------------------
bool vtkMRMLColorLogic::InitializeTerminologyMappingFromFile(std::string mapFileName)
{
  std::cout << "Initializing terminology mapping for map file " << mapFileName << std::endl;

  std::ifstream mapFile(mapFileName.c_str());
  bool status = mapFile.is_open();
  std::string lutName = "";
  bool addFlag = true;
  bool assocFlag = true;
  bool parseFlag = true;
  if (status)
    {
    while (!mapFile.eof())
      {
      std::string lineIn;
      std::getline(mapFile, lineIn);

      if (lineIn[0] == '#')
        {
        continue;
        }
      if (lineIn.find("SlicerLUT=") == std::string::npos)
        {
        continue;
        }
      size_t delim = lineIn.find("=");
      lutName = lineIn.substr(delim+1,lineIn.length()-delim);
      assocFlag = this->CreateNewTerminology(lutName);
      break;
      }

    while (!mapFile.eof())
      {
      std::string lineIn, lineLeft;
      std::getline(mapFile, lineIn);
      if (lineIn.length()<30 || lineIn[0] == '#')
        {
        continue;
        }
      std::vector<std::string> tokens;
      std::stringstream ss(lineIn);
      std::string item;
      while (std::getline(ss,item,','))
        {
        tokens.push_back(item);
        }

      if (tokens.size() < 5)
        {
        vtkWarningMacro("InitializeTerminologyMappingFromFile: line has incorrect number of tokens: "
                        << tokens.size()
                        << " < 5");
        parseFlag = false;
        }
      else
        {
        int labelValue = atoi(tokens[0].c_str());
        StandardTerm category, type, modifier;
        if (this->ParseTerm(tokens[2],category) &&
            this->ParseTerm(tokens[3],type))
          {
          // modifier is optional, ParseTerm will return false on an empty string
          this->ParseTerm(tokens[4],modifier);

          // for now region doesn't appear in the file
          StandardTerm region, regionModifier;
          addFlag = addFlag && this->AddTermToTerminologyMapping(lutName, labelValue, category, type, modifier, region, regionModifier);
          }
        else
          {
          vtkWarningMacro("InitializeTerminologyMappingFromFile: failed to parse category or type: "
                          << tokens[2].c_str() << "\n"
                          << tokens[3].c_str() << "\n"
                          << tokens[4].c_str());
          parseFlag = false;
          }
        }
     }
  }
  std::cout << this->ColorCategorizationMaps[lutName].size()
            << " terms were read for Slicer LUT " << lutName << std::endl;

  return status && addFlag && assocFlag && parseFlag;
}

//-------------------------------------------------------------------------------
bool vtkMRMLColorLogic::
  LookupCategorizationFromLabel(int label, ColorLabelCategorization& labelCat, const char *lutName)
{
  bool success = false;


  if (this->TerminologyExists(lutName))
    {
    // set the label value so that if it's not found, it's still a valid categorisation
    labelCat.LabelValue = label;
    if (this->ColorCategorizationMaps[lutName].find(label) !=
      this->ColorCategorizationMaps[lutName].end())
      {
      labelCat = this->ColorCategorizationMaps[lutName][label];
      success = true;
      }
    }
  return success;
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorLogic::
  GetTerminologyFromLabel(const std::string& categorization,
                          const std::string& standardTerm,
                          int label, const char *lutName)
{
  std::string returnString;
  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    StandardTerm term;
    if (categorization.compare("SegmentedPropertyCategory") == 0)
      {
      term = labelCat.SegmentedPropertyCategory;
      }
    else if (categorization.compare("SegmentedPropertyType") == 0)
      {
      term = labelCat.SegmentedPropertyType;
      }
    else if (categorization.compare("SegmentedPropertyTypeModifier") == 0)
      {
      term = labelCat.SegmentedPropertyTypeModifier;
      }
    else if (categorization.compare("AnatomicRegion") == 0)
      {
      term = labelCat.AnatomicRegion;
      }
    else if (categorization.compare("AnatomicRegionModifier") == 0)
      {
      term = labelCat.AnatomicRegionModifier;
      }
    // now get the requested coding from the standard term
    if (standardTerm.compare("CodeValue") == 0)
      {
      returnString = term.CodeValue;
      }
    else if (standardTerm.compare("CodeMeaning") == 0)
      {
      returnString = term.CodeMeaning;
      }
    else if (standardTerm.compare("CodingSchemeDesignator") == 0)
      {
      returnString = term.CodingSchemeDesignator;
      }
    }
  return returnString;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorLogic::PrintCategorizationFromLabel(int label, const char *lutName)
{
  ColorLabelCategorization labelCat;
  if (!this->TerminologyExists(lutName))
    {
    return false;
    }
  if (this->ColorCategorizationMaps[lutName].find(label) !=
    this->ColorCategorizationMaps[lutName].end())
    {
    labelCat = this->ColorCategorizationMaps[lutName][label];
    labelCat.Print(std::cout);
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorLogic::RemoveLeadAndTrailSpaces(std::string in)
{
  std::string ret = in;
  ret.erase(ret.begin(), std::find_if(ret.begin(),ret.end(),
    std::not1(std::ptr_fun<int,int>(isspace))));
  ret.erase(std::find_if(ret.rbegin(),ret.rend(),
    std::not1(std::ptr_fun<int,int>(isspace))).base(), ret.end());
  return ret;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorLogic::ParseTerm(const std::string inputStr, StandardTerm& term)
{
  std::string str = inputStr;
  str = this->RemoveLeadAndTrailSpaces(str);
  if (str.length() < 10)
    {
    // can get empty strings for optional modifiers
    return false;
    }
  // format check, should be enclosed in parentheses, have two ;'s
  if (str.at(0) != '(' ||
      str.at(str.length()-1) != ')')
    {
    vtkWarningMacro(<< __LINE__
                    << ": ParseTerm: input string doesn't start/end with parentheses "
                    << str);
    return false;
    }
  size_t n = std::count(str.begin(), str.end(), ';');
  if (n != 2)
    {
    vtkWarningMacro(<< __LINE__
                    << ": ParseTerm: input string doesn't have 2 semi colons "
                    << str);
    return false;
    }
  // get rid of parentheses
  str = str.substr(1,str.length()-2);
  size_t found = str.find(";");
  term.CodeValue = str.substr(0,found);
  str = str.substr(found+1,str.length());
  found = str.find(";");
  term.CodingSchemeDesignator = str.substr(0,found);
  str = str.substr(found+1, str.length());
  term.CodeMeaning = str;
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyCategoryCodeValue(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyCategory.CodeValue;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyCategoryCodeMeaning(int label, const char *lutName)
{
 std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyCategory.CodeMeaning;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyCategoryCodingSchemeDesignator(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyCategory.CodingSchemeDesignator;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyCategory(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    std::string sep = std::string(":");
    returnString = labelCat.SegmentedPropertyCategory.CodeValue
      + sep
      + labelCat.SegmentedPropertyCategory.CodingSchemeDesignator
      + sep
      + labelCat.SegmentedPropertyCategory.CodeMeaning;
    if (returnString.compare("::") == 0)
      {
      // reset it to an empty string
      returnString = "";
      }
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeCodeValue(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyType.CodeValue;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeCodeMeaning(int label, const char *lutName)
{
 std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyType.CodeMeaning;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeCodingSchemeDesignator(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyType.CodingSchemeDesignator;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyType(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    std::string sep = std::string(":");
    returnString = labelCat.SegmentedPropertyType.CodeValue
      + sep
      + labelCat.SegmentedPropertyType.CodingSchemeDesignator
      + sep
      + labelCat.SegmentedPropertyType.CodeMeaning;
    if (returnString.compare("::") == 0)
      {
      // reset it to an empty string
      returnString = "";
      }
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeModifierCodeValue(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyTypeModifier.CodeValue;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeModifierCodeMeaning(int label, const char *lutName)
{
 std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyTypeModifier.CodeMeaning;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeModifierCodingSchemeDesignator(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.SegmentedPropertyTypeModifier.CodingSchemeDesignator;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetSegmentedPropertyTypeModifier(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    std::string sep = std::string(":");
    returnString = labelCat.SegmentedPropertyTypeModifier.CodeValue
      + sep
      + labelCat.SegmentedPropertyTypeModifier.CodingSchemeDesignator
      + sep
      + labelCat.SegmentedPropertyTypeModifier.CodeMeaning;
    if (returnString.compare("::") == 0)
      {
      // reset it to an empty string
      returnString = "";
      }
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionCodeValue(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.AnatomicRegion.CodeValue;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionCodeMeaning(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.AnatomicRegion.CodeMeaning;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionCodingSchemeDesignator(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.AnatomicRegion.CodingSchemeDesignator;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegion(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    std::string sep = std::string(":");
    returnString = labelCat.AnatomicRegion.CodeValue
      + sep
      + labelCat.AnatomicRegion.CodingSchemeDesignator
      + sep
      + labelCat.AnatomicRegion.CodeMeaning;
    if (returnString.compare("::") == 0)
      {
      // reset it to an empty string
      returnString = "";
      }
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionModifierCodeValue(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.AnatomicRegionModifier.CodeValue;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionModifierCodeMeaning(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.AnatomicRegionModifier.CodeMeaning;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionModifierCodingSchemeDesignator(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    returnString = labelCat.AnatomicRegionModifier.CodingSchemeDesignator;
    }

  return returnString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLColorLogic::GetAnatomicRegionModifier(int label, const char *lutName)
{
  std::string returnString;

  ColorLabelCategorization labelCat;
  if (this->LookupCategorizationFromLabel(label, labelCat, lutName))
    {
    std::string sep = std::string(":");
    returnString = labelCat.AnatomicRegionModifier.CodeValue
      + sep
      + labelCat.AnatomicRegionModifier.CodingSchemeDesignator
      + sep
      + labelCat.AnatomicRegionModifier.CodeMeaning;
    if (returnString.compare("::") == 0)
      {
      // reset it to an empty string
      returnString = "";
      }
    }

  return returnString;
}
