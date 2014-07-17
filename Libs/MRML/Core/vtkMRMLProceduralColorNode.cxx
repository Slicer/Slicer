/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLProceduralColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLProceduralColorStorageNode.h"

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkEventBroker.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLProceduralColorNode);


//----------------------------------------------------------------------------
vtkMRMLProceduralColorNode::vtkMRMLProceduralColorNode()
{
  this->ColorTransferFunction = NULL;
  vtkColorTransferFunction* ctf=vtkColorTransferFunction::New();
  this->SetAndObserveColorTransferFunction(ctf);
  ctf->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLProceduralColorNode::~vtkMRMLProceduralColorNode()
{
  this->SetAndObserveColorTransferFunction(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}


//----------------------------------------------------------------------------
// Copy the anode's attributes to this object.
void vtkMRMLProceduralColorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLProceduralColorNode *node = (vtkMRMLProceduralColorNode *) anode;
  if (!node)
    {
    vtkWarningMacro("Copy: Input node is not a procedural color node!");
    return;
    }

  int oldModified=this->StartModify();
  if (node->GetColorTransferFunction()!=NULL)
    {
    if (this->ColorTransferFunction==NULL)
      {
      vtkColorTransferFunction* ctf=vtkColorTransferFunction::New();
      this->SetAndObserveColorTransferFunction(ctf);
      ctf->Delete();
      }
    if (this->ColorTransferFunction!=node->GetColorTransferFunction())
      {
      this->ColorTransferFunction->DeepCopy(node->GetColorTransferFunction());
      }
    }
  else
    {
    this->SetAndObserveColorTransferFunction(NULL);
    }
  this->EndModify(oldModified);

}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  if (this->ColorTransferFunction != NULL)
    {
    os << indent << "ColorTransferFunction:" << endl;
    this->ColorTransferFunction->PrintSelf(os, indent.GetNextIndent());
    }
}

//-----------------------------------------------------------

void vtkMRMLProceduralColorNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  if (caller!=NULL && caller==this->ColorTransferFunction && event==vtkCommand::ModifiedEvent)
    {
    Modified();
    }
  return;
}

//-----------------------------------------------------------
vtkScalarsToColors* vtkMRMLProceduralColorNode::GetScalarsToColors()
{
  return this->GetColorTransferFunction();
}

//---------------------------------------------------------------------------
const char * vtkMRMLProceduralColorNode::GetTypeAsString()
{
  const char *type = Superclass::GetTypeAsString();
  if (type && strcmp(type,"(unknown)") != 0)
    {
    return type;
    }
  return this->GetName();
}

//---------------------------------------------------------------------------
bool vtkMRMLProceduralColorNode::SetNameFromColor(int index)
{
  double colour[4];
  this->GetColor(index, colour);
  //this->ColorTransferFunction->GetColor(index, colour);
  std::stringstream ss;
  ss.precision(3);
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss << "R=";
  ss << colour[0];
  ss << " G=";
  ss << colour[1];
  ss << " B=";
  ss << colour[2];
  if (this->SetColorName(index, ss.str().c_str()) == 0)
    {
    vtkErrorMacro("SetNamesFromColors: error setting name " <<  ss.str().c_str() << " for color index " << index);
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLProceduralColorNode::GetNumberOfColors()
{
  /*
  double *range = this->ColorTransferFunction->GetRange();
  if (!range)
    {
    return 0;
    }
  int numPoints = static_cast<int>(floor(range[1] - range[0]));
  if (range[0] < 0 && range[1] >= 0)
    {
    // add one for zero
    numPoints++;
    }
  return numPoints;
  */
  if (this->ColorTransferFunction==NULL)
    {
    return 0;
    }
  return this->ColorTransferFunction->GetSize();
}

//---------------------------------------------------------------------------
bool vtkMRMLProceduralColorNode::GetColor(int entry, double color[4])
{
  if (entry < 0 || entry >= this->GetNumberOfColors())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetNumberOfColors() << ", call SetNumberOfColors" << endl);
    return false;
    }
  /*
  double *range = this->ColorTransferFunction->GetRange();
  if (!range)
    {
    return false;
    }
  this->ColorTransferFunction->GetColor(range[0] + entry, color);
  color[3] = this->ColorTransferFunction->GetAlpha();
  return true;
  */
  double val[6];
  this->ColorTransferFunction->GetNodeValue(entry, val);
  color[0] = val[1]; // r
  color[1] = val[2]; // g
  color[2] = val[3]; // b
  color[3] = this->ColorTransferFunction->GetAlpha();
  return true;
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode * vtkMRMLProceduralColorNode::CreateDefaultStorageNode()
{
  return vtkMRMLProceduralColorStorageNode::New();
}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::SetAndObserveColorTransferFunction(vtkColorTransferFunction *ctf)
{
  if (ctf==this->ColorTransferFunction)
    {
    return;
    }
  if (this->ColorTransferFunction != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->ColorTransferFunction, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    this->ColorTransferFunction->UnRegister(this);
    this->ColorTransferFunction=NULL;
    }
  this->ColorTransferFunction=ctf;
  if ( this->ColorTransferFunction )
    {
    this->ColorTransferFunction->Register(this);
    vtkEventBroker::GetInstance()->AddObservation (
      this->ColorTransferFunction, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLProceduralColorNode::IsColorMapEqual(vtkColorTransferFunction* tf1, vtkColorTransferFunction* tf2)
{
  if (tf1==tf2)
    {
    return true;
    }
  if (tf1==NULL || tf2==NULL)
    {
    return false;
    }
  if (tf1->GetSize()!=tf2->GetSize())
    {
    return false;
    }
  const int NUMBER_OF_VALUES_PER_POINT=6; // x, red, green, blue, midpoint, sharpness
  double values1[NUMBER_OF_VALUES_PER_POINT]={0};
  double values2[NUMBER_OF_VALUES_PER_POINT]={0};
  int numberOfPoints=tf1->GetSize();
  for (int pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
    {
    tf1->GetNodeValue(pointIndex, values1);
    tf2->GetNodeValue(pointIndex, values2);
    for (int valueIndex=0; valueIndex<NUMBER_OF_VALUES_PER_POINT; ++valueIndex)
      {
      if (values1[valueIndex]!=values2[valueIndex])
        {
        // found a difference
        return false;
        }
      }
    }
  // found no difference
  return true;
}
