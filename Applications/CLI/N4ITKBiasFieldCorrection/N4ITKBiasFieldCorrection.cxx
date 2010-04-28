#include "itkBSplineControlPointImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkN4MRIBiasFieldCorrectionImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkShrinkImageFilter.h"

#include "itkTimeProbe.h"

#include <string>
#include <algorithm>
#include <vector>

#include "N4ITKBiasFieldCorrectionCLP.h"
#include "itkPluginUtilities.h"

template<class TFilter>
class CommandIterationUpdate : public itk::Command
{
public:
  typedef CommandIterationUpdate   Self;
  typedef itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate() {};
public:

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
    Execute( (const itk::Object *) caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
    const TFilter * filter =
      dynamic_cast< const TFilter * >( object );
    if( typeid( event ) != typeid( itk::IterationEvent ) )
      { return; }
    std::cout << "Progress: " << filter->GetProgress() << std::endl;
    }

};


int main(int argc, char** argv){

  PARSE_ARGS;

  typedef float RealType;
  const int ImageDimension = 3;

  typedef itk::Image<RealType, ImageDimension> ImageType;
  ImageType::Pointer inputImage = NULL;

  typedef itk::Image<unsigned char, ImageDimension> MaskImageType;
  MaskImageType::Pointer maskImage = NULL;

  typedef itk::N4MRIBiasFieldCorrectionImageFilter<ImageType, MaskImageType,
    ImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName( inputImageName.c_str() );
  reader->Update();
  inputImage = reader->GetOutput();

  /**
   * handle he mask image
   */

  if( maskImageName != ""){
    typedef itk::ImageFileReader<MaskImageType> ReaderType;
    ReaderType::Pointer maskreader = ReaderType::New();
    maskreader->SetFileName( maskImageName.c_str() );
    maskreader->Update();
    maskImage = maskreader->GetOutput();
  } 
  
  if( !maskImage ) {
    std::cout << "Mask no read.  Creaing Otsu mask." << std::endl;
    typedef itk::OtsuThresholdImageFilter<ImageType, MaskImageType>
      ThresholderType;
    ThresholderType::Pointer otsu = ThresholderType::New();
    otsu->SetInput( inputImage );
    otsu->SetNumberOfHistogramBins( 200 );
    otsu->SetInsideValue( 0 );
    otsu->SetOutsideValue( 1 );
    otsu->Update();

    maskImage = otsu->GetOutput();
  }

  ImageType::Pointer weightImage = NULL;

  if( weightImageName != "" ){
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer weightreader = ReaderType::New();
    weightreader->SetFileName( weightImageName.c_str() );
    weightreader->Update();
    weightImage = weightreader->GetOutput();
  }

  /**
   * convergence opions
   */
  if(numberOfIterations.size()>1 && numberOfIterations[0]){
    CorrecterType::VariableSizeArrayType
      maximumNumberOfIterations( numberOfIterations.size() );
    for(unsigned d=0;d<numberOfIterations.size();d++)
      maximumNumberOfIterations[d] = numberOfIterations[d];
    correcter->SetMaximumNumberOfIterations( maximumNumberOfIterations );
    
    CorrecterType::ArrayType numberOfFittingLevels;
    numberOfFittingLevels.Fill( numberOfIterations.size() );
    correcter->SetNumberOfFittingLevels( numberOfFittingLevels );
  }

  if( convergenceThreshold )
    correcter->SetConvergenceThreshold( convergenceThreshold );

  /**
   * B-spline opions -- we place his here o ake care of he case where
   * he user wans o specify hings in erms of he spline disance.
   */

  bool useSplineDistance = false;
  ImageType::IndexType inputImageIndex =
    inputImage->GetLargestPossibleRegion().GetIndex();
  ImageType::SizeType inputImageSize =
    inputImage->GetLargestPossibleRegion().GetSize();
  ImageType::IndexType maskImageIndex =
    maskImage->GetLargestPossibleRegion().GetIndex();
  ImageType::SizeType maskImageSize =
    maskImage->GetLargestPossibleRegion().GetSize();

  ImageType::PointType newOrigin = inputImage->GetOrigin();

  if(bsplineOrder)
    correcter->SetSplineOrder(bsplineOrder);
  if(alpha)
    correcter->SetSigmoidNormalizedAlpha( alpha );
  if(beta)
    correcter->SetSigmoidNormalizedBeta( beta );

  CorrecterType::ArrayType numberOfControlPoints;
  if(splineDistance){
    useSplineDistance = true;
      
    unsigned long lowerBound[ImageDimension];
    unsigned long upperBound[ImageDimension];

    for( unsigned  d = 0; d < 3; d++ ){
      float domain = static_cast<RealType>( inputImage->
        GetLargestPossibleRegion().GetSize()[d] - 1 ) * inputImage->GetSpacing()[d];
      unsigned int numberOfSpans = static_cast<unsigned int>( vcl_ceil( domain / splineDistance ) );
      unsigned long extraPadding = static_cast<unsigned long>( ( numberOfSpans *
        splineDistance - domain ) / inputImage->GetSpacing()[d] + 0.5 );
      lowerBound[d] = static_cast<unsigned long>( 0.5 * extraPadding );
      upperBound[d] = extraPadding - lowerBound[d];
      newOrigin[d] -= ( static_cast<RealType>( lowerBound[d] ) *
        inputImage->GetSpacing()[d] );
      numberOfControlPoints[d] = numberOfSpans + correcter->GetSplineOrder();
    }

    typedef itk::ConstantPadImageFilter<ImageType, ImageType> PadderType;
    PadderType::Pointer padder = PadderType::New();
    padder->SetInput( inputImage );
    padder->SetPadLowerBound( lowerBound );
    padder->SetPadUpperBound( upperBound );
    padder->SetConstant( 0 );
    padder->Update();
    inputImage = padder->GetOutput();

    typedef itk::ConstantPadImageFilter<MaskImageType, MaskImageType> MaskPadderType;
    MaskPadderType::Pointer maskPadder = MaskPadderType::New();
    maskPadder->SetInput( maskImage );
    maskPadder->SetPadLowerBound( lowerBound );
    maskPadder->SetPadUpperBound( upperBound );
    maskPadder->SetConstant( 0 );
    maskPadder->Update();
    maskImage = maskPadder->GetOutput();

    if( weightImage ){
      PadderType::Pointer weightPadder = PadderType::New();
      weightPadder->SetInput( weightImage );
      weightPadder->SetPadLowerBound( lowerBound );
      weightPadder->SetPadUpperBound( upperBound );
      weightPadder->SetConstant( 0 );
      weightPadder->Update();
      weightImage = weightPadder->GetOutput();
    }
    correcter->SetNumberOfControlPoints( numberOfControlPoints );
  } else if(initialMeshResolution.size() == 3){
    for( unsigned d = 0; d < 3; d++ )
      numberOfControlPoints[d] = static_cast<unsigned int>( initialMeshResolution[d] ) +
        correcter->GetSplineOrder();
    correcter->SetNumberOfControlPoints( numberOfControlPoints );
  }
  

  typedef itk::ShrinkImageFilter<ImageType, ImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( inputImage );
  shrinker->SetShrinkFactors( 1 );

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  shrinker->SetShrinkFactors( shrinkFactor );
  maskshrinker->SetShrinkFactors( shrinkFactor );
  shrinker->Update();
  maskshrinker->Update();

  itk::TimeProbe timer;
  timer.Start();

  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );
  if( weightImage ) {
    typedef itk::ShrinkImageFilter<ImageType, ImageType> WeightShrinkerType;
    WeightShrinkerType::Pointer weightshrinker = WeightShrinkerType::New();
    weightshrinker->SetInput( weightImage );
    weightshrinker->SetShrinkFactors( 1 );
    weightshrinker->SetShrinkFactors( shrinkFactor );
    weightshrinker->Update();
    correcter->SetConfidenceImage( weightshrinker->GetOutput() );
  }

  typedef CommandIterationUpdate<CorrecterType> CommandType;
  CommandType::Pointer observer = CommandType::New();
  correcter->AddObserver( itk::IterationEvent(), observer );

  /**
   * hisogram sharpening opions
   */
  if( histogramSharpening.size() && histogramSharpening[0] )
    correcter->SetBiasFieldFullWidthAtHalfMaximum( histogramSharpening[0] );
  if( histogramSharpening.size()>1 && histogramSharpening[1] )
    correcter->SetWeinerFilterNoise( histogramSharpening[0] );
  if( histogramSharpening.size()>2 && histogramSharpening[3] )
    correcter->SetNumberOfHistogramBins( histogramSharpening[0] );

  try
    {
    itk::PluginFilterWatcher watchN4(correcter, "N4 Bias field correction", CLPProcessInformation, 1.0/1.0, 0.0);
    correcter->Update();
    }
  catch(...)
    {
    std::cerr << "Excepion caugh." << std::endl;
    return EXIT_FAILURE;
    }

  correcter->Print( std::cout, 3 );

  timer.Stop();
  std::cout << "Elapsed ime: " << timer.GetMeanTime() << std::endl;


  /**
   * ouput
   */
  if( outputImageName != ""){
    /**
     * Reconsruct the bias field at full image resoluion.  Divide
     * the original input image by the bias field to get the final
     * corrected image.
     */
    typedef itk::BSplineControlPointImageFilter<
      CorrecterType::BiasFieldControlPointLatticeType,
      CorrecterType::ScalarImageType> BSplinerType;
    BSplinerType::Pointer bspliner = BSplinerType::New();
    bspliner->SetInput( correcter->GetLogBiasFieldControlPointLattice() );
    bspliner->SetSplineOrder( correcter->GetSplineOrder() );
    bspliner->SetSize( inputImage->GetLargestPossibleRegion().GetSize() );
    bspliner->SetOrigin( newOrigin );
    bspliner->SetDirection( inputImage->GetDirection() );
    bspliner->SetSpacing( inputImage->GetSpacing() );
    bspliner->Update();

    ImageType::Pointer logField = ImageType::New();
    logField->SetOrigin( inputImage->GetOrigin() );
    logField->SetSpacing( inputImage->GetSpacing() );
    logField->SetRegions( inputImage->GetLargestPossibleRegion() );
    logField->SetDirection( inputImage->GetDirection() );
    logField->Allocate();

    itk::ImageRegionIterator<CorrecterType::ScalarImageType> IB(
      bspliner->GetOutput(),
      bspliner->GetOutput()->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<ImageType> IF( logField,
      logField->GetLargestPossibleRegion() );
    for( IB.GoToBegin(), IF.GoToBegin(); !IB.IsAtEnd(); ++IB, ++IF )
      IF.Set( IB.Get()[0] );

    typedef itk::ExpImageFilter<ImageType, ImageType> ExpFilterType;
    ExpFilterType::Pointer expFilter = ExpFilterType::New();
    expFilter->SetInput( logField );
    expFilter->Update();

    typedef itk::DivideImageFilter<ImageType, ImageType, ImageType> DividerType;
    DividerType::Pointer divider = DividerType::New();
    divider->SetInput1( inputImage );
    divider->SetInput2( expFilter->GetOutput() );
    divider->Update();

    ImageType::RegionType inputRegion;
    inputRegion.SetIndex( inputImageIndex );
    inputRegion.SetSize( inputImageSize );

    typedef itk::ExtractImageFilter<ImageType, ImageType> CropperType;
    CropperType::Pointer cropper = CropperType::New();
    cropper->SetInput( divider->GetOutput() );
    cropper->SetExtractionRegion( inputRegion );
    cropper->Update();

    CropperType::Pointer biasFieldCropper = CropperType::New();
    biasFieldCropper->SetInput( expFilter->GetOutput() );
    biasFieldCropper->SetExtractionRegion( inputRegion );
    biasFieldCropper->Update();

    typedef  itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( cropper->GetOutput() );
    writer->SetFileName( outputImageName.c_str() );
    writer->Update();

    if(outputBiasFieldName != "" ){
      typedef itk::ImageFileWriter<ImageType> WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( outputBiasFieldName.c_str() );
      writer->SetInput( biasFieldCropper->GetOutput() );
      writer->Update();
    }
  }

  return EXIT_SUCCESS;
}

