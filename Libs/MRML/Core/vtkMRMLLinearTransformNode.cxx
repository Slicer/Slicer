/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLLinearTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLLinearTransformNode);

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode::vtkMRMLLinearTransformNode()
{
  this->CachedMatrixTransformToParent=vtkMatrix4x4::New();
  this->CachedMatrixTransformFromParent=vtkMatrix4x4::New();

  vtkNew<vtkMatrix4x4> matrix;
  this->SetMatrixTransformToParent(matrix.GetPointer());

}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode::~vtkMRMLLinearTransformNode()
{
  this->CachedMatrixTransformToParent->Delete();
  this->CachedMatrixTransformToParent=NULL;
  this->CachedMatrixTransformFromParent->Delete();
  this->CachedMatrixTransformFromParent=NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  vtkNew<vtkMatrix4x4> matrix;
  GetMatrixTransformToParent(matrix.GetPointer());

  std::stringstream ss;
  for (int row=0; row<4; row++)
    {
    for (int col=0; col<4; col++)
      {
      ss << matrix->GetElement(row, col);
      if (!(row==3 && col==3))
        {
        ss << " ";
        }
      }
    if ( row != 3 )
      {
      ss << " ";
      }
    }
  of << indent << " matrixTransformToParent=\"" << ss.str() << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::ReadXMLAttributes(const char** atts)
{
  // Temporarily disable all Modified and TransformModified events to make sure that
  // the operations are performed without interruption.
  int oldTransformModify=this->StartTransformModify();
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "matrixTransformToParent"))
      {
      vtkNew<vtkMatrix4x4> matrix;
      std::stringstream ss;
      double val;
      ss << attValue;
      for (int row=0; row<4; row++)
        {
        for (int col=0; col<4; col++)
          {
          ss >> val;
          matrix->SetElement(row, col, val);
          }
        }
      this->SetMatrixTransformToParent(matrix.GetPointer());
      }
    if (!strcmp(attName, "matrixTransformFromParent"))
      {
      vtkNew<vtkMatrix4x4> matrix;
      std::stringstream ss;
      double val;
      ss << attValue;
      for (int row=0; row<4; row++)
        {
        for (int col=0; col<4; col++)
          {
          ss >> val;
          matrix->SetElement(row, col, val);
          }
        }
      this->SetMatrixTransformFromParent(matrix.GetPointer());
      }

    }

  // For legacy scenes only
  // Old scene files (that still had readWriteAsTransformToParent) was saved incorrectly for linear transforms:
  // the transform file was always transformFromParent, regardless of the readWriteAsTransformToParent value.
  // We set it this->ReadAsTransformToParent accordingly (to maintain compatibility with earlier scenes).
  this->ReadAsTransformToParent = 0;

  this->EndModify(disabledModify);
  this->EndTransformModify(oldTransformModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLLinearTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkNew<vtkMatrix4x4> toParentMatrix;
  this->GetMatrixTransformToParent(toParentMatrix.GetPointer());

  os << indent << "MatrixTransformToParent: " << "\n";
  for (int row=0; row<4; row++)
    {
    for (int col=0; col<4; col++)
      {
      os << toParentMatrix->GetElement(row, col);
      if (!(row==3 && col==3))
        {
        os << " ";
        }
      else
        {
        os << "\n";
        }
      } // for (int col
    } // for (int row
}


//----------------------------------------------------------------------------
int vtkMRMLLinearTransformNode::GetMatrixTransformToParent(vtkMatrix4x4* matrix)
{
  if (matrix==NULL)
    {
    vtkErrorMacro("vtkMRMLLinearTransformNode::GetMatrixTransformToParent failed: matrix is invalid");
    return 0;
    }
  vtkTransform* transform=vtkTransform::SafeDownCast(GetTransformToParentAs("vtkTransform"));
  if (transform==NULL)
    {
    matrix->Identity();
    return 0;
    }
  transform->GetMatrix(matrix);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLLinearTransformNode::GetMatrixTransformFromParent(vtkMatrix4x4* matrix)
{
  vtkNew<vtkMatrix4x4> transformToParentMatrix;
  int result = GetMatrixTransformToParent(transformToParentMatrix.GetPointer());
  vtkMatrix4x4::Invert(transformToParentMatrix.GetPointer(), matrix);
  return result;
}

//----------------------------------------------------------------------------
int  vtkMRMLLinearTransformNode::GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld)
{
  if (this->IsTransformToWorldLinear() != 1)
    {
    vtkWarningMacro("Failed to retrieve matrix to world from transform, the requested transform is not linear");
    transformToWorld->Identity();
    return 0;
    }

  // vtkMatrix4x4::Multiply4x4 computes the result in a separate buffer, so it is safe to use the input as output as well
  vtkNew<vtkMatrix4x4> matrixTransformToParent;
  if (!this->GetMatrixTransformToParent(matrixTransformToParent.GetPointer()))
    {
    vtkErrorMacro("Failed to retrieve matrix from linear transform");
    transformToWorld->Identity();
    return 0;
    }
  vtkMatrix4x4::Multiply4x4(matrixTransformToParent.GetPointer(), transformToWorld, transformToWorld);

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL)
    {
    vtkMRMLLinearTransformNode *lparent = vtkMRMLLinearTransformNode::SafeDownCast(parent);
    if (lparent)
      {
      return (lparent->GetMatrixTransformToWorld(transformToWorld));
      }
    else
      {
      vtkErrorMacro("vtkMRMLLinearTransformNode::GetMatrixTransformToWorld failed: expected parent linear transform");
      transformToWorld->Identity();
      return 0;
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int  vtkMRMLLinearTransformNode::GetMatrixTransformToNode(vtkMRMLTransformNode* node,
                                                          vtkMatrix4x4* transformToNode)
{
  if (node == NULL)
    {
    return this->GetMatrixTransformToWorld(transformToNode);
    }
  if (this->IsTransformToNodeLinear(node) != 1)
    {
    transformToNode->Identity();
    return 0;
    }

  if (this->IsTransformNodeMyParent(node))
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    vtkNew<vtkMatrix4x4> toParentMatrix;
    this->GetMatrixTransformToParent(toParentMatrix.GetPointer());
    if (parent != NULL)
      {
      vtkMatrix4x4::Multiply4x4(toParentMatrix.GetPointer(), transformToNode, transformToNode);
      if (strcmp(parent->GetID(), node->GetID()) )
        {
        this->GetMatrixTransformToNode(node, transformToNode);
        }
      }
    else
      {
      vtkMatrix4x4::Multiply4x4(toParentMatrix.GetPointer(), transformToNode, transformToNode);
      }
    }
  else if (this->IsTransformNodeMyChild(node))
    {
    vtkMRMLLinearTransformNode *lnode = dynamic_cast <vtkMRMLLinearTransformNode *> (node);
    vtkMRMLLinearTransformNode *parent = dynamic_cast <vtkMRMLLinearTransformNode *> (node->GetParentTransformNode());
    vtkNew<vtkMatrix4x4> toParentMatrix;
    lnode->GetMatrixTransformToParent(toParentMatrix.GetPointer());
    if (parent != NULL)
      {
      vtkMatrix4x4::Multiply4x4(toParentMatrix.GetPointer(), transformToNode, transformToNode);
      if (strcmp(parent->GetID(), this->GetID()) )
        {
        this->GetMatrixTransformToNode(this, transformToNode);
        }
      }
    else
      {
      vtkMatrix4x4::Multiply4x4(toParentMatrix.GetPointer(), transformToNode, transformToNode);
      }
    }
  else
    {
    this->GetMatrixTransformToWorld(transformToNode);
    vtkNew<vtkMatrix4x4> transformToWorld2;

    node->GetMatrixTransformToWorld(transformToWorld2.GetPointer());
    transformToWorld2->Invert();

    vtkMatrix4x4::Multiply4x4(transformToWorld2.GetPointer(), transformToNode, transformToNode);
    }
  return 1;
}


//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::SetMatrixTransformToParent(vtkMatrix4x4 *matrix)
{
  // Temporarily disable all Modified and TransformModified events to make sure that
  // the operations are performed without interruption.
  int oldTransformModify=this->StartTransformModify();
  int oldModify=this->StartModify();

  vtkTransform* currentTransform = NULL;
  if (this->TransformToParent!=NULL)
    {
    currentTransform = vtkTransform::SafeDownCast(GetTransformToParentAs("vtkTransform"));
    }

  if (currentTransform)
    {
    // Reset InverseFlag (in case an external module has changed it)
    if (currentTransform->GetInverseFlag())
      {
      currentTransform->Inverse();
      }
    // Set matrix
    if (matrix)
      {
      currentTransform->SetMatrix(matrix);
      }
    else
      {
      currentTransform->Identity();
      }
    }
  else
    {
    // Transform does not exist or not the right type, replace it with a new one
    vtkNew<vtkTransform> transform;
    if (matrix)
      {
      transform->SetMatrix(matrix);
      }
    this->SetAndObserveTransformToParent(transform.GetPointer());
    }

  this->TransformToParent->Modified();
  this->EndModify(oldModify);
  this->EndTransformModify(oldTransformModify);
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::SetMatrixTransformFromParent(vtkMatrix4x4 *matrix)
{
  vtkNew<vtkMatrix4x4> inverseMatrix;
  vtkMatrix4x4::Invert(matrix, inverseMatrix.GetPointer());
  SetMatrixTransformToParent(inverseMatrix.GetPointer());
}

//----------------------------------------------------------------------------
bool vtkMRMLLinearTransformNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  if (transformMatrix==NULL)
    {
    vtkErrorMacro("vtkMRMLLinearTransformNode::ApplyTransformMatrix failed: input transform is invalid");
    return;
    }
  // vtkMatrix4x4::Multiply4x4 computes the output in an internal buffer and then
  // copies the result to the output matrix, therefore it is safe to use
  // one of the input matrices as output
  vtkNew<vtkMatrix4x4> matrixToParent;
  this->GetMatrixTransformToParent(matrixToParent.GetPointer());
  vtkMatrix4x4::Multiply4x4(transformMatrix, matrixToParent.GetPointer(), matrixToParent.GetPointer());
  SetMatrixTransformToParent(matrixToParent.GetPointer());
}

// Deprecated methods, kept temporarily for compatibility with extensions that are not yet updated

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent(vtkMatrix4x4 *matrix)
{
  vtkWarningMacro("vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent method is deprecated. Use vtkMRMLLinearTransformNode::SetMatrixTransformToParent instead");
  SetMatrixTransformToParent(matrix);
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformFromParent(vtkMatrix4x4 *matrix)
{
  vtkWarningMacro("vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformFromParent method is deprecated. Use vtkMRMLLinearTransformNode::SetMatrixTransformFromParent instead");
  SetMatrixTransformFromParent(matrix);
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLLinearTransformNode::GetMatrixTransformToParent()
{
  vtkWarningMacro("vtkMRMLLinearTransformNode::GetMatrixTransformToParent() method is deprecated. Use vtkMRMLLinearTransformNode::GetMatrixTransformToParent(vtkMatrix4x4*) instead");
  GetMatrixTransformToParent(this->CachedMatrixTransformToParent);
  return this->CachedMatrixTransformToParent;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLLinearTransformNode::GetMatrixTransformFromParent()
{
  vtkWarningMacro("vtkMRMLLinearTransformNode::GetMatrixTransformFromParent() method is deprecated. Use vtkMRMLLinearTransformNode::GetMatrixTransformFromParent(vtkMatrix4x4*) instead");
  GetMatrixTransformFromParent(this->CachedMatrixTransformFromParent);
  return this->CachedMatrixTransformFromParent;
}
