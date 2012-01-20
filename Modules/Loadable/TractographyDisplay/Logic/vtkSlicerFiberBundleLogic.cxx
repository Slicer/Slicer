/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/


#include "vtkSlicerFiberBundleLogic.h"

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"

#include "vtkPolyData.h"

#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

vtkCxxRevisionMacro(vtkSlicerFiberBundleLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiberBundleLogic);

//----------------------------------------------------------------------------
vtkSlicerFiberBundleLogic::vtkSlicerFiberBundleLogic()
{
  this->MaxNumberOfFibersToShowByDefault = 10000;
}

//----------------------------------------------------------------------------
vtkSlicerFiberBundleLogic::~vtkSlicerFiberBundleLogic()
{

}

//----------------------------------------------------------------------------
int vtkSlicerFiberBundleLogic::AddFiberBundles (const char* dirname, const char* suffix )
{
  std::string ssuf = suffix;
  itksys::Directory dir;
  dir.Load(dirname);
 
  int nfiles = dir.GetNumberOfFiles();
  int res = 1;
  for (int i=0; i<nfiles; i++) {
    const char* filename = dir.GetFile(i);
    std::string sname = filename;
    if (!itksys::SystemTools::FileIsDirectory(filename))
      {
      if ( sname.find(ssuf) != std::string::npos )
        {
        std::string fullPath = std::string(dir.GetPath())
            + "/" + filename;
        if (this->AddFiberBundle(fullPath.c_str()) == NULL)
          {
          res = 0;
          }
        }
      }
  }
  return res;
}

//----------------------------------------------------------------------------
int vtkSlicerFiberBundleLogic::AddFiberBundles (const char* dirname, std::vector< std::string > suffix )
{
  itksys::Directory dir;
  dir.Load(dirname);
 
  int nfiles = dir.GetNumberOfFiles();
  int res = 1;
  for (int i=0; i<nfiles; i++) {
    const char* filename = dir.GetFile(i);
    std::string sname = filename;
    if (!itksys::SystemTools::FileIsDirectory(filename))
      {
      for (unsigned int s=0; s<suffix.size(); s++)
        {
        std::string ssuf = suffix[s];
        if ( sname.find(ssuf) != std::string::npos )
          {
          std::string fullPath = std::string(dir.GetPath())
              + "/" + filename;
          if (this->AddFiberBundle(fullPath.c_str()) == NULL)
            {
            res = 0;
            }
          } //if (sname
        } // for (int s=0;
      }
  }
  return res;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleNode* vtkSlicerFiberBundleLogic::AddFiberBundle (const char* filename)
{
  vtkDebugMacro("Adding fiber bundle from filename " << filename);

  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::New();
  vtkMRMLFiberBundleLineDisplayNode *displayLineNode = vtkMRMLFiberBundleLineDisplayNode::New();
  vtkMRMLFiberBundleTubeDisplayNode *displayTubeNode = vtkMRMLFiberBundleTubeDisplayNode::New();
  vtkMRMLFiberBundleGlyphDisplayNode *displayGlyphNode = vtkMRMLFiberBundleGlyphDisplayNode::New();
  vtkMRMLFiberBundleStorageNode *storageNode = vtkMRMLFiberBundleStorageNode::New();

  vtkMRMLDiffusionTensorDisplayPropertiesNode *lineDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
  vtkMRMLDiffusionTensorDisplayPropertiesNode *tubeDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
  vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
  glyphDTDPN->SetGlyphGeometry(vtkMRMLDiffusionTensorDisplayPropertiesNode::Ellipsoids);

  storageNode->SetFileName(filename);
  if (storageNode->ReadData(fiberBundleNode) != 0)
    {
    const itksys_stl::string fname(filename);
    itksys_stl::string name = itksys::SystemTools::GetFilenameWithoutExtension(fname);
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));
    fiberBundleNode->SetName(uname.c_str());

    const vtkIdType numberOfFibers = fiberBundleNode->GetPolyData()->GetNumberOfLines();
    float subsamplingRatio = 1.;

    if (numberOfFibers > this->GetMaxNumberOfFibersToShowByDefault() )
      {
      subsamplingRatio = this->GetMaxNumberOfFibersToShowByDefault() * 1. / numberOfFibers;
      subsamplingRatio = floor(subsamplingRatio * 1e2) / 1e2; //Rounding to 2 decimals
      }

    fiberBundleNode->SetSubsamplingRatio(subsamplingRatio);
    
    fiberBundleNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayLineNode->SetScene(this->GetMRMLScene());
    displayTubeNode->SetScene(this->GetMRMLScene());
    displayGlyphNode->SetScene(this->GetMRMLScene());
  
    displayLineNode->SetVisibility(1);
    displayTubeNode->SetVisibility(0);
    displayGlyphNode->SetVisibility(0);

    this->GetMRMLScene()->SaveStateForUndo();
    this->GetMRMLScene()->AddNode(lineDTDPN);
    this->GetMRMLScene()->AddNode(tubeDTDPN);
    this->GetMRMLScene()->AddNode(glyphDTDPN);

    displayLineNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(lineDTDPN->GetID());
    displayTubeNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(tubeDTDPN->GetID());
    displayGlyphNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
 
    this->GetMRMLScene()->AddNode(storageNode);
    this->GetMRMLScene()->AddNode(displayLineNode);
    this->GetMRMLScene()->AddNode(displayTubeNode);
    this->GetMRMLScene()->AddNode(displayGlyphNode);

    fiberBundleNode->SetAndObserveStorageNodeID(storageNode->GetID());
    displayLineNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
    displayTubeNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
    displayGlyphNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");

    fiberBundleNode->SetAndObserveDisplayNodeID(displayLineNode->GetID());  
    fiberBundleNode->AddAndObserveDisplayNodeID(displayTubeNode->GetID());  
    fiberBundleNode->AddAndObserveDisplayNodeID(displayGlyphNode->GetID());  
    displayLineNode->SetPolyData(fiberBundleNode->GetFilteredPolyData());
    displayTubeNode->SetPolyData(fiberBundleNode->GetFilteredPolyData());
    displayGlyphNode->SetPolyData(fiberBundleNode->GetFilteredPolyData());

    this->GetMRMLScene()->AddNode(fiberBundleNode);

    // Set up display logic and any other logic classes in future
    //this->InitializeLogicForFiberBundleNode(fiberBundleNode);

    //this->Modified();  

    fiberBundleNode->Delete();
    }
  else
    {
    vtkErrorMacro("Couldn't read file, returning null fiberBundle node: " << filename);
    fiberBundleNode->Delete();
    fiberBundleNode = NULL;
    }
  storageNode->Delete();
  displayLineNode->Delete();
  displayTubeNode->Delete();
  displayGlyphNode->Delete();
  
  //displayLogic->Delete();

  lineDTDPN->Delete();
  tubeDTDPN->Delete();
  glyphDTDPN->Delete();

  return fiberBundleNode;  
}
//----------------------------------------------------------------------------
int vtkSlicerFiberBundleLogic::SaveFiberBundle (const char* filename, vtkMRMLFiberBundleNode *fiberBundleNode)
{
   if (fiberBundleNode == NULL || filename == NULL)
    {
    return 0;
    }
  
  vtkMRMLFiberBundleStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = fiberBundleNode->GetStorageNode();
  if (snode != NULL)
    {
    storageNode = vtkMRMLFiberBundleStorageNode::SafeDownCast(snode);
    }
  if (storageNode == NULL)
    {
    storageNode = vtkMRMLFiberBundleStorageNode::New();
    storageNode->SetScene(this->GetMRMLScene());
    this->GetMRMLScene()->AddNode(storageNode);  
    fiberBundleNode->SetAndObserveStorageNodeID(storageNode->GetID());
    storageNode->Delete();
    }

  //storageNode->SetAbsoluteFileName(true);
  storageNode->SetFileName(filename);

  int res = storageNode->WriteData(fiberBundleNode);

  
  return res;

}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiberBundleLogic:             " << this->GetClassName() << "\n";

}


