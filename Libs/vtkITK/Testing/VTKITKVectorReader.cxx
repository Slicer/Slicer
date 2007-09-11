#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <itksys/SystemTools.hxx>
#include <vtkITKArchetypeImageSeriesVectorReaderFile.h>
#include <vtkITKArchetypeImageSeriesVectorReaderSeries.h>
#include <vtkImageData.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
    std::cout << "ERROR: need to specify a file to try reading on the command line." << std::endl;
    return 1;
    }
  std::cout << "Trying to read file '" << argv[1] << "'" << std::endl;

  vtkITKArchetypeImageSeriesVectorReaderFile *vectorReader = vtkITKArchetypeImageSeriesVectorReaderFile::New();
  vectorReader->SetArchetype(argv[1]);
  vectorReader->SetOutputScalarTypeToNative();
  vectorReader->SetDesiredCoordinateOrientationToNative();

  try
    {
    vectorReader->Update();
    }
  catch (itk::ExceptionObject err)
    {
    std::cout << "Unable to read file '" << argv[1] << "', err = \n" << err << std::endl;
    vectorReader->Delete();
    return 1;
    }
  
  // now assign it to another image
  vtkImageData *imageData;

  imageData = vectorReader->GetOutput();
  if (imageData)
    {
    std::cout << "Read file, image data has " << imageData->GetNumberOfPoints() << " points" << std::endl;
    }
  else
    {
    std::cout << "ERROR: image data is null from vector reader" << std::endl;
    vectorReader->Delete();
    return 1;
    }
  
  std::cout << "Deleting vector reader" << std::endl;
  
  vectorReader->Delete();
  
  return 0;
}
