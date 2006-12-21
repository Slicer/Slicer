/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkDoubleArray.h"

//------------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode* vtkMRMLDiffusionWeightedVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionWeightedVolumeNode");
  if(ret)
    {
    return (vtkMRMLDiffusionWeightedVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionWeightedVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionWeightedVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionWeightedVolumeNode");
  if(ret)
    {
    return (vtkMRMLDiffusionWeightedVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionWeightedVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode::vtkMRMLDiffusionWeightedVolumeNode()
{
  this->NumberOfGradients = 7; //6 gradients + 1 baseline
  this->Gradients = vtkDoubleArray::New();
  this->Gradients->SetNumberOfComponents(3);
  this->Gradients->SetNumberOfTuples(this->NumberOfGradients);
  
  this->BValues = vtkDoubleArray::New();
  this->BValues->SetNumberOfTuples(this->NumberOfGradients);
  
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      this->MeasurementFrame[i][j] = (i == j) ? 1.0 : 0.0;
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode::~vtkMRMLDiffusionWeightedVolumeNode()
{
  this->Gradients->Delete();
  this->BValues->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  std::stringstream ss;
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      ss << this->MeasurementFrame[i][j] << " ";
      if ( i != 2 && j != 2 )
        {
        ss << "  ";
        }
      }
    }
    of << indent << "measurementFrame=\"" << ss.str() << "\" ";

  ss.clear();
  
  for(int g=0; g<this->Gradients->GetNumberOfTuples(); g++)
    {
    for (int k=0; k<3; k++)
      {
      ss << this->Gradients->GetComponent(g,k) << " ";
      }
    }
  
  of << indent << "gradients=\"" << ss.str() << "\" ";
  
  ss.clear();
  
  for (int g=0; g<this->BValues->GetNumberOfTuples(); g++)
    {
    ss << this->BValues->GetValue(g) << " ";
    }
  of << indent << "bValues=\"" << ss.str() << "\" ";

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "measurementFrame"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        for(int j=0; j<3; j++)
          {
          ss >> val;
          this->MeasurementFrame[i][j] = val;
          }
        }
      }
    if (!strcmp(attName, "gradients"))
      {
      std::stringstream ss;
      ss << attValue;
      double g[3];
      this->Gradients->Reset();
      this->Gradients->SetNumberOfComponents(3);
      while (!ss.eof())
        {
        for (int i=0; i<3; i++)
          {
          ss >> g[i];
          }
        this->Gradients->InsertNextTuple(g);
        }
      this->NumberOfGradients = this->Gradients->GetNumberOfTuples();  
      }
    if (!strcmp(attName, "bValues"))
      {
      std::stringstream ss;
      double val;   
      this->BValues->Reset();
      while (!ss.eof())
        {
        ss >> val;
        this->BValues->InsertNextValue(val);  
        }
      }
   }    
}  

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetMeasurementFrame(const double mf[3][3])
{
  for (int i=0; i<3; i++) 
    {
    for (int j=0; j<3; j++) 
      {
      this->MeasurementFrame[i][j] = mf[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::GetMeasurementFrame(double mf[3][3])
{
  for (int i=0; i<3; i++) 
    {
    for (int j=0; j<3; j++) 
      {
      mf[i][j] = this->MeasurementFrame[i][j];
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetMeasurementFrame(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs)
{
  MeasurementFrame[0][0] = xr;
  MeasurementFrame[0][1] = xa;
  MeasurementFrame[0][2] = xs;
  MeasurementFrame[1][0] = yr;
  MeasurementFrame[1][1] = ya;
  MeasurementFrame[1][2] = ys;
  MeasurementFrame[2][0] = zr;
  MeasurementFrame[2][1] = za;
  MeasurementFrame[2][2] = zs;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetNumberOfGradients(int val)
{
  if (this->NumberOfGradients != val)
    {
      this->Gradients->Reset();
      this->BValues->Reset();
      vtkDebugMacro ("setting num gradients to " << val);
      // internal array for storage of gradient vectors
      this->Gradients->SetNumberOfTuples(val);
      this->BValues->SetNumberOfTuples(val);
      // this class's info
      this->NumberOfGradients = val;
      this->Modified();
    }
}  

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetGradients(int num,const double grad[3])
{
  if (this->Gradients->GetNumberOfTuples()<num)
    {
    this->Gradients->SetComponent(num,0,grad[0]);
    this->Gradients->SetComponent(num,1,grad[1]);
    this->Gradients->SetComponent(num,2,grad[2]);
    this->Modified();
    }
  else
    {
    vtkErrorMacro("Gradient number is out of range. Allocate first the number of gradients with SetNumberOfGradients");
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetGradients(vtkDoubleArray *grad)
{
  this->Gradients->DeepCopy(grad);
  this->NumberOfGradients = this->Gradients->GetNumberOfTuples();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::GetGradients(int num,double grad[3])
{  
  if(this->Gradients->GetNumberOfTuples()<num) 
    {
    grad[0]=this->Gradients->GetComponent(num,0);
    grad[1]=this->Gradients->GetComponent(num,1);
    grad[2]=this->Gradients->GetComponent(num,2);
    } 
  else 
    {
    vtkErrorMacro("Gradient number is out of range");
    }
}

//----------------------------------------------------------------------------
double *vtkMRMLDiffusionWeightedVolumeNode::GetGradients(int num)
{
  if(this->Gradients->GetNumberOfTuples()<num) 
    {
    return static_cast <double *> (this->Gradients->GetVoidPointer(num*3));
    }
  else
    {
    vtkErrorMacro("Gradient number is out of range");
    return NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetBValues(vtkDoubleArray *bValues)
{
  this->BValues->DeepCopy(bValues);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetBValues(int num, const double b)
{
  if (this->BValues->GetNumberOfTuples()<num)
    {
    this->BValues->SetValue(num,b);
    this->Modified();
    }
  else
    {
    vtkErrorMacro("B value number is out of range. Allocate first the number of gradients with SetNumberOfGradients");
    }
}

//----------------------------------------------------------------------------
double vtkMRMLDiffusionWeightedVolumeNode::GetBValues(int num)
{
  if (this->BValues->GetNumberOfTuples()<num)
    {
    return this->BValues->GetValue(num);
    }
  else
    {
    vtkErrorMacro("B value number is out of range");
    return 0;
    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionWeightedVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionWeightedVolumeNode *node = (vtkMRMLDiffusionWeightedVolumeNode *) anode;

  // Matrices
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      this->MeasurementFrame[i][j] = node->MeasurementFrame[i][j];
      }
    }
    
  if (this->Gradients)
    {
    this->Gradients->DeepCopy(node->Gradients);
    }
  if (this->BValues)
    {
    this->BValues->DeepCopy(node->BValues);
    }
  this->NumberOfGradients= node->NumberOfGradients;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "MeasurementFrame:\n";
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      os << indent << " " << this->MeasurementFrame[i][j];
      }
      os << indent << "\n";
    }
  os << "\n";

  os << "Gradients:\n";
  for (int g =0; g<this->Gradients->GetNumberOfTuples(); g++)
    {
    for(int j=0; j<3; j++) 
      {
      os << indent << " " << this->Gradients->GetComponent(g,j);
      }
    os<< indent << "\n";
    }
  os << "\n";
    
  os << "B-values:\n";
  
  for(int k=0; k<this->BValues->GetNumberOfTuples(); k++) 
    {
    os << indent << " " << this->BValues->GetValue(k);
    }
}
