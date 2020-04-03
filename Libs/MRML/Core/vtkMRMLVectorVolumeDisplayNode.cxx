/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVectorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLVectorVolumeDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageRGBToHSI.h>
#include <vtkImageShiftScale.h>
#include <vtkImageStencil.h>
#include <vtkImageThreshold.h>
#include <vtkVersion.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVectorVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode::vtkMRMLVectorVolumeDisplayNode()
{
 this->ScalarMode = this->scalarModeMagnitude;
 this->GlyphMode = this->glyphModeLines;

 this->ShiftScale = vtkImageShiftScale::New();
 this->RGBToHSI = vtkImageRGBToHSI::New();
 this->ExtractIntensity = vtkImageExtractComponents::New();

 this->ShiftScale->SetOutputScalarTypeToUnsignedChar();
 this->ShiftScale->SetClampOverflow(1);

 this->ExtractIntensity->SetInputConnection( this->RGBToHSI->GetOutputPort() );
 this->ExtractIntensity->SetComponents( 2 );

 this->Threshold->SetInputConnection( this->ExtractIntensity->GetOutputPort() );

 this->AppendComponents->RemoveAllInputs();
 this->AppendComponents->AddInputConnection(0, this->ShiftScale->GetOutputPort());
 this->AppendComponents->AddInputConnection(0, this->MultiplyAlpha->GetOutputPort());

 this->MultiplyAlpha->RemoveAllInputs();
 this->MultiplyAlpha->SetInputConnection(0, this->Threshold->GetOutputPort() );
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode::~vtkMRMLVectorVolumeDisplayNode()
{
  this->ShiftScale->Delete();
  this->RGBToHSI->Delete();
  this->ExtractIntensity->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection)
{
  this->ShiftScale->SetInputConnection(imageDataConnection);
  this->RGBToHSI->SetInputConnection(imageDataConnection);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorVolumeDisplayNode::GetInputImageDataConnection()
{
  return this->ShiftScale->GetNumberOfInputConnections(0) ?
    this->ShiftScale->GetInputConnection(0,0) : nullptr;
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorVolumeDisplayNode::GetScalarImageDataConnection()
{
  return this->GetInputImageDataConnection();
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

  std::stringstream ss;

  ss.clear();
  ss << this->ScalarMode;
  of << " scalarMode=\"" << ss.str() << "\"";

  ss.clear();
  ss << this->GlyphMode;
  of << " glyphMode=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
void vtkMRMLVectorVolumeDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLVectorVolumeDisplayNode *node = vtkMRMLVectorVolumeDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  this->SetScalarMode(node->ScalarMode);
  this->SetGlyphMode(node->GlyphMode);
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
