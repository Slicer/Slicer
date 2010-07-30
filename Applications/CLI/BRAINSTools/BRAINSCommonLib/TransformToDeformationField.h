#ifndef __TransformToDeformationField_h
#define __TransformToDeformationField_h
#include "itkIO.h"
#include "CrossOverAffineSystem.h"
// #include "itkImageRegionIteratorWithIndex.h"
#include <itkTransformToDeformationFieldSource.h>

/**
  * Go from any subclass of Transform, to the corresponding deformation field
  */
template< typename DeformationFieldPointerType, typename TransformPointerType >
DeformationFieldPointerType
TransformToDeformationField
  (itk::ImageBase< DeformationFieldPointerType::ObjectType::ImageDimension > *templateImage,
  TransformPointerType xfrm)
{
#if 1
  typedef typename DeformationFieldPointerType::ObjectType OutputType;
  typedef typename itk::TransformToDeformationFieldSource< OutputType, double >
  TodefType;
  typename TodefType::Pointer todef( TodefType::New() );
  todef->SetOutputParametersFromImage(templateImage);
  todef->SetTransform(xfrm);
  try
    {
    todef->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    throw err; // pass the buck up.
    }
  // copy image
  // itk::ImageRegionIterator<OutputType>
  //   from(todef->GetOutput(),todef->GetOutput()->GetLargestPossibleRegion()),
  //   to(deformation,deformation->GetLargestPossibleRegion());
  // for(from.GoToBegin(),to.GoToBegin();
  //     from != from.End() && to != to.End(); from++,to++)
  //   {
  //   to.Value() = from.Value();
  //   }
  return todef->GetOutput();
#else
  typedef typename TransformPointerType::ObjectType TransformType;
  typedef typename DeformationFieldPointerType::ObjectType
  TDeformationField;
  typedef typename TDeformationField::PixelType
  DeformationPixelType;

  typedef itk::ImageRegionIteratorWithIndex< TDeformationField > IteratorType;
  IteratorType it( deformation, deformation->GetBufferedRegion() );
  it.GoToBegin();
  while ( !it.IsAtEnd() )
    {
    typename TDeformationField::IndexType CurrentIndex = it.GetIndex();
    typename itk::Point< typename DeformationPixelType::ValueType,
                         DeformationPixelType::Dimension > IndexPhysicalLocation;
    deformation->TransformIndexToPhysicalPoint(CurrentIndex,
                                               IndexPhysicalLocation);
    // Need to copy because the types may not be the same.
    typename TransformType::InputPointType TransformIndexPhysicalLocation;
    for ( unsigned int curr_dim = 0;
          curr_dim < DeformationPixelType::Dimension;
          curr_dim++ )
      {
      TransformIndexPhysicalLocation[curr_dim] =
        IndexPhysicalLocation[curr_dim];
      }

    const typename TransformType::OutputPointType TransformedPhysicalLocation =
      xfrm->TransformPoint(TransformIndexPhysicalLocation);

    DeformationPixelType DisplacementInPhysicalSpace;
    for ( unsigned int curr_dim = 0;
          curr_dim < DeformationPixelType::Dimension;
          curr_dim++ )
      {
      DisplacementInPhysicalSpace[curr_dim] =
        static_cast< typename DeformationPixelType::ValueType >(
          TransformedPhysicalLocation[curr_dim] - IndexPhysicalLocation[curr_dim] );
      }
    it.Set(DisplacementInPhysicalSpace);
    ++it;
    }
#endif
}

#endif // TransformToDeformationField_h
