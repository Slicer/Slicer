/*
#include <BRAINSDemonWarpCommonLibWin32Header.h>
#include <BRAINSCommonLibWin32Header.h>
#include <BRAINSThreadControl.h>
#include <BRAINSDemonWarpTemplates.h>
*/

#include <stdlib.h>
#include <ChangeTrackerDemonsRegistrationSegmentationMetricCLP.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageDuplicator.h>
#include <itkImageRegionIteratorWithIndex.h>

#define DEMONS_RESULT "ctdr_registered.nrrd"
#define DEMONS_DF_RESULT "ctdr_registered_DF.nrrd"

#define DEMONS_REVERSE_RESULT "ctdr-seg_reverse_registered.nrrd"
#define DEMONS_REVERSE_DF_RESULT "ctdr-seg_reverse_registered_DF.nrrd"
#define DEMONS_WARPED_SEGMENTATION "ctdr-seg_followup_segmentation.nrrd"
#define PLUGINS_PATH ""
#define GROWTH_LABEL 14
#define SHRINK_LABEL 12

int main(int argc, char *argv[])
{

  PARSE_ARGS;

  std::ostringstream cmdLine;
  int ret;

  std::string warpedSegmentationFileName = tmpDirectory+"/"+DEMONS_WARPED_SEGMENTATION;

  // run registration with the followup as fixed image
  cmdLine << PLUGINS_PATH << "BRAINSDemonWarp " <<
    "--registrationFilterType Demons -n 3 -i 20,20,20 " << // 3 levels, with 20 iterations each
    " --minimumFixedPyramid 2,2,2 --minimumMovingPyramid 2,2,2 " <<
    " --movingVolume " << baselineVolume <<
    " --fixedVolume " << followupVolume <<
    " --outputVolume " << tmpDirectory << "/" << DEMONS_REVERSE_RESULT <<
    " --outputDeformationFieldVolume " << tmpDirectory << "/" << DEMONS_REVERSE_DF_RESULT;
  std::cout << cmdLine.str() << std::endl;
  ret = system(cmdLine.str().c_str());
  if(ret){
    std::cerr << "ERROR during demons registration" << std::endl;
    return -1;
  } else {
    std::cout << "Demons registration completed OK" << std::endl;
  }

  cmdLine.str("");

  // resample the segmentation of the baseline image to the fixed image
  cmdLine << PLUGINS_PATH << "BRAINSResample " <<
    " --referenceVolume " << followupVolume <<
    " --inputVolume " << baselineSegmentationVolume <<
    " --interpolationMode NearestNeighbor " <<
    " --deformationVolume " << tmpDirectory << "/" << DEMONS_REVERSE_DF_RESULT <<
    " --outputVolume " << warpedSegmentationFileName;
  std::cout << cmdLine.str() << std::endl;
  system(cmdLine.str().c_str());
  if(ret){
    std::cerr << "ERROR during resampling" << std::endl;
    return -1;
  } else {
    std::cout << "Demons registration completed OK" << std::endl;
  }

  // read the warped segmentation, and create the change map
  typedef itk::Image<char, 3> ImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;
  typedef itk::ImageDuplicator<ImageType> DuplicatorType;
  typedef itk::ImageRegionIteratorWithIndex<ImageType> IteratorType;

  ReaderType::Pointer reader1 = ReaderType::New();
  reader1->SetFileName(warpedSegmentationFileName.c_str());
  reader1->Update();
  
  ReaderType::Pointer reader2 = ReaderType::New();
  reader2->SetFileName(baselineSegmentationVolume.c_str());
  reader2->Update();

  DuplicatorType::Pointer dup = DuplicatorType::New();
  ImageType::Pointer followupSegmentation = reader1->GetOutput(),
    baselineSegmentation = reader2->GetOutput(),
    changesLabel;

  dup->SetInputImage(baselineSegmentation);
  dup->Update();
  changesLabel = dup->GetOutput();
  changesLabel->FillBuffer(0);

  IteratorType bIt(baselineSegmentation, baselineSegmentation->GetBufferedRegion());
  float growthPixels = 0, shrinkPixels = 0;
  for(bIt.GoToBegin();!bIt.IsAtEnd();++bIt){
    ImageType::IndexType idx = bIt.GetIndex();
    ImageType::PixelType bPxl, fPxl;
    bPxl = bIt.Get();
    fPxl = followupSegmentation->GetPixel(idx);

    if(bPxl && !fPxl){
      changesLabel->SetPixel(idx, 12);
      shrinkPixels++;
    } else if(!bPxl && fPxl) {
      changesLabel->SetPixel(idx, 14);
      growthPixels++;
    }
  }
  
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(changesLabel);
  writer->SetFileName(outputVolume.c_str());
  writer->Update();

  if(reportFileName != ""){
    std::cout << "Saving report to " << reportFileName << std::endl;
    ImageType::SpacingType s = changesLabel->GetSpacing();
    float sv = s[0]*s[1]*s[2];
    std::ofstream report(reportFileName.c_str());

    float growthVolume = growthPixels*sv, shrinkVolume = shrinkPixels*sv, totalVolume = (growthPixels-shrinkPixels)*sv, totalPixels = (growthPixels-shrinkPixels);

    report << "<span style=\"font-family:arial,helvetica,sans-serif;\"><strong><span style=\"color:#ff0000;\">Growth</span>: " << growthVolume << " mm<sup>3 </sup></strong>(" << growthPixels << " pixels)</span></p>";
    report << "<p><span style=\"font-family:arial,helvetica,sans-serif;\"><strong><span style=\"color:#008000;\">Shrinkage</span>: " << shrinkVolume << " mm<sup>3&nbsp;</sup></strong>(" << shrinkPixels << " pixels)</span></p><p>";
    report << "<span style=\"font-family:arial,helvetica,sans-serif;\"><strong>Total: " <<  totalVolume << " mm<sup>3&nbsp;</sup></strong>(" << totalPixels << " pixels)</span></p>";
  }
  
  return EXIT_SUCCESS;
}
