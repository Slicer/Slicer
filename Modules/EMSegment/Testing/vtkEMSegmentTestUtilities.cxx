#include "vtkEMSegmentTestUtilities.h"
#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"

//
// This function checks to see if the image stored in standardFilename
// differs from resultData.  True is returned if the images differ,
// false is returned if they are identical.
bool ImageDiff(vtkImageData* resultData, std::string standardFilename)
{
  bool imagesDiffer = false;

  //
  // read segmentation result standard
  vtkITKArchetypeImageSeriesReader* standardReader = 
    vtkITKArchetypeImageSeriesScalarReader::New();
  standardReader->SetArchetype(standardFilename.c_str());
  standardReader->SetOutputScalarTypeToNative();
  standardReader->SetDesiredCoordinateOrientationToNative();
  standardReader->SetUseNativeOriginOn();
  try
  {
    standardReader->Update();
  }
  catch (...)
  {
    std::cerr << "Error reading standard image: " << std::endl;
    standardReader->Delete();
    return true;
  }

  //
  // compare image origin and spacing
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (resultData->GetSpacing()[i] != 
        standardReader->GetOutput()->GetSpacing()[i] ||
        resultData->GetOrigin()[i] != 
        standardReader->GetOutput()->GetOrigin()[i])
    {
      //
      // display spacing and origin info for resultData
      std::cerr << "Image spacing and/or origin does not match standard!" 
                << std::endl;
      std::cerr << "result origin: " 
                << resultData->GetOrigin()[0] << " "
                << resultData->GetOrigin()[1] << " "
                << resultData->GetOrigin()[2] << std::endl;
      std::cerr << "result spacing: " 
                << resultData->GetSpacing()[0] << " "
                << resultData->GetSpacing()[1] << " "
                << resultData->GetSpacing()[2] << std::endl;

      std::cerr << "Standard origin: " 
                << standardReader->GetOutput()->GetOrigin()[0] << " "
                << standardReader->GetOutput()->GetOrigin()[1] << " "
                << standardReader->GetOutput()->GetOrigin()[2] << std::endl;
      std::cerr << "Standard spacing: " 
                << standardReader->GetOutput()->GetSpacing()[0] << " "
                << standardReader->GetOutput()->GetSpacing()[1] << " "
                << standardReader->GetOutput()->GetSpacing()[2] << std::endl;
      imagesDiffer = true;
    }
  }
  if (!imagesDiffer)
  {
    std::cerr << "Result image origin and spacing match." << std::endl;
  }

  //
  // compare image voxels
  vtkImageMathematics* imageDifference = vtkImageMathematics::New();
  imageDifference->SetOperationToSubtract();
  imageDifference->SetInput1(resultData);
  imageDifference->SetInput2(standardReader->GetOutput());

  vtkImageAccumulate* differenceAccumulator = vtkImageAccumulate::New();
  differenceAccumulator->SetInputConnection(imageDifference->GetOutputPort());
  //differenceAccumulator->IgnoreZeroOn();
  differenceAccumulator->Update();
  
  //imagesDiffer = differenceAccumulator->GetVoxelCount() > 0;
  imagesDiffer = 
    differenceAccumulator->GetMin()[0] != 0.0 ||
    differenceAccumulator->GetMax()[0] != 0.0;
    
  if (imagesDiffer)
  {
    std::cerr << "((temporarily not) ignoring zero) Num / Min / Max / Mean difference = " 
              << differenceAccumulator->GetVoxelCount()  << " / "
              << differenceAccumulator->GetMin()[0]      << " / "
              << differenceAccumulator->GetMax()[0]      << " / "
              << differenceAccumulator->GetMean()[0]     << std::endl;
  }
  else
  {
    std::cerr << "Result image voxels match." << std::endl;
  }

  standardReader->Delete();
  imageDifference->Delete();
  differenceAccumulator->Delete();

  return imagesDiffer;
}
