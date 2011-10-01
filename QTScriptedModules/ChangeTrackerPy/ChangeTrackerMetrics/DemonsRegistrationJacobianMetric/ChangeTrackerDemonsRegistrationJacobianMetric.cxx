/*
#include <BRAINSDemonWarpCommonLibWin32Header.h>
#include <BRAINSCommonLibWin32Header.h>
#include <BRAINSThreadControl.h>
#include <BRAINSDemonWarpTemplates.h>
*/

#include <stdlib.h>
#include <ChangeTrackerDemonsRegistrationJacobianMetricCLP.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageDuplicator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkVector.h>
#include <itkDeformationFieldJacobianDeterminantFilter.h>

#include "itkWarpJacobianDeterminantFilter.h"

#define DEMONS_RESULT "ctdr_registered.nrrd"
#define DEMONS_DF_RESULT "ctdr_registered_DF.nrrd"

#define DEMONS_REVERSE_RESULT "ctdr-seg_reverse_registered.nrrd"
#define DEMONS_REVERSE_DF_RESULT "ctdr-seg_reverse_registered_DF.nrrd"
#define DEMONS_WARPED_SEGMENTATION "ctdr-seg_followup_segmentation.nrrd"
// this will go away once bug #1455 is resolved
#define PLUGINS_PATH "/Users/fedorov/Slicer/Slicer4-Superbuild/Slicer-build/lib/Slicer-4.0/cli-modules/"
#define GROWTH_LABEL 14
#define SHRINK_LABEL 12

int main(int argc, char *argv[])
{

  PARSE_ARGS;

  std::ostringstream cmdLine;
  int ret;

  std::string dfFileName = tmpDirectory+"/"+DEMONS_DF_RESULT;

  // run registration with the baseline as fixed image
  cmdLine << PLUGINS_PATH << "/BRAINSDemonWarp " <<
    "--registrationFilterType Demons -n 3 -i 20,20,20 " << // 3 levels, with 20 iterations each
    " --minimumFixedPyramid 2,2,2 --minimumMovingPyramid 2,2,2 " <<
    " --movingVolume " << followupVolume <<
    " --fixedVolume " << baselineVolume <<
    " --outputVolume " << tmpDirectory << "/" << DEMONS_RESULT <<
    " --outputDeformationFieldVolume " << dfFileName;
  std::cout << cmdLine.str() << std::endl;
  ret = system(cmdLine.str().c_str());
  if(ret){
    std::cerr << "ERROR during demons registration" << std::endl;
    return -1;
  } else {
    std::cout << "Demons registration completed OK" << std::endl;
  }

  // read the deformation field, baseline label, and create the change map
  typedef itk::Image<char, 3> ImageType;
  typedef itk::Image<float, 3> JacImageType;
  typedef itk::Image<itk::Vector<float,3>, 3> DFImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileReader<DFImageType> DFReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;
  typedef itk::ImageDuplicator<ImageType> DuplicatorType;
  typedef itk::ImageRegionIteratorWithIndex<ImageType> IteratorType;
  //typedef itk::DeformationFieldJacobianDeterminantFilter<DFImageType, float, JacImageType> JacFilterType;
  typedef itk::WarpJacobianDeterminantFilter<DFImageType, JacImageType> JacFilterType;

  DFReaderType::Pointer dfreader = DFReaderType::New();
  dfreader->SetFileName(dfFileName.c_str());
  dfreader->Update();

  ReaderType::Pointer reader1 = ReaderType::New();
  reader1->SetFileName(baselineSegmentationVolume.c_str());
  reader1->Update();
  
  ImageType::Pointer baselineSegmentation = reader1->GetOutput(), 
    changesLabel;

  DuplicatorType::Pointer dup = DuplicatorType::New();
  dup->SetInputImage(reader1->GetOutput());
  dup->Update();
  changesLabel = dup->GetOutput();
  changesLabel->FillBuffer(0);

  JacFilterType::Pointer jac = JacFilterType::New();
  jac->SetInput(dfreader->GetOutput());
  jac->SetUseImageSpacingOn();
  jac->Update();

  JacImageType::Pointer jacImage = jac->GetOutput();

  float jacDetSum = 0, nSegVoxels = 0;

  IteratorType bIt(baselineSegmentation, baselineSegmentation->GetBufferedRegion());
  float growthPixels = 0, shrinkPixels = 0;
  for(bIt.GoToBegin();!bIt.IsAtEnd();++bIt){
    ImageType::IndexType idx = bIt.GetIndex();
    ImageType::PixelType bPxl = bIt.Get();

    if(bPxl){
      JacImageType::PixelType jPxl = jacImage->GetPixel(idx);
      jacDetSum += jPxl;
      nSegVoxels++;
      if(jPxl>1.1)
        changesLabel->SetPixel(idx, 14);
      if(jPxl<0.9)
        changesLabel->SetPixel(idx, 12);
    }
  }
  
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(changesLabel);
  writer->SetFileName(outputVolume.c_str());
  writer->Update();

  if(reportFileName != ""){
    ImageType::SpacingType s = changesLabel->GetSpacing();
    float sv = s[0]*s[1]*s[2];
    std::ofstream rep(reportFileName.c_str());
    rep << "Growth: " << (jacDetSum-nSegVoxels)*sv << " mm^3 (" << jacDetSum-nSegVoxels << " pixels) " << std::endl;
    rep << "Shrinkage: " << shrinkPixels*sv << " mm^3 (" << shrinkPixels << " pixels) " << std::endl;
    rep << "Total: " << (growthPixels-shrinkPixels)*sv << " mm^3 (" << growthPixels-shrinkPixels << " pixels) " << std::endl;
  }
  
  return EXIT_SUCCESS;
}
