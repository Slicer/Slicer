#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkMRIBiasFieldCorrectionFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkBSplineControlPointImageFilter.h"

#include "itkBinaryThresholdImageFilter.h"

#include "itkPluginUtilities.h"
#include "MRIBiasFieldCorrectionCommandLineCLP.h"

#include "itkBSplineControlPointImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkShrinkImageFilter.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {

template<class T> int DoIt( int argc, char * argv[], T )
{

  PARSE_ARGS;

  //Set-up reading and writing functionnalities
  typedef    float   InputPixelType;
  typedef    T       OutputPixelType;

  typedef itk::Image< InputPixelType,  2 >   InputImageType;
  typedef itk::Image< unsigned char,   2 >   MaskImageType;
  typedef itk::Image< InputPixelType,  2 >   OutputImageType;
  typedef itk::ImageFileReader< InputImageType >  ReaderType;


  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName( inputVolume.c_str() );
  reader->Update();

  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  typename ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( reader->GetOutput() );
  shrinker->SetShrinkFactors( 1 );

  // Thresholding filter

  typename MaskImageType::Pointer maskImage = NULL;

  typedef itk::BinaryThresholdImageFilter<
               InputImageType, MaskImageType>  mFilterType;
  typename mFilterType::Pointer mfilter = mFilterType::New();

  mfilter->SetInput( reader->GetOutput() );
  mfilter->SetLowerThreshold(threshold);
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);
  mfilter->Update();

  maskImage = mfilter->GetOutput();

  // Shrink Filter

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  typename MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  shrinker->SetShrinkFactors(3);
  maskshrinker->SetShrinkFactors(3);
  
  
  shrinker->Update();
  maskshrinker->Update();
  
// Bias Correction Filter

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType, MaskImageType,
    InputImageType> CorrecterType;
  typename CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );
  
  
    correcter->SetMaximumNumberOfIterations( numberOfIterations );

    correcter->SetNumberOfFittingLevels(numberOfFitting);
    
    correcter->SetWeinerFilterNoise(weinerFilterNoise);
    
    correcter->SetBiasFieldFullWidthAtHalfMaximum(biasFieldFullWidthAtHalfMaximum);
    
    correcter->SetConvergenceThreshold(convergenceThreshold);

  try
    {
    correcter->Update();
    }
  catch(...)
    {
    std::cerr << "Exception caught." << std::endl;
    return EXIT_FAILURE;
    }
  
// BSpline Control Filter
  
  typedef itk::BSplineControlPointImageFilter<typename
    CorrecterType::BiasFieldControlPointLatticeType, typename
    CorrecterType::ScalarImageType> BSplinerType;
  typename BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
    reader->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( reader->GetOutput()->GetOrigin() );
  bspliner->SetDirection( reader->GetOutput()->GetDirection() );
  bspliner->SetSpacing( reader->GetOutput()->GetSpacing() );
  bspliner->Update();

  typename InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator<typename CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  typedef itk::ExpImageFilter<InputImageType, InputImageType> ExpFilterType;
  typename ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  typedef itk::DivideImageFilter<InputImageType, InputImageType, InputImageType> DividerType;
  typename DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( reader->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();

  typedef itk::ImageFileWriter<OutputImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( divider->GetOutput());
  writer->Update();


  
 
  return EXIT_SUCCESS;
}

} // end of anonymous namespace


int main( int argc, char * argv[] )
{
  
  PARSE_ARGS;
  
  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;
  
  try
    {
    itk::GetImageType (inputVolume, pixelType, componentType);

    // This filter handles all types
    
    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0));
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0));
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0));
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0));
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0));
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0));
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0));
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0));
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0));
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0));
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cerr<< "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
