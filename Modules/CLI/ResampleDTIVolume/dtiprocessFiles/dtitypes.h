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
using RealType = double;
using TransformRealType = double;
using LabelType = unsigned char;
const unsigned int DIM = 3;

using ScalarPixelType = unsigned short;
using TensorPixelType = itk::DiffusionTensor3D<double>;
using DeformationPixelType = itk::Vector<double, 3>;
using GradientPixelType = itk::CovariantVector<double, 3>;

using VectorImageType = itk::VectorImage<ScalarPixelType, DIM>;
using TensorImageType = itk::Image<TensorPixelType, DIM>;

using DeformationImageType = itk::Image<DeformationPixelType, DIM>;
using GradientImageType = itk::Image<GradientPixelType, DIM>;

using RealImageType = itk::Image<RealType, DIM>;
using IntImageType = itk::Image<ScalarPixelType, DIM>;
using LabelImageType = itk::Image<LabelType, DIM>;
using RGBImageType = itk::Image<itk::RGBPixel<unsigned char>, 3>;

using ImageSizeType = TensorImageType::SizeType;
using ImageSpacingType = TensorImageType::SpacingType;

using AffineTransformType = itk::AffineTransform<TransformRealType, 3>;

using GradientType = vnl_vector_fixed<double, 3>;
using GradientListType = itk::VectorContainer<unsigned int, GradientType>;

// enum InterpolationType {NearestNeighbor, Linear, Cubic};
// enum TensorReorientationType {FiniteStrain, PreservationPrincipalDirection};

// enum EigenValueIndex {Lambda1 = 0, Lambda2, Lambda3};

// typedef itk::DTITubeSpatialObject<3> DTITubeType;
// typedef DTITubeType::TubePointType DTIPointType;
// typedef DTITubeType::PointListType DTIPointListType;

// typedef itk::GroupSpatialObject<3> GroupType;
// typedef GroupType::ChildrenListType ChildrenListType;

#endif
