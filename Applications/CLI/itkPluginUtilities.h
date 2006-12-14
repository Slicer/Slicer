#ifndef __itkPluginUtilities_h
#define __itkPluginUtilities_h

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkPluginFilterWatcher.h"
#include <vector>
#include <string>

namespace itk
{
  // Description:
  // Get the PixelType and ComponentType from fileName
  void GetImageType (std::string fileName,
                     ImageIOBase::IOPixelType &pixelType,
                     ImageIOBase::IOComponentType &componentType)
    {
      typedef itk::Image<unsigned char, 3> ImageType;
      itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
      imageReader->SetFileName(fileName.c_str());
      imageReader->UpdateOutputInformation();

      pixelType = imageReader->GetImageIO()->GetPixelType();
      componentType = imageReader->GetImageIO()->GetComponentType();
    }

  // Description:
  // Get the PixelTypes and ComponentTypes from fileNames
  void GetImageTypes (std::vector<std::string> fileNames,
                      std::vector<ImageIOBase::IOPixelType> &pixelTypes,
                      std::vector<ImageIOBase::IOComponentType> &componentTypes)
    {
    pixelTypes.clear();
    componentTypes.clear();

    // For each file, find the pixel and component type
    for (std::vector<std::string>::size_type i = 0; i < fileNames.size(); i++)
      {
      ImageIOBase::IOPixelType pixelType;
      ImageIOBase::IOComponentType componentType;

      GetImageType (fileNames[i],
                    pixelType,
                    componentType);
      pixelTypes.push_back(pixelType);  
      componentTypes.push_back(componentType);  
      }
    }
  
} // end namespace itk

#endif
