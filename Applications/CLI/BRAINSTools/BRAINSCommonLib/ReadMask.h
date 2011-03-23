#ifndef __ReadMask_h
#define __ReadMask_h


#include "itkIO.h"
#include "itkImageMaskSpatialObject.h"

template< class MaskType, unsigned VDimension >
typename MaskType::Pointer
ReadImageMask(const std::string & filename,
              typename itk::ImageBase< VDimension > * /*referenceImage*/)
{
  typedef unsigned char                                    MaskPixelType;
  typedef typename itk::Image< MaskPixelType, VDimension > MaskImageType;
  typename MaskImageType::Pointer OrientedMaskImage = NULL;

  OrientedMaskImage = itkUtil::ReadImage< MaskImageType >(filename);
  // TODO:  May want to check that physical spaces overlap?

  // convert mask image to mask
  typedef typename itk::ImageMaskSpatialObject< VDimension >
  ImageMaskSpatialObjectType;
  typename ImageMaskSpatialObjectType::Pointer mask =
    ImageMaskSpatialObjectType::New();
  mask->SetImage(OrientedMaskImage);
  //
  mask->ComputeObjectToWorldTransform();
  // return pointer to mask
  typename MaskType::Pointer p = dynamic_cast< MaskType * >( mask.GetPointer() );
  if ( p.IsNull() )
    {
    std::cout << "ERROR::" << __FILE__ << " " << __LINE__ << std::endl;
    exit(-1);
    }
  return p;
}

#endif // LoadMask_h
