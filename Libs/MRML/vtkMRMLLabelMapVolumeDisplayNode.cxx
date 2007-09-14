/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLLabelMapVolumeDisplayNode* vtkMRMLLabelMapVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLLabelMapVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLLabelMapVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLLabelMapVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLLabelMapVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLLabelMapVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLLabelMapVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLLabelMapVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeDisplayNode::vtkMRMLLabelMapVolumeDisplayNode()
{
  this->MapToColors = vtkImageMapToColors::New();
  this->MapToColors->SetOutputFormatToRGBA();
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeDisplayNode::~vtkMRMLLabelMapVolumeDisplayNode()
{
   this->MapToColors->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLLabelMapVolumeDisplayNode::SetDefaultColorMap()
{
  // set up a default color node
  // TODO: figure out if can use vtkSlicerColorLogic's helper methods
  this->SetAndObserveColorNodeID("vtkMRMLColorTableNodeLabels");
}

//----------------------------------------------------------------------------
void vtkMRMLLabelMapVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLLabelMapVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLLabelMapVolumeDisplayNode::UpdateImageDataPipeline()
{
  Superclass::UpdateImageDataPipeline();

  vtkLookupTable *lookupTable = NULL;
  if (this->GetColorNode())
    {
    lookupTable = this->GetColorNode()->GetLookupTable();
    }

  if (lookupTable != this->MapToColors->GetLookupTable())
    {
    this->MapToColors->SetLookupTable(lookupTable);
    }
}
