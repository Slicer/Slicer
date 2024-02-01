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
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLLinearTransformNode);

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode::vtkMRMLLinearTransformNode()
{
  vtkNew<vtkMatrix4x4> matrix;
  this->SetMatrixTransformToParent(matrix.GetPointer());

  this->CenterOfTransformation[0] = 0.0;
  this->CenterOfTransformation[1] = 0.0;
  this->CenterOfTransformation[2] = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode::~vtkMRMLLinearTransformNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->IsLinear())
    {
    // Only write the matrix to the scene if the object stores a linear transform
    vtkNew<vtkMatrix4x4> matrix;
    this->GetMatrixTransformToParent(matrix.GetPointer());

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
    of << " matrixTransformToParent=\"" << ss.str() << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::ReadXMLAttributes(const char** atts)
{
  // Temporarily disable all Modified and TransformModified events to make sure that
  // the operations are performed without interruption.
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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

    // For backward compatibility only (because readWriteAsTransformToParent
    // is not present anymore in current scenes, because transforms are always
    // written as TransformFromParent)
    if (!strcmp(attName, "readWriteAsTransformToParent"))
      {
      // There was a bug in the scene writing for linear transforms
      // which caused readWriteAsTransformToParent to be written incorrectly.
      // We correct it here by setting ReadAsToParent to 0 if readWriteAsTransformToParent is true
      // In the long term (when backward compatibility with old scenes is not a strong requirement
      // anymore) vtkMRMLLinearTransformNode and readWriteAsTransformToParent attribute management
      // can be completely removed.
      if (!strcmp(attValue,"true"))
        {
        this->ReadAsTransformToParent = 0;
        }
      else
        {
        this->ReadAsTransformToParent = 1;
        }
      }

    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  if (this->IsLinear())
    {
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
}
