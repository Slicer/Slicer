/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGridTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLGridTransformNode.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkGridTransform.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLGridTransformNode);

//----------------------------------------------------------------------------
vtkMRMLGridTransformNode::vtkMRMLGridTransformNode()
{
  this->ReadWriteAsTransformToParent = 0;
  vtkNew<vtkGridTransform> warp;
  this->SetAndObserveTransformFromParent(warp.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLGridTransformNode::~vtkMRMLGridTransformNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkGridTransform* grid=NULL;

  if (this->ReadWriteAsTransformToParent)
    {
    grid=vtkGridTransform::SafeDownCast(GetTransformToParentAs("vtkGridTransform"));
    }
  else
    {
    grid=vtkGridTransform::SafeDownCast(GetTransformFromParentAs("vtkGridTransform"));
    }

  if (grid != NULL)
    {
    // this transform should be a grid transform
    of << " interpolationMode=\"" << grid->GetInterpolationMode() << "\" ";
    of << " displacementScale=\"" << grid->GetDisplacementScale() << "\" ";
    of << " displacementShift=\"" << grid->GetDisplacementShift() << "\" ";

    vtkImageData * image = grid->GetDisplacementGrid();
    int* N = image->GetDimensions();
    of << " dimension=\"" << N[0] << " " << N[1] << " " << N[2] << "\" ";
    double* spacing = image->GetSpacing();
    of << " spacing=\"" << spacing[0] << " " << spacing[1] << " " << spacing[2] << "\" ";    
    double* origin = image->GetOrigin();
    of << " origin=\"" << origin[0] << " " << origin[1] << " " << origin[2] << "\" ";
    }
  else
    {
    vtkErrorMacro("vtkMRMLGridTransformNode::WriteXML failed: the transform is not a vtkGridTransform");
    }

}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  vtkNew<vtkGridTransform> vtkgrid;
  vtkNew<vtkImageData> image;
  image->Initialize();
  image->SetNumberOfScalarComponents( 3 );
  image->SetScalarTypeToDouble();
  int num_of_displacement = 0;

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "interpolationMode"))
      {
      std::stringstream ss;
      ss << attValue;
      int val;
      if( ss >> val )
        {
        vtkgrid->SetInterpolationMode(val);
        }
      else
        {
        vtkErrorMacro( "couldn't parse grid interpolationMode" );
        return;
        }
      }
    else if (!strcmp(attName, "displacementScale"))
      {
      double val;
      std::stringstream ss;
      ss << attValue;
      if( ss >> val )
        {
        vtkgrid->SetDisplacementScale(val);
        }
      else
        {
        vtkErrorMacro( "couldn't parse grid displacementScale" );
        return;
        }
      }
    else if (!strcmp(attName, "displacementShift"))
      {
      double val;
      std::stringstream ss;
      ss << attValue;
      if( ss >> val )
        {
        vtkgrid->SetDisplacementShift(val);
        }
      else
        {
        vtkErrorMacro( "couldn't parse grid displacementShift" );
        return;
        }
      }
    else if (!strcmp(attName, "dimension"))
      {
      int val;
      std::stringstream ss;
      ss << attValue;
      std::vector<int> vals;
      while( ss >> val )
        {
        vals.push_back( val );
        }
      if( vals.size() != 3 )
        {
        vtkErrorMacro( "Incorrect number of dimension: expecting 3; got "
                       << vals.size() );
        return;
        }
      num_of_displacement = vals[0] * vals[1] * vals[2] * 3;
      image->SetDimensions( vals[0], vals[1], vals[2] );
      image->AllocateScalars();
      }
    else if (!strcmp(attName, "spacing"))
      {
      double val;
      std::stringstream ss;
      ss << attValue;
      std::vector<double> vals;
      while( ss >> val )
        {
        vals.push_back( val );
        }
      if( vals.size() != 3 )
        {
        vtkErrorMacro( "Incorrect number of spacing: expecting 3; got "
                       << vals.size() );
        return;
        }
      image->SetSpacing( vals[0], vals[1], vals[2] );
      }
    else if (!strcmp(attName, "origin"))
      {
      double val;
      std::stringstream ss;
      ss << attValue;
      std::vector<double> vals;
      while (ss >> val)
        {
        vals.push_back( val );
        }
      if (vals.size() != 3)
        {
        vtkErrorMacro( "Incorrect number of origin: expecting 3; got "
                       << vals.size() );
        return;
        }
      image->SetOrigin( vals[0], vals[1], vals[2] );
      }
    else if (!strcmp(attName, "displacement"))
      {
      if (num_of_displacement == 0)
        {
        vtkErrorMacro( "dimension attribute must be processed before displacement attributes" );
        return;
        }
      double val;
      std::stringstream ss;
      ss << attValue;
      std::vector<double> vals;
      while( ss >> val )
        {
        vals.push_back( val );
        }
      if ((int)(vals.size()) !=  num_of_displacement)
        {
        vtkErrorMacro( "Incorrect number of origin: expecting " << num_of_displacement << "; got "
                       << vals.size() );
        return;
        }
      double* dataPtr = reinterpret_cast<double*>(image->GetScalarPointer());
      std::vector<double>::const_iterator iter = vals.begin();
      for (int i=0; i<num_of_displacement; ++i, iter++, ++dataPtr )
        {
        *dataPtr = *iter;
        }
      }
    }

  vtkgrid->SetDisplacementGrid(image.GetPointer());

  if( vtkgrid.GetPointer() != 0 )
    {
    if (this->ReadWriteAsTransformToParent)
      {
      this->SetAndObserveTransformToParent(vtkgrid.GetPointer());
      }
    else
      {
      this->SetAndObserveTransformFromParent(vtkgrid.GetPointer());
      }
    }
  else
    {
    vtkErrorMacro("vtkMRMLGridTransformNode::ReadXML failed: could not create a vtkGridTransform");
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLGridTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

// End
