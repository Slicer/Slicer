/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include <math.h>

#include "vtkObjectFactory.h"
#include "vtkExtractSelectedPolyDataIds.h"
#include "vtkCleanPolyData.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkInformation.h"
#include "vtkIdTypeArray.h"

//#include "vtkMRMLStorageNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"

#include "vtkMRMLFiberBundleStorageNode.h"

#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiberBundleNode* vtkMRMLFiberBundleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleNode");
  if(ret)
    {
    ((vtkMRMLFiberBundleNode*)ret)->PrepareSubsampling();
    return (vtkMRMLFiberBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  ret = new vtkMRMLFiberBundleNode;
  ((vtkMRMLFiberBundleNode*)ret)->PrepareSubsampling();
  return (vtkMRMLFiberBundleNode*)ret;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleNode");
  if(ret)
    {
    ((vtkMRMLFiberBundleNode*)ret)->PrepareSubsampling();
    return (vtkMRMLFiberBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  ret = new vtkMRMLFiberBundleNode;
  ((vtkMRMLFiberBundleNode*)ret)->PrepareSubsampling();
  return (vtkMRMLFiberBundleNode*)ret;
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLFiberBundleNode::GetSubsampledPolyData() 
  {
//    if (this->SubsamplingRatio < 1.)
//    {
      return this->CleanPolyData->GetOutput();
//    } else {
//      return this->GetPolyData();
//    }
  };

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetLineDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLFiberBundleLineDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleLineDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      break;
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetTubeDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLFiberBundleTubeDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      break;
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetGlyphDisplayNode()
{
  int nnodes = this->GetNumberOfDisplayNodes();
  vtkMRMLFiberBundleGlyphDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLFiberBundleGlyphDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (node) 
      {
      break;
      }
    }
  return node;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::AddLineDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *node = this->GetLineDisplayNode();
  if (node == NULL)
    {
    node = vtkMRMLFiberBundleLineDisplayNode::New();
    if (this->GetScene())
      {
      this->GetScene()->AddNode(node);
      node->Delete();

      vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->GetScene()->AddNode(glyphDTDPN);
      node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
      node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
      glyphDTDPN->Delete();

      this->AddAndObserveDisplayNodeID(node->GetID());
      node->SetPolyData(this->GetSubsampledPolyData());
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::AddTubeDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *node = this->GetTubeDisplayNode();
  if (node == NULL)
    {
    node = vtkMRMLFiberBundleTubeDisplayNode::New();
    if (this->GetScene())
      {
      this->GetScene()->AddNode(node);
      node->Delete();

      vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->GetScene()->AddNode(glyphDTDPN);
      node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
      glyphDTDPN->Delete();
      node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
      
      this->AddAndObserveDisplayNodeID(node->GetID());
      node->SetPolyData(this->GetSubsampledPolyData());
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::AddGlyphDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *node = this->GetGlyphDisplayNode();
  if (node == NULL)
    {
    node = vtkMRMLFiberBundleGlyphDisplayNode::New();
    if (this->GetScene())
      {
      this->GetScene()->AddNode(node);
      node->Delete();

      vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->GetScene()->AddNode(glyphDTDPN);
      node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());
      glyphDTDPN->Delete();
      node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");

      this->AddAndObserveDisplayNodeID(node->GetID());
      node->SetPolyData(this->GetSubsampledPolyData());
      }
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetPolyData(vtkPolyData* polyData)
{
  vtkMRMLModelNode::SetPolyData(polyData);
  this->UpdateSubsampling();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetSubsamplingRatio (float _arg)
  {
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting subsamplingRatio to " << _arg);
  if (this->SubsamplingRatio != (_arg<0.?0.:(_arg>1.?1.:_arg)))
    {
    this->SubsamplingRatio = (_arg<0.?0.:(_arg>1.?1.:_arg));
    this->UpdateSubsampling();
    this->Modified();
    }
  }

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrepareSubsampling()
{
  vtkSelection* sel = vtkSelection::New();
  vtkSelectionNode* node = vtkSelectionNode::New();
  vtkIdTypeArray* arr = vtkIdTypeArray::New();

  this->SubsamplingRatio = 1.;

  this->ExtractSelectedPolyDataIds = vtkExtractSelectedPolyDataIds::New();

  this->CleanPolyData = vtkCleanPolyData::New();

  this->CleanPolyData->ConvertLinesToPointsOff();
  this->CleanPolyData->ConvertPolysToLinesOff();
  this->CleanPolyData->ConvertStripsToPolysOff();
  this->CleanPolyData->PointMergingOff();


  sel->AddNode(node);

  node->GetProperties()->Set(vtkSelectionNode::CONTENT_TYPE(), vtkSelectionNode::INDICES);
  node->GetProperties()->Set(vtkSelectionNode::FIELD_TYPE(), vtkSelectionNode::CELL);

  arr->SetNumberOfTuples(0);
  node->SetSelectionList(arr);
   
  this->ExtractSelectedPolyDataIds->SetInput(0,this->GetPolyData());
  this->ExtractSelectedPolyDataIds->SetInput(1, sel);
  this->CleanPolyData->SetInput(this->ExtractSelectedPolyDataIds->GetOutput());

  arr->Delete();
  node->Delete();
  sel->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateSubsampling()
{
  if (this->GetPolyData())
  {
    vtkSelection* sel = vtkSelection::SafeDownCast(this->ExtractSelectedPolyDataIds->GetInput(1));
    if (sel)
    {
      this->ExtractSelectedPolyDataIds->SetInput(0, this->GetPolyData());
      vtkSelectionNode* node = sel->GetNode(0);

      vtkIdTypeArray* arr = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
      vtkIdType numberOfCellsToKeep = vtkIdType(floor(this->GetPolyData()->GetNumberOfLines() * this->SubsamplingRatio));
      arr->SetNumberOfTuples(numberOfCellsToKeep);

      if (numberOfCellsToKeep > 0)
      {
        float step = this->GetPolyData()->GetNumberOfLines() * 1. / numberOfCellsToKeep;

        vtkIdType i = 0;
        for (i=0; i<numberOfCellsToKeep; i++)
        {
          arr->SetValue(i, int(i * step+0.5));
        }
      }

      arr->Modified();
      node->Modified();
      sel->Modified();
    }
  }
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLFiberBundleNode::CreateDefaultStorageNode()
{
  vtkWarningMacro("vtkMRMLFiberBundleNode::CreateDefaultStorageNode");
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLFiberBundleStorageNode::New());
}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::CreateDefaultDisplayNodes()
{
  vtkWarningMacro("vtkMRMLFiberBundleNode::CreateDefaultDisplayNodes");
  
  vtkMRMLFiberBundleDisplayNode *fbdn = this->AddLineDisplayNode();
  fbdn->SetVisibility(1);
  fbdn = this->AddTubeDisplayNode();
  fbdn->SetVisibility(0);
  fbdn = this->AddGlyphDisplayNode();
  fbdn->SetVisibility(0);
  
}
