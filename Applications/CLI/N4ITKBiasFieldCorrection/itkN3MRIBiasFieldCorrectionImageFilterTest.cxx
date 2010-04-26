#include "itkBSplineControlPointImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkShrinkImageFilter.h"

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

    std::cout << "Iteration " << filter->GetElapsedIterations()
      << " (of " << filter->GetMaximumNumberOfIterations() << ").  ";
    std::cout << " Current convergence value = "
      << filter->GetCurrentConvergenceMeasurement()
      << " (threshold = " << filter->GetConvergenceThreshold()
      << ")" << std::endl;
    }

};

template <unsigned int ImageDimension>
int itkN3MRIBiasFieldCorrectionImageFilterTest( int argc, char *argv[] )
{
  typedef float RealType;

  typedef itk::Image<RealType, ImageDimension> ImageType;
  typedef itk::Image<unsigned char, ImageDimension> MaskImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[2] );
  reader->Update();

  typedef itk::ShrinkImageFilter<ImageType, ImageType> ShrinkerType;
  typename ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( reader->GetOutput() );
  shrinker->SetShrinkFactors( 1 );

  typename MaskImageType::Pointer maskImage = NULL;

  if( argc > 5 )
    {
    typedef itk::ImageFileReader<MaskImageType> MaskReaderType;
    typename MaskReaderType::Pointer maskreader = MaskReaderType::New();
    maskreader->SetFileName( argv[5] );

    try
      {
      maskreader->Update();
      maskImage = maskreader->GetOutput();
      }
    catch(...)
      {
      std::cout << "Mask file not read.  Generating mask file using otsu"
        << " thresholding." << std::endl;
      }
    }
  if( !maskImage )
    {
    typedef itk::OtsuThresholdImageFilter<ImageType, MaskImageType>
      ThresholderType;
    typename ThresholderType::Pointer otsu = ThresholderType::New();
    otsu->SetInput( reader->GetOutput() );
    otsu->SetNumberOfHistogramBins( 200 );
    otsu->SetInsideValue( 0 );
    otsu->SetOutsideValue( 1 );
    otsu->Update();

    maskImage = otsu->GetOutput();
    }
  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  typename MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  if( argc > 4 )
    {
    shrinker->SetShrinkFactors( atoi( argv[4] ) );
    maskshrinker->SetShrinkFactors( atoi( argv[4] ) );
    }
  shrinker->Update();
  maskshrinker->Update();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<ImageType, MaskImageType,
    ImageType> CorrecterType;
  typename CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );

  if( argc > 6 )
    {
    correcter->SetMaximumNumberOfIterations( atoi( argv[6] ) );
    }
  if( argc > 7 )
    {
    correcter->SetNumberOfFittingLevels( atoi( argv[7] ) );
    }


  typedef CommandIterationUpdate<CorrecterType> CommandType;
  typename CommandType::Pointer observer = CommandType::New();
  correcter->AddObserver( itk::IterationEvent(), observer );

  try
    {
    correcter->Update();
    }
  catch(...)
    {
    std::cerr << "Exception caught." << std::endl;
    return EXIT_FAILURE;
    }

//  correcter->Print( std::cout, 3 );

  /**
   * Reconstruct the bias field at full image resolution.  Divide
   * the original input image by the bias field to get the final
   * corrected image.
   */
  typedef itk::BSplineControlPointImageFilter<typename
    CorrecterType::BiasFieldControlPointLatticeType, typename
    CorrecterType::ScalarImageType> BSplinerType;
  typename BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetLogBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
    reader->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( reader->GetOutput()->GetOrigin() );
  bspliner->SetDirection( reader->GetOutput()->GetDirection() );
  bspliner->SetSpacing( reader->GetOutput()->GetSpacing() );
  bspliner->Update();

  typename ImageType::Pointer logField = ImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator<typename CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<ImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  typedef itk::ExpImageFilter<ImageType, ImageType> ExpFilterType;
  typename ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  typedef itk::DivideImageFilter<ImageType, ImageType, ImageType> DividerType;
  typename DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( reader->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();

  typedef itk::ImageFileWriter<ImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( argv[3] );
  writer->SetInput( divider->GetOutput() );
  writer->Update();

  if( argc > 8 )
    {
    typedef itk::ImageFileWriter<ImageType> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( argv[8] );
    writer->SetInput( expFilter->GetOutput() );
    writer->Update();
    }

  return EXIT_SUCCESS;
}

int main( int argc, char *argv[] )
{
  if ( argc < 4 )
    {
    std::cerr << "Usage: " << argv[0] << " imageDimension inputImage "
     << "outputImage [shrinkFactor] [maskImage] [numberOfIterations] "
     << "[numberOfFittingLevels] [outputBiasField] " << std::endl;
    exit( EXIT_FAILURE );
    }

  switch( atoi( argv[1] ) )
   {
   case 2:
     itkN3MRIBiasFieldCorrectionImageFilterTest<2>( argc, argv );
     break;
   case 3:
     itkN3MRIBiasFieldCorrectionImageFilterTest<3>( argc, argv );
     break;
   default:
      std::cerr << "Unsupported dimension" << std::endl;
      exit( EXIT_FAILURE );
   }
}

