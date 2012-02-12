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
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLdGEMRICProceduralColorNode.h"
#include "vtkMRMLPETProceduralColorNode.h"
#include "vtkMRMLColorTableStorageNode.h"

// VTK sys includes
#include <vtkLookupTable.h>
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkColorTransferFunction.h>

vtkCxxRevisionMacro(vtkMRMLColorLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLColorLogic);

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
  for (int i = 0; i << this->ColorFiles.size(); i++)
    {
    os << indent.GetNextIndent() << i << " " << this->ColorFiles[i].c_str() << "\n";
    }
  os << indent << "User Color Files:\n";
  for (int i = 0; i << this->UserColorFiles.size(); i++)
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

  // add a random procedural node that covers full integer range
  this->AddRandomNode();  

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

   // remove the random procedural color nodes (after the fs proc nodes as
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
  const char *id;
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  basicNode->SetType(type);

  //std::string id = std::string(basicNode->GetClassName()) + std::string(basicNode->GetTypeAsString());
  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();
  //basicNode = NULL;
  //return id.c_str();

  return (id);
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetFreeSurferColorNodeID(int type)
{
  const char *id;
  vtkMRMLFreeSurferProceduralColorNode *basicNode = vtkMRMLFreeSurferProceduralColorNode::New();
  basicNode->SetType(type);

  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetPETColorNodeID (int type )
{
  const char *id;
  vtkMRMLPETProceduralColorNode *basicNode = vtkMRMLPETProceduralColorNode::New();
  basicNode->SetType(type);
  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}


//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetdGEMRICColorNodeID(int type)
{
  const char *id;
  vtkMRMLdGEMRICProceduralColorNode *basicNode = vtkMRMLdGEMRICProceduralColorNode::New();
  basicNode->SetType(type);
  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}

//----------------------------------------------------------------------------
const char * vtkMRMLColorLogic::GetProceduralColorNodeID(const char *name)
{
  char *id;
  std::string idStr = std::string("vtkMRMLProceduralColorNode") + std::string(name);
  id = new char[idStr.length() + 1];
  strcpy(id, idStr.c_str());
  return id;
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
  char *id;
  std::string idStr =
    std::string("vtkMRMLColorTableNode") +
    vtkMRMLColorLogic::GetFileColorNodeSingletonTag(fileName);
  id =  new char[idStr.length() + 1];
  strcpy(id, idStr.c_str());
  return id;
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
  vtkMRMLColorTableNode* node = this->CreateFileNode(fileName);
  
  if (!node)
    {
    return 0;
    }

  node->SetAttribute("Category", "File");  
  node->SaveWithSceneOn();
  node->GetStorageNode()->SaveWithSceneOn();
  node->HideFromEditorsOff();

  if (nodeName != NULL)
    {
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(nodeName));
    node->SetName(uname.c_str());
    }
  //std::string id = std::string(node->GetSingletonTag());
  //if (this->GetMRMLScene()->GetNodeByID(id) != NULL)
  //  {
  //  vtkDebugMacro("LoadColorFile: File already exists: " <<  fileName);  
  //  return 0;
  //  }
  //this->GetMRMLScene()->RequestNodeID(node, id.c_str());
  this->GetMRMLScene()->AddNode(node);
  vtkDebugMacro("LoadColorFile: Done: Read and added file node: " <<  fileName);
  // don't add the name to the list of files, otherwise it will get loaded
  //again with the default ones
  //this->AddColorFile(fileName);
  node->Delete();  
  
  return node;
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
  vtkDebugMacro("vtkMRMLColorLogic::AddDefaultColorNodes: making a random int mrml proc color node");
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
  vtkMRMLColorTableStorageNode *colorStorageNode = vtkMRMLColorTableStorageNode::New();
  colorStorageNode->SaveWithSceneOff();
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->AddNode(colorStorageNode);
    ctnode->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    }
  colorStorageNode->Delete();
  
  ctnode->GetStorageNode()->SetFileName(fileName);
  std::string basename = vtksys::SystemTools::GetFilenameWithoutExtension(fileName);
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(basename.c_str()));
  ctnode->SetName(uname.c_str());
  
  vtkDebugMacro("AddDefaultColorFiles: About to read user file " << fileName);

  if (ctnode->GetStorageNode()->ReadData(ctnode) == 0)
    {
      vtkErrorMacro("Unable to read freesurfer colour file " << (ctnode->GetFileName() ? ctnode->GetFileName() : ""));
      
    if (this->GetMRMLScene())
      {
      ctnode->SetAndObserveStorageNodeID(NULL);
      ctnode->SetScene(NULL);
      this->GetMRMLScene()->RemoveNode(colorStorageNode);
      }
      
      ctnode->Delete();
      return 0;
    }
  vtkDebugMacro("AddDefaultColorFiles: finished reading user file " << fileName);
  ctnode->SetSingletonTag(
    this->GetFileColorNodeSingletonTag(fileName).c_str());

  return ctnode;
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
void vtkMRMLColorLogic::AddRandomNode()
{  
  vtkMRMLProceduralColorNode* randomNode = this->CreateRandomNode();
  //if (this->GetMRMLScene()->GetNodeByID(randomNode->GetSingletonTag()))
    {
    // Add the node after it has been initialized
    //this->GetMRMLScene()->RequestNodeID(randomNode, randomNode->GetSingletonTag());
    this->GetMRMLScene()->AddNode(randomNode);
    }
  randomNode->Delete();
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
  ctnode->Delete();
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
    colorNode->SetColor(i, nodeToCopy->GetColorName(i), color[0], color[1], color[2]);
    }
  return colorNode;
}

