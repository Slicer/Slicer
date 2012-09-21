/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelectedPolyDataIds.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPlanes.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>

// TractographyMRML includes
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <math.h>
#include <vector>

//------------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLFiberBundleNode, AnnotationNodeID);

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiberBundleNode);

//------------------------------------------------------------------------------
vtkIdType vtkMRMLFiberBundleNode::MaxNumberOfFibersToShowByDefault = 10000;

//-----------------------------------------------------------------------------
vtkMRMLFiberBundleNode::vtkMRMLFiberBundleNode()
{
  this->ShuffledIds = 0;
  this->ExtractSelectedPolyDataIds = 0;
  this->CleanPolyDataPostSubsampling = 0;
  this->CleanPolyDataPostROISelection = 0;
  this->SubsamplingRatio = 0;
  this->SelectWithAnnotationNode = 0;
  this->SelectionWithAnnotationNodeMode = vtkMRMLFiberBundleNode::PositiveAnnotationNodeSelection;
  this->AnnotationNode = 0;
  this->AnnotationNodeID = 0;
  this->ExtractPolyDataGeometry = 0;
  this->Planes = 0;
  this->SelectWithAnnotationNode = 0;

  this->PrepareSubsampling();
  this->PrepareROISelection();
}

//-----------------------------------------------------------------------------
vtkMRMLFiberBundleNode::~vtkMRMLFiberBundleNode()
{
  this->CleanROISelection();
  this->CleanSubsampling();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->AnnotationNodeID != NULL) 
    {
    of << indent << " AnnotationNodeRef=\"" << this->AnnotationNodeID << "\"";
    }
  of << indent << " SelectWithAnnotationNode=\"" << this->SelectWithAnnotationNode << "\"";
  of << indent << " SelectionWithAnnotationNodeMode=\"" << this->SelectionWithAnnotationNodeMode << "\"";
  of << indent << " SubsamplingRatio=\"" << this->SubsamplingRatio << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);
  if (this->AnnotationNodeID != NULL)
    {
    delete[] this->AnnotationNodeID;
    }
  this->SelectWithAnnotationNode = 0;

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "AnnotationNodeRef")) 
      {
      const size_t n = strlen(attValue) + 1;
      this->AnnotationNodeID = new char[n];
      strcpy(this->AnnotationNodeID, attValue);
      }
    else if (!strcmp(attName, "SelectWithAnnotationNode")) 
      {
      this->SelectWithAnnotationNode = atoi(attValue);
      }
    else if (!strcmp(attName, "SelectionWithAnnotationNodeMode")) 
      {
      this->SelectionWithAnnotationNodeMode = atoi(attValue);
      }
    else if (!strcmp(attName, "SubsamplingRatio")) 
      {
      this->SubsamplingRatio = atof(attValue);
      }
    }


  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLFiberBundleNode *node = vtkMRMLFiberBundleNode::SafeDownCast(anode);

  if (node)
    {
    this->SetSubsamplingRatio(node->SubsamplingRatio);
    this->SetAnnotationNodeID(node->AnnotationNodeID);
    this->SetSelectWithAnnotationNode(node->SelectWithAnnotationNode);
    this->SetSelectionWithAnnotationNodeMode(node->SelectionWithAnnotationNodeMode);
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::ProcessMRMLEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData )
{
  if (vtkMRMLAnnotationROINode::SafeDownCast(caller) && (event == vtkCommand::ModifiedEvent))
  {
   vtkDebugMacro("Updating the ROI node");
   this->UpdateROISelection();
  }

  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
   int disabledModify = this->StartModify();

  //We are forcing the update of the fields as UpdateScene should only be called after loading data

   if (this->GetAnnotationNodeID() != NULL)
     {
     char* AnnotationNodeID = new char[strlen(this->GetAnnotationNodeID()) + 1];
     strcpy(AnnotationNodeID, this->GetAnnotationNodeID());
     delete[] this->GetAnnotationNodeID();
     this->AnnotationNodeID = NULL;
     this->SetAndObserveAnnotationNodeID(NULL);
     this->SetAndObserveAnnotationNodeID(AnnotationNodeID);
     }
   else
    {
      this->SelectWithAnnotationNode = 0;
    }


   const int ActualSelectWithAnnotationNode = this->SelectWithAnnotationNode;
   this->SelectWithAnnotationNode = -1;
   this->SetSelectWithAnnotationNode(ActualSelectWithAnnotationNode);

   const int ActualSelectionWithAnnotationNodeMode = this->SelectionWithAnnotationNodeMode;
   this->SelectionWithAnnotationNodeMode = -1;
   this->SetSelectionWithAnnotationNodeMode(ActualSelectionWithAnnotationNodeMode);

   double ActualSubsamplingRatio = this->SubsamplingRatio;
   this->SubsamplingRatio = 0.;
   this->SetSubsamplingRatio(ActualSubsamplingRatio);

   this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateReferences()
{
  if (this->AnnotationNodeID != NULL && this->Scene->GetNodeByID(this->AnnotationNodeID) == NULL)
    {
    this->SetAndObserveAnnotationNodeID(NULL);
    }
  this->Superclass::UpdateReferences();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->AnnotationNodeID && !strcmp(oldID, this->AnnotationNodeID))
    {
    this->SetAnnotationNodeID(newID);
    }
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLFiberBundleNode::GetFilteredPolyData()
{
  if (this->SelectWithAnnotationNode)
    {
    return this->CleanPolyDataPostROISelection->GetOutput();
    }
  else
    {
    return this->CleanPolyDataPostSubsampling->GetOutput();
    }
}

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
      }
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetAndObservePolyData(vtkPolyData* polyData)
{
  this->ExtractSelectedPolyDataIds->SetInput(0, polyData);
  this->Superclass::SetAndObservePolyData(polyData);

  if (polyData)
    {
    const vtkIdType numberOfFibers = polyData->GetNumberOfLines();

    std::vector<vtkIdType> idVector;
    for(vtkIdType i = 0;  i < numberOfFibers; i++ )
      idVector.push_back(i);

    random_shuffle ( idVector.begin(), idVector.end() );

    this->ShuffledIds->Initialize();
    this->ShuffledIds->SetNumberOfTuples(numberOfFibers);
    for(vtkIdType i = 0;  i < numberOfFibers; i++ )
      {
      this->ShuffledIds->SetValue(i, idVector[i]);
      }
    float subsamplingRatio = 1.;

    if (numberOfFibers > this->GetMaxNumberOfFibersToShowByDefault() )
      {
      subsamplingRatio = this->GetMaxNumberOfFibersToShowByDefault() * 1. / numberOfFibers;
      subsamplingRatio = floor(subsamplingRatio * 1e2) / 1e2;
      if (subsamplingRatio < 0.01)
        subsamplingRatio = 0.01;
      }

    this->SetSubsamplingRatio(subsamplingRatio);

    this->UpdateSubsampling();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode
::SetPolyDataToDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode)
{
  assert(modelDisplayNode->IsA("vtkMRMLFiberBundleDisplayNode"));
  modelDisplayNode->SetInputPolyData(this->GetFilteredPolyData());
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetSubsamplingRatio (float _arg)
  {
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting subsamplingRatio to " << _arg);
  const float oldSubsampling = this->SubsamplingRatio;
  const float newSubsamplingRatio = (_arg<0.?0.:(_arg>1.?1.:_arg));
  if (oldSubsampling != newSubsamplingRatio)
    {
    this->SubsamplingRatio = newSubsamplingRatio;
    this->UpdateSubsampling();
    this->Modified();
    }
  }


//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetSelectWithAnnotationNode(int _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this
                << "): setting SelectWithAnnotationNode  to " << _arg);
  if (this->SelectWithAnnotationNode != _arg)
    {
    this->SelectWithAnnotationNode = _arg;
    this->SetPolyDataToDisplayNodes();
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetSelectionWithAnnotationNodeMode(int _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SelectionWithAnnotationNodeMode  to " << _arg); 
  if (this->SelectionWithAnnotationNodeMode != _arg)
    { 
    this->SelectionWithAnnotationNodeMode = _arg;

    if (_arg == vtkMRMLFiberBundleNode::PositiveAnnotationNodeSelection)
    {
      this->ExtractPolyDataGeometry->ExtractInsideOn();
      this->ExtractPolyDataGeometry->ExtractBoundaryCellsOn();
    } else if (_arg == vtkMRMLFiberBundleNode::NegativeAnnotationNodeSelection) {
      this->ExtractPolyDataGeometry->ExtractInsideOff();
      this->ExtractPolyDataGeometry->ExtractBoundaryCellsOff();
    }

    this->Modified();
    // \tbd really needed ?
    this->InvokeEvent(vtkMRMLModelNode::PolyDataModifiedEvent, this);
    }
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationNode* vtkMRMLFiberBundleNode::GetAnnotationNode ( )
{
  vtkMRMLAnnotationNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetAnnotationNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->AnnotationNodeID );
    node = vtkMRMLAnnotationNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetAndObserveAnnotationNodeID ( const char *id )
{
  if (id)
    {
    vtkDebugMacro("Observing annotation Node: "<<id);
    }
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro (this->AnnotationNode, NULL);

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetAnnotationNodeID ( id );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetAnnotationNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->AnnotationNode , cnode );
  
  this->UpdateROISelection();

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrepareSubsampling()
{
  vtkSelection* sel = vtkSelection::New();
  vtkSelectionNode* node = vtkSelectionNode::New();
  vtkIdTypeArray* arr = vtkIdTypeArray::New();

  this->SubsamplingRatio = 1.;

  this->ShuffledIds = vtkIdTypeArray::New();

  this->ExtractSelectedPolyDataIds = vtkExtractSelectedPolyDataIds::New();

  sel->AddNode(node);

  node->GetProperties()->Set(vtkSelectionNode::CONTENT_TYPE(), vtkSelectionNode::INDICES);
  node->GetProperties()->Set(vtkSelectionNode::FIELD_TYPE(), vtkSelectionNode::CELL);

  arr->SetNumberOfTuples(0);
  node->SetSelectionList(arr);

  this->ExtractSelectedPolyDataIds->SetInput(1, sel);

  this->CleanPolyDataPostSubsampling = vtkCleanPolyData::New();
  this->CleanPolyDataPostSubsampling->ConvertLinesToPointsOff();
  this->CleanPolyDataPostSubsampling->ConvertPolysToLinesOff();
  this->CleanPolyDataPostSubsampling->ConvertStripsToPolysOff();
  this->CleanPolyDataPostSubsampling->PointMergingOff();

  this->CleanPolyDataPostSubsampling->SetInputConnection(
    this->ExtractSelectedPolyDataIds->GetOutputPort());

  arr->Delete();
  node->Delete();
  sel->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateSubsampling()
{
  vtkDebugMacro(<< this->GetClassName() << "Updating the subsampling");
  vtkSelection* sel = vtkSelection::SafeDownCast(this->ExtractSelectedPolyDataIds->GetInput(1));
  vtkPolyData* polyData = this->GetPolyData();
  if (sel && polyData)
    {
    vtkSelectionNode* node = sel->GetNode(0);

    vtkIdTypeArray* arr = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
    vtkIdType numberOfCellsToKeep = vtkIdType(floor(this->GetPolyData()->GetNumberOfLines() * this->SubsamplingRatio));

    arr->Initialize();
    arr->SetNumberOfTuples(numberOfCellsToKeep);
    if (numberOfCellsToKeep > 0)
      {
      for (vtkIdType i=0; i<numberOfCellsToKeep; i++)
        {
        arr->SetValue(i, this->ShuffledIds->GetValue(i));
        }
      }

    arr->Modified();
    node->Modified();
    sel->Modified();
    }

  /*
  vtkMRMLFiberBundleDisplayNode *node = this->GetLineDisplayNode();
  if (node != NULL)
    {
      node->SetPolyData(this->GetFilteredPolyData());
    }

  node = this->GetTubeDisplayNode();
  if (node != NULL)
    {
      node->SetPolyData(this->GetFilteredPolyData());
    }
  node = this->GetGlyphDisplayNode();
  if (node != NULL)
    {
      node->SetPolyData(this->GetFilteredPolyData());
    }
  }
    */
    // \tbd why not Modified() instead ?
  this->InvokeEvent(vtkMRMLModelNode::PolyDataModifiedEvent, this);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::CleanSubsampling()
{
  this->CleanPolyDataPostSubsampling->Delete();
  this->ExtractSelectedPolyDataIds->Delete();
  this->ShuffledIds->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrepareROISelection()
{
  this->AnnotationNode = NULL;
  this->AnnotationNodeID = NULL;

  this->ExtractPolyDataGeometry = vtkExtractPolyDataGeometry::New();
  this->Planes = vtkPlanes::New();

  this->ExtractPolyDataGeometry->ExtractInsideOn();
  this->ExtractPolyDataGeometry->ExtractBoundaryCellsOn();
  this->ExtractPolyDataGeometry->SetInputConnection(
    this->CleanPolyDataPostSubsampling->GetOutputPort());

  this->SelectionWithAnnotationNodeMode = vtkMRMLFiberBundleNode::PositiveAnnotationNodeSelection;

  this->CleanPolyDataPostROISelection = vtkCleanPolyData::New();
  this->CleanPolyDataPostROISelection->ConvertLinesToPointsOff();
  this->CleanPolyDataPostROISelection->ConvertPolysToLinesOff();
  this->CleanPolyDataPostROISelection->ConvertStripsToPolysOff();
  this->CleanPolyDataPostROISelection->PointMergingOff();

  this->CleanPolyDataPostROISelection->SetInputConnection(
    this->ExtractPolyDataGeometry->GetOutputPort());

  this->SelectWithAnnotationNode = 0;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateROISelection()
{
  vtkMRMLAnnotationROINode* AnnotationROI =
    vtkMRMLAnnotationROINode::SafeDownCast(this->AnnotationNode);
  if (AnnotationROI)
    {
    AnnotationROI->GetTransformedPlanes(this->Planes);
    this->ExtractPolyDataGeometry->SetImplicitFunction(this->Planes);
    }
  if (this->GetSelectWithAnnotationNode())
    {
    this->InvokeEvent(vtkMRMLModelNode::PolyDataModifiedEvent, this);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::CleanROISelection()
{
  this->SetAndObserveAnnotationNodeID(NULL);
  this->CleanPolyDataPostROISelection->Delete();
  this->ExtractPolyDataGeometry->Delete();
  this->Planes->Delete();
}


//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLFiberBundleNode::CreateDefaultStorageNode()
{
  vtkDebugMacro("vtkMRMLFiberBundleNode::CreateDefaultStorageNode");
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLFiberBundleStorageNode::New());
}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::CreateDefaultDisplayNodes()
{
  vtkDebugMacro("vtkMRMLFiberBundleNode::CreateDefaultDisplayNodes");
  
  vtkMRMLFiberBundleDisplayNode *fbdn = this->AddLineDisplayNode();
  fbdn->SetVisibility(1);
  fbdn = this->AddTubeDisplayNode();
  fbdn->SetVisibility(0);
  fbdn = this->AddGlyphDisplayNode();
  fbdn->SetVisibility(0);
}

