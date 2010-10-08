/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerColorLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

#include <vtkSmartPointer.h>
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerColorLogic.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLdGEMRICProceduralColorNode.h"
#include "vtkMRMLPETProceduralColorNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkColorTransferFunction.h"

#include "vtkMRMLColorTableStorageNode.h"

#include "vtkSlicerConfigure.h" // for Slicer_INSTALL_SHARE_DIR

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#endif

vtkCxxRevisionMacro(vtkSlicerColorLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerColorLogic);

//----------------------------------------------------------------------------
vtkSlicerColorLogic::vtkSlicerColorLogic()
{
  this->UserColorFilePaths = NULL;
  
  // find the color files
  this->FindColorFiles();
}

//----------------------------------------------------------------------------
vtkSlicerColorLogic::~vtkSlicerColorLogic()
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

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::ProcessMRMLEvents(vtkObject * vtkNotUsed(caller),
                                            unsigned long event,
                                            void * vtkNotUsed(callData))
{
  vtkDebugMacro("vtkSlicerColorLogic::ProcessMRMLEvents: got an event " << event);
  
  // when there's a new scene, add the default nodes
  //if (event == vtkMRMLScene::NewSceneEvent || event == vtkMRMLScene::SceneClosedEvent)
  if (event == vtkMRMLScene::NewSceneEvent)
    {
    vtkDebugMacro("vtkSlicerColorLogic::ProcessMRMLEvents: got a NewScene event " << event);
    this->AddDefaultColorNodes();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerColorLogic:             " << this->GetClassName() << "\n";

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
void vtkSlicerColorLogic::AddDefaultColorNodes()
{
    // create the default color nodes, they don't get saved with the scenes as
    // they'll be created on start up, and when a new
    // scene is opened
  if (this->GetMRMLScene() == NULL)
    {
    vtkWarningMacro("vtkSlicerColorLogic::AddDefaultColorNodes: no scene to which to add nodes\n");
    return;
    }
  
  this->GetMRMLScene()->SetIsImporting(1);

  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();

  // add the labels first
  vtkMRMLColorTableNode *labelsNode = vtkMRMLColorTableNode::New();
  labelsNode->SetTypeToLabels();
  labelsNode->SetAttribute("Category", "Discrete");
  labelsNode->SaveWithSceneOff();
  labelsNode->SetName(labelsNode->GetTypeAsString());      
  std::string labelsID = std::string(this->GetDefaultColorTableNodeID(labelsNode->GetType()));
  labelsNode->SetSingletonTag(labelsID.c_str());
  if (this->GetMRMLScene()->GetNodeByID(labelsID.c_str()) == NULL)
    {
    this->GetMRMLScene()->RequestNodeID(labelsNode, labelsID.c_str());
    this->GetMRMLScene()->AddNode(labelsNode);
    }
  labelsNode->Delete();

  // add the rest of the default color table nodes
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't add a second Lables node, File node or the old atlas node
    if (i != vtkMRMLColorTableNode::Labels &&
        i != vtkMRMLColorTableNode::File &&
        i != vtkMRMLColorTableNode::Obsolete)
      {
      vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::New();
      node->SetType(i);
      if (strstr(node->GetTypeAsString(), "Tint") != NULL)
        {
        node->SetAttribute("Category", "Tint");
        }
      else if (strstr(node->GetTypeAsString(), "Shade") != NULL)
        {
        node->SetAttribute("Category", "Shade");
        }
      else
        {
        node->SetAttribute("Category", "Discrete");
        }
      if (strcmp(node->GetTypeAsString(), "(unknown)") != 0)
        {
        node->SaveWithSceneOff();
        node->SetName(node->GetTypeAsString());      
        std::string id = std::string(this->GetDefaultColorTableNodeID(i));
        node->SetSingletonTag(id.c_str());
        if (this->GetMRMLScene()->GetNodeByID(id.c_str()) == NULL)
          {
          vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: requesting id " << id.c_str() << endl);
          this->GetMRMLScene()->RequestNodeID(node, id.c_str());
          this->GetMRMLScene()->AddNode(node);
          vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << id.c_str() << ", type = " << node->GetTypeAsString() << endl);
          }
        else
          {
          vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
          }
        }
      node->Delete();
      }
    }
  basicNode->Delete();

  // add a random procedural node that covers full integer range
  vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: making a random int mrml proc color node");
  vtkMRMLProceduralColorNode *procNode = vtkMRMLProceduralColorNode::New();
  procNode->SetName("RandomIntegers");
  procNode->SetAttribute("Category", "Discrete");
  procNode->SaveWithSceneOff();
  const char* procNodeID = this->GetDefaultProceduralColorNodeID(procNode->GetName());
  procNode->SetSingletonTag(procNodeID);
  if (this->GetMRMLScene()->GetNodeByID(procNodeID) == NULL)
    {
    this->GetMRMLScene()->RequestNodeID(procNode, procNodeID);        
    this->GetMRMLScene()->AddNode(procNode);
    
    vtkColorTransferFunction *func = procNode->GetColorTransferFunction();
    int minValue =  VTK_INT_MIN;
    int maxValue =  VTK_INT_MAX;
    double m = (double)minValue;
    double diff = (double)maxValue - (double)minValue;
    double spacing = diff/1000.0;
    while (m <= maxValue)
      {
      double r = (rand()%255)/255.0;
      double b = (rand()%255)/255.0;
      double g = (rand()%255)/255.0;
      func->AddRGBPoint( m, r, g, b);     
      m += (int)(spacing);
      }
    func->Build();
    }

  delete [] procNodeID;
  procNode->Delete();
  
  // add freesurfer nodes
  vtkDebugMacro("AddDefaultColorNodes: Adding Freesurfer nodes");
  vtkMRMLFreeSurferProceduralColorNode *basicFSNode = vtkMRMLFreeSurferProceduralColorNode::New();
  vtkDebugMacro("AddDefaultColorNodes: first type = " <<  basicFSNode->GetFirstType() << ", last type = " <<  basicFSNode->GetLastType());
  for (int i = basicFSNode->GetFirstType(); i <= basicFSNode->GetLastType(); i++)
    {
    vtkMRMLFreeSurferProceduralColorNode *node = vtkMRMLFreeSurferProceduralColorNode::New();
    vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: setting free surfer proc color node type to " << i);
    node->SetType(i);
    node->SetAttribute("Category", "FreeSurfer");
    node->SaveWithSceneOff();
    if (node->GetTypeAsString() == NULL)
      {
      vtkWarningMacro("Node type as string is null");      
      node->SetName("NoName");
      }
    else
      {
      vtkDebugMacro("Got node type as string " << node->GetTypeAsString());
      node->SetName(node->GetTypeAsString());
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: set proc node name to " << node->GetName());
      }
    /*
    if (this->GetDefaultFreeSurferColorNodeID(i) == NULL)
      {
      vtkDebugMacro("Error getting default node id for freesurfer node " << i);
      }
    */
    vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: Getting default fs color node id for type " << i);
    const char* id = this->GetDefaultFreeSurferColorNodeID(i);
    vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: got default node id " << id << " for type " << i);
    node->SetSingletonTag(id);
    vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: requesting id " << id << endl);
    if (this->GetMRMLScene()->GetNodeByID(id) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(node, id);        
      this->GetMRMLScene()->AddNode(node);
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << id << ", type = " << node->GetTypeAsString() << endl);
      }
    else
      {
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
      }
    node->Delete();
    }
  
  // add a regular colour tables holding the freesurfer volume file colours and
  // surface colours
  vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::New();
  node->SetTypeToFile();
  node->SetAttribute("Category", "FreeSurfer");
  node->SaveWithSceneOff();
  node->SetScene(this->GetMRMLScene());
  // make a storage node
  vtkMRMLColorTableStorageNode *colorStorageNode1 = vtkMRMLColorTableStorageNode::New();
  colorStorageNode1->SaveWithSceneOff();
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->AddNode(colorStorageNode1);
    node->SetAndObserveStorageNodeID(colorStorageNode1->GetID());
    }
  colorStorageNode1->Delete();
  
  vtkDebugMacro("Adding FreeSurfer Labels file node");
  std::string colorFileName;

  // volume labels
  node->SetName("FreeSurferLabels");
  if (basicFSNode->GetLabelsFileName() == NULL)
    {
    vtkErrorMacro("Unable to get the labels file name, not adding");
    }
  else
    {
    colorFileName = std::string(basicFSNode->GetLabelsFileName());
    vtkDebugMacro("Trying to read colour file " << colorFileName.c_str());
  
    node->GetStorageNode()->SetFileName(colorFileName.c_str());
    if (node->GetStorageNode()->ReadData(node))
      {
      std::string id = std::string(this->GetDefaultFreeSurferLabelMapColorNodeID());
      node->SetSingletonTag(id.c_str());
      if (this->GetMRMLScene()->GetNodeByID(id) == NULL)
        {
        this->GetMRMLScene()->RequestNodeID(node, id.c_str());        
        this->GetMRMLScene()->AddNode(node);
        }
      else
        {
        vtkDebugMacro("Unable to add a new colour node " << id.c_str() << " with freesurfer colours, from file: " << node->GetStorageNode()->GetFileName() << " as there is already a node with this id in the scene");
        }
      }
    else
      {
      vtkErrorMacro("Unable to read freesurfer colour file " << (node->GetFileName() ? node->GetFileName() : ""));
      }
    }
  // node->Delete();

 
  basicFSNode->Delete();
  node->Delete();

  //---
  // add the PET nodes
  //---
  vtkDebugMacro("AddDefaultColorNodes: adding PET nodes");
  vtkMRMLPETProceduralColorNode *basicPETNode = vtkMRMLPETProceduralColorNode::New();
  for (int i = basicPETNode->GetFirstType(); i <= basicPETNode->GetLastType(); i++)
    {
    vtkMRMLPETProceduralColorNode *nodepcn = vtkMRMLPETProceduralColorNode::New();
    nodepcn->SetType(i);
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
    const char *id1 = this->GetDefaultPETColorNodeID(i);
    nodepcn->SetSingletonTag(id1);
    if (this->GetMRMLScene()->GetNodeByID(id1) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(nodepcn, id1);        
      this->GetMRMLScene()->AddNode(nodepcn);
      }
    nodepcn->Delete();
    }
  basicPETNode->Delete();

    
  //---
  // add the dGEMRIC nodes
  //---
  vtkDebugMacro("AddDefaultColorNodes: adding dGEMRIC nodes");
  vtkMRMLdGEMRICProceduralColorNode *basicdGEMRICNode = vtkMRMLdGEMRICProceduralColorNode::New();
  for (int i = basicdGEMRICNode->GetFirstType(); i <= basicdGEMRICNode->GetLastType(); i++)
    {
    vtkMRMLdGEMRICProceduralColorNode *pcnode = vtkMRMLdGEMRICProceduralColorNode::New();
    pcnode->SetType(i);
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
    const char *id = this->GetDefaultdGEMRICColorNodeID(i);
    pcnode->SetSingletonTag(id);
    if (this->GetMRMLScene()->GetNodeByID(id) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(pcnode, id);        
      this->GetMRMLScene()->AddNode(pcnode);
      }
    pcnode->Delete();
    }
  basicdGEMRICNode->Delete();


  
  
  //  file based labels
  // first check for any new ones
  this->FindColorFiles();

  // load the one from the default resources directory
  vtkDebugMacro("AddDefaultColorNodes: found " <<  this->ColorFiles.size() << " default color files");
  for (unsigned int i = 0; i < this->ColorFiles.size(); i++)
    {
    vtkMRMLColorTableNode * ctnode =  vtkMRMLColorTableNode::New();
    ctnode->SetTypeToFile();
    ctnode->SaveWithSceneOff();
    ctnode->SetScene(this->GetMRMLScene());
    // make a storage node
    vtkMRMLColorTableStorageNode *colorStorageNode2 = vtkMRMLColorTableStorageNode::New();
    colorStorageNode2->SaveWithSceneOff();
    if (this->GetMRMLScene())
      {
      this->GetMRMLScene()->AddNode(colorStorageNode2);
      ctnode->SetAndObserveStorageNodeID(colorStorageNode2->GetID());
      }
    colorStorageNode2->Delete();
    ctnode->GetStorageNode()->SetFileName(this->ColorFiles[i].c_str());
    std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(ctnode->GetStorageNode()->GetFileName()).c_str();
    std::string uname( this->MRMLScene->GetUniqueNameByString(name.c_str()));
    ctnode->SetName(uname.c_str());
    vtkDebugMacro("AddDefaultColorFiles: About to read file " << this->ColorFiles[i].c_str());
    if (ctnode->GetStorageNode()->ReadData(ctnode))
      {
      vtkDebugMacro("AddDefaultColorFiles: finished reading file " << this->ColorFiles[i].c_str());
      const char* colorNodeID = this->GetDefaultFileColorNodeID(this->ColorFiles[i].c_str());
      std::string id =  std::string(colorNodeID);

      ctnode->SetSingletonTag(id.c_str());
      if (this->GetMRMLScene()->GetNodeByID(id) == NULL)
        {
        this->GetMRMLScene()->RequestNodeID(ctnode, id.c_str());
        this->GetMRMLScene()->AddNode(ctnode);
        vtkDebugMacro("AddDefaultColorFiles: Read and added file node: " <<  this->ColorFiles[i].c_str());
        }
      else
        {
        vtkDebugMacro("AddDefaultColorFiles: node " << id << " already in scene");
        }

      // check if this is the new default one read in from file
      if (strcmp(ctnode->GetName(),"GenericColors") == 0 ||
          strcmp(ctnode->GetName(),"GenericAnatomyColors") == 0)
        {
        vtkDebugMacro("Found default lut node");
        // remove the category attribute so it floats to the top of the node
        // selector
        // can't unset an attribute, so just don't set it at all
        }
      else
        {
        ctnode->SetAttribute("Category", "Default Labels from File");
        }
      delete [] colorNodeID;
      }
    else
      {
      vtkWarningMacro("Unable to read color file " << this->ColorFiles[i].c_str());
      }
    ctnode->Delete();
    }

  // now add ones in files that the user pointed to, these ones are not hidden
  // from the editors
  vtkDebugMacro("AddDefaultColorNodes: found " <<  this->UserColorFiles.size() << " user color files");
   for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    vtkMRMLColorTableNode * ctnode =  vtkMRMLColorTableNode::New();
    ctnode->SetTypeToFile();
    ctnode->SetAttribute("Category", "Auto Loaded User Color Files");
    ctnode->SaveWithSceneOn();
    ctnode->HideFromEditorsOff();
    ctnode->SetScene(this->GetMRMLScene());
    // make a storage node
    vtkMRMLColorTableStorageNode *colorStorageNode2 = vtkMRMLColorTableStorageNode::New();
    colorStorageNode2->SaveWithSceneOff();
    if (this->GetMRMLScene())
      {
      this->GetMRMLScene()->AddNode(colorStorageNode2);
      ctnode->SetAndObserveStorageNodeID(colorStorageNode2->GetID());
      }
    colorStorageNode2->Delete();
    ctnode->GetStorageNode()->SetFileName(this->UserColorFiles[i].c_str());
    std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(ctnode->GetStorageNode()->GetFileName()).c_str();
    std::string uname( this->MRMLScene->GetUniqueNameByString(name.c_str()));
    ctnode->SetName(uname.c_str());
    vtkDebugMacro("AddDefaultColorFiles: About to read user file " << this->UserColorFiles[i].c_str());
    if (ctnode->GetStorageNode()->ReadData(ctnode))
      {
      vtkDebugMacro("AddDefaultColorFiles: finished reading user file " << this->UserColorFiles[i].c_str());
      const char* colorNodeID = this->GetDefaultFileColorNodeID(this->UserColorFiles[i].c_str());
      std::string id =  std::string(colorNodeID);

      ctnode->SetSingletonTag(id.c_str());
      if (this->GetMRMLScene()->GetNodeByID(id) == NULL)
        {
        this->GetMRMLScene()->RequestNodeID(ctnode, id.c_str());
        this->GetMRMLScene()->AddNode(ctnode);
        vtkDebugMacro("AddDefaultColorFiles: Read and added user file node: " <<  this->UserColorFiles[i].c_str());
        }
      else
        {
        vtkDebugMacro("AddDefaultColorFiles: node " << id << " already in scene");
        }
      delete [] colorNodeID;
      }
    else
      {
      vtkWarningMacro("Unable to read user color file " << this->UserColorFiles[i].c_str());
      }
    ctnode->Delete();
    }
  vtkDebugMacro("Done adding default color nodes");
  this->GetMRMLScene()->SetIsImporting(0);
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::RemoveDefaultColorNodes()
{
  // try to find any of the default colour nodes that are still in the scene
  if (this->GetMRMLScene() == NULL)
    {
    // nothing can do, it's gone
    return;
    }
  
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  vtkMRMLColorTableNode *node;
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't have a File node...
    if (i != vtkMRMLColorTableNode::File
        && i != vtkMRMLColorTableNode::Obsolete)
      {
      //std::string id = std::string(this->GetDefaultColorTableNodeID(i));
      const char* id = this->GetDefaultColorTableNodeID(i);
      vtkDebugMacro("vtkSlicerColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
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
    const char* id = this->GetDefaultFreeSurferColorNodeID(i);
    vtkDebugMacro("vtkSlicerColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
    fsnode =  vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
    if (fsnode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(fsnode);
      }
    }
  basicFSNode->Delete();

   // remove the random procedural color nodes (after the fs proc nodes as
   // getting them by class)
  int numProcNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLProceduralColorNode");
  for (int i = 0; i < numProcNodes; i++)
    {
    vtkMRMLProceduralColorNode *procNode =  vtkMRMLProceduralColorNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLProceduralColorNode"));
    if (procNode != NULL &&
        strcmp(procNode->GetID(), this->GetDefaultProceduralColorNodeID(procNode->GetName())) == 0)
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
    const char* id = this->GetDefaultPETColorNodeID(i);
    vtkDebugMacro("vtkSlicerColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
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
    const char* id = this->GetDefaultdGEMRICColorNodeID(i);
    vtkDebugMacro("vtkSlicerColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
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
    node =  vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetDefaultFileColorNodeID(this->ColorFiles[i].c_str())));
    if (node != NULL)
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
    }
  for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    node =  vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetDefaultFileColorNodeID(this->UserColorFiles[i].c_str())));
    if (node != NULL)
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
    }
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultColorTableNodeID(int type)
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
const char * vtkSlicerColorLogic::GetDefaultFreeSurferColorNodeID(int type)
{
  const char *id;
  vtkMRMLFreeSurferProceduralColorNode *basicNode = vtkMRMLFreeSurferProceduralColorNode::New();
  basicNode->SetType(type);

  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}

//----------------------------------------------------------------------------
const char * vtkSlicerColorLogic::GetDefaultPETColorNodeID (int type )
{
  const char *id;
  vtkMRMLPETProceduralColorNode *basicNode = vtkMRMLPETProceduralColorNode::New();
  basicNode->SetType(type);
  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}


//----------------------------------------------------------------------------
const char * vtkSlicerColorLogic::GetDefaultdGEMRICColorNodeID(int type)
{
  const char *id;
  vtkMRMLdGEMRICProceduralColorNode *basicNode = vtkMRMLdGEMRICProceduralColorNode::New();
  basicNode->SetType(type);
  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultVolumeColorNodeID()
{
  return vtkSlicerColorLogic::GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Grey);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultLabelMapColorNodeID()
{
  return vtkSlicerColorLogic::GetDefaultFileColorNodeID("GenericColors.txt");
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultEditorColorNodeID()
{
  return vtkSlicerColorLogic::GetDefaultFileColorNodeID("GenericAnatomyColors.txt");
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultModelColorNodeID()
{
  return vtkSlicerColorLogic::GetDefaultFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::Heat);
}

//----------------------------------------------------------------------------
const char * vtkSlicerColorLogic::GetDefaultFreeSurferLabelMapColorNodeID()
{
  return vtkSlicerColorLogic::GetDefaultFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::Labels);
}

//----------------------------------------------------------------------------
char * vtkSlicerColorLogic::GetDefaultProceduralColorNodeID(const char *name)
{
  char *id;
  std::string idStr = std::string("vtkMRMLProceduralColorNode") + std::string(name);
  id = new char[idStr.length() + 1];
  strcpy(id, idStr.c_str());
  return id;
}

//----------------------------------------------------------------------------
char *vtkSlicerColorLogic::GetDefaultFileColorNodeID(const char * fileName)
{
  char *id;
  vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fileName);
  std::string idStr = std::string("vtkMRMLColorTableNodeFile") + name;
  id =  new char[idStr.length() + 1];
  strcpy(id, idStr.c_str());
  return id;
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::FindColorFiles()
{
    // get the slicer home dir
  vtksys_stl::string slicerHome;
  if (vtksys::SystemTools::GetEnv("Slicer_HOME") == NULL)
    {
    if (vtksys::SystemTools::GetEnv("PWD") != NULL)
      {
      slicerHome =  vtksys_stl::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      slicerHome =  vtksys_stl::string("");
      }
    }
  else
    {
    slicerHome = vtksys_stl::string(vtksys::SystemTools::GetEnv("Slicer_HOME"));
    }
  // build up the vector
  vtksys_stl::vector<vtksys_stl::string> filesVector;
  filesVector.push_back(""); // for relative path
  filesVector.push_back(slicerHome);
  filesVector.push_back(vtksys_stl::string(Slicer_INSTALL_SHARE_DIR) + "/SlicerBaseLogic/Resources/ColorFiles");
  vtksys_stl::string resourcesDirString = vtksys::SystemTools::JoinPath(filesVector);

  // now make up a vector to iterate through of dirs to look in
  std::vector<std::string> DirectoriesToCheck;

  DirectoriesToCheck.push_back(resourcesDirString);

  // add the list of dirs set from the application
  if (this->UserColorFilePaths != NULL)
    {
    vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
    // parse out the list, breaking at delimiter strings
#ifdef WIN32
    const char *delim = ";";
#else
    const char *delim = ":";
#endif
    char *ptr = strtok(this->UserColorFilePaths, delim);
    while (ptr != NULL)
      {
      std::string dir = std::string(ptr);
      vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
      DirectoriesToCheck.push_back(dir);
      ptr = strtok(NULL, delim);     
      }
    } else { vtkDebugMacro("\nFindColorFiles: oops, the user color file paths aren't set!"); }

  // create a storage node so can check for supported file types
  vtkSmartPointer<vtkMRMLColorTableStorageNode>colorStorageNode = vtkSmartPointer<vtkMRMLColorTableStorageNode>::New();
  
  // get the list of colour files in these dir
  for (unsigned int d = 0; d < DirectoriesToCheck.size(); d++)
    {
    vtksys_stl::string dirString = DirectoriesToCheck[d];
    vtkDebugMacro("FindColorFiles: checking for colour files in dir " << d << " = " << dirString.c_str());
    bool userPath = false;
    if (dirString.compare(resourcesDirString) == 0)
      {
      vtkDebugMacro("FindColorFiles: Checking resources dir " << dirString);
      }
    else
      {
      vtkDebugMacro("FindColorFiles: Checking user specified path: " << dirString);
      userPath = true;
      }
    filesVector.clear();
    filesVector.push_back(dirString);
    filesVector.push_back(vtksys_stl::string("/"));

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
        filesVector.push_back(vtksys_stl::string(findData.cFileName));
#else
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dirString.c_str())) == NULL)
      {
      vtkErrorMacro("\nError(" << errno << ") opening user specified color path: " << dirString.c_str() << ", no color files will be loaded from that directory\n(check View -> Application Settings -> Module Settings to adjust your User defined color file paths)");
      }
    else
      {
      while ((dirp = readdir(dp)) != NULL)
        {
        // add this file to the vector holding the base dir name
        filesVector.push_back(vtksys_stl::string(dirp->d_name));
#endif
        
        vtksys_stl::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector); 
        int fileType = vtksys::SystemTools::DetectFileType(fileToCheck.c_str());
        if (fileType == vtksys::SystemTools::FileTypeText)
          {
          // check that it's a supported file type
          if (colorStorageNode->SupportedFileType(fileToCheck.c_str()))
            {
            vtkDebugMacro("FindColorFiles: Adding " << fileToCheck.c_str() << " to list of potential colour files. Type = " << fileType);
            if (userPath)
              {
              // add it to the user list
              this->AddColorFile(fileToCheck.c_str(), &(this->UserColorFiles));
              }
            else
              {
              // add it to the default list
              this->AddColorFile(fileToCheck.c_str(), &(this->ColorFiles));
              }
            }
          else
            {
            vtkWarningMacro("FindColorFiles: not a supported file type:\n"<< fileToCheck);
            }
          }
        else
          {
          vtkDebugMacro("FindColorFiles: Skipping potential colour file " << fileToCheck.c_str() << ", not a text file (file type = " << fileType << ")");
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
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::AddColorFile(const char *fileName, std::vector<std::string> *Files)
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
int vtkSlicerColorLogic::LoadColorFile(const char *fileName, const char *nodeName)
{
  vtkSmartPointer<vtkMRMLColorTableNode> node = vtkSmartPointer<vtkMRMLColorTableNode>::New();
  
  node->SetTypeToFile();
  node->SetAttribute("Category", "File");
  node->SetScene(this->GetMRMLScene());
  node->SaveWithSceneOn();
  node->HideFromEditorsOff();
  // make a storage node
  vtkSmartPointer<vtkMRMLColorTableStorageNode>colorStorageNode = vtkSmartPointer<vtkMRMLColorTableStorageNode>::New();
  colorStorageNode->SaveWithSceneOn();
  
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->AddNode(colorStorageNode);
    node->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    }
  
  node->SetFileName(fileName);
  colorStorageNode->SetFileName(fileName);

  if (nodeName == NULL)
    {
    std::string name = vtksys::SystemTools::GetFilenameName(node->GetFileName()).c_str();
    std::string uname( this->MRMLScene->GetUniqueNameByString(name.c_str()));
    node->SetName(uname.c_str());
    }
  else
    {
    std::string uname( this->MRMLScene->GetUniqueNameByString(nodeName));
    node->SetName(uname.c_str());
    }
  std::string id;
  if (colorStorageNode->ReadData(node)) // ReadFile())
    {
    id =  std::string(this->GetDefaultFileColorNodeID(colorStorageNode->GetFileName()));
    if (this->GetMRMLScene()->GetNodeByID(id) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(node, id.c_str());
      this->GetMRMLScene()->AddNode(node);
      vtkDebugMacro("LoadColorFile: Done: Read and added file node: " <<  fileName);
      // don't add the name to the list of files, otherwise it will get loaded
      //again with the default ones
      //this->AddColorFile(fileName);
      return 1;
      }
    }
  else
    {
    vtkWarningMacro("Unable to read color file " << fileName << ", removing storage node from scene");
    if (this->GetMRMLScene())
      {
      node->SetAndObserveStorageNodeID(NULL);
      node->SetScene(NULL);
      this->GetMRMLScene()->RemoveNode(colorStorageNode);
      }
    }
  vtkDebugMacro("LoadColorFile: failed to reaad color file " << fileName << ", returning null");
  return 0;
}

//------------------------------------------------------------------------------
void vtkSlicerColorLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  // This code is only executed in SlicerQT via:
  // colorLogic->SetMRMLScene(scene)
  // Slicer in KWWidget runs the old style:
  //   colorLogic->SetAndObserveMRMLSceneEvents(scene, events)
  vtkIntArray* sceneEvents = vtkIntArray::New();
  sceneEvents->InsertNextValue(vtkMRMLScene::NewSceneEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
  sceneEvents->Delete();
  if (newScene)
    {
    this->AddDefaultColorNodes();
    }
}
