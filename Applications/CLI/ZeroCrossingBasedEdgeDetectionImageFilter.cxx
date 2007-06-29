#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPluginUtilities.h"


#include "itkZeroCrossingBasedEdgeDetectionImageFilter.h"
#include "itkCastImageFilter.h"

#include "ZeroCrossingBasedEdgeDetectionImageFilterCLP.h"

template<class T> int DoIt( int argc, char * argv[], T )
{
 
  PARSE_ARGS;

  typedef T InputPixelType;  
  typedef float InternalPixelType;
  typedef T OutputPixelType;  

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< InternalPixelType, 3 > InternalImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  typedef itk::CastImageFilter<
          InputImageType, InternalImageType >   InputFilterType;
  typedef itk::ZeroCrossingBasedEdgeDetectionImageFilter< 
          InternalImageType, InternalImageType >    ZeroFilterType;
  typedef itk::CastImageFilter<
          InternalImageType, OutputImageType > OutputFilterType;

  typename InputFilterType::Pointer inputFilter = InputFilterType::New();
  typename ZeroFilterType::Pointer  zeroFilter = ZeroFilterType::New();
  typename OutputFilterType::Pointer outputFilter = OutputFilterType::New(); 

  itk::PluginFilterWatcher watcher(zeroFilter, "Zero Crossing Based Edge Detection",
    CLPProcessInformation); 

  inputFilter->SetInput( reader->GetOutput() );

  zeroFilter->SetInput( inputFilter->GetOutput() );
  zeroFilter->SetMaximumError( maximumError ); 
  zeroFilter->SetVariance( variance );

  outputFilter->SetInput( zeroFilter->GetOutput() );

  writer->SetInput( outputFilter->GetOutput() );
  writer->Update();
  return EXIT_SUCCESS;    
}


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
         std::cout << "unknown component type" << std::endl;   
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
