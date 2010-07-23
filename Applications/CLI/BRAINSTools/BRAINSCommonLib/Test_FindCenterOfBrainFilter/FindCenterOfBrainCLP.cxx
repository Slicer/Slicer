/*  KENT  -- This CLP Wrapped test program needs to exercise
 *  the itkFindCenterOfBrainCLP.h class.
 *
 * As part of this testing, please move as many of the hard-coded
 * debuggging images out of the txx files, and make it so that this
 * test program will create those images from the command line.
 *
 * You will have to make some more member variables of the class for
 * some of the intermediate images like "AfterGridComputationsForeground.nii.gz"
 * so that the class can expose them to the test program.
 *
 * Please also write an ADD_TEST section to the CMakeLists.txt file that will execute this test program.
 */
#include "itkFindCenterOfBrainFilter.h"
#include "itkIO.h"
#include <FindCenterOfBrainCLP.h>

int main(int argc, char **argv)
{
  PARSE_ARGS;
  if ( InputVolume == "" )
    {
    std::cerr << "FindCenterOfBrain: missing input image name" << std::endl;
    exit(1);
    }
  typedef itk::Image< unsigned char, 3 >            ImageType;
  typedef itk::FindCenterOfBrainFilter< ImageType > FindCenterFilterType;
  typedef FindCenterFilterType::MaskImageType       MaskImageType;

  ImageType::Pointer inputImage = itkUtil::ReadImage< ImageType >(InputVolume);
  if ( inputImage.IsNull() )
    {
    std::cerr << "FindCenterOfBrain: Can't read input image "
              << InputVolume << std::endl;
    exit(2);
    }

  FindCenterFilterType::Pointer filter = FindCenterFilterType::New();
  filter->SetInput(inputImage);

  MaskImageType::Pointer imageMask;
  if ( ImageMask != "" )
    {
    imageMask = itkUtil::ReadImage< MaskImageType >(ImageMask);
    if ( imageMask.IsNull() )
      {
      std::cerr << "FindCenterOfBrain: Can't read mask "
                << ImageMask << std::endl;
      exit(3);
      }
    filter->SetImageMask(imageMask);
    }
  filter->SetMaximize(Maximize);
  filter->SetAxis(Axis);
  filter->SetOtsuPercentileThreshold(OtsuPercentileThreshold);
  filter->SetClosingSize(ClosingSize);
  filter->SetHeadSizeLimit(HeadSizeLimit);
  filter->SetHeadSizeEstimate(HeadSizeEstimate);
  filter->SetBackgroundValue(BackgroundValue);
  filter->SetGenerateDebugImages(GenerateDebugImages);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    exit(4);
    }
  FindCenterFilterType::PointType center =
    filter->GetCenterOfBrain();
  std::cout << "Center Of Brain:"
            << " " << center[0]
            << " " << center[1]
            << " " << center[2]
            << std::endl;
  if ( ClippedImageMask != "" )
    {
    MaskImageType::Pointer clippedMask =
      filter->GetClippedImageMask();
    itkUtil::WriteImage< MaskImageType >(clippedMask,
                                         ClippedImageMask);
    }
  if ( !GenerateDebugImages )
    {
    exit(0);
    }
  if ( DebugDistanceImage != "" )
    {
    FindCenterFilterType::DistanceImagePointer distImage =
      filter->GetDebugDistanceImage();
    itkUtil::WriteImage< FindCenterFilterType::DistanceImageType >(distImage,
                                                                   DebugDistanceImage);
    }
  if ( DebugGridImage != "" )
    {
    FindCenterFilterType::InputImagePointer gridImage =
      filter->GetDebugGridImage();
    itkUtil::WriteImage< ImageType >(gridImage, DebugGridImage);
    }
  if ( DebugAfterGridComputationsForegroundImage != "" )
    {
    MaskImageType::Pointer afterImage =
      filter->GetDebugAfterGridComputationsForegroundImage();
    itkUtil::WriteImage< MaskImageType >(afterImage,
                                         DebugAfterGridComputationsForegroundImage);
    }
  if ( DebugClippedImageMask != "" )
    {
    MaskImageType::Pointer clippedMask =
      filter->GetDebugClippedImageMask();
    itkUtil::WriteImage< MaskImageType >(clippedMask,
                                         DebugClippedImageMask);
    }
  if ( DebugTrimmedImage != "" )
    {
    ImageType::Pointer trimmedImage =
      filter->GetDebugTrimmedImage();
    itkUtil::WriteImage< ImageType >(trimmedImage,
                                     DebugTrimmedImage);
    }
  exit(0);
}
