// Copied from dtiprocess
// available there: http://www.nitrc.org/projects/dtiprocess/
/*=========================================================================

  Program:   NeuroLib (DTI command line tools)
  Language:  C++
  Date:      $Date: 2010/03/09 18:01:02 $
  Version:   $Revision: 1.2 $
  Author:    Casey Goodlett (gcasey@sci.utah.edu)

  Copyright (c)  Casey Goodlett. All rights reserved.
  See NeuroLibCopyright.txt or http://www.ia.unc.edu/dev/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef DTITYPES_H
#define DTITYPES_H

// ITK Data types
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkVector.h>
#include <itkCovariantVector.h>
#include <itkDiffusionTensor3D.h>
#include <itkAffineTransform.h>
#include <itkDTITubeSpatialObject.h>
#include <itkGroupSpatialObject.h>
#include <itkRGBPixel.h>
#include <itkVectorImage.h>

// VNL Includes
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector_fixed.h>

// Define necessary types for images
typedef double        RealType;
typedef double        TransformRealType;
typedef unsigned char LabelType;
const unsigned int DIM = 3;

typedef unsigned short                  ScalarPixelType;
typedef itk::DiffusionTensor3D<double>  TensorPixelType;
typedef itk::Vector<double, 3>          DeformationPixelType;
typedef itk::CovariantVector<double, 3> GradientPixelType;

typedef itk::VectorImage<ScalarPixelType, DIM> VectorImageType;
typedef itk::Image<TensorPixelType, DIM>       TensorImageType;

typedef itk::Image<DeformationPixelType, DIM> DeformationImageType;
typedef itk::Image<GradientPixelType, DIM>    GradientImageType;

typedef itk::Image<RealType, DIM>                   RealImageType;
typedef itk::Image<ScalarPixelType, DIM>            IntImageType;
typedef itk::Image<LabelType, DIM>                  LabelImageType;
typedef itk::Image<itk::RGBPixel<unsigned char>, 3> RGBImageType;

typedef TensorImageType::SizeType    ImageSizeType;
typedef TensorImageType::SpacingType ImageSpacingType;

typedef itk::AffineTransform<TransformRealType, 3> AffineTransformType;

typedef vnl_vector_fixed<double, 3>                      GradientType;
typedef itk::VectorContainer<unsigned int, GradientType> GradientListType;

// enum InterpolationType {NearestNeighbor, Linear, Cubic};
// enum TensorReorientationType {FiniteStrain, PreservationPrincipalDirection};

// enum EigenValueIndex {Lambda1 = 0, Lambda2, Lambda3};

// typedef itk::DTITubeSpatialObject<3> DTITubeType;
// typedef DTITubeType::TubePointType DTIPointType;
// typedef DTITubeType::PointListType DTIPointListType;

// typedef itk::GroupSpatialObject<3> GroupType;
// typedef GroupType::ChildrenListType ChildrenListType;

#endif
