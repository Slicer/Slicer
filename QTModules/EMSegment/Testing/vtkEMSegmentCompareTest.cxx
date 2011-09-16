#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentTestUtilities.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"

int main(int argc, char** argv)
{
  std::cerr << "Starting Compare test..." << std::endl;
  std::vector<std::string> targetImageFilenames;
  std::string resultImageTestFilename;
  std::string resultImageOrigFilename;

  //
  // parse command line
  if (argc < 2)
  {
    std::cerr
      << "Usage: vtkEMSegmentBlackBoxSegmentationTest"     << std::endl
      <<         "<resultImageTestFilename>"               << std::endl
      <<         "<resultImageOrigFilename>"               << std::endl
      << std::endl;
    return EXIT_FAILURE;
  }

  resultImageTestFilename      = argv[1];
  resultImageOrigFilename      = argv[2];

  //
  // read segmentation result standard
  vtkITKArchetypeImageSeriesReader* standardReader = vtkITKArchetypeImageSeriesScalarReader::New();
  standardReader->SetArchetype(resultImageTestFilename.c_str());
  standardReader->SetOutputScalarTypeToNative();
  standardReader->SetDesiredCoordinateOrientationToNative();
  standardReader->SetUseNativeOriginOn();
  try
  {
    standardReader->Update();
  }
  catch (...)
  {
    std::cerr << "Error reading image: " << std::endl;
    standardReader->Delete();
    return EXIT_FAILURE;
  }


  vtkITKArchetypeImageSeriesReader* standardReader2 = vtkITKArchetypeImageSeriesScalarReader::New();
  standardReader2->SetArchetype(resultImageOrigFilename.c_str());
  standardReader2->SetOutputScalarTypeToNative();
  standardReader2->SetDesiredCoordinateOrientationToNative();
  standardReader2->SetUseNativeOriginOn();
  try
  {
    standardReader2->Update();
  }
  catch (...)
  {
    std::cerr << "Error reading image: " << std::endl;
    standardReader2->Delete();
    return EXIT_FAILURE;
  }


  vtkImageData* resultImageTest     = standardReader->GetOutput();
  vtkImageData* resultImageOrig     = standardReader2->GetOutput();

//  int EXACT = 1;
  int NONEXACT = 0;
  double rate = CompareTwoVolumes(resultImageTest, resultImageOrig, NONEXACT);

  std::cerr << "Cleanup...";
  standardReader->Delete();
  standardReader2->Delete();
  std::cerr << "...done" << std::endl;

  if ( rate > 1.0 ) {
    return EXIT_FAILURE;
  }
  else {
    return EXIT_SUCCESS;
  }

//            RANGE = MathImg->GetOutput()->GetScalarRange();
//            if {([lindex $RANGE 0] != 0) || ([lindex $RANGE 1] != 0) } {
//             VolumeMathWriter [MathImg GetOutput] [file join [file dirname $Volume(1,filePrefix)] compare]
//            }
}

