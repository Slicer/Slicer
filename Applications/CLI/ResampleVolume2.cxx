#include <iostream>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageIOBase.h>
#include <itkOrientedImage.h>
#include <itkPoint.h>
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkRigid3DTransform.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkAffineTransform.h>
#include <string.h>
#include <vector>
#include <stdio.h>
#include <itkMatrix.h>
#include <itkPeriodicBoundaryCondition.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkTransformFileReader.h>
#include "ResampleVolume2CLP.h"
#include <itkTransform.h>
#include <itkImageRegionIterator.h>
#include <itkVectorImage.h>
#include <itkVariableLengthVector.h>
#include <itkNrrdImageIO.h>
#include <itkMetaDataObject.h>

#define RADIUS 3

struct parameters
{
  int numberOfThread ;
  std::string interpolationType ;
  std::string transformType ;
  std::vector< double > transformMatrix ;
  std::string inputVolume ;
  std::string outputVolume ;
  std::string referenceVolume ;
  std::vector< float > rotationPoint ;
  std::string transformationFile ;
  bool inverseITKTransformation ;
  std::string windowFunction ;
  unsigned int splineOrder ;
  std::string space ;
  bool centeredTransform ;
  std::vector< double > outputImageSpacing ;
  std::vector< double > outputImageSize ;
  std::vector< float > outputImageOrigin ;
  std::vector< double > directionMatrix ;
} ;


void GetImageType (std::string fileName,
                     itk::ImageIOBase::IOPixelType &pixelType,
                     itk::ImageIOBase::IOComponentType &componentType)
{
  typedef itk::Image< unsigned char , 3 > ImageType ;
  itk::ImageFileReader< ImageType >::Pointer imageReader =
  itk::ImageFileReader< ImageType >::New();
  imageReader->SetFileName( fileName.c_str() ) ;
  imageReader->UpdateOutputInformation() ;
  pixelType = imageReader->GetImageIO()->GetPixelType() ;
  componentType = imageReader->GetImageIO()->GetComponentType() ;
}

//Read the transform in the ITK transform file
void ReadTransform( parameters list , itk::TransformFileReader::Pointer &transformFile)
{
 if( list.transformationFile.compare( "" ) )
    {
    transformFile=itk::TransformFileReader::New() ;
    transformFile->SetFileName( list.transformationFile.c_str() ) ;
    transformFile->Update() ;
    }
}

//Write back the vector of images into a image vector
template<class PixelType>
int AddImage(typename itk::VectorImage< PixelType, 3 >
               ::Pointer &imagePile,
         const std::vector< typename itk::OrientedImage< PixelType , 3 > ::Pointer > &vectorImage)
{
  typedef itk::OrientedImage< PixelType , 3 > ImageType ;
  imagePile->SetRegions( vectorImage.at( 0 )->GetLargestPossibleRegion().GetSize() ) ;
  imagePile->SetOrigin( vectorImage.at( 0 )->GetOrigin() ) ;
  imagePile->SetDirection( vectorImage.at( 0 )->GetDirection() ) ;
  imagePile->SetSpacing( vectorImage.at( 0 )->GetSpacing() ) ;
  imagePile->SetVectorLength( vectorImage.size() ) ;
  imagePile->Allocate() ;
  typename itk::ImageRegionIterator< itk::VectorImage< PixelType , 3 > > out( imagePile , imagePile->GetLargestPossibleRegion() ) ;
  typedef typename itk::ImageRegionIterator< ImageType > IteratorImageType ;
  std::vector< IteratorImageType > in ;
  for(int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
  {
    IteratorImageType intemp( vectorImage.at( i ) , vectorImage.at( i )->GetLargestPossibleRegion() ) ;
    intemp.GoToBegin();
    in.push_back( intemp ) ;
  }
  itk::VariableLengthVector< PixelType > value ;
  value.SetSize( vectorImage.size() ) ;
  for( out.GoToBegin() ; !out.IsAtEnd() ; ++out )
    {
    for( int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
      {
      value.SetElement( i , in.at( i ).Get() ) ;
      ++in[ i ] ;
      }
    out.Set( value ) ;
    }
  return 0 ;
}

//Separate the vector image into a vector of images
template< class PixelType >
int SeparateImages(const typename itk::VectorImage< PixelType , 3 >
                                              ::Pointer &imagePile ,
                                              std::vector< typename itk::OrientedImage< PixelType , 3 >::Pointer > &vectorImage)
{
  typedef itk::OrientedImage< PixelType , 3 > ImageType ;
  typename itk::VectorImage< PixelType , 3 >::SizeType size = imagePile->GetLargestPossibleRegion().GetSize() ;
  typename itk::VectorImage< PixelType , 3 >::DirectionType direction=imagePile->GetDirection() ;
  typename itk::VectorImage< PixelType , 3 >::PointType origin=imagePile->GetOrigin() ;
  typename itk::VectorImage< PixelType , 3 >::SpacingType spacing=imagePile->GetSpacing() ;
  typename itk::ImageRegionIterator< itk::VectorImage< PixelType , 3 > > in(imagePile,imagePile->GetLargestPossibleRegion()) ;
  typedef typename itk::ImageRegionIterator< ImageType > IteratorImageType ;
  std::vector< IteratorImageType > out ;
  for( int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
    {
    typename ImageType::Pointer imageTemp=ImageType::New() ;
    imageTemp->SetRegions( size ) ;
    imageTemp->SetOrigin( origin ) ;
    imageTemp->SetDirection( direction ) ;
    imageTemp->SetSpacing( spacing ) ;
    imageTemp->Allocate() ;
    vectorImage.push_back( imageTemp ) ;
    IteratorImageType outtemp( imageTemp , imageTemp->GetLargestPossibleRegion() ) ;
    outtemp.GoToBegin() ;
    out.push_back( outtemp ) ;
    }
  for( in.GoToBegin() ; !in.IsAtEnd() ; ++in )
    {
    itk::VariableLengthVector< PixelType > value = in.Get() ;
    for( int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
      {
      out[ i ].Set( value[ i ] ) ;
      ++out[ i ] ;
      }
    }
  return 0 ;
}


//Verify if some input parameters are null
bool VectorIsNul( std::vector< double > vec )
{
  bool zero = 1 ;
  for( int i = 0 ; i < vec.size() ; i++ ) { if( vec[i] != 0 ) { zero = 0 ; } }
  return zero;
}



//Set Output parameters
template< class PixelType >
void SetOutputParameters(const parameters &list ,
           typename itk::ResampleImageFilter< itk::OrientedImage< PixelType , 3 >, itk::OrientedImage< PixelType , 3 > >
                       ::Pointer &resampler ,
           typename itk::OrientedImage< PixelType , 3 >::Pointer &image )
{
  typedef itk::OrientedImage< PixelType , 3 > ImageType ;
  typedef itk::ImageFileReader< ImageType > FileReaderType ;  
  typedef itk::ResampleImageFilter< ImageType, ImageType > ResamplerType ;
  typename FileReaderType::Pointer readerReference ;
  
  //is there a reference image to set the size, the orientation,
  // the spacing and the origin of the output image
  if( list.referenceVolume.compare( "" ) )
    {
    readerReference = FileReaderType::New() ;
    readerReference->SetFileName(list.referenceVolume.c_str() );
    readerReference->Update();
    }
  resampler->SetOutputParametersFromImage( image ) ;
  typename ResamplerType::OutputImageType::SpacingType m_Spacing ;
  typename ResamplerType::OutputImageType::PointType m_Origin ;
  typename ResamplerType::OutputImageType::DirectionType m_Direction ;
  typename ResamplerType::OutputImageType::SizeType m_Size ;
  if( VectorIsNul( list.outputImageSpacing ) )
    {
    if( list.referenceVolume.compare( "" ) )
       { m_Spacing = readerReference->GetOutput()->GetSpacing() ; }
    else
      { m_Spacing = image->GetSpacing() ; }
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
      { m_Size = image->GetLargestPossibleRegion().GetSize() ; }
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
      { m_Origin = image->GetOrigin() ; }
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
      { m_Direction = image->GetDirection() ; }
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
  resampler->SetSize( m_Size ) ;
  resampler->SetOutputOrigin( m_Origin ) ;
  resampler->SetOutputDirection( m_Direction ) ; 
}


template<class PixelType>
void CheckDWMRI(itk::MetaDataDictionary &dico ,
                           itk::Transform< double , 3 , 3 >::Pointer &transform ,
                           parameters list )
{
  typedef std::vector< std::vector< double > > DoubleVectorType ;
  typedef itk::MetaDataObject< DoubleVectorType > MetaDataDoubleVectorType ;
  typedef itk::MetaDataObject< std::string > MetaDataStringType ;
  typedef itk::AffineTransform< double , 3 > AffineTransformType;
  typedef itk::Rigid3DTransform< double > RotationType;
  itk::Matrix< double , 3 , 3 > measurementFrame ;
  bool dtmri = 0 ;
  itk::MetaDataDictionary::ConstIterator itr = dico.Begin() ;
  itk::MetaDataDictionary::ConstIterator end = dico.End() ;
  while( itr != end )
    {
    itk::MetaDataObjectBase::Pointer entry = itr->second ;
    MetaDataDoubleVectorType::Pointer entryvalue
            = dynamic_cast< MetaDataDoubleVectorType* >( entry.GetPointer() ) ; 
    if( entryvalue )
      {
      int pos = itr->first.find( "NRRD_measurement frame" ) ;
      if( pos != -1 )
        {
        DoubleVectorType tagvalue = entryvalue->GetMetaDataObjectValue() ;
        for( int i = 0 ; i < 3 ; i++ )
          {
          for( int j = 0 ; j < 3 ; j++ )
            {
            measurementFrame[ i ][ j ] = tagvalue.at( j ).at( i ) ;
            tagvalue.at(j).at(i) = ( i == j ? 1 : 0 );
            }
          }
        entryvalue->SetMetaDataObjectValue(tagvalue);
        } 
      }
      ++itr ;
    }
  //transform gradient
  bool noninvertible = 0 ;
  itk::Transform< double , 3 , 3 >::Pointer inverseTransform ;
  try
    {
    typename AffineTransformType::Pointer affine = dynamic_cast<AffineTransformType* > ( transform.GetPointer() ) ;
    if(affine )//Rotation around a selected point
      {
      typename AffineTransformType::Pointer affinetemp=AffineTransformType::New();
      affine->GetInverse(affinetemp);
      inverseTransform=affinetemp;
      }
    else
      {
      typename RotationType::Pointer rigid = dynamic_cast<RotationType* > ( transform.GetPointer() ) ;
      if( rigid )
        {
        typename RotationType::Pointer rigidtemp = RotationType::New() ;
        rigid->GetInverse( rigidtemp ) ;
        inverseTransform = rigidtemp ;
        }
      else 
        {
        noninvertible = 1 ;
        }
      }
    }
    catch(...)
      {
      std::cout << "Exception Detected" << std::endl ;
      noninvertible = 1 ;
      }
    itr = dico.Begin() ;
    end = dico.End() ;
    while( itr != end )
      {
      itk::MetaDataObjectBase::Pointer entry = itr->second ;
      MetaDataStringType::Pointer entryvalue
             = dynamic_cast<MetaDataStringType* >( entry.GetPointer() ) ;  
      if( entryvalue )
        {
        int pos = itr->first.find( "DWMRI_gradient" ) ;
        if( pos != -1 )
          {
          if( !noninvertible )
            {
            std::string tagvalue = entryvalue->GetMetaDataObjectValue() ;
            itk::Vector< double , 3 > vec ;
            itk::Vector< double , 3 > transformedVector ;
            std::sscanf( tagvalue.c_str() , "%lf %lf %lf\n" , &vec[0], &vec[1], &vec[2] ) ;
            transformedVector = inverseTransform->TransformVector( measurementFrame * vec ) ;
            if( transformedVector.GetNorm() > .00001 ) //gradient not null
              { transformedVector.Normalize() ; }
            char buffer[ 150 ] ;
            std::sprintf( buffer , "%lf %lf %lf" , transformedVector[ 0 ] , transformedVector[ 1 ] , transformedVector[ 2 ] ) ;
            entryvalue->SetMetaDataObjectValue( buffer ) ;
            }
          dtmri = 1 ;
        }
      }
    ++itr ;
    }
  if( noninvertible && dtmri )
    {
    std::cerr << "The gradient transformation is not handle correctly with the current transformation"<< std::endl ;
    }
}


template< class PixelType > void RASLPS(typename itk::VectorImage< PixelType, 3 >::Pointer image)
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

template< class PixelType > int Rotate( parameters list )
{
  typedef itk::OrientedImage< PixelType , 3 > ImageType ;
  typedef itk::ImageFileReader< ImageType > FileReaderType ;   
  typename FileReaderType::Pointer readerCopyInfo = FileReaderType::New() ;
  typename ImageType::IndexType index ;
  typedef itk::InterpolateImageFunction< ImageType , double > InterpolatorType ;
  typedef itk::NearestNeighborInterpolateImageFunction< ImageType , double > NearestNeighborInterpolateType ;
  typedef itk::LinearInterpolateImageFunction< ImageType , double > LinearInterpolateType;
  typedef itk::ConstantBoundaryCondition< itk::OrientedImage< PixelType , 3 > > BoundaryCondition ;
  typedef itk::ResampleImageFilter< ImageType, ImageType > ResampleType ;
  typedef itk::Transform< double , 3 , 3 > TransformType ;
  typedef itk::AffineTransform< double , 3 > AffineTransformType ;
  typedef itk::Rigid3DTransform< double > RotationType ;
  typename TransformType::Pointer nonRigidFile ;
  typedef itk::BSplineInterpolateImageFunction< ImageType , double , double > BSplineInterpolateFunction ;
  itk::Matrix< double , 3 , 3 > transformMatrix ;
  typename ImageType::Pointer image ;
  
  ///////////////////////////////////////////
  typename itk::ImageFileReader< itk::VectorImage< PixelType , 3 > >::Pointer reader = itk::ImageFileReader< itk::VectorImage< PixelType , 3 > >::New() ;
  reader->SetFileName( list.inputVolume.c_str()) ;
  reader->Update() ;
  if( !list.space.compare( "RAS" ) && list.transformationFile.compare( "" ) )
    { RASLPS<PixelType>( reader->GetOutput() ); }
  itk::MetaDataDictionary dico = reader->GetOutput()->GetMetaDataDictionary() ;
  std::vector< typename ImageType::Pointer > vectorImage ;
  std::vector< typename ImageType::Pointer > vectorOutputImage ;
  SeparateImages< PixelType >( reader->GetOutput() , vectorImage ) ;
  ////////////////////////////////////////////////////
  typename NearestNeighborInterpolateType::Pointer interpolator = NearestNeighborInterpolateType::New() ;
  typename LinearInterpolateType::Pointer linearinterpolator = LinearInterpolateType::New() ;
  typename InterpolatorType::Pointer interpol ;
  
  if( !list.interpolationType.compare( "linear" ) )
    { interpol = linearinterpolator ; }
  else if( !list.interpolationType.compare( "nn" ) )
    { interpol = interpolator ; }
  else if( !list.interpolationType.compare( "ws" ) )
    {
    if( !list.windowFunction.compare( "h" ) )
      {
      typedef itk::Function::HammingWindowFunction< RADIUS > windowFunction ;
      typedef itk::WindowedSincInterpolateImageFunction< ImageType ,
                                    RADIUS ,
                                    windowFunction ,
                                    BoundaryCondition ,
                                    double > WindowedSincInterpolateImageFunctionType ;
      interpol = WindowedSincInterpolateImageFunctionType::New() ;
      }
    else if( !list.windowFunction.compare( "c" ) )
      {
      typedef itk::Function::CosineWindowFunction< RADIUS > windowFunction ;
      typedef itk::WindowedSincInterpolateImageFunction< ImageType ,
                                    RADIUS ,
                                    windowFunction ,
                                    BoundaryCondition ,
                                    double > WindowedSincInterpolateImageFunctionType ;
      interpol = WindowedSincInterpolateImageFunctionType::New() ;
      }
    else if( !list.windowFunction.compare( "w" ) )
      {
      typedef itk::Function::WelchWindowFunction< RADIUS > windowFunction ;
      typedef itk::WindowedSincInterpolateImageFunction< ImageType,
                                    RADIUS ,
                                    windowFunction ,
                                    BoundaryCondition ,
                                    double > WindowedSincInterpolateImageFunctionType ;
      interpol = WindowedSincInterpolateImageFunctionType::New() ;
      }
    else if(!list.windowFunction.compare( "l" ) )
      {
      typedef itk::Function::LanczosWindowFunction< RADIUS > windowFunction ;
      typedef itk::WindowedSincInterpolateImageFunction< ImageType ,
                                    RADIUS ,
                                    windowFunction ,
                                    BoundaryCondition ,
                                    double > WindowedSincInterpolateImageFunctionType ;
      interpol = WindowedSincInterpolateImageFunctionType::New() ;
      }
    else if(!list.windowFunction.compare( "b" ) )
      {
      typedef itk::Function::BlackmanWindowFunction< RADIUS > windowFunction ;
      typedef itk::WindowedSincInterpolateImageFunction< ImageType ,
                                    RADIUS ,
                                    windowFunction ,
                                    BoundaryCondition ,
                                    double > WindowedSincInterpolateImageFunctionType ;
      interpol = WindowedSincInterpolateImageFunctionType::New() ;
      }   
    }
  else if( !list.interpolationType.compare( "bs" ) )
    {
    typename BSplineInterpolateFunction::Pointer bSplineInterpolator = BSplineInterpolateFunction::New() ;
    bSplineInterpolator->SetSplineOrder( list.splineOrder ) ;
    interpol = bSplineInterpolator ;
    }
  for( int idx = 0 ; idx < vectorImage.size() ; idx++ )
    {  
    image = vectorImage[ idx ] ;
    typedef itk::TransformFileReader::Pointer TransformReaderPointer ;
    TransformReaderPointer transformFile ;
    ReadTransform( list , transformFile ) ;
    do
      {
      typename ResampleType::Pointer resample = ResampleType::New() ;
      itk::Vector< double , 3 > vec ;
      typename TransformType::Pointer transform ;
      itk::Matrix< double , 4 , 4 > transformMatrix4x4 ;
      transformMatrix4x4.SetIdentity() ;
      //int size = list.transformMatrix.size() ;
      if( list.transformationFile.compare( "" ) )//Get transformation matrix from command line if no file given
        {
        list.transformMatrix.resize( 0 ) ;
        list.rotationPoint.resize( 0 ) ;
        //size = transformFile->GetTransformList()->front()->GetParameters().GetSize() ;
        AffineTransformType::Pointer affinetransform
           = dynamic_cast< AffineTransformType* > ( transformFile->GetTransformList()->front().GetPointer() ) ;
          if( affinetransform )
            {
            list.transformType.assign( "a" ) ;
            //std::cout<<rigid3dtransform->GetMatrix()<<std::endl;
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
          RotationType::Pointer rigid3dtransform
             = dynamic_cast< RotationType* > ( transformFile->GetTransformList()->front().GetPointer() ) ;
          if( rigid3dtransform )
            {
            list.transformType.assign( "rt" ) ;
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
            nonRigidFile = dynamic_cast< TransformType* >
               ( transformFile->GetTransformList()->front().GetPointer() ) ;
            if( nonRigidFile )//if non rigid Transform loaded
              {
              list.transformType.assign( "nr" ) ;
              }
            else//something else
              {
              std::cerr<< "Transformation type not yet implemented"
                   << std::endl ;
              return -1 ;
              }
            }
          }     
        if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
          {
          //get transform matrix and translation vector
          /*for( int i = 0 ; i < transformFile->GetTransformList()->front()->GetParameters().GetSize() ; i++ )
            { 
            list.transformMatrix.push_back( 
                transformFile->GetTransformList()->front()->GetParameters().GetElement( i ) ) ;
            }*/
            //Get center of transform
/*          for( int i = 0 ; 
             i < transformFile->GetTransformList()->front()->GetFixedParameters().GetSize() ;
             i++ )
            { list.rotationPoint.push_back( 
               transformFile->GetTransformList()->front()->GetFixedParameters().GetElement( i ) ) ;
            }*/
            //if problem in the number of parameters
          if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
            {
            std::cerr<< "Error in the file containing the matrix transformation"
                 << std::endl ;
            return -1 ;
            }
          }
        transformFile->GetTransformList()->pop_front() ;
        }
      if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
        {  
        /*for( int i = 0 ; i < 3 ; i++ )
          {
          for( int j = 0 ; j < 3 ; j++ )
            {
            transformMatrix4x4[ i ][ j ] = ( double ) list.transformMatrix[ i + j * 3 ] ;
            }
          transformMatrix4x4[ i ][ 3 ] = ( double ) list.transformMatrix[ 9 + i ] ;
          }*/
        itk::Point< double > center ;
        itk::Vector< double > translation ;
        if( list.centeredTransform )
          {
          typename ImageType::SizeType sizeim=vectorImage.at(idx)->GetLargestPossibleRegion().GetSize() ;
          itk::Point< double , 3 > point ;
          itk::Point< double , 3 > pointOpposite ;
          for( int i = 0 ; i < 3 ; i++ )
            { index[ i ] = 0 ; }
            vectorImage.at( idx )->TransformIndexToPhysicalPoint( index , point ) ;
          for( int i = 0 ; i < 3 ; i++ )
            { index[ i ] = sizeim[ i ] - 1 ; }
          vectorImage.at( idx )->TransformIndexToPhysicalPoint( index , pointOpposite ) ;
          for( int i = 0 ; i < 3 ; i++ )
            { center[ i ] = ( point[ i ] + pointOpposite[ i ] ) / 2 ; }
          }
        else
          {
          for( int i = 0 ; i < 3 ; i++ )
            { center[ i ] = list.rotationPoint[ i ] ; }
          }
        for( int i = 0 ; i < 3 ; i++ )
          {
          for( int j = 0 ; j < 3 ; j++ )
            {
            transformMatrix4x4[ i ][ j ] = ( double )list.transformMatrix[ i*3 + j  ] ;    
            }
          translation[ i ] = ( double )list.transformMatrix[ 9 + i ] ;
          }
        for( int i = 0 ; i < 3 ; i++ )
          {
          transformMatrix4x4[ i ][ 3 ] = translation[ i ] + center[ i ] ;
          for( int j = 0 ; j < 3 ; j++ )
            {
            transformMatrix4x4[ i ][ 3 ] -= transformMatrix4x4[ i ][ j ] * center[ j ] ;
            }
          }     
        if( list.inverseITKTransformation )
          {
          transformMatrix4x4 = transformMatrix4x4.GetInverse() ;
          }
        if( !list.space.compare( "RAS" ) && !list.transformationFile.compare( "" ) )
          {
          itk::Matrix< double , 4 , 4 > ras ;
          ras.SetIdentity() ;
          ras[ 0 ][ 0 ] = -1 ;
          ras[ 1 ][ 1 ] = -1 ;
          transformMatrix4x4 = ras * transformMatrix4x4 * ras ;            
          }
        for( int i = 0 ; i < 3 ; i++ )
          {
        for( int j = 0 ; j < 3 ; j++ )
          {
          transformMatrix[ i ][ j ] = transformMatrix4x4[ i ][ j ] ;
          }
        vec[ i ] = transformMatrix4x4[ i ][ 3 ] ;
        }
      }
    if( !list.transformType.compare( "rt" ) ) //Rotation around a selected point
      {
      try
         {
      typename RotationType::Pointer rotation = RotationType::New() ;
      rotation->SetRotationMatrix( transformMatrix ) ;
      rotation->SetTranslation( vec ) ;
      transform = rotation ;
         }
      catch(itk::ExceptionObject exp)
         {
         std::string exception = exp.GetDescription();
         if( exception.find("Attempting to set a non-orthogonal rotation matrix") != -1 )
            {
            list.transformType = "a" ;
            std::cerr<<"Non-orthogonal rotation matrix: uses affine transform"<<std::endl;
            }
         else
            {
            throw exp;
            }
         }
      }
    if( !list.transformType.compare( "a" ) ) //Affine transform
      {
      typename AffineTransformType::Pointer affine = AffineTransformType::New() ;
      affine->SetMatrix( transformMatrix ) ;
      affine->SetTranslation( vec ) ;
      transform = affine ;
      }
    else if( !list.transformType.compare( "nr" ) ) //non-rigid
      {
      nonRigidFile = dynamic_cast< TransformType* >
        ( transformFile->GetTransformList()->front().GetPointer() ) ;
      if( nonRigidFile )//if non rigid Transform loaded
        {
        transform = nonRigidFile ;
        }
      }
    /////////////////////////////////////
    if(list.numberOfThread)
      { resample->SetNumberOfThreads( list.numberOfThread ) ; }
    resample->SetInterpolator( interpol ) ;
    SetOutputParameters< PixelType >( list , resample , vectorImage.at( idx ) ) ;
    resample->SetInput( image ) ;
    resample->SetTransform( transform ) ;
    resample->Update() ;
    image = resample->GetOutput() ;
    image->DisconnectPipeline();
    if( idx == 0 )
      { CheckDWMRI< PixelType >( dico , transform , list ) ; }
    }while( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() ) ;
  vectorOutputImage.push_back( image ) ;
}
typename itk::VectorImage< PixelType, 3 >::Pointer outputImage = itk::VectorImage< PixelType , 3 >::New() ;
AddImage< PixelType >( outputImage , vectorOutputImage ) ;
typename itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New() ;
io->SetFileTypeToBinary() ;
io->SetMetaDataDictionary( dico ) ;

if( !list.space.compare( "RAS" ) && list.transformationFile.compare( "" ) )
  { RASLPS<PixelType>( outputImage); }
typedef itk::ImageFileWriter< typename itk::VectorImage< PixelType, 3 > > WriterType ;
typename WriterType::Pointer writer = WriterType::New() ;
writer->UseInputMetaDataDictionaryOff() ;
writer->SetInput( outputImage ) ;
writer->SetImageIO( io ) ;
writer->SetFileName( list.outputVolume.c_str() ) ;
writer->UseCompressionOn() ;
writer->Update() ;
return 0 ;
}


int main( int argc , const char * argv[] )
{
  PARSE_ARGS ;
  parameters list ;
  list.numberOfThread = numberOfThread ;
  list.interpolationType = interpolationType ;
  list.transformType = transformType ;
  list.transformMatrix = transformMatrix ;
  list.inputVolume = inputVolume ;
  list.referenceVolume = referenceVolume ;
  list.outputVolume = outputVolume ;
  list.rotationPoint=rotationPoint ;
  list.transformationFile=transformationFile ;
  list.inverseITKTransformation = inverseITKTransformation ;
  list.windowFunction = windowFunction ;
  list.splineOrder = splineOrder ;
  list.space = space ;
  list.centeredTransform = centeredTransform ;
  list.outputImageSpacing = outputImageSpacing ;
  list.outputImageSize = outputImageSize ;
  list.outputImageOrigin = outputImageOrigin ;
  list.directionMatrix = directionMatrix ;
  if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
    {
    std::cerr<<"Argument(s) having wrong size"<<std::endl ;
    return -1 ;
    }   
  itk::ImageIOBase::IOPixelType pixelType ;
  itk::ImageIOBase::IOComponentType componentType ;
  GetImageType( list.inputVolume , pixelType , componentType ) ;

  switch( componentType )
    {
    case itk::ImageIOBase::UCHAR:
      return Rotate< unsigned char >( list ) ;
      break ;
    case itk::ImageIOBase::CHAR:
      return Rotate< char >( list ) ;
      break ;
    case itk::ImageIOBase::USHORT:
      return Rotate< unsigned short >( list ) ;
      break ;
    case itk::ImageIOBase::SHORT:
      return Rotate< short >( list ) ;
      break ;
    case itk::ImageIOBase::UINT:
      return Rotate< unsigned int >( list ) ;
      break ;
    case itk::ImageIOBase::INT:
      return Rotate< int >( list ) ;
      break ;
    case itk::ImageIOBase::ULONG:
      return Rotate< unsigned long >( list ) ;
      break ;
    case itk::ImageIOBase::LONG:
      return Rotate< long >( list ) ;
      break ;
    case itk::ImageIOBase::FLOAT:
      return Rotate< float >( list ) ;
      break ;
    case itk::ImageIOBase::DOUBLE:
      return Rotate< double >( list ) ;
      break ;
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    default:
      std::cout << "unknown component type" << std::endl ;
      break ;
    }
  return 0 ;
}


