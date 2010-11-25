/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVectorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageLogic.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageThreshold.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode* vtkMRMLVectorVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVectorVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode::vtkMRMLVectorVolumeDisplayNode()
{
 this->VisualizationMode = this->visModeScalar;
 this->ScalarMode = this->scalarModeMagnitude;
 this->GlyphMode = this->glyphModeLines;

 this->ShiftScale = vtkImageShiftScale::New();
 this->RGBToHSI = vtkImageRGBToHSI::New();
 this->ExtractIntensity = vtkImageExtractComponents::New();

 this->ShiftScale->SetOutputScalarTypeToUnsignedChar();
 this->ShiftScale->SetClampOverflow(1);

 this->ExtractIntensity->SetInput( this->RGBToHSI->GetOutput() );
 this->ExtractIntensity->SetComponents( 2 );

 this->Threshold->SetInput( this->ExtractIntensity->GetOutput() );

 this->AppendComponents->RemoveAllInputs();
 //this->AppendComponents->SetInputConnection(0, this->ShiftScale->GetOutput()->GetProducerPort());
 //this->AppendComponents->SetInput(0, this->RGBToHSI->GetInput());
 this->AppendComponents->SetInput(0, this->ShiftScale->GetOutput());
 this->AppendComponents->SetInput(1, this->Threshold->GetOutput());

}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode::~vtkMRMLVectorVolumeDisplayNode()
{
  this->ShiftScale->Delete();
  this->RGBToHSI->Delete();
  this->ExtractIntensity->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::SetImageData(vtkImageData *imageData)
{
  this->ShiftScale->SetInput( imageData );
  this->RGBToHSI->SetInput( imageData );

  this->AppendComponents->RemoveAllInputs();
  //this->AppendComponents->SetInputConnection(0, this->ShiftScale->GetOutput()->GetProducerPort());
  //this->AppendComponents->SetInput(0, imageData);
  this->AppendComponents->SetInput(0, this->ShiftScale->GetOutput());
  this->AppendComponents->SetInput(1, this->Threshold->GetOutput());
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::SetBackgroundImageData(vtkImageData *imageData)
{
  /// TODO: what is this for?  The comment above is unhelpful!
  this->ResliceAlphaCast->SetInput(imageData);
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVectorVolumeDisplayNode::GetImageData() 
{
  if ( this->RGBToHSI->GetInput() != NULL )
    {
    this->UpdateImageDataPipeline();
    this->AppendComponents->Update();
    return this->AppendComponents->GetOutput();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::UpdateImageDataPipeline()
{
  Superclass::UpdateImageDataPipeline();

  double halfWindow = (this->GetWindow() / 2.);
  double min = this->GetLevel() - halfWindow;
  this->ShiftScale->SetShift ( -min );
  this->ShiftScale->SetScale ( 255. / (this->GetWindow()) );
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;

  ss.clear();
  ss << this->ScalarMode;
  of << indent << " scalarMode=\"" << ss.str() << "\"";

  ss.clear();
  ss << this->GlyphMode;
  of << indent << " glyphMode=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "scalarMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ScalarMode;
      }
    else if (!strcmp(attName, "glyphMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphMode;
      }
    }     

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVectorVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLVectorVolumeDisplayNode *node = (vtkMRMLVectorVolumeDisplayNode *) anode;
  
  this->SetScalarMode(node->ScalarMode);
  this->SetGlyphMode(node->GlyphMode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Scalar Mode:   " << this->ScalarMode << "\n";
  os << indent << "Glyph Mode:    " << this->GlyphMode << "\n";
}



//---------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}
