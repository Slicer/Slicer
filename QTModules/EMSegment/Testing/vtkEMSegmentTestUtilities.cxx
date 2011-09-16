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


// returns the overlap rate in %
double CompareTwoVolumes ( vtkImageData* Volume1, vtkImageData* Volume2 , int Flag ) {
  // 1. Subtract the two volumes form each other

  vtkImageMathematics* MathImg = vtkImageMathematics::New();
  MathImg->SetInput1(Volume1);
  MathImg->SetInput2(Volume2);
  MathImg->SetOperationToSubtract();
  MathImg->Update();

  int bins = 100;
  // 2. Find out if there are any differences

  vtkImageAccumulate* histo = vtkImageAccumulate::New();

  double* Extrema = GenerateHistogram(histo, MathImg->GetOutput(), bins);

  double error_rate = -1;

  int min = Extrema[0];
  int max = Extrema[1];
  if (Flag) {
    if (min == 0 && max == 0) {
      std::cout << "OK" << std::endl;
      error_rate = 100.0;
    } else {
      std::cout << "Failed: Minimum(" << min << ") Maximum(" << max <<")" << std::endl;
    }
  } else {
    if ( max - min > 100) {
      bins = int(max - min);
      histo->SetComponentExtent(0, bins, 0, 0, 0, 0);
    }
    histo->SetComponentSpacing(double(max - min)/double(bins), 0,0);
    histo->Update();

    //    PrintHistogram->histo();
    double spacing = histo->GetComponentSpacing()[0];
    double origin = Extrema[2];
    int VoxelError = 0;
    vtkImageData* data = histo->GetOutput();

    for (int i=0; i <= bins; i++) {
      if (double(i) * spacing + origin != 0.0) {
        VoxelError += int( data->GetScalarComponentAsFloat(i, 0, 0, 0) );
      }
    }
    error_rate = double(VoxelError)/double(histo->GetVoxelCount())*100.0;
    std::cerr << "Sum: " << VoxelError << std::endl;
    std::cerr << "Rate(%): " << error_rate << std::endl;
  }
  histo->Delete();
  MathImg->Delete();
  delete [] Extrema;

  return error_rate;
}

double* GenerateHistogram (vtkImageAccumulate* Histogram, vtkImageData* InputVolume, int bins) {

  Histogram->SetInput(InputVolume);
  Histogram->Update();
  int min = Histogram->GetMin()[0];
  int max = Histogram->GetMax()[0];
  std::cout << "VoxelCount: " << Histogram->GetVoxelCount() << std::endl;
  if ( bins < 0 ) {
    if (max != min) {
      bins = int(max - min);
    } else {
      bins = 3;
    }
  }

  double spacing;
  double origin;
  if (max != min) {
    spacing = (max - min)/ double(bins);
    origin = min;
  } else {
    spacing = 1.0;
    origin = min - 1.0;
  }

  Histogram->SetComponentOrigin(origin, 0.0, 0.0);
  Histogram->SetComponentExtent(0, bins, 0, 0, 0, 0);
  Histogram->SetComponentSpacing(spacing, 0.0, 0.0);

  Histogram->Update();
  double* ret = new double [5];
  ret[0] = min;
  ret[1] = max;
  ret[2] = origin;
  ret[3] = bins;
  ret[4] = spacing;
  return ret;
}

