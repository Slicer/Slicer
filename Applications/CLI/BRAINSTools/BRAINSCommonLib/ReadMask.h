#ifndef __ReadMask_h
#define __ReadMask_h

#include "itkBrains2MaskImageIO.h"
#include "itkBrains2MaskImageIOFactory.h"
#include "itkIO.h"
#include "itkImageMaskSpatialObject.h"

inline
void RegisterBrains2MaskFactory()
{
  itk::ObjectFactoryBase::RegisterFactory( itk::Brains2MaskImageIOFactory::New() );
}

template< class MaskType, unsigned VDimension >
typename MaskType::Pointer
ReadImageMask(const std::string & filename,
              typename itk::ImageBase< VDimension > *referenceImage)
{
  typedef unsigned char                                    MaskPixelType;
  typedef typename itk::Image< MaskPixelType, VDimension > MaskImageType;
  typename MaskImageType::Pointer OrientedMaskImage = NULL;

  if ( itksys::SystemTools::GetFilenameLastExtension(filename) == ".mask" )
    {
    // HACK:  THIS ASSUMES THAT THE MASK IS A BRAINS2 mask with improper
    // orientation and origin, and then proceeds to
    //       force the origin and direction orientation.
    //       The test suite should be re-written so that only if the brains2
    // read in the mask
    OrientedMaskImage = itkUtil::ReadImageAndOrient< MaskImageType >( filename,
                                                                      referenceImage->GetDirection() );
    OrientedMaskImage->SetOrigin( referenceImage->GetOrigin() );
    // TODO: Should also check that spacing is the same;
    }
  else
    {
    OrientedMaskImage = itkUtil::ReadImage< MaskImageType >(filename);
    // TODO:  May want to check that physical spaces overlap?
    }
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
