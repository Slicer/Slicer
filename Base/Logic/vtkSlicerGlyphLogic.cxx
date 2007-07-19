/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceGlyphLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkSlicerGlyphLogic.h"
#include "vtkIndent.h" 

vtkCxxRevisionMacro(vtkSlicerGlyphLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerGlyphLogic);

//----------------------------------------------------------------------------
vtkSlicerGlyphLogic::vtkSlicerGlyphLogic()
{

  this->VolumeDisplayNode = NULL;
  this->ImageData = NULL;
  this->PolyData = vtkPolyData::New();
}

//----------------------------------------------------------------------------
vtkSlicerGlyphLogic::~vtkSlicerGlyphLogic()
{
  if ( this->VolumeDisplayNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeDisplayNode , NULL );
    }
  if ( this->ImageData )
    {
    vtkSetAndObserveMRMLNodeMacro( this->ImageData , NULL );
    }
  if (this->PolyData )
  {
    this->PolyData->Delete();
  }
}
//----------------------------------------------------------------------------
void vtkSlicerGlyphLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();
  
  os << indent << "SlicerSliceGlyphLogic:             " << this->GetClassName() << "\n";

  os << indent << "VolumeDisplayNode: " <<
    (this->VolumeDisplayNode ? this->VolumeDisplayNode->GetID() : "(none)") << "\n";
  if (this->VolumeDisplayNode)
    {
    this->VolumeDisplayNode->PrintSelf(os, nextIndent);
    }
  os << indent << "ImageData: " ;
  if (this->ImageData)
    {
    this->ImageData->PrintSelf(os, nextIndent);
    } else {
    os << "NULL \n";
    }
  os << indent << "PolyData: ";
  if (this->PolyData)
    {
    this->PolyData->PrintSelf(os, nextIndent);
    } else {
    os << "NULL \n";
    }
}
//----------------------------------------------------------------------------
void vtkSlicerGlyphLogic::SetVolumeDisplayNode(vtkMRMLVolumeDisplayNode *volumeDisplayNode)
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->VolumeDisplayNode, volumeDisplayNode, events );
  events->Delete();
}

vtkPolyData* vtkSlicerGlyphLogic::GetPolyData()
{
  vtkIndent indent;

  if ( this->ImageData && this->VolumeDisplayNode && ( this->VolumeDisplayNode->IsA("vtkMRMLVolumeGlyphDisplayNode") ) )
  {
    vtkErrorMacro("Trying to return the PolyData displayNode:"<<this->VolumeDisplayNode<<" ImageData:"<<this->ImageData);
    vtkErrorMacro("getting the PolyData from the child node");
    vtkMRMLVolumeGlyphDisplayNode* VolumeGlyphDisplayNode = vtkMRMLVolumeGlyphDisplayNode::SafeDownCast(this->VolumeDisplayNode);
    if (!VolumeGlyphDisplayNode)
    {
      vtkErrorMacro("The display node("<<this->VolumeDisplayNode<<") is not a glyphDisplayNode");
      return NULL;
    }
    vtkErrorMacro("Setting the ImageData");

    this->ImageData->PrintSelf(std::cout,indent);

    vtkErrorMacro("To the displayNode");
    VolumeGlyphDisplayNode->PrintSelf(std::cout,indent);

    vtkErrorMacro("DONE");

    vtkPolyData* polyData =  VolumeGlyphDisplayNode->ExecuteGlyphPipeLineAndGetPolyData( this->ImageData );


    if (polyData!=NULL)
    {
#if 0
      PolyData->DeepCopy(polyData); 

      vtkErrorMacro("Getting the PolyData:");
      PolyData->PrintSelf(std::cout, vtkIndent());
      return PolyData;
#endif

      this->PolyData = polyData;
      return (this->PolyData);

    } else {
      return NULL;
    }
  }
  //Select the right element for your data
  //Update the glyphing
  //Return the polydata
  return NULL;
} 
//----------------------------------------------------------------------------

