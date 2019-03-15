/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/


#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLScene.h"

#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <vnl/vnl_double_3.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDiffusionWeightedVolumeNode);

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode::vtkMRMLDiffusionWeightedVolumeNode() :
  DiffusionGradients(vtkDoubleArray::New()),
  BValues(vtkDoubleArray::New())
{
  this->DiffusionGradients->SetNumberOfComponents(3);
  this->SetNumberOfGradientsInternal(7); //6 gradients + 1 baseline

  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j] = (i == j) ? 1.0 : 0.0;
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode::~vtkMRMLDiffusionWeightedVolumeNode()
{
  this->DiffusionGradients->Delete();
  this->BValues->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  std::stringstream ss;
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      ss << this->MeasurementFrameMatrix[i][j] << " ";
      if ( i != 2 && j != 2 )
        {
        ss << "  ";
        }
      }
    }
    of << " measurementFrameMatrix=\"" << ss.str() << "\"";

  ss.clear();

  for(int g=0; g<this->DiffusionGradients->GetNumberOfTuples(); g++)
    {
    for (int k=0; k<3; k++)
      {
      ss << this->DiffusionGradients->GetComponent(g,k) << " ";
      }
    }

  of << " gradients=\"" << ss.str() << "\"";

  ss.clear();

  for (int g=0; g<this->BValues->GetNumberOfTuples(); g++)
    {
    ss << this->BValues->GetValue(g) << " ";
    }
  of << " bValues=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "measurementFrameMatrix"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        for(int j=0; j<3; j++)
          {
          ss >> val;
          this->MeasurementFrameMatrix[i][j] = val;
          }
        }
      }
    if (!strcmp(attName, "gradients"))
      {
      std::stringstream ss;
      ss << attValue;
      double g[3];
      this->DiffusionGradients->Reset();
      this->DiffusionGradients->SetNumberOfComponents(3);
      while (!ss.eof())
        {
        for (int i=0; i<3; i++)
          {
          ss >> g[i];
          }
        this->DiffusionGradients->InsertNextTuple(g);
        }
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

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetMeasurementFrameMatrix(const double mf[3][3])
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j] = mf[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::GetMeasurementFrameMatrix(double mf[3][3])
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      mf[i][j] = this->MeasurementFrameMatrix[i][j];
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetMeasurementFrameMatrix(
    const double xr, const double xa, const double xs,
    const double yr, const double ya, const double ys,
    const double zr, const double za, const double zs)
{
  MeasurementFrameMatrix[0][0] = xr;
  MeasurementFrameMatrix[0][1] = xa;
  MeasurementFrameMatrix[0][2] = xs;
  MeasurementFrameMatrix[1][0] = yr;
  MeasurementFrameMatrix[1][1] = ya;
  MeasurementFrameMatrix[1][2] = ys;
  MeasurementFrameMatrix[2][0] = zr;
  MeasurementFrameMatrix[2][1] = za;
  MeasurementFrameMatrix[2][2] = zs;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetMeasurementFrameMatrix(vtkMatrix4x4 *mf)
{
  if (!mf)
    {
    return;
    }
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j]=mf->GetElement(i,j);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::GetMeasurementFrameMatrix(vtkMatrix4x4 *mf)
{
  if (!mf)
    {
    return;
    }
  mf->Identity();
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      mf->SetElement(i,j,this->MeasurementFrameMatrix[i][j]);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetNumberOfGradients(int val)
{
  if (this->GetNumberOfGradients() == val)
    {
    return;
    }
  this->SetNumberOfGradientsInternal(val);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetNumberOfGradientsInternal(int val)
{
  vtkDebugMacro(<< "setting num gradients to " << val);
  this->DiffusionGradients->Reset();
  this->BValues->Reset();
  // internal array for storage of gradient vectors
  this->DiffusionGradients->SetNumberOfTuples(val);
  this->BValues->SetNumberOfTuples(val);
  for (int tupleIdx = 0; tupleIdx < val; ++tupleIdx)
    {
    for (int componentIdx = 0;
         componentIdx < this->DiffusionGradients->GetNumberOfComponents();
         ++componentIdx)
      {
      this->DiffusionGradients->SetComponent(tupleIdx, componentIdx, 0.0);
      }
    this->BValues->SetValue(tupleIdx, 0.0);
    }
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionWeightedVolumeNode::GetNumberOfGradients()
{
  return this->DiffusionGradients->GetNumberOfTuples();
}

//------------------------------------------------------------------------------

inline bool valid_grad_length(vnl_double_3 grad) {
  // returns true if grad length is: within GRAD_EPS of 0.0 or 1.0
  return (grad.two_norm() < 1e-6) || (fabs(1.0 - grad.two_norm()) < 1e-6);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetDiffusionGradient(int num, const double grad[3])
{
  if ((num < 0) ||
      (num >= this->DiffusionGradients->GetNumberOfTuples()))
    {
    vtkErrorMacro(<< "Gradient number is out of range. "
                     "Allocate first the number of gradients with SetNumberOfGradients");
    return;
    }

  vnl_double_3 tmp_grad(grad[0], grad[1], grad[2]);
  if (!valid_grad_length(tmp_grad))
    {
    vtkErrorMacro(<< "vtkMRMLDiffusionWeightedVolumeNode only accepts gradient vectors with length 0.0 or 1.0!"
                  << "  Got vector with length: " << tmp_grad.two_norm());
    return;
    }

  this->DiffusionGradients->SetTuple3(num, grad[0], grad[1], grad[2]);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetDiffusionGradients(vtkDoubleArray *grad)
{
  // gradients must all be length 0 (baseline) or 1.
  vnl_double_3 tmp_grad;
  for (int i = 0; i < grad->GetNumberOfTuples(); i++)
    {
    tmp_grad.copy_in(grad->GetTuple3(i));
    if (!valid_grad_length(tmp_grad))
      {
      vtkErrorMacro(<< "vtkMRMLDiffusionWeightedVolumeNode only accepts gradient vectors with length 0.0 or 1.0!"
                    << " Got vector with length: " << tmp_grad.two_norm());
      return;
      }
    }

  this->DiffusionGradients->DeepCopy(grad);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::GetDiffusionGradient(int num,double grad[3])
{
  if (num < 0 || num >= this->DiffusionGradients->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "Gradient number is out of range.");
    return;
    }
  grad[0]=this->DiffusionGradients->GetComponent(num,0);
  grad[1]=this->DiffusionGradients->GetComponent(num,1);
  grad[2]=this->DiffusionGradients->GetComponent(num,2);
}

//----------------------------------------------------------------------------
double *vtkMRMLDiffusionWeightedVolumeNode::GetDiffusionGradient(int num)
{
  if (num < 0 || num >= this->DiffusionGradients->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "Gradient number is out of range.");
    return nullptr;
    }
  return static_cast <double *> (this->DiffusionGradients->GetVoidPointer(num*3));
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetBValues(vtkDoubleArray *bValues)
{
  this->BValues->DeepCopy(bValues);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::SetBValue(int num, const double b)
{
  if (num < 0 || num >= this->BValues->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "B value number is out of range. "
                     "Allocate first the number of gradients with SetNumberOfGradients");
    return;
    }
  this->BValues->SetValue(num,b);
  this->Modified();
}

//----------------------------------------------------------------------------
double vtkMRMLDiffusionWeightedVolumeNode::GetBValue(int num)
{
  if (num < 0 || num >= this->BValues->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "B value number is out of range.");
    return 0;
    }
  return this->BValues->GetValue(num);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionWeightedVolumeNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDiffusionWeightedVolumeNode *node = (vtkMRMLDiffusionWeightedVolumeNode *) anode;

  // Matrices
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j] = node->MeasurementFrameMatrix[i][j];
      }
    }

  if (this->DiffusionGradients)
    {
    this->DiffusionGradients->DeepCopy(node->DiffusionGradients);
    }
  if (this->BValues)
    {
    this->BValues->DeepCopy(node->BValues);
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "MeasurementFrameMatrix:\n";
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      os << indent << " " << this->MeasurementFrameMatrix[i][j];
      }
      os << indent << "\n";
    }
  os << "\n";

  os << "Gradients:\n";
  for (int g =0; g < this->DiffusionGradients->GetNumberOfTuples(); g++)
    {
    for(int j=0; j < this->DiffusionGradients->GetNumberOfComponents(); j++)
      {
      os << indent << " " << this->DiffusionGradients->GetComponent(g,j);
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

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeDisplayNode* vtkMRMLDiffusionWeightedVolumeNode::GetDiffusionWeightedVolumeDisplayNode()
{
  return vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLDiffusionWeightedVolumeNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLNRRDStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkMRMLDiffusionWeightedVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLDiffusionWeightedVolumeDisplayNode* dispNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLDiffusionWeightedVolumeDisplayNode") );
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
