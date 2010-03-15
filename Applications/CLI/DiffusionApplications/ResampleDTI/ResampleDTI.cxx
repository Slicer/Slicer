/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/



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
#include "dtiprocessFiles/deformationfieldio.h"
#include "itkWarpTransform3D.h"
#include <itkImageRegionIteratorWithIndex.h>
#include <list>
#include <itkVectorLinearInterpolateImageFunction.h>
#include "itkTransformDeformationFieldFilter.h"
#include <itkVectorResampleImageFilter.h>


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
  std::string correction ;
  std::string deffield ;
  std::string typeOfField ;
  double defaultPixelValue ;
};


//Verify if some input parameters are null
bool VectorIsNul( std::vector< double > vec )
{
  bool zero = 1 ;
  for( ::size_t i = 0 ; i < vec.size() ; i++ )
    {
    if( vec[i] != 0 ) { zero = 0 ; }
    }
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
    {
    interpol = NearestNeighborhoodInterpolatorType::New() ;
    }
  else if( !interpolationType.compare( "linear" ) )//linear
    {
    interpol = LinearInterpolatorType::New() ;
    }
  else if( !interpolationType.compare( "ws" ) )//windowed sinc
    {
    if( !windowFunction.compare( "h" ) ) //Hamming window
      {
      interpol = HammingWindowedSincInterpolateFunctionType::New() ;
      }
    else if( !windowFunction.compare( "c" ) )//cosine window
      {
      interpol = CosineWindowedSincInterpolateFunctionType::New() ;
      }
    else if( !windowFunction.compare( "w" ) ) //Welch window
      {
      interpol = WelchWindowedSincInterpolateFunctionType::New() ;
      }
    else if( !windowFunction.compare( "l" ) ) // Lanczos window
      {
      interpol = LanczosWindowedSincInterpolateFunctionType::New() ;
      }
    else if( !windowFunction.compare( "b" ) ) // Blackman window
      {
      interpol = BlackmanWindowedSincInterpolateFunctionType::New() ;
      }   
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



//Copy rigid or affine transform to list.transformMatrix
template< class PixelType >
void
SetListFromTransform( const typename itk::MatrixOffsetTransformBase< PixelType , 3 , 3 >::Pointer &transform ,
                      parameters &list
                    )
{
  for( int i = 0 ; i < 3 ; i++ )
  {
    for( int j = 0 ; j < 3 ; j++ )
    {
      list.transformMatrix.push_back( (double)transform->GetMatrix()[ i ][ j ] ) ;
    }
  }
  for(int i = 0 ; i < 3 ; i++ )
  {
    list.transformMatrix.push_back( (double)transform->GetTranslation()[ i ] ) ;
    list.rotationPoint.push_back( (double)transform->GetCenter()[ i ] ) ;
  }
}


template< class PixelType >
itk::Matrix< double , 4 , 4 >
ComputeTransformMatrix( const parameters &list ,
                        const typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >
                          ::Pointer &image
                      )
{
  itk::Matrix< double , 4 , 4 > transformMatrix4x4 ;
  transformMatrix4x4.SetIdentity() ;
  itk::Point< double > center ;
  itk::Vector< double > translation ;
  itk::Vector< double > offset ;
  //if centered transform set, ignore the transformation center
  // set before and use the center of the image instead
  if( list.centeredTransform )
  {
    typename itk::Index< 3 > index ;
    itk::Size< 3 > sizeim = image->GetLargestPossibleRegion().GetSize() ;
    itk::Point< double , 3 > point ;
    itk::Point< double , 3 > pointOpposite ;
    for( int i = 0 ; i < 3 ; i++ )
    {
      index[ i ] = 0 ;
    }
    image->TransformIndexToPhysicalPoint( index , point ) ;
    for( int i = 0 ; i < 3 ; i++ )
    {
      index[ i ] = sizeim[ i ] - 1 ;
    }
    image->TransformIndexToPhysicalPoint( index , pointOpposite ) ;
    for( int i = 0 ; i < 3 ; i++ )
    {
      center[ i ] = ( point[ i ] + pointOpposite[ i ] ) / 2 ;
    }
  }
  else
  {
    for( int i = 0 ; i < 3 ; i++ )
    {
      center[ i ] = list.rotationPoint[ i ] ;
    }
  }
  //Set the transform matrix  
  for( int i = 0 ; i < 3 ; i++ )
  {
    center[ i ] = list.rotationPoint[ i ] ;
    translation[ i ] = ( double ) list.transformMatrix[ 9 + i ] ;
    offset[ i ] = translation[ i ] + center[ i ] ;
    for( int j = 0 ; j< 3 ; j++ )
    {
      transformMatrix4x4[ i ][ j ]
        = ( double ) list.transformMatrix[ i * 3 + j  ] ;
      offset[ i ] -= transformMatrix4x4[ i ][ j ] * center[ j ] ;
    }
    //Compute the 4th column of the matrix
    transformMatrix4x4[ i ][ 3 ] = offset[ i ] ;
  }
  //If set, inverse the given transform.
  //The given transform therefore transform the input image into the output image    
  if( list.inverseITKTransformation )
  {
    transformMatrix4x4 = transformMatrix4x4.GetInverse() ;
  }
  //If the transform is in RAS, transform it in LPS  
  if( !list.space.compare( "RAS" )  && !list.transformationFile.compare( "" )  )
  {
    itk::Matrix< double , 4 , 4 > ras ;
    ras.SetIdentity() ;
    ras[ 0 ][ 0 ] = -1 ;
    ras[ 1 ][ 1 ] = -1 ;
    transformMatrix4x4 = ras * transformMatrix4x4 * ras ;            
  }
  return transformMatrix4x4 ;
}


template< class PixelType >
typename itk::DiffusionTensor3DTransform< PixelType >::Pointer
SetUpTransform( const parameters &list ,
                const typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >
                  ::Pointer &image ,
                const typename itk::DiffusionTensor3DNonRigidTransform< PixelType >::TransformType
                  ::Pointer &nonRigidFile ,
                bool precisionChecking
              )
{
  typedef itk::DiffusionTensor3DTransform< PixelType > TransformType ;
  typedef itk::DiffusionTensor3DNonRigidTransform< PixelType >
    NonRigidTransformType ;
  typedef typename TransformType::Pointer TransformTypePointer ;
  itk::Matrix< double , 4 , 4 > transformMatrix4x4 ;
  typedef itk::DiffusionTensor3DFSAffineTransform< PixelType >
    FSAffineTransformType ;
  typedef itk::DiffusionTensor3DPPDAffineTransform< PixelType >
    PPDAffineTransformType ;
  typedef itk::DiffusionTensor3DRigidTransform< PixelType > RigidTransformType ;
  TransformTypePointer transform ;
  if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
    {
    transformMatrix4x4 = ComputeTransformMatrix< PixelType >( list , image ) ;
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
        typename FSAffineTransformType::Pointer affinefs = FSAffineTransformType::New() ;
        affinefs->SetMatrix4x4( transformMatrix4x4 ) ;
        transform = affinefs ;
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
    typename itk::DiffusionTensor3DAffineTransform< PixelType >::Pointer affine ;
    affine=affineppd ;
    nonRigid->SetAffineTransformType( affine ) ;
    transform = nonRigid ;
    }
  return transform ;
}

//Set the transformation
template< class PixelType >
typename itk::DiffusionTensor3DTransform< PixelType >::Pointer
SetTransform( parameters list , 
              const typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >
                ::Pointer &image ,
              itk::TransformFileReader::Pointer &transformFile
            )
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
  //Get transformation matrix from the given file
  if( list.transformationFile.compare( "" ) )
  {
    list.transformMatrix.resize( 0 ) ;
    list.rotationPoint.resize( 0 ) ;
    typename itk::MatrixOffsetTransformBase< double , 3 , 3 >
                       ::Pointer doubleMatrixOffsetTransform ;
    typename itk::MatrixOffsetTransformBase< float , 3 , 3 >
                        ::Pointer floatMatrixOffsetTransform ;
    typename FSAffineTransformType::Superclass::AffineTransformType::Pointer
        doubleAffineTransform = dynamic_cast< 
        typename FSAffineTransformType::Superclass::AffineTransformType* >
        ( transformFile->GetTransformList()->back().GetPointer() ) ;
    if( doubleAffineTransform )//if affine transform in double
    {
      list.transformType.assign( "a" ) ;
      doubleMatrixOffsetTransform = doubleAffineTransform ;
      SetListFromTransform< double >( doubleMatrixOffsetTransform , list ) ;
    }
    else
    {
      itk::AffineTransform< float , 3 >::Pointer
      floatAffineTransform = dynamic_cast< itk::AffineTransform< float , 3 >* >
                   ( transformFile->GetTransformList()->back().GetPointer() ) ;
      if( floatAffineTransform )//if affine transform in float
      {
        list.transformType.assign( "a" ) ;
        floatMatrixOffsetTransform = floatAffineTransform ;
        SetListFromTransform< float >( floatMatrixOffsetTransform , list ) ;
      }
      else
      {
        typename RigidTransformType::Rigid3DTransformType::Pointer
            doubleRigid3DTransform = dynamic_cast< 
            typename RigidTransformType::Rigid3DTransformType* >
            ( transformFile->GetTransformList()->back().GetPointer() ) ;
        if( doubleRigid3DTransform )//if rigid3D transform in double
        {
          list.transformType.assign( "rt" ) ;
          precisionChecking = 0 ;
          doubleMatrixOffsetTransform = doubleRigid3DTransform ;
          SetListFromTransform< double >( doubleMatrixOffsetTransform , list ) ;
        }
        else
        {
          itk::Rigid3DTransform< float >::Pointer
              floatRigid3DTransform = dynamic_cast< 
              itk::Rigid3DTransform< float >* >
              ( transformFile->GetTransformList()->back().GetPointer() ) ;
          if( floatRigid3DTransform )//if rigid3D transform in float
          {
            list.transformType.assign( "rt" ) ;
            precisionChecking = 0 ;
            floatMatrixOffsetTransform = floatRigid3DTransform ;
            SetListFromTransform< float >( floatMatrixOffsetTransform , list ) ;
          }
          else//if non-rigid
          {
            nonRigidFile = dynamic_cast<
            typename NonRigidTransformType::TransformType* >
                ( transformFile->GetTransformList()->back().GetPointer() ) ;
            if(nonRigidFile)//if non rigid Transform loaded
            {
              list.transformType.assign( "nr" ) ;
            }
            else//something else
            {
              std::cerr << "Transformation type not yet implemented for tensors"
                        << std::endl ;
              return NULL ;
            }
          }
        }
      }
    }
    if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
    {
      //if problem in the number of parameters
      if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
      {
        std::cerr << "Error in the file containing the transformation"
                  << std::endl ;
        return NULL ;
      }
    }
    transformFile->GetTransformList()->pop_back();
  }
  return  SetUpTransform< PixelType >( list , image , nonRigidFile , precisionChecking ) ;
}


//Read the transform file and return the number of non-rigid transform.
//If the transform file contain a transform that the program does not
//handle, the function returns -1
template< class PixelType >
int ReadTransform( const parameters &list ,
                   const typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >::Pointer &image ,
                   itk::TransformFileReader::Pointer &transformFile
                 )
{
  int numberOfNonRigidTransform = 0 ;
  if( list.transformationFile.compare( "" ) )
  {
    transformFile = itk::TransformFileReader::New() ;
    transformFile->SetFileName( list.transformationFile.c_str() ) ;
    transformFile->Update() ;
    //Check if any of the transform is not supported and counts the number of non-rigid transform
    do
    {
      if( !SetTransform< PixelType >( list , image , transformFile ) )
      {
        return -1 ;
      }
      if( !list.transformType.compare( "nr" ) )
      {
        numberOfNonRigidTransform++ ;
      }
    }while( transformFile->GetTransformList()->size() ) ;
    transformFile->Update() ;
    return numberOfNonRigidTransform ;
  }
  return 0 ;
}


//Set Output parameters
template< class PixelType >
void SetOutputParameters( const parameters &list ,
                          typename itk::DiffusionTensor3DResample< PixelType , PixelType >
                            ::Pointer &resampler ,
                          const typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >
                            ::Pointer &image
                        )
{
  typedef itk::OrientedImage< unsigned char , 3 > ImageType ;
  typedef itk::ImageFileReader< ImageType > ReaderType ;
  typedef itk::DiffusionTensor3DResample< PixelType , PixelType > ResamplerType ;
  typedef typename ReaderType::Pointer ReaderTypePointer ;
  ReaderTypePointer readerReference ;
  //is there a reference image to set the size, the orientation,
  // the spacing and the origin of the output image?
  if( list.referenceVolume.compare( "" ) )
    {
    readerReference = ReaderType::New() ;
    readerReference->SetFileName( list.referenceVolume.c_str() ) ;
//    readerReference->Update() ;
    readerReference->UpdateOutputInformation() ;
    if( !list.space.compare( "RAS" ) && list.transformationFile.compare( "" ) )
      {
      typename ReaderType::OutputImageType::PointType originReference ;
      typename ReaderType::OutputImageType::DirectionType directionReference ;
      originReference = readerReference->GetOutput()->GetOrigin() ;
      directionReference = readerReference->GetOutput()->GetDirection() ;
      originReference[ 0 ] = -originReference[ 0 ] ;
      originReference[ 1 ] = -originReference[ 1 ] ;
      itk::Matrix< double , 3 , 3 > ras ;
      ras.SetIdentity() ;
      ras[ 0 ][ 0 ] = -1 ;
      ras[ 1 ][ 1 ] = -1 ;
      directionReference = ras * directionReference ;
      readerReference->GetOutput()->SetOrigin( originReference ) ;
      readerReference->GetOutput()->SetDirection( directionReference ) ; 
      }
    }
  resampler->SetOutputParametersFromImage( image ) ;//is probably useless but doesn't cost much
  typename ResamplerType::OutputImageType::SpacingType m_Spacing ;
  typename ResamplerType::OutputImageType::PointType m_Origin ;
  typename ResamplerType::OutputImageType::DirectionType m_Direction ;
  typename ResamplerType::OutputImageType::SizeType m_Size ;
  if( VectorIsNul( list.outputImageSpacing ) )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Spacing = readerReference->GetOutput()->GetSpacing() ;
      }
    else
      {
        m_Spacing = image->GetSpacing() ;
      }
    }
  else
    {
    for( int i = 0 ; i < 3 ; i++ )
      {
      m_Spacing[ i ] = list.outputImageSpacing[ i ] ;
      }
    }
  if( VectorIsNul( list.outputImageSize) ) 
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Size
        = readerReference->GetOutput()->GetLargestPossibleRegion().GetSize() ;
      }
    else
      {
      m_Size = image->GetLargestPossibleRegion().GetSize() ;
      }
    }
  else
    {
    for( int i = 0 ; i < 3 ; i++ ) 
      {
      m_Size[ i ] = ( unsigned long ) list.outputImageSize[ i ] ;
      }
    }
  if( list.outputImageOrigin.size() == 0 )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Origin = readerReference->GetOutput()->GetOrigin() ;
      }
    else
      {
      m_Origin = image->GetOrigin() ;
      }
    }
  else
    {
    for(int i = 0 ; i < 3 ; i++ )
      {
      m_Origin[ i ] = list.outputImageOrigin[ i ] ;
      }
    }
  if( VectorIsNul( list.directionMatrix ) )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Direction = readerReference->GetOutput()->GetDirection() ;
      }
    else
      {
      m_Direction = image->GetDirection() ;
      }
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



template< class PixelType >
void RASLPS( typename itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >::Pointer &image )
{
  typename itk::OrientedImage< PixelType, 3 >::PointType m_Origin ;
  typename itk::OrientedImage< PixelType, 3 >::DirectionType m_Direction ;
  m_Origin = image->GetOrigin() ;
  m_Direction = image->GetDirection() ;
  m_Origin[ 0 ] = -m_Origin[ 0 ] ;
  m_Origin[ 1 ] = -m_Origin[ 1 ] ;
  itk::Matrix< double , 3 , 3 > ras ;
  ras.SetIdentity() ;
  ras[ 0 ][ 0 ] = -1 ;
  ras[ 1 ][ 1 ] = -1 ;
  m_Direction = ras * m_Direction ;
  image->SetOrigin( m_Origin ) ;
  image->SetDirection( m_Direction ) ;
}

//resamples field to output image size; local filter so that the memory is freed once it has run
void ResampleDeformationField( DeformationImageType::Pointer &field ,
                               const itk::Point< double , 3 > &origin ,
                               const itk::Vector< double , 3 > &spacing ,
                               const itk::Size< 3 > &size ,
                               const itk::Matrix< double , 3 , 3 > &direction
                             )
{
  //Check if the field does not already have the same properties as the output image:
  //It would save some time if we did not have to resample the field
  itk::Point< double , 3 > fieldOrigin ;
  itk::Vector< double , 3 > fieldSpacing ;
  itk::Size< 3 > fieldSize ;
  itk::Matrix< double , 3 , 3 > fieldDirection ;
  fieldOrigin = field->GetOrigin() ;
  fieldSpacing = field->GetSpacing() ;
  fieldSize = field->GetLargestPossibleRegion().GetSize() ;
  fieldDirection = field->GetDirection() ;
  if(  fieldSize == size
    && fieldSpacing == spacing
    && fieldDirection == direction
    && fieldOrigin == origin
    )
  {
    return ;
  }
  typedef itk::VectorLinearInterpolateImageFunction< DeformationImageType > VectorInterpolatorType ;
  VectorInterpolatorType::Pointer linearVectorInterpolator = VectorInterpolatorType::New() ;
  typedef itk::VectorResampleImageFilter< DeformationImageType ,
                                          DeformationImageType ,
                                          double
                                        > ResampleImageFilter ;
  ResampleImageFilter::Pointer resampleFieldFilter = ResampleImageFilter::New() ;
  DeformationPixelType defaultPixel ;
  defaultPixel.Fill( 0.0 ) ;
  resampleFieldFilter->SetDefaultPixelValue( defaultPixel ) ;
  resampleFieldFilter->SetInput( field ) ;
  resampleFieldFilter->SetInterpolator( linearVectorInterpolator ) ;
  resampleFieldFilter->SetOutputDirection( direction ) ;
  resampleFieldFilter->SetSize( size ) ;
  resampleFieldFilter->SetOutputSpacing( spacing ) ;
  resampleFieldFilter->SetOutputOrigin( origin ) ;
  resampleFieldFilter->Update() ;
  field = resampleFieldFilter->GetOutput() ;
}

//Check all input parameters and transform the input image
//Template over the pixeltype of the input image
template< class PixelType >
int Do( parameters list )
{
    typedef itk::OrientedImage< itk::DiffusionTensor3D< PixelType > , 3 >
      InputImageType ;
    typename InputImageType::Pointer image ;
    typedef itk::DiffusionTensor3DWrite< PixelType > WriterType ;
    typedef typename WriterType::Pointer WriterTypePointer ;
    WriterTypePointer writer = WriterType::New() ;
    itk::Matrix< double , 3 , 3 > measurementFrame ;
    try
    {
      typedef itk::DiffusionTensor3DRead< PixelType > ReaderType ;
      typedef typename ReaderType::Pointer ReaderTypePointer ;
      ReaderTypePointer reader = ReaderType::New() ;
      //Read input volume
      if(list.numberOfThread) 
      {
        reader->SetNumberOfThreads( list.numberOfThread ) ;
      }
      reader->Update( list.inputVolume.c_str() ) ;
      image = reader->GetOutput() ;
      writer->SetMetaDataDictionary( reader->GetMetaDataDictionary() ) ;
      measurementFrame = reader->GetMeasurementFrame() ;
    }
    catch( itk::ExceptionObject & Except )
    {
      std::cerr << "Reading input image: Exception caught!"
                << std::endl ;
      std::cerr << Except << std::endl ;
      return -1 ;
    }
    if( !list.space.compare( "RAS" ) &&  list.transformationFile.compare( "" ) )
    {
      RASLPS<PixelType>( image ) ;
    }
    typedef itk::DiffusionTensor3DInterpolateImageFunction< PixelType >
                    InterpolatorType ; 
    typedef typename InterpolatorType::Pointer InterpolatorTypePointer ;
    InterpolatorTypePointer interpol ;
    //Select interpolation type
    interpol = InterpolationType< PixelType > ( list.interpolationType ,
                                                list.windowFunction ,
                                                list.splineOrder
                                              ) ;

    //Select the transformation
    typedef itk::TransformFileReader::Pointer TransformReaderPointer ;
    TransformReaderPointer transformFile ;
    int nonRigidTransforms ;
    nonRigidTransforms = ReadTransform< PixelType >( list , image , transformFile ) ;
    if( nonRigidTransforms < 0 )//The transform file contains a transform that is not handled by resampleDTI, it exits.
    {
      return -1 ;
    }
    //if the transform is in RAS space coordinate, add a transform to transform the image into that space 
    typedef itk::DiffusionTensor3DTransform< PixelType > TransformType ;
    typedef typename TransformType::Pointer TransformTypePointer ;
    typedef itk::DiffusionTensor3DResample< PixelType , PixelType > ResamplerType ;
    typedef typename ResamplerType::Pointer ResamplerTypePointer ;
    typename InputImageType::PointType originOutput ;
    typename InputImageType::SpacingType spacingOutput ;
    typename InputImageType::SizeType sizeOutput ;
    typename InputImageType::DirectionType directionOutput ;
    {//local for memory management
      ResamplerTypePointer tempResampler = ResamplerType::New() ;//this resampler is just used to be able to use SetOutputParameters and get the origin, size, direction and spacing of the output image
      SetOutputParameters< PixelType > ( list , tempResampler , image ) ;
      originOutput = tempResampler->GetOutputOrigin() ;
      spacingOutput = tempResampler->GetOutputSpacing() ;
      sizeOutput = tempResampler->GetOutputSize() ;
      directionOutput = tempResampler->GetOutputDirection() ;
    }
    //Check if displacement or h-field transform
    typename DeformationImageType::Pointer fieldPointer ;
    if( list.deffield.compare( "" ) )
    {
       //set if the field is a displacement or a H- field
       DeformationFieldType dftype = HField ;
       if( !list.typeOfField.compare( "displacement" ) )
       {
         dftype = Displacement ;
       }
       //reads deformation field and if it is a h-field, it transforms it to a displacement field
       fieldPointer = readDeformationField( list.deffield , dftype ) ;
       nonRigidTransforms++ ;
    }
    //If more than one transform or if hfield, add all transforms and compute the deformation field and its Jacobian
    TransformTypePointer transform ;
    if( (list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() > 1 && nonRigidTransforms > 0) || list.deffield.compare( "" ) )
    {
      //Create warp transform
      typedef itk::WarpTransform3D< double > WarpTransformType ;
      typename WarpTransformType::Pointer warpTransform = WarpTransformType::New() ;
      typename DeformationImageType::Pointer field ;
      if( list.deffield.compare( "" ) )
      {
        field = fieldPointer ;
        //Resample the deformation field so that it has the same properties as the output image we want to compute
        ResampleDeformationField( field , originOutput , spacingOutput , sizeOutput , directionOutput ) ;
      }
      else//if no deformation field was loaded, we create an empty one
      {
        field = DeformationImageType::New() ;
        field->SetSpacing( spacingOutput ) ;
        field->SetOrigin( originOutput ) ;
        field->SetRegions( sizeOutput ) ;
        field->SetDirection( directionOutput ) ;
        field->Allocate() ;
        DeformationPixelType vectorNull ;
        vectorNull.Fill( 0.0 ) ;
        field->FillBuffer( vectorNull ) ;
      }
      //Compute the transformation field adding all the transforms together
      while( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() )
      {
        typedef itk::TransformDeformationFieldFilter< double , double , 3 > itkTransformDeformationFieldFilterType ;
        typename itkTransformDeformationFieldFilterType::Pointer transformDeformationFieldFilter = itkTransformDeformationFieldFilterType::New() ;
        transform = SetTransform< PixelType > ( list , image , transformFile ) ;
        if( list.numberOfThread ) 
        {
          transformDeformationFieldFilter->SetNumberOfThreads( list.numberOfThread ) ;
        }
        transformDeformationFieldFilter->SetInput( field ) ;
        transformDeformationFieldFilter->SetTransform( transform->GetTransform() ) ;
        transformDeformationFieldFilter->Update() ;
        field = transformDeformationFieldFilter->GetOutput() ;
        field->DisconnectPipeline() ;
      }
      //Create the DTI transform
      warpTransform->SetDeformationField( field ) ;
      typedef itk::DiffusionTensor3DNonRigidTransform< PixelType > NonRigidTransformType ;
      typedef itk::DiffusionTensor3DPPDAffineTransform< PixelType > PPDAffineTransformType ;
      typename NonRigidTransformType::TransformType::Pointer nonRigidFile ;
      typename NonRigidTransformType::Pointer nonRigid = NonRigidTransformType::New() ;
      nonRigid->SetTransform( dynamic_cast< typename NonRigidTransformType::TransformType* > (warpTransform.GetPointer() ) ) ;
      typename PPDAffineTransformType::Pointer affineppd = PPDAffineTransformType::New() ;
      typename itk::DiffusionTensor3DAffineTransform< PixelType >::Pointer affine ;
      affine = affineppd ;
      nonRigid->SetAffineTransformType( affine ) ;
      transform = nonRigid ;
    }
    //multiple rigid/affine transforms: concatenate them
    else if( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() > 1 )
    {
      typedef itk::DiffusionTensor3DMatrix3x3Transform< PixelType > MatrixTransformType ;
      itk::Matrix< double , 4 , 4 > composedMatrix ;
      composedMatrix.SetIdentity() ;
      itk::Matrix< double , 4 , 4 > tempMatrix ;
      itk::Matrix< double , 3 , 3 > matrix ;
      itk::Vector< double , 3 > vector ;
      composedMatrix.SetIdentity() ;
      do
      {
        transform = SetTransform< PixelType > ( list , image , transformFile ) ;
        typename MatrixTransformType::Pointer localTransform ;
        localTransform = dynamic_cast< MatrixTransformType* > (transform.GetPointer() ) ;
        if( !localTransform )//should never happen, just for security
        {
           std::cerr << "An affine or rigid transform was not convertible to DiffusionTensor3DMatrix3x3Transform" << std::endl ;
           return -1 ;
        }
        matrix = localTransform->GetMatrix3x3() ;
        vector = localTransform->GetTranslation() ;
        tempMatrix.SetIdentity() ;
        for( int i = 0 ; i < 3 ; i++ )
        {
          for( int j = 0 ; j < 3 ; j++ )
          {
            tempMatrix[ i ][ j ] = matrix[ i ][ j ] ;
          }
          tempMatrix[ i ][ 3 ] = vector[ i ] ;
        }
        tempMatrix *= composedMatrix ;
        composedMatrix = tempMatrix ;
      }while( transformFile->GetTransformList()->size() ) ;
      typedef itk::DiffusionTensor3DFSAffineTransform< PixelType >
         FSAffineTransformType ;
      typedef itk::DiffusionTensor3DPPDAffineTransform< PixelType >
         PPDAffineTransformType ;
      //Finite Strain
      if( !list.ppd )
        {
        typename FSAffineTransformType::Pointer affine = FSAffineTransformType::New() ;
        affine->SetMatrix4x4( composedMatrix ) ;
        transform = affine ;
        }
      // Preservation of principal direction
      else
        {
        typename PPDAffineTransformType::Pointer affineppd
          = PPDAffineTransformType::New() ;
        affineppd->SetMatrix4x4( composedMatrix ) ;
        transform = affineppd ;
        }
    }
    else
    {
      //only one transform, just load it
      transform = SetTransform< PixelType > ( list , image , transformFile ) ;
    }
    double defaultPixelValue = list.defaultPixelValue ;
    //start transform
    transform->SetMeasurementFrame( measurementFrame ) ;//This was probably wrong in the old versions
  {//local for memory management: the input image should not stay in memory after we exit this portion of code
      ResamplerTypePointer resampler = ResamplerType::New() ;
      resampler->SetInput( image ) ;
      resampler->SetTransform( transform ) ;
      if( list.numberOfThread )
      {
        resampler->SetNumberOfThreads( list.numberOfThread ) ;
      }
      resampler->SetInterpolator( interpol ) ;
      SetOutputParameters< PixelType > ( list , resampler , image ) ;
      resampler->SetDefaultPixelValue( static_cast<PixelType> ( defaultPixelValue ) ) ;//Could be set directly in the interpolator, but we keep ITKResampleImageFilter design
    //Compute the resampled image
      resampler->Update() ;
      writer->SetMeasurementFrame( resampler->GetOutputMeasurementFrame() ) ;//measurement frame set to Identity after resampling
      image = resampler->GetOutput() ;
    }
    //Tensors Corrections
    if( !list.correction.compare( "zero" ) )
    {
      typedef itk::DiffusionTensor3DZeroCorrectionFilter<InputImageType , InputImageType > ZeroCorrection ;
      typename ZeroCorrection::Pointer zeroFilter ;
      zeroFilter = ZeroCorrection::New() ;
      zeroFilter->SetInput( image ) ;
      if( list.numberOfThread ) 
      {
        zeroFilter->SetNumberOfThreads( list.numberOfThread ) ;
      }
      zeroFilter->Update() ;
      image = zeroFilter->GetOutput() ;
    }
    else if( !list.correction.compare( "abs" ) )
    {
      typedef itk::DiffusionTensor3DAbsCorrectionFilter<InputImageType , InputImageType > AbsCorrection ;
      typename AbsCorrection::Pointer absFilter ;
      absFilter = AbsCorrection::New() ;
      absFilter->SetInput( image ) ;
      if( list.numberOfThread ) 
      {
        absFilter->SetNumberOfThreads( list.numberOfThread ) ;
      }
      absFilter->Update() ;
      image = absFilter->GetOutput() ;
    }
    else if( !list.correction.compare( "nearest" ) )
    {
      typedef itk::DiffusionTensor3DNearestCorrectionFilter<InputImageType , InputImageType > NearestCorrection ;
      typename NearestCorrection::Pointer nearestFilter ;
      nearestFilter = NearestCorrection::New() ;
      nearestFilter->SetInput( image ) ;
      if( list.numberOfThread ) 
      {
        nearestFilter->SetNumberOfThreads( list.numberOfThread ) ;
      }
      nearestFilter->Update() ;
      image = nearestFilter->GetOutput() ;
    }
    //Save result
    if( !list.space.compare( "RAS" ) &&  list.transformationFile.compare( "" ) )
    {
      RASLPS< PixelType >( image ) ;
    }
    if( list.numberOfThread ) 
    {
      writer->SetNumberOfThreads( list.numberOfThread ) ;
    }
    writer->SetInput( image ) ;
    //Save output image
    try
    {
      writer->Update( list.outputVolume.c_str() ) ;
    }
    catch( itk::ExceptionObject & Except )
    {
      std::cerr << "Writing output image: Exception caught!"
                << std::endl ;
      std::cerr << Except << std::endl ;
      return -1 ;
    }
    return 0 ;
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
  list.deffield = deffield ;
  list.typeOfField = typeOfField ;
  list.defaultPixelValue = defaultPixelValue ;
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
