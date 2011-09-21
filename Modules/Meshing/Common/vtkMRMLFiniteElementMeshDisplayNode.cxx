/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiniteElementMeshDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkMeshQuality.h"
#include "vtkDataSetWriter.h"
#include "vtkShrinkFilter.h"

#include "vtkMRMLFiniteElementMeshDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMimxBoundingBoxSource.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiniteElementMeshDisplayNode);


void vtkMRMLFiniteElementMeshDisplayNode::UpdatePolyDataPipeline()
{
   // set the type of metric to display here and the parameters for coloring, etc. 
   //this->ShrinkFactor = whatever-was-in-the-GUI
   //this->ShrinkPolyData->SetShrinkFactor(this->ShrinkFactor); 
}

//----------------------------------------------------------------------------
vtkMRMLFiniteElementMeshDisplayNode::vtkMRMLFiniteElementMeshDisplayNode()
{
  this->SavedMeshQualityFilter = vtkMeshQuality::New();
  //vtkMeshQualityExtended* this->SavedMeshQualityFilter = vtkMeshQualityExtended::New();
  this->SavedShrinkFilter = vtkShrinkFilter::New();
  this->ShrinkFactor = 0.80;

}



vtkMRMLFiniteElementMeshDisplayNode::~vtkMRMLFiniteElementMeshDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->GeometryFilter->Delete();
  this->ShrinkPolyData->Delete();
  this->SavedShrinkFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshDisplayNode::SetUnstructuredGrid(vtkUnstructuredGrid *grid)
{
    // assign the filter to add mesh quality scalars to points & cells
    this->SavedMeshQualityFilter->SetInput(grid);
    //this->SavedMeshQualityFilter->SetHexQualityMeasureToJacobian();
    this->SavedMeshQualityFilter->SetHexQualityMeasureToEdgeRatio();
    this->SavedMeshQualityFilter->SaveCellQualityOn(); 

    // shrink the output because the mappers will remove interior detail otherwise
    
    this->SavedShrinkFilter->SetInput(this->SavedMeshQualityFilter->GetOutput());
    this->SavedShrinkFilter->SetShrinkFactor(this->ShrinkFactor);   
    this->GeometryFilter->SetInput(this->SavedShrinkFilter->GetOutput());

}


  

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);  
  of << " ";
   vtkIndent indent(nIndent);
//   {
//     std::stringstream ss;
//     ss << this->actor->GetDataType();
//     of << indent << " savedVisibilityState =\"" << this->savedVisibilityState << "\"";
//   }
}



//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  //int intAttribute;
  
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);


    }  
}

// declare a rendering pipeline for bblock data in this class
vtkPolyData* vtkMRMLFiniteElementMeshDisplayNode::GetPolyData()
{
  vtkDebugMacro("MeshDisplayNode invoked");
//  vtkDataSetWriter *write = vtkDataSetWriter::New();
//  write->SetInput(this->ShrinkPolyData->GetOutput());
//  write->SetFileName("mesh-with-quality-from-display-node.vtk");
//  write->Write();
  return this->GeometryFilter->GetOutput();
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiniteElementMeshDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiniteElementMeshDisplayNode *node = (vtkMRMLFiniteElementMeshDisplayNode *) anode;
  this->SetShrinkFactor(node->ShrinkFactor);
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "ShrinkFactor:             " << this->ShrinkFactor << "\n";
}


//---------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}
