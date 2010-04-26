#ifndef TransformToDeformationField_h
#define TransformToDeformationField_h
#include "itkIO.h"
#include "CrossOverAffineSystem.h"
#include "itkImageRegionIteratorWithIndex.h"

/**
 * Go from any subclass of Transform, to the corresponding deformation field
 */
template <typename DeformationFieldPointerType, typename TransformPointerType>
void
TransformToDeformationField (
  DeformationFieldPointerType deformation,
  TransformPointerType xfrm)
{
  typedef typename TransformPointerType::ObjectType TransformType;
  typedef typename DeformationFieldPointerType::ObjectType
  TDeformationField;
  typedef typename TDeformationField::PixelType
  DeformationPixelType;

  typedef itk::ImageRegionIteratorWithIndex<TDeformationField> IteratorType;
  IteratorType it( deformation, deformation->GetBufferedRegion() );
  it.GoToBegin();
  while ( !it.IsAtEnd() )
    {
    typename TDeformationField::IndexType CurrentIndex = it.GetIndex();
    typename itk::Point<typename DeformationPixelType::ValueType,
      DeformationPixelType::Dimension> IndexPhysicalLocation;
    deformation->TransformIndexToPhysicalPoint(CurrentIndex,
      IndexPhysicalLocation);
    // Need to copy because the types may not be the same.
    typename TransformType::InputPointType TransformIndexPhysicalLocation;
    for ( unsigned int curr_dim = 0;
          curr_dim < DeformationPixelType::Dimension;
          curr_dim++ )
      {
      TransformIndexPhysicalLocation[curr_dim]
        = IndexPhysicalLocation[curr_dim];
      }

    const typename TransformType::OutputPointType TransformedPhysicalLocation
      = xfrm->TransformPoint(TransformIndexPhysicalLocation);

    DeformationPixelType DisplacementInPhysicalSpace;
    for ( unsigned int curr_dim = 0;
          curr_dim < DeformationPixelType::Dimension;
          curr_dim++ )
      {
      DisplacementInPhysicalSpace[curr_dim]
        = static_cast<typename DeformationPixelType::ValueType>(
        TransformedPhysicalLocation[curr_dim] - IndexPhysicalLocation[curr_dim] );
      }
    it.Set(DisplacementInPhysicalSpace);
    ++it;
    }
}

#if 0
template <typename TDeformationField>
void
AffineTransformToDeformationField
(
  typename TDeformationField::Pointer deformation,
  typename CrossOverAffineSystem<double, 3>::AffineTransformPointer & xfrm)
{
  typedef typename TDeformationField::PixelType DeformationPixelType;
  TransformToDeformationField
  <TDeformationField,
    typename CrossOverAffineSystem<typename DeformationPixelType::ValueType,
      DeformationPixelType::Dimension>::AffineTransformType>
                                 (deformation, xfrm);
}

#endif

#endif // TransformToDeformationField_h
