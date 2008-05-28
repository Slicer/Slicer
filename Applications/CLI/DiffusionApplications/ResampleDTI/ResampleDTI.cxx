#include "itkDiffusionTensor3DResample.h"
#include "itkDiffusionTensor3DRigidTransform.h"
#include "itkDiffusionTensor3DFSAffineTransform.h"
#include "itkDiffusionTensor3DPPDAffineTransform.h"
#include "itkDiffusionTensor3DNonRigidTransform.h"
#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.h"
#include "itkDiffusionTensor3DLinearInterpolateFunction.h"
#include "itkDiffusionTensor3DWindowedSincInterpolateImageFunction.h"
#include "itkDiffusionTensor3DBSplineInterpolateImageFunction.h"
#include <itkPeriodicBoundaryCondition.h>
#include <itkMatrix.h>
#include <itkPoint.h>
#include <itkImageFileReader.h>
#include <itkImageIOBase.h>
#include <itkOrientedImage.h>
#include <itkTransformFileReader.h>
#include "ResampleDTICLP.h"
#include <itkDiffusionTensor3D.h>
#include "itkDiffusionTensor3DZeroCorrection.h"
#include "itkDiffusionTensor3DAbsCorrection.h"
#include "itkDiffusionTensor3DNearestCorrection.h"
#include "itkDiffusionTensor3DRead.h"
#include "itkDiffusionTensor3DWrite.h"

#define RADIUS 3


//Structure of the command lign parameters
struct parameters
{
  std::vector< double > outputImageSpacing ;
  std::vector< double > outputImageSize ;
  std::vector< float > outputImageOrigin ;
  int numberOfThread ;
  std::string interpolationType ;
  std::string transformType ;
  std::vector< double > directionMatrix ;
  std::vector< double > transformMatrix ;
  std::string inputVolume ;
  std::string outputVolume ;
  std::vector< float > rotationPoint ;
  std::string transformationFile ;
  bool inverseITKTransformation ;
  std::string windowFunction ;
  unsigned int splineOrder ;
  std::string referenceVolume ;
  std::string space ;
  bool centeredTransform ;
  bool ppd ;
  std::string correction;
};


//Verify if some input parameters are null
bool VectorIsNul( std::vector< double > vec )
{
  bool zero = 1 ;
  for( int i = 0 ; i < vec.size() ; i++ ) { if( vec[i] != 0 ) { zero = 0 ; } }
  return zero;
}

//What pixeltype is the image 
void GetImageType( std::string fileName ,
                   itk::ImageIOBase::IOPixelType &pixelType ,
                   itk::ImageIOBase::IOComponentType &componentType )
{
  typedef itk::Image< unsigned char , 3 > ImageType ;
  itk::ImageFileReader< ImageType >::Pointer imageReader =
                             itk::ImageFileReader< ImageType >::New();
  imageReader->SetFileName( fileName.c_str() ) ;
  imageReader->UpdateOutputInformation() ;
  pixelType = imageReader->GetImageIO()->GetPixelType() ;
  componentType = imageReader->GetImageIO()->GetComponentType() ;
}




//Select which interpolator will be used
template< class PixelType >
typename itk::DiffusionTensor3DInterpolateImageFunction< PixelType >
::Pointer
InterpolationType( std::string interpolationType ,
                             std::string windowFunction ,
                             unsigned int splineOrder )
{
  typedef itk::DiffusionTensor3DInterpolateImageFunction< PixelType >
                                                             InterpolatorType ; 
  typedef itk::DiffusionTensor3DNearestNeighborInterpolateFunction< PixelType >
                                          NearestNeighborhoodInterpolatorType ;
  typedef itk::DiffusionTensor3DLinearInterpolateFunction< PixelType >
                                                       LinearInterpolatorType ;
  typedef itk::ConstantBoundaryCondition< itk::OrientedImage< PixelType,3 > >
                                                        BoundaryConditionType ;
  typedef itk::Function::HammingWindowFunction< RADIUS > 
                                                 HammingwindowFunctionType ;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
         < PixelType ,
           RADIUS ,
           HammingwindowFunctionType ,
           BoundaryConditionType > HammingWindowedSincInterpolateFunctionType ;
  typedef itk::Function::CosineWindowFunction<RADIUS>
                                    CosinewindowFunctionType ;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
          < PixelType ,
            RADIUS ,
            CosinewindowFunctionType ,
            BoundaryConditionType > CosineWindowedSincInterpolateFunctionType ;
  typedef itk::Function::WelchWindowFunction< RADIUS >
                                      WelchwindowFunctionType ;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
           < PixelType ,
             RADIUS ,
             WelchwindowFunctionType ,
             BoundaryConditionType > WelchWindowedSincInterpolateFunctionType ;
  typedef itk::Function::LanczosWindowFunction< RADIUS >
                                          LanczoswindowFunctionType ;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
          < PixelType ,
            RADIUS ,
            LanczoswindowFunctionType ,
            BoundaryConditionType > LanczosWindowedSincInterpolateFunctionType ;
  typedef itk::Function::BlackmanWindowFunction< RADIUS > 
                                  BlackmanwindowFunctionType ;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
        < PixelType ,
          RADIUS ,
          BlackmanwindowFunctionType ,
          BoundaryConditionType > BlackmanWindowedSincInterpolateFunctionType ;
  typedef itk::DiffusionTensor3DBSplineInterpolateImageFunction< PixelType >
                                          BSplineInterpolateImageFunctionType ;
  typedef typename InterpolatorType::Pointer InterpolatorTypePointer ;
  InterpolatorTypePointer interpol ;
  if( !interpolationType.compare( "nn" ) )//nearest neighborhood
    { interpol = NearestNeighborhoodInterpolatorType::New() ; }
  else if( !interpolationType.compare( "linear" ) )//linear
    { interpol = LinearInterpolatorType::New() ; }
  else if( !interpolationType.compare( "ws" ) )//windowed sinc
  {
    if( !windowFunction.compare( "h" ) ) //Hamming window
      { interpol = HammingWindowedSincInterpolateFunctionType::New() ; }
    else if( !windowFunction.compare( "c" ) )//cosine window
      { interpol = CosineWindowedSincInterpolateFunctionType::New() ; }
    else if( !windowFunction.compare( "w" ) ) //Welch window
      { interpol = WelchWindowedSincInterpolateFunctionType::New() ; }
    else if( !windowFunction.compare( "l" ) ) // Lanczos window
      { interpol = LanczosWindowedSincInterpolateFunctionType::New() ; }
    else if( !windowFunction.compare( "b" ) ) // Blackman window
      { interpol = BlackmanWindowedSincInterpolateFunctionType::New() ; }   
  }
  else if( !interpolationType.compare( "bs" ) )//BSpline interpolation
    {
    typename BSplineInterpolateImageFunctionType::Pointer
               bSplineInterpolator=BSplineInterpolateImageFunctionType::New() ;
    bSplineInterpolator->SetSplineOrder( splineOrder ) ;
    interpol = bSplineInterpolator ;
    }
  return interpol ;
}


void ReadTransform( parameters list , itk::TransformFileReader::Pointer &transformFile)
{
 if( list.transformationFile.compare( "" ) )
    {
    transformFile=itk::TransformFileReader::New() ;
    transformFile->SetFileName( list.transformationFile.c_str() ) ;
    transformFile->Update() ;
    }
}

//Set the transformation
template< class PixelType >
typename itk::DiffusionTensor3DTransform< PixelType >::Pointer
SetTransform( parameters list , 
              typename itk::DiffusionTensor3DRead< PixelType >
                          ::Pointer &reader ,itk::TransformFileReader::Pointer &transformFile )
{    
  typedef itk::DiffusionTensor3DTransform< PixelType > TransformType ;
  typedef itk::DiffusionTensor3DNonRigidTransform< PixelType >
                                                     NonRigidTransformType ;
  typedef typename TransformType::Pointer TransformTypePointer ;

  typedef itk::DiffusionTensor3DRigidTransform< PixelType > RigidTransformType ;
  typedef itk::DiffusionTensor3DFSAffineTransform< PixelType >
                                                 FSAffineTransformType ;
  typedef itk::DiffusionTensor3DPPDAffineTransform< PixelType >
                                                PPDAffineTransformType ;
  typename NonRigidTransformType::TransformType::Pointer nonRigidFile ;
  bool precisionChecking = 1 ;
  TransformTypePointer transform ;
  itk::Vector< double , 3 > vec ;
  itk::Matrix< double , 4 , 4 > transformMatrix4x4 ;
  transformMatrix4x4.SetIdentity() ;
  //int size = list.transformMatrix.size() ;
  //Get transformation matrix from the given file
  if( list.transformationFile.compare( "" ) )
    {
    list.transformMatrix.resize( 0 ) ;
    list.rotationPoint.resize( 0 ) ;
    //size=transformFile->GetTransformList()->front()->GetParameters().GetSize() ;
    typename FSAffineTransformType::Superclass::AffineTransformType::Pointer
       affinetransform = dynamic_cast< 
         typename FSAffineTransformType::Superclass::AffineTransformType* >
          ( transformFile->GetTransformList()->front().GetPointer() ) ;
    if( affinetransform )//if affine transform
      {
      list.transformType.assign( "a" ) ;
      for( int i = 0 ; i < 3 ; i++ )
             {
             for( int j = 0 ; j < 3 ; j++)
                {
                list.transformMatrix.push_back(affinetransform->GetMatrix()[i][j]);
                }
             }
            for(int i = 0 ; i < 3 ; i++)
               {
               list.transformMatrix.push_back(affinetransform->GetTranslation()[i]) ;
               list.rotationPoint.push_back( affinetransform->GetCenter()[i] );
               }
      }
    else
      {
      typename RigidTransformType::Rigid3DTransformType::Pointer
         rigid3dtransform = dynamic_cast< 
           typename RigidTransformType::Rigid3DTransformType* >
             (transformFile->GetTransformList()->front().GetPointer() ) ;
      if( rigid3dtransform )//if rigid3D transform
        {
        list.transformType.assign( "rt" ) ;
        precisionChecking = 0 ;
        for( int i = 0 ; i < 3 ; i++ )
             {
             for( int j = 0 ; j < 3 ; j++)
                {
                list.transformMatrix.push_back(rigid3dtransform->GetMatrix()[i][j]);
                }
             }
            for(int i = 0 ; i < 3 ; i++)
               {
               list.transformMatrix.push_back(rigid3dtransform->GetTranslation()[i]) ;
               list.rotationPoint.push_back( rigid3dtransform->GetCenter()[i] );
               }
        }
      else//if non-rigid
        {
        nonRigidFile = dynamic_cast<
           typename NonRigidTransformType::TransformType* >
              ( transformFile->GetTransformList()->front().GetPointer() ) ;
        if(nonRigidFile)//if non rigid Transform loaded
          {
          list.transformType.assign( "nr" ) ;
          }
        else//something else
          {
          std::cerr<< "Transformation type not yet implemented for tensors"
                   << std::endl ;
          return NULL ;
          }
        }
      }
    if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
      {
      /*//get transform matrix and translation vector
      for( int i = 0 ; i < size ; i++ )
        { list.transformMatrix.push_back( 
              transformFile->GetTransformList()->front()->GetParameters().GetElement( i ) ) ; }
      //Get center of transform
      for( int i = 0 ; 
           i < transformFile->GetTransformList()->front()->GetFixedParameters().GetSize() ;
           i++ )
        { list.rotationPoint.push_back( 
             transformFile->GetTransformList()->front()->GetFixedParameters().GetElement( i ) ) ; }*/
      //if problem in the number of parameters
      if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
        {
        std::cerr<< "Error in the file containing the transformation"
                 << std::endl ;
        return NULL ;
        }
      }
    transformFile->GetTransformList()->pop_front();
    }
  if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
    {
    itk::Point< double > center ;
    itk::Vector< double > translation ;
    itk::Vector< double > offset ;
    //if centered transform set, ignore the transformation center
    // set before and use the center of the image instead
    if( list.centeredTransform )
      {
      typename itk::DiffusionTensor3DRead< PixelType >
                  ::DiffusionImageType::IndexType index ;
      typename itk::DiffusionTensor3DRead< PixelType >
                  ::DiffusionImageType::SizeType sizeim
                  = reader->GetOutput()->GetLargestPossibleRegion().GetSize() ;
      itk::Point< double , 3 > point ;
      itk::Point< double , 3 > pointOpposite ;
      for( int i = 0 ; i < 3 ; i++ ) { index[ i ] = 0 ; }
      reader->GetOutput()->TransformIndexToPhysicalPoint( index , point ) ;
      for( int i = 0 ; i < 3 ; i++ ) { index[ i ] = sizeim[ i ] - 1 ; }
      reader->GetOutput()->TransformIndexToPhysicalPoint( index ,
                                                      pointOpposite ) ;
      for( int i = 0 ; i < 3 ; i++ )
              { center[ i ] = ( point[ i ] + pointOpposite[ i ] )/2 ; }
      }
    else
      {
      for( int i = 0 ; i < 3 ; i++ )
            { center[ i ] = list.rotationPoint[ i ] ; }
      } 
    //Set the transform matrix  
    for( int i = 0 ; i < 3 ; i++ )
      {
      for( int j = 0 ; j< 3 ; j++ )
        {
        transformMatrix4x4[ i ][ j ]
            = ( double ) list.transformMatrix[ i*3 + j  ] ;    
        }
      translation[ i ] = ( double ) list.transformMatrix[ 9 + i ] ;
      }
    //Compute the 4th column of the matrix
    for( int i = 0 ; i < 3 ; i++ )
      {
      offset[ i ] = translation[ i ] + center[ i ] ;
      for( int j = 0 ; j < 3 ; j++ )
        {
        offset[ i ] -= transformMatrix4x4[ i ][ j ] * center[ j ] ;
        }
      }    
    for( int i = 0 ; i < 3 ; i++ )
      {
      transformMatrix4x4[ i ][ 3 ] = offset[ i ] ;
      }     
    //If set, inverse the given transform.
    //The given transform therefore transform the input image into the output image    
    if( list.inverseITKTransformation )
      {
      transformMatrix4x4 = transformMatrix4x4.GetInverse() ;
      }
    //If the transform is in RAS, transform it in LPS  
    if( !list.space.compare( "RAS" )  & !list.transformationFile.compare( "" )  )
      {
      itk::Matrix< double , 4 , 4 > ras ;
      ras.SetIdentity() ;
      ras[ 0 ][ 0 ] = -1 ;
      ras[ 1 ][ 1 ] = -1 ;
      transformMatrix4x4 = ras * transformMatrix4x4 * ras ;            
      }
    //Rigid Transform (rotation+translation)
    if( !list.transformType.compare( "rt" ) )
      {    
      typename RigidTransformType::Pointer rotation = RigidTransformType::New() ;
      rotation->SetPrecisionChecking( precisionChecking ) ;
      rotation->SetMatrix4x4( transformMatrix4x4 ) ;
      transform = rotation ;
      }
    //Affine transform
    else if( !list.transformType.compare( "a" ) )
      {
      // Finite Strain  
      if( !list.ppd )
        {
        typename FSAffineTransformType::Pointer affine = FSAffineTransformType::New() ;
        affine->SetMatrix4x4( transformMatrix4x4 ) ;
        transform = affine ;
        }
      // Preservation of principal direction
      else
        {
        typename PPDAffineTransformType::Pointer affineppd
                           = PPDAffineTransformType::New() ;
        affineppd->SetMatrix4x4( transformMatrix4x4 ) ;
        transform = affineppd ;
        }
      }
    }
  else
    {
    typename NonRigidTransformType::Pointer nonRigid
                          = NonRigidTransformType::New() ;
    nonRigid->SetTransform( nonRigidFile ) ;
    typename PPDAffineTransformType::Pointer affineppd
                           = PPDAffineTransformType::New() ;
    typename itk::DiffusionTensor3DAffineTransform<PixelType>::Pointer affine;
    affine=affineppd;
    nonRigid->SetAffineTransformType(affine);
    transform = nonRigid ;
    }
 return transform ;
}





//Set Output parameters
template< class PixelType >
void SetOutputParameters(parameters list ,
           typename itk::DiffusionTensor3DResample< PixelType , PixelType >
                       ::Pointer &resampler ,
           typename itk::DiffusionTensor3DRead< PixelType >::Pointer &reader )
{
  typedef itk::DiffusionTensor3DResample< PixelType , PixelType > ResamplerType ;
  typedef itk::DiffusionTensor3DRead< PixelType > ReaderType ;
  typedef typename ReaderType::Pointer ReaderTypePointer ;
  ReaderTypePointer readerReference ;
  //is there a reference image to set the size, the orientation,
  // the spacing and the origin of the output image?
  if( list.referenceVolume.compare( "" ) )
    {
    readerReference = ReaderType::New() ;
    readerReference->Update( list.referenceVolume.c_str() ) ;
    }
  resampler->SetOutputParametersFromImage( reader->GetOutput() ) ;
  typename ResamplerType::OutputImageType::SpacingType m_Spacing ;
  typename ResamplerType::OutputImageType::PointType m_Origin ;
  typename ResamplerType::OutputImageType::DirectionType m_Direction ;
  typename ResamplerType::OutputImageType::SizeType m_Size ;
  if( VectorIsNul( list.outputImageSpacing ) )
    {
    if( list.referenceVolume.compare( "" ) )
       { m_Spacing = readerReference->GetOutput()->GetSpacing() ; }
    else
      { m_Spacing = reader->GetOutput()->GetSpacing() ; }
    }
  else
    {
    for( int i = 0 ; i < 3 ; i++ ) { m_Spacing[ i ] = list.outputImageSpacing[ i ] ; }
    }
  if( VectorIsNul( list.outputImageSize) ) 
    {
    if( list.referenceVolume.compare( "" ) )
      { m_Size
        = readerReference->GetOutput()->GetLargestPossibleRegion().GetSize() ;
      }
    else
      { m_Size = reader->GetOutput()->GetLargestPossibleRegion().GetSize() ; }
    }
  else
    {
    for( int i = 0 ; i < 3 ; i++ ) 
      { m_Size[ i ] = ( unsigned long ) list.outputImageSize[ i ] ; }
    }
  if( list.outputImageOrigin.size() == 0 )
    {
    if( list.referenceVolume.compare( "" ) )
      { m_Origin = readerReference->GetOutput()->GetOrigin() ; }
    else
      { m_Origin = reader->GetOutput()->GetOrigin() ; }
    }
  else
    {
    for(int i = 0 ; i < 3 ; i++ )
      { m_Origin[ i ] = list.outputImageOrigin[ i ] ; }
    }
  if( VectorIsNul( list.directionMatrix ) )
    {
    if( list.referenceVolume.compare( "" ) )
      { m_Direction = readerReference->GetOutput()->GetDirection() ; }
    else
      { m_Direction = reader->GetOutput()->GetDirection() ; }
    }
  else
    {
    for( int i = 0 ; i < 3 ; i++ )
      {
      for( int j = 0 ; j < 3 ; j++ )
        {
        m_Direction[ i ][ j ] = ( double ) list.directionMatrix[ i * 3 + j ] ;
        }
      }
    }
  resampler->SetOutputSpacing( m_Spacing ) ;
  resampler->SetOutputSize( m_Size ) ;
  resampler->SetOutputOrigin( m_Origin ) ;
  resampler->SetOutputDirection( m_Direction ) ; 
}



template< class PixelType > void RASLPS(typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >::Pointer image)
{
  typename itk::VectorImage< PixelType, 3 >::PointType m_Origin ;
  typename itk::VectorImage< PixelType, 3 >::DirectionType m_Direction ;
  m_Origin = image->GetOrigin() ;
  m_Direction = image->GetDirection();
  m_Origin[0]=-m_Origin[0];
  m_Origin[1]=-m_Origin[1];
  itk::Matrix< double , 3 , 3 > ras ;
  ras.SetIdentity() ;
  ras[ 0 ][ 0 ] = -1 ;
  ras[ 1 ][ 1 ] = -1 ;
  m_Direction=ras*m_Direction;
  image->SetOrigin( m_Origin ) ;
  image->SetDirection( m_Direction ) ; 
}



//Check all input parameters and transform the input image
//Template over the pixeltype of the input image
template< class PixelType >
int Do( parameters list )
{
  try
  {
    typedef itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >
                                                               InputImageType ;
    typename InputImageType::Pointer image;
    typedef itk::DiffusionTensor3DRead< PixelType > ReaderType ;
    typedef typename ReaderType::Pointer ReaderTypePointer ;
    typedef itk::DiffusionTensor3DWrite< PixelType > WriterType ;
    typedef typename WriterType::Pointer WriterTypePointer ;
    ReaderTypePointer reader = ReaderType::New() ;
    //Read input volume
    if(list.numberOfThread) 
      { reader->SetNumberOfThreads( list.numberOfThread ) ; }
    reader->Update( list.inputVolume.c_str() ) ;
    image=reader->GetOutput();
    if( !list.space.compare( "RAS" ) &&  list.transformationFile.compare( "" ) )
      { RASLPS<PixelType>(image); }
    WriterTypePointer writer = WriterType::New() ;
    if(list.numberOfThread) 
      { writer->SetNumberOfThreads( list.numberOfThread ) ; }
    writer->SetMetaDataDictionary( reader->GetMetaDataDictionary() ) ;
    typedef itk::DiffusionTensor3DInterpolateImageFunction< PixelType >
                                                        InterpolatorType ; 
    typedef typename InterpolatorType::Pointer InterpolatorTypePointer ;
    InterpolatorTypePointer interpol ;
    //Select interpolation type
    interpol=InterpolationType< PixelType > ( list.interpolationType ,
                                            list.windowFunction ,
                                            list.splineOrder ) ;

      
    //Select the transformation
    typedef itk::TransformFileReader::Pointer TransformReaderPointer ;
    TransformReaderPointer transformFile ;
    ReadTransform( list , transformFile );
    //if the transform is in RAS space coordinate, add a transform to transform the image into that space 
    typedef itk::DiffusionTensor3DTransform< PixelType > TransformType ;
    typedef typename TransformType::Pointer TransformTypePointer ;
    //start transform
    //loop over the transform list (multiple transforms can be use through itkTransform files)
    do
      {
      TransformTypePointer transform ;
      transform = SetTransform< PixelType > ( list , reader , transformFile ) ;
      if( transform.IsNull() )
        { return -1 ; }
      transform->SetMeasurementFrame( reader->GetMeasurementFrame() ) ;
      typedef itk::DiffusionTensor3DResample< PixelType , PixelType > ResamplerType ;
      typedef typename ResamplerType::Pointer ResamplerTypePointer ;
      ResamplerTypePointer resampler = ResamplerType::New() ;
      resampler->SetInput( image ) ;
      resampler->SetTransform( transform ) ;
      if(list.numberOfThread) 
        { resampler->SetNumberOfThreads( list.numberOfThread ) ; }
      resampler->SetInterpolator( interpol ) ;
      SetOutputParameters< PixelType > ( list , resampler , reader) ;
      //Compute the resampled image
      resampler->Update() ;
      image = resampler->GetOutput();
      writer->SetMeasurementFrame(resampler->GetOutputMeasurementFrame() ) ;
      }while( list.transformationFile.compare( "" ) 
              && transformFile->GetTransformList()->size() );
    //Tensors Corrections
    if( !list.correction.compare("zero") )
    {
      typedef itk::DiffusionTensor3DZeroCorrectionFilter<InputImageType , InputImageType > ZeroCorrection;
      typename ZeroCorrection::Pointer zeroFilter;
      zeroFilter = ZeroCorrection::New();
      zeroFilter->SetInput(image);
      if(list.numberOfThread) 
        { zeroFilter->SetNumberOfThreads( list.numberOfThread ) ; }
      zeroFilter->Update();
      image = zeroFilter->GetOutput();
    }
    else if( !list.correction.compare("abs") )
    {
      typedef itk::DiffusionTensor3DAbsCorrectionFilter<InputImageType , InputImageType > AbsCorrection;
      typename AbsCorrection::Pointer absFilter;
      absFilter = AbsCorrection::New();
      absFilter->SetInput(image);
      if(list.numberOfThread) 
        { absFilter->SetNumberOfThreads( list.numberOfThread ) ; }
      absFilter->Update();
      image = absFilter->GetOutput();
    }
    else if( !list.correction.compare("nearest") )
    {
      typedef itk::DiffusionTensor3DNearestCorrectionFilter<InputImageType , InputImageType > NearestCorrection;
      typename NearestCorrection::Pointer nearestFilter;
      nearestFilter = NearestCorrection::New();
      nearestFilter->SetInput(image);
      if(list.numberOfThread) 
        { nearestFilter->SetNumberOfThreads( list.numberOfThread ) ; }
      nearestFilter->Update();
      image = nearestFilter->GetOutput();
    }
    //Save result
    if( !list.space.compare( "RAS" ) &&  list.transformationFile.compare( "" ) )
      { RASLPS<PixelType>(image); }
    writer->SetInput( image );
    //Save output image
    writer->Update( list.outputVolume.c_str() ) ;
    return 0 ;
    }
  catch( itk::ExceptionObject & Except )
    {
    std::cerr<< "Main: Do: Exception caught!"
           << std::endl ;
    std::cerr<< Except << std::endl ;
    return -1 ;
    }
}




int main( int argc , const char * argv[] )
{
  PARSE_ARGS ;
  parameters list ;

  //Copy all the input parameters into a structure,
  // to pass everything to the template function
  list.outputImageSpacing = outputImageSpacing ;
  list.outputImageSize = outputImageSize ;
  list.outputImageOrigin = outputImageOrigin ;
  list.numberOfThread = numberOfThread ;
  list.interpolationType = interpolationType ;
  list.transformType = transformType ;
  list.directionMatrix = directionMatrix ;
  list.transformMatrix = transformMatrix ;
  list.inputVolume = inputVolume ;
  list.outputVolume = outputVolume ;
  list.rotationPoint = rotationPoint ;
  list.transformationFile = transformationFile ;
  list.inverseITKTransformation = inverseITKTransformation ;
  list.windowFunction = windowFunction ;
  list.splineOrder = splineOrder ;
  list.referenceVolume = referenceVolume ;
  list.space = space ;
  list.centeredTransform = centeredTransform ;
  list.ppd = ppd ;
  list.correction = correction ;
  //verify if all the vector parameters have the good length
  if( list.outputImageSpacing.size() != 3 || list.outputImageSize.size() != 3
     || ( list.outputImageOrigin.size() != 3 
          && list.outputImageOrigin.size() != 0 )
     || list.directionMatrix.size() != 9 || rotationPoint.size() != 3
     || list.transformMatrix.size() != 12 )
    {
    std::cerr<< "Argument(s) having wrong size" << std::endl ;
    return -1 ;
    }
  itk::ImageIOBase::IOPixelType pixelType ;
  itk::ImageIOBase::IOComponentType componentType ;
  //Check the input image pixel type
  GetImageType( inputVolume , pixelType , componentType ) ;
  switch( componentType )
    {
    case itk::ImageIOBase::UCHAR :
         return Do< unsigned char > ( list ) ;
         break ;
    case itk::ImageIOBase::CHAR :
         return Do< char > ( list ) ;
         break ;
    case itk::ImageIOBase::USHORT :
         return Do< unsigned short >( list ) ;
         break ;
    case itk::ImageIOBase::SHORT :
         return Do< short > ( list ) ;
         break ;
    case itk::ImageIOBase::UINT :
         return Do< unsigned int > ( list ) ;
         break ;
    case itk::ImageIOBase::INT :
         return Do< int > ( list ) ;
         break ;
    case itk::ImageIOBase::ULONG :
         return Do< unsigned long > ( list ) ;
         break ;
    case itk::ImageIOBase::LONG :
         return Do< long > ( list ) ;
         break ;
    case itk::ImageIOBase::FLOAT :
         return Do< float > ( list ) ;
         break ;
    case itk::ImageIOBase::DOUBLE :
         return Do< double > ( list ) ;
         break ;
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    default :
         std::cerr<< "unknown component type" << std::endl ;
         break ;
    }
  return EXIT_SUCCESS ;
}


