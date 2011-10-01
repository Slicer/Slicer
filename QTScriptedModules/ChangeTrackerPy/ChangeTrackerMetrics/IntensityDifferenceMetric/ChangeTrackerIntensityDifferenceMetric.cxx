#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkTernaryAddImageFilter.h"

#include "ChangeTrackerIntensityDifferenceMetricCLP.h"


#include "itkImage.h"


using namespace std;

typedef int   PixelType;
typedef float DTPixelType;
const   unsigned int      Dimension = 3;
typedef itk::Image< PixelType, Dimension >    ImageType;
typedef itk::Image< DTPixelType, Dimension >   DTImageType;

typedef itk::ImageFileReader< ImageType >  ReaderType;
typedef itk::ImageFileWriter< ImageType >  WriterType;
typedef itk::ImageFileWriter< DTImageType >  DTWriterType;
typedef itk::HistogramMatchingImageFilter<ImageType,ImageType> HistogramMatchingType;
typedef itk::ThresholdImageFilter<ImageType> ThresholdType;
typedef itk::BinaryThresholdImageFilter<ImageType,ImageType> BinaryThresholdType;
typedef itk::BinaryThresholdImageFilter<DTImageType,ImageType> DTBinaryThresholdType;
typedef itk::CastImageFilter<DTImageType,ImageType> CastType;
typedef itk::MinimumMaximumImageFilter<ImageType> MinMaxType;
typedef itk::SubtractImageFilter<ImageType,ImageType> SubtractType;
typedef itk::MedianImageFilter<ImageType,ImageType> MedianType;
typedef itk::MaskImageFilter<ImageType,ImageType,ImageType> MaskType;
typedef itk::ImageRegionConstIterator<ImageType> ConstIterType;
typedef itk::ConnectedComponentImageFilter<ImageType,ImageType> ConnectedCompType;
typedef itk::RelabelComponentImageFilter<ImageType,ImageType> RelabelType;
typedef itk::SignedMaurerDistanceMapImageFilter<ImageType,DTImageType> DTType;
typedef itk::MedianImageFilter<ImageType,ImageType> MedianType;
typedef itk::SubtractImageFilter<ImageType,ImageType> SubtractType;
typedef itk::AddImageFilter<ImageType,ImageType> AddType;
typedef itk::ImageRegionConstIterator<ImageType> ConstIterType;
typedef itk::ImageRegionIteratorWithIndex<ImageType> IterType;
typedef itk::ImageDuplicator<ImageType> DupType;

void SaveImage(ImageType::Pointer,const char*);
void SaveHisto(std::vector<float>, const char*);
void SaveDTImage(DTImageType::Pointer,const char*);
DTImageType::Pointer DetectChanges(ImageType::Pointer,ImageType::Pointer,ImageType::Pointer,bool);
void CalculateRegionHistograms(ImageType::Pointer diff, ImageType::Pointer mask, 
  std::vector<float> &, std::vector<float> &,  std::vector<float> &, std::vector<float> &);
ImageType::Pointer EnforceConnectedness(ImageType::Pointer margin, ImageType::Pointer input);

int main( int argc, char ** argv )
{
  PARSE_ARGS;

  // error checking
  if(changingBandSize <= 0){
    std::cerr << "Changing band size must be a positive number" << std::endl;
    return EXIT_FAILURE;
  }
  
  if(sensitivityThreshold<=0 || sensitivityThreshold>1){
    std::cerr << "Sensitivity threshold must be in (0; 1]" << std::endl;
    return EXIT_FAILURE;
  }

  ReaderType::Pointer reader0 = ReaderType::New();
  ReaderType::Pointer reader1 = ReaderType::New();
  ReaderType::Pointer readerS = ReaderType::New();

  // these parameters are required for the CLI, so they should be initialized
  const char * inputFilename0  = baselineVolume.c_str();
  const char * inputFilename1 = followupVolume.c_str();
  const char * segmFilename = baselineSegmentationVolume.c_str();
  const char * outFilename = outputVolume.c_str();

  reader0->SetFileName( inputFilename0 );
  reader1->SetFileName( inputFilename1 );
  readerS->SetFileName( segmFilename );

  // calculate masks for stable, growth and shrink regions
  ImageType::Pointer growthMask, shrinkMask, stableMask, diffMask, marginMask;
  
  // calculate the signed distance from the object segmentation
  DTType::Pointer dt = DTType::New();
  dt->SetInput(readerS->GetOutput());
  dt->SetSquaredDistance(0);
  dt->SetUseImageSpacing(1);  // the changing region size is defined in mm
  dt->Update();

  //  mark voxels inside the "changing band"
  DTBinaryThresholdType::Pointer thresh = DTBinaryThresholdType::New();
  thresh->SetInsideValue(1);
  thresh->ReleaseDataFlagOn();
  thresh->SetInput(dt->GetOutput());
  thresh->SetLowerThreshold(-changingBandSize);
  thresh->SetUpperThreshold(changingBandSize);
  thresh->Update();

  
  diffMask = thresh->GetOutput();
  diffMask->DisconnectPipeline();

  thresh->SetInput(dt->GetOutput());
  thresh->SetLowerThreshold(-changingBandSize);
  thresh->SetUpperThreshold(0);
  thresh->Update();
  shrinkMask = thresh->GetOutput();
  shrinkMask->DisconnectPipeline();

  thresh->SetInput(dt->GetOutput());
  thresh->SetLowerThreshold(0);
  thresh->SetUpperThreshold(changingBandSize);
  thresh->Update();
  growthMask = thresh->GetOutput();
  growthMask->DisconnectPipeline();

  // "stable mask" is inside the structure of interest, outside the changing
  // band mask
  thresh->SetInput(dt->GetOutput());
  thresh->SetLowerThreshold(-1000);
  thresh->SetUpperThreshold(-changingBandSize-.01);
  thresh->Update();
  stableMask = thresh->GetOutput();
  stableMask->DisconnectPipeline();

  thresh->SetInput(dt->GetOutput());
  thresh->SetLowerThreshold(0);
  thresh->SetUpperThreshold(1);
  thresh->Update();
  marginMask = thresh->GetOutput();
  marginMask->DisconnectPipeline();

  // smooth the inputs with median filter
  MedianType::Pointer tp0med = MedianType::New(), tp1med = MedianType::New();
  ImageType::SizeType medRad;
  medRad.Fill(1);
  tp0med->SetRadius(medRad);
  tp1med->SetRadius(medRad);
  tp0med->SetInput(reader0->GetOutput());
  tp1med->SetInput(reader1->GetOutput());
 
  // find the difference
  SubtractType::Pointer sub = SubtractType::New();
  sub->SetInput1(tp1med->GetOutput());
  sub->SetInput2(tp0med->GetOutput());
  sub->Update();

  ImageType::Pointer diffImage = sub->GetOutput();

  std::vector<float> cumHistPosDiff;
  std::vector<float> cumHistNegDiff;
  std::vector<float> histPosDiff;
  std::vector<float> histNegDiff;

  std::vector<float> cumHistPosStable;
  std::vector<float> cumHistNegStable;
  std::vector<float> histPosStable;
  std::vector<float> histNegStable;

  std::vector<float> cumHistPosShrink;
  std::vector<float> cumHistNegShrink;
  std::vector<float> histPosShrink;
  std::vector<float> histNegShrink;

  std::vector<float> cumHistPosGrowth;
  std::vector<float> cumHistNegGrowth;
  std::vector<float> histPosGrowth;
  std::vector<float> histNegGrowth;

  std::cout << "Difference region" << std::endl;
  CalculateRegionHistograms(diffImage, diffMask, histPosDiff, cumHistPosDiff, histNegDiff, cumHistNegDiff);
  std::cout << "Stable region" << std::endl;
  CalculateRegionHistograms(diffImage, stableMask, histPosStable, cumHistPosStable, histNegStable, cumHistNegStable);
  std::cout << "Shrink region" << std::endl;
  CalculateRegionHistograms(diffImage, shrinkMask, histPosShrink, cumHistPosShrink, histNegShrink, cumHistNegShrink);
  std::cout << "Growth region" << std::endl;
  CalculateRegionHistograms(diffImage, growthMask, histPosGrowth, cumHistPosGrowth, histNegGrowth, cumHistNegGrowth);
  SaveImage(diffImage, "diff.nrrd");

  std::cout << "Stable region bounds: -" << histNegStable.size()-1 << " to " << histPosStable.size()-1 << std::endl;
  std::cout << "Changing region bounds: -" << histNegDiff.size()-1 << " to " << histPosDiff.size()-1 << std::endl;

//  SaveHisto(histPosDiff, "growth_histo.txt");
//  SaveHisto(histNegDiff, "shrink_histo.txt");
//  SaveHisto(histPosStable, "stable_pos.txt");
//  SaveHisto(cumHistPosStable, "stable_pos_cum.txt");

//    {
//    std::ofstream sghisto("shrink_growth_histo.txt");
//    for(int i=histNegDiff.size()-1;i>0;i--)
//      sghisto << -i << " " << histNegDiff[i] << std::endl;
//    for(int i=1;i<histNegDiff.size();i++)
//      sghisto << i << " " << histNegDiff[i] << std::endl;
//    }

  BinaryThresholdType::Pointer growthThresh = BinaryThresholdType::New();
  BinaryThresholdType::Pointer shrinkThresh = BinaryThresholdType::New();

  float cutoffThresh = histNegStable.size()>histPosStable.size() ? histNegStable.size()-1 : histPosStable.size()-1;
  
  /*
  if(argc>4){
    cutoffThresh = atoi(argv[4]);
    std::cout << "Percentage at cutoff: " << cumHistPosDiff[cutoffThresh] << std::endl;
  } 
  */
  
    std::cout << "Cutoff at " << sensitivityThreshold << std::endl;
    for(int i=0;i<cumHistPosDiff.size();i++){
      if(cumHistPosDiff[i]>=sensitivityThreshold){
        cutoffThresh = i;
        break;
      }
    }
  
  std::cout << "cutoff intensity threshold value: " << cutoffThresh << std::endl;

  DupType::Pointer dup1 = DupType::New();
  DupType::Pointer dup2 = DupType::New();
  
  dup1->SetInputImage(diffImage);
  dup1->Update();

  dup2->SetInputImage(diffImage);
  dup2->Update();

  ImageType::Pointer cdSeg = dup1->GetOutput(), cdNoSeg = dup2->GetOutput();
  cdSeg->FillBuffer(0); cdNoSeg->FillBuffer(0);

  IterType itDiff(diffImage, diffImage->GetBufferedRegion());
  IterType itDiffMask(diffMask, diffMask->GetBufferedRegion());
  IterType itGrowthMask(growthMask, growthMask->GetBufferedRegion());
  IterType itShrinkMask(growthMask, growthMask->GetBufferedRegion());

  itDiff.GoToBegin();itDiffMask.GoToBegin();
  itGrowthMask.GoToBegin();itShrinkMask.GoToBegin();

  int cdSegGrowthCnt = 0, cdSegShrinkCnt = 0, cdNoSegGrowthCnt = 0, cdNoSegShrinkCnt = 0;
  for(;!itDiff.IsAtEnd();++itDiff,++itDiffMask,++itGrowthMask,++itShrinkMask){
    if(abs(itDiff.Get())<cutoffThresh)
      continue;
    if(itDiff.Get()>0){
      // growth
      if(itDiffMask.Get()){
        cdNoSeg->SetPixel(itDiff.GetIndex(), 14);
      }
      if(itGrowthMask.Get()){
        cdSeg->SetPixel(itDiff.GetIndex(),14);      
      }
    } else {
      // shrink
      if(itDiffMask.Get()){
        cdNoSeg->SetPixel(itDiff.GetIndex(),12);
      }
      if(itShrinkMask.Get()){
        cdSeg->SetPixel(itDiff.GetIndex(),12);
      }
    }
  }
 
  // only those pixels connected to the margin of the structure segmentation
  // are considered as valid change
  cdNoSeg = EnforceConnectedness(marginMask, cdNoSeg);
  cdSeg = EnforceConnectedness(marginMask, cdSeg);

  IterType it1(cdNoSeg, cdNoSeg->GetBufferedRegion());
  IterType it2(cdSeg, cdSeg->GetBufferedRegion());
  for(it1.GoToBegin(),it2.GoToBegin();!it1.IsAtEnd();++it1,++it2){
    if(it1.Get()==12)
      cdNoSegShrinkCnt++;
    if(it2.Get()==12)
      cdSegShrinkCnt++;
    if(it1.Get()==14)
      cdNoSegGrowthCnt++;
    if(it2.Get()==14)
      cdSegGrowthCnt++;
  }

  ImageType::SpacingType spacing = diffImage->GetSpacing();
  float pixelVol = spacing[0]*spacing[1]*spacing[2];
  std::cout << "cdNoSeg: growthVol = " << pixelVol*cdNoSegGrowthCnt << ", shrinkVol = " << pixelVol*cdNoSegShrinkCnt << "mm^3" << std::endl;
  std::cout << "cdSeg: growthVol = " << pixelVol*cdSegGrowthCnt << ", shrinkVol = " << pixelVol*cdSegShrinkCnt << "mm^3" << std::endl;
  std::cout << "cdNoSeg: growthCnt = " << cdNoSegGrowthCnt << ", shrinkCnt = " << cdNoSegShrinkCnt << std::endl;
  std::cout << "cdSeg: growthCnt = " << cdSegGrowthCnt << ", shrinkCnt = " << cdSegShrinkCnt << std::endl;

  if(reportFileName != ""){
    std::ofstream report(reportFileName.c_str());
    report << "<span style=\"font-family:arial,helvetica,sans-serif;\"><strong><span style=\"color:#ff0000;\">Growth</span>: " << cdSegGrowthCnt*pixelVol << " mm<sup>3 </sup></strong>(" << cdSegGrowthCnt << " pixels)</span></p>";
    report << "<p><span style=\"font-family:arial,helvetica,sans-serif;\"><strong><span style=\"color:#008000;\">Shrinkage</span>: " << cdSegShrinkCnt*pixelVol << " mm<sup>3&nbsp;</sup></strong>(" << cdSegShrinkCnt << " pixels)</span></p><p>";
    report << "<span style=\"font-family:arial,helvetica,sans-serif;\"><strong>Total: " <<  (cdSegGrowthCnt-cdSegShrinkCnt)*pixelVol << " mm<sup>3&nbsp;</sup></strong>(" << cdSegGrowthCnt-cdSegShrinkCnt << " pixels)</span></p>";
    /*
    report << "Growth: " << cdSegGrowthCnt*pixelVol << " mm^3 (" << cdSegGrowthCnt << " voxels)" << std::endl;
    report << "Shrinkage: " << cdSegShrinkCnt*pixelVol << " mm^3 (" << cdSegShrinkCnt << " voxels)" << std::endl;
    report << "Total: " << (cdSegGrowthCnt-cdSegShrinkCnt)*pixelVol << " mm^3 (" << cdSegGrowthCnt-cdSegShrinkCnt << " voxels)" << std::endl;
    */
  }
 
//  SaveImage(cdNoSeg, "/tmp/cdNoSegResult.nrrd");
  SaveImage(cdSeg, outFilename);

  return EXIT_SUCCESS;
}

void SaveImage(ImageType::Pointer image,const char* name){
  WriterType::Pointer w = WriterType::New();
  w->SetFileName(name);
  w->SetInput(image);
  w->Update();
}

void SaveDTImage(DTImageType::Pointer image,const char* name){
  DTWriterType::Pointer w = DTWriterType::New();
  w->SetFileName(name);
  w->SetInput(image);
  w->Update();
}

void CalculateRegionHistograms(ImageType::Pointer input, ImageType::Pointer mask, 
    std::vector<float> &histPos, std::vector<float> &cumHistPos,
  std::vector<float> &histNeg, std::vector<float> &cumHistNeg){

  MaskType::Pointer masker = MaskType::New();
  masker->SetInput1(input);
  masker->SetInput2(mask);
  masker->Update();

  ImageType::Pointer diff = masker->GetOutput();

  // calculate the histogram
  MinMaxType::Pointer minmax = MinMaxType::New();
  minmax->SetInput(diff);
  minmax->Update();

  std::cout << "Min: " << minmax->GetMinimum() << ", Max: " << minmax->GetMaximum() << std::endl;
  cumHistPos.resize(minmax->GetMaximum()+1);
  cumHistNeg.resize(abs(minmax->GetMinimum())+1);
  histPos.resize(minmax->GetMaximum()+1);
  histNeg.resize(abs(minmax->GetMinimum())+1);
  float posCnt = 0, negCnt = 0, posCntCum = 0, negCntCum = 0;
  unsigned i;

  ConstIterType sit(diff,diff->GetLargestPossibleRegion());
  for(sit.GoToBegin();!sit.IsAtEnd();++sit){
    float val = sit.Get();
    if(!val)
      continue;
    if(val<0){
      histNeg[abs(val)]++;
      negCnt++;
    }
    if(val>0){
      histPos[val]++;
      posCnt++;
    }
  }

  int posThresh = 0, negThresh = 0;
  for(i=0;i<histPos.size();i++){
    posCntCum += histPos[i];
    cumHistPos[i] = posCntCum/posCnt;
  }
  for(i=0;i<histNeg.size();i++){
    negCntCum += histNeg[i];
    cumHistNeg[i]= negCntCum/negCnt;
  }

}

void SaveHisto(std::vector<float> hist, const char* name){
  std::ofstream nhf(name);
  for(int i=0;i<hist.size();i++){
    nhf << i << " " << hist[i] << std::endl;
  }
}

ImageType::Pointer EnforceConnectedness(ImageType::Pointer margin, ImageType::Pointer input){
  // make sure we include only those pixels that are connected with the
  // segmentation margin
  
  MaskType::Pointer mask = MaskType::New();
  BinaryThresholdType::Pointer thresh = BinaryThresholdType::New();
  AddType::Pointer add = AddType::New();
  ConnectedCompType::Pointer conn = ConnectedCompType::New();
  RelabelType::Pointer relabel = RelabelType::New();

  add->SetInput1(margin);
  add->SetInput2(input);
  conn->SetInput(add->GetOutput());
  conn->FullyConnectedOn();
  relabel->SetInput(conn->GetOutput());
  thresh->SetInput(relabel->GetOutput());
  thresh->SetUpperThreshold(1);
  thresh->SetLowerThreshold(1);
  thresh->SetInsideValue(1);
  thresh->SetOutsideValue(0);

  mask->SetInput1(input);
  mask->SetInput2(thresh->GetOutput());
  mask->Update();

  return mask->GetOutput();
}
