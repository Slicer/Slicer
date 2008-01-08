#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageFileReader.h"
#include "CalculateVolumeStatisticsCLP.h"

int main(int argc, char* argv[]){
  PARSE_ARGS;

  //define the input/output types
  typedef itk::Image< short int, 3 > ROIImageType;
  typedef itk::Image< double, 3 > StatisticImageType;
  
  //define the readers
  typedef itk::ImageFileReader< ROIImageType > ROIImageReaderType;
  typedef itk::ImageFileReader< StatisticImageType > StatisticImageReaderType;
  
  ROIImageReaderType::Pointer roireader = ROIImageReaderType::New();
  roireader->SetFileName( roifilename.c_str() );
  roireader->Update();
  
  StatisticImageReaderType::Pointer statreader = StatisticImageReaderType::New();
  statreader->SetFileName( statfilename.c_str() );
  statreader->Update();
  
  typedef itk::ImageRegionConstIterator< ROIImageType > ROIImageIteratorType;
  ROIImageIteratorType ROIImageIt( roireader->GetOutput(),
    roireader->GetOutput()->GetRequestedRegion() );  
  
  //open these files
  std::ofstream outputfile( outputfilename.c_str() );
  if(!outputfile.is_open()){
    std::cerr<<"Could not open Output file!\n";
    return EXIT_FAILURE;
  }
  
  for(ROIImageIt.GoToBegin(); !ROIImageIt.IsAtEnd(); ++ROIImageIt){
    if(ROIImageIt.Get() == roilabel){
      outputfile<<statreader->GetOutput()->GetPixel(ROIImageIt.GetIndex())<<std::endl;
    }
  }
  outputfile.close();
}
