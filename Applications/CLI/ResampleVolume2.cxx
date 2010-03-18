/*=========================================================================

  Program:   Slicer3
  Language:  C++
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include <iostream>
#include <sstream>
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
#include "DiffusionApplications/ResampleDTI/itkWarpTransform3D.h"
#include "DiffusionApplications/ResampleDTI/itkTransformDeformationFieldFilter.h"
#include "DiffusionApplications/ResampleDTI/dtiprocessFiles/deformationfieldio.h"
#include "DiffusionApplications/ResampleDTI/dtiprocessFiles/dtitypes.h"
#include "DiffusionApplications/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h"
#include <itkVectorLinearInterpolateImageFunction.h>
#include <itkVectorResampleImageFilter.h>


// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {

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
  std::string deffield ;
  std::string typeOfField ;
  double defaultPixelValue;
} ;


void GetImageType (std::string fileName,
                   itk::ImageIOBase::IOPixelType &pixelType,
                   itk::ImageIOBase::IOComponentType &componentType)
{
  typedef itk::Image< unsigned char , 3 > ImageType ;
  itk::ImageFileReader< ImageType >::Pointer imageReader ;
  imageReader = itk::ImageFileReader< ImageType >::New() ;
  imageReader->SetFileName( fileName.c_str() ) ;
  imageReader->UpdateOutputInformation() ;
  pixelType = imageReader->GetImageIO()->GetPixelType() ;
  componentType = imageReader->GetImageIO()->GetComponentType() ;
}




//Set the transformation
template< class PixelType >
typename itk::Transform< double , 3 , 3 >::Pointer
SetTransform( parameters list ,
              const typename itk::OrientedImage< PixelType , 3 >::Pointer &image ,
              itk::TransformFileReader::Pointer &transformFile
            )
{
   typedef itk::OrientedImage< PixelType , 3 > ImageType ;
   typedef itk::TransformFileReader::Pointer TransformReaderPointer ;
   typedef itk::AffineTransform< double , 3 > AffineTransformType ;
   typedef itk::Rigid3DTransform< double > RotationType ;
   typedef itk::Transform< double , 3 , 3 > TransformType ;
   typename TransformType::Pointer nonRigidFile ;
   itk::Matrix< double , 3 , 3 > transformMatrix ;
//   TransformReaderPointer transformFile ;
//   itk::Transform< double , 3 , 3 >ReadTransform( list , transformFile ) ;
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
            = dynamic_cast< AffineTransformType* > ( transformFile->GetTransformList()->back().GetPointer() ) ;
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
               = dynamic_cast< RotationType* > ( transformFile->GetTransformList()->back().GetPointer() ) ;
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
                  ( transformFile->GetTransformList()->back().GetPointer() ) ;
            if( nonRigidFile )//if non rigid Transform loaded
            {
               list.transformType.assign( "nr" ) ;
               transform = nonRigidFile ;
            }
            else//something else
            {
               std::cerr<< "Transformation type not yet implemented"
                     << std::endl ;
               return NULL ;
            }
         }
      }
      if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
      {
            //if problem in the number of parameters
         if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
         {
            std::cerr<< "Error in the file containing the matrix transformation"
                  << std::endl ;
            return NULL ;
         }
      }
      transformFile->GetTransformList()->pop_back() ;
   }
   if( list.transformType.compare( "nr" ) ) //if rigid or affine transform
   {
      itk::Point< double > center ;
      itk::Vector< double > translation ;
      if( list.centeredTransform )
      {
         typename ImageType::SizeType sizeim=image->GetLargestPossibleRegion().GetSize() ;
         itk::Point< double , 3 > point ;
         itk::Point< double , 3 > pointOpposite ;
         itk::Index< 3 > index ;
         for( int i = 0 ; i < 3 ; i++ )
         { index[ i ] = 0 ; }
         image->TransformIndexToPhysicalPoint( index , point ) ;
         for( int i = 0 ; i < 3 ; i++ )
         { index[ i ] = sizeim[ i ] - 1 ; }
         image->TransformIndexToPhysicalPoint( index , pointOpposite ) ;
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
         if( exception.find("Attempting to set a non-orthogonal rotation matrix") != std::string::npos )
         {
            list.transformType = "a" ;
            std::cerr<<"Non-orthogonal rotation matrix: uses affine transform"<<std::endl;
         }
         else
         {
            return NULL ;
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
   return transform ;
}



//Read the transform file and return the number of non-rigid transform.
//If the transform file contain a transform that the program does not
//handle, the function returns -1
template< class PixelType >
int ReadTransform( const parameters &list ,
                   const typename itk::OrientedImage< PixelType , 3 >::Pointer &image ,
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

template< class PixelType >
itk::Transform< double , 3 , 3 >::Pointer
SetUpTransform( parameters list ,
                typename itk::ResampleImageFilter< itk::OrientedImage< PixelType , 3 > ,
                                                   itk::OrientedImage< PixelType , 3 > >::Pointer resampler ,
                typename itk::OrientedImage< PixelType , 3 >::Pointer image )
{
   typedef itk::Transform< double , 3 , 3 > TransformType ;
   typedef itk::OrientedImage< PixelType , 3 > ImageType ;
   typedef itk::AffineTransform< double , 3 > AffineTransformType ;
   typedef itk::Rigid3DTransform< double > RotationType ;
//   typename TransformType::Pointer nonRigidFile ;
   typename DeformationImageType::Pointer fieldPointer ;
   typedef itk::TransformFileReader::Pointer TransformReaderPointer ;
   TransformReaderPointer transformFile ;
   int nonRigidTransforms = 0 ;
   nonRigidTransforms = ReadTransform< PixelType >( list , image , transformFile ) ;
   if( nonRigidTransforms < 0 )//The transform file contains a transform that is not handled by ResampleVolume2, it exits.
   {
      return NULL ;
   }
   if( list.deffield.compare( "" ) )
   {
       //set if the field is a displacement or a h- field
      DeformationFieldType dftype = HField ;
      if( !list.typeOfField.compare( "displacement" ) )
      {
         dftype = Displacement ;
      }
       //reads deformation field and if it is a h-field, it transforms it to a displacement field
      fieldPointer = readDeformationField( list.deffield , dftype ) ;
      nonRigidTransforms++ ;
   }
   TransformType::Pointer transform ;
   typename ImageType::PointType originOutput ;
   typename ImageType::SpacingType spacingOutput ;
   typename ImageType::SizeType sizeOutput ;
   typename ImageType::DirectionType directionOutput ;
   originOutput = resampler->GetOutputOrigin() ;
   spacingOutput = resampler->GetOutputSpacing() ;
   sizeOutput = resampler->GetSize() ;
   directionOutput = resampler->GetOutputDirection() ;
   //If more than one transform or if hfield, add all transforms and compute the deformation field
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
         transformDeformationFieldFilter->SetTransform( transform ) ;
         transformDeformationFieldFilter->Update() ;
         field = transformDeformationFieldFilter->GetOutput() ;
         field->DisconnectPipeline() ;
      }
      //Create the DTI transform
      warpTransform->SetDeformationField( field ) ;
      transform = warpTransform ;
   }
    //multiple rigid/affine transforms: concatenate them
   else if( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() > 1 )
   {
      typedef itk::MatrixOffsetTransformBase< double , 3 , 3 > MatrixTransformType ;
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
            std::cerr << "An affine or rigid transform was not convertible to itk::MatrixOffsetTransformBase< double , 3 , 3 >" << std::endl ;
            return NULL ;
         }
         matrix = localTransform->GetMatrix() ;
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
      typename AffineTransformType::Pointer affine = AffineTransformType::New() ;
      //copy 4x4 matrix into a 3x3 matrix and a vector ;
      for( int i = 0 ; i < 3 ; i++ )
      {
         for( int j = 0 ; j < 3 ; j++ )
         {
            matrix[ i ][ j ] = composedMatrix[ i ][ j ] ;
         }
         vector[ i ] = composedMatrix[ i ][ 3 ] ;
      }
      affine->SetMatrix( matrix ) ;
      affine->SetTranslation( vector ) ;
      transform = affine ;
   }
   else
   {
      //only one transform, just load it
      transform = SetTransform< PixelType > ( list , image , transformFile ) ;
   }
return transform ;
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
  for(unsigned int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
    {
    IteratorImageType intemp( vectorImage.at( i ) , vectorImage.at( i )->GetLargestPossibleRegion() ) ;
    intemp.GoToBegin();
    in.push_back( intemp ) ;
    }
  itk::VariableLengthVector< PixelType > value ;
  value.SetSize( vectorImage.size() ) ;
  for( out.GoToBegin() ; !out.IsAtEnd() ; ++out )
    {
    for( unsigned int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
      {
      value.SetElement( i , in.at( i ).Get() ) ;
      ++in[ i ] ;
      }
    out.Set( value ) ;
    }
  return EXIT_SUCCESS ;
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
  for( unsigned int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
    {
    typename ImageType::Pointer imageTemp = ImageType::New() ;
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
    for( unsigned int i = 0 ; i < imagePile->GetVectorLength() ; i++ )
      {
      out[ i ].Set( value[ i ] ) ;
      ++out[ i ] ;
      }
    }
  return EXIT_SUCCESS ;
}


//Verify if some input parameters are null
bool VectorIsNul( std::vector< double > vec )
{
  bool zero = 1 ;
  for( ::size_t i = 0 ; i < vec.size() ; i++ ) { if( vec[i] != 0 ) { zero = 0 ; } }
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
    if( !list.space.compare( "RAS" ) && list.transformationFile.compare( "" ) )
      {
      typename ImageType::PointType originReference ;
      typename ImageType::DirectionType directionReference ;
      originReference = readerReference->GetOutput()->GetOrigin() ;
      directionReference = readerReference->GetOutput()->GetDirection();
      originReference[0]=-originReference[0];
      originReference[1]=-originReference[1];
      itk::Matrix< double , 3 , 3 > ras ;
      ras.SetIdentity() ;
      ras[ 0 ][ 0 ] = -1 ;
      ras[ 1 ][ 1 ] = -1 ;
      directionReference=ras*directionReference;
      readerReference->GetOutput()->SetOrigin( originReference ) ;
      readerReference->GetOutput()->SetDirection( directionReference ) ; 
      }
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
  resampler->SetDefaultPixelValue( static_cast<PixelType> (list.defaultPixelValue) );
}

typedef itk::Transform<double,3,3>::Pointer Transform3DPointer;
template<class PixelType>
void CheckDWMRI(itk::MetaDataDictionary &dico ,
                Transform3DPointer &transform ,
                parameters itkNotUsed(list) )
{
   typedef std::vector< std::vector< double > > DoubleVectorType ;
   typedef itk::MetaDataObject< DoubleVectorType > MetaDataDoubleVectorType ;
   typedef itk::MetaDataObject< std::string > MetaDataStringType ;
   typedef itk::AffineTransform< double , 3 > AffineTransformType;
   typedef itk::Rigid3DTransform< double > RotationType;
   //Inverse the transform if possible
   bool noninvertible = false ;
   itk::Transform< double , 3 , 3 >::Pointer inverseTransform ;
   try
   {
      typename AffineTransformType::Pointer affine = dynamic_cast<AffineTransformType* > ( transform.GetPointer() ) ;
      if(affine )//Rotation around a selected point
      {
         typename AffineTransformType::Pointer affinetemp = AffineTransformType::New() ;
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
            noninvertible = true ;
         }
      }
   }
   catch(...)
   {
      std::cout << "Exception Detected" << std::endl ;
      noninvertible = true ;
   }
   //if the transform is not invertible, we continue to see if the image is a DWMRI
   itk::Matrix< double , 3 , 3 > measurementFrame ;
   itk::MetaDataDictionary::ConstIterator itr = dico.Begin() ;
   itk::MetaDataDictionary::ConstIterator end = dico.End() ;
   //We look for the measurement frame in the metadatadictionary
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
               //we copy the measurement frame
               measurementFrame[ i ][ j ] = tagvalue.at( j ).at( i ) ;
               tagvalue.at(j).at(i) = ( i == j ? 1 : 0 ) ;
               }
            }
            //if the transform is invertible, we set the measurement frame to the identity matrix
            //because we are going to apply this measurement frame to the gradient vectors before transforming them
            if( !noninvertible )
            {
            entryvalue->SetMetaDataObjectValue( tagvalue );
            }
         }
      }
      ++itr ;
   }
   //even if the transform is not invertible, we still go through the metadatadictionary to check if the image is a DWMRI
   //if the image is not, everything is fine. If the image is, we print an error message. The gradient direction of the transform image is not going to be correct
   itr = dico.Begin() ;
   end = dico.End() ;
   bool dtmri = 0 ;
   while( itr != end )
   {
      itk::MetaDataObjectBase::Pointer entry = itr->second ;
      MetaDataStringType::Pointer entryvalue
         = dynamic_cast<MetaDataStringType* >( entry.GetPointer() ) ;
      if( entryvalue )
      {
         //get the gradient directions
         int pos = itr->first.find( "DWMRI_gradient" ) ;
         if( pos != -1 )
         {
            dtmri = 1 ;
            if( !noninvertible )
            {
               std::string tagvalue = entryvalue->GetMetaDataObjectValue() ;
               itk::Vector< double , 3 > vec ;
               itk::Vector< double , 3 > transformedVector ;
               std::istringstream iss( tagvalue ) ;
               iss >> vec[ 0 ] >> vec[ 1 ] >> vec[ 2 ] ;//we copy the metavalue in an itk::vector
               if( iss.fail() )
               {
                  iss.str( tagvalue ) ;
                  iss.clear() ;
                  std::string trash ;
                  iss >> vec[ 0 ] >> trash >> vec[ 1 ] >> trash >> vec[ 2 ] ;//in case the separator between the values is something else than spaces
                  if( iss.fail() )//problem reading the gradient values
                  {
                     std::cerr << "Error reading a DWMRI gradient value" << std::endl ;
                  }
               }
               //We transform the gradient vector with the measurement frame and then apply the inverse transform
               transformedVector = inverseTransform->TransformVector( measurementFrame * vec ) ;
               if( transformedVector.GetNorm() > .00001 ) //gradient not null
               {
                  transformedVector.Normalize() ;
               }
               std::ostringstream oss ;
               //write the new gradient values (after transformation) in the metadatadictionary
               oss << transformedVector[ 0 ] << "  " <<
                      transformedVector[ 1 ] << "  " <<
                      transformedVector[ 2 ] << std::ends ;
               entryvalue->SetMetaDataObjectValue( oss.str() ) ;
            }
            else//if the image is a DWMRI we exit the loop
            {
               break ;
            }
         }
      }
      ++itr ;
   }
   //print an error message if the transform is not invertible and if the image is a DWMRI 
   if( noninvertible && dtmri )
   {
      std::cerr << "The gradient transformation is not handle correctly with the current transformation.\nThe gradient direction of the output image is probably wrong"<< std::endl ;
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

template< class ImageType >
typename itk::InterpolateImageFunction< ImageType , double >::Pointer
SetInterpolator( const parameters &list )
{
   typedef itk::ConstantBoundaryCondition< ImageType > BoundaryCondition ;
   typedef itk::InterpolateImageFunction< ImageType , double > InterpolatorType ;
   typedef itk::NearestNeighborInterpolateImageFunction< ImageType , double > NearestNeighborInterpolateType ;
   typedef itk::LinearInterpolateImageFunction< ImageType , double > LinearInterpolateType;
   typedef itk::BSplineInterpolateImageFunction< ImageType , double , double > BSplineInterpolateFunction ;
   typename InterpolatorType::Pointer interpol ;
   if( !list.interpolationType.compare( "linear" ) )
   {
      typename LinearInterpolateType::Pointer linearinterpolator = LinearInterpolateType::New() ;
      interpol = linearinterpolator ;
   }
   else if( !list.interpolationType.compare( "nn" ) )
   {
      typename NearestNeighborInterpolateType::Pointer interpolator = NearestNeighborInterpolateType::New() ;
      interpol = interpolator ;
   }
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
   return interpol ;
}


template< class PixelType > int Rotate( parameters list )
{
   typedef itk::OrientedImage< PixelType , 3 > ImageType ;
   typedef itk::ImageFileReader< ImageType > FileReaderType ;   
   //typename FileReaderType::Pointer readerCopyInfo = FileReaderType::New() ;
   typedef itk::InterpolateImageFunction< ImageType , double > InterpolatorType ;
   typedef itk::ResampleImageFilter< ImageType, ImageType > ResampleType ;
   typedef itk::Transform< double , 3 , 3 > TransformType ;
   typename ImageType::Pointer image ;
   ///////////////////////////////////////////
   typename itk::ImageFileReader< itk::VectorImage< PixelType , 3 > >::Pointer reader ;
   reader = itk::ImageFileReader< itk::VectorImage< PixelType , 3 > >::New() ;
   reader->SetFileName( list.inputVolume.c_str()) ;
   reader->Update() ;
   if( !list.space.compare( "RAS" ) && list.transformationFile.compare( "" ) )
   {
      RASLPS<PixelType>( reader->GetOutput() ) ;
   }
   itk::MetaDataDictionary dico = reader->GetOutput()->GetMetaDataDictionary() ;
   std::vector< typename ImageType::Pointer > vectorImage ;
   std::vector< typename ImageType::Pointer > vectorOutputImage ;
   SeparateImages< PixelType >( reader->GetOutput() , vectorImage ) ;
   ////////////////////////////////////////////////////
   typename InterpolatorType::Pointer interpol ;
   interpol = SetInterpolator< ImageType >( list ) ;
   typename ResampleType::Pointer resample = ResampleType::New() ;
   SetOutputParameters< PixelType >( list , resample , vectorImage[ 0 ] ) ;
   TransformType::Pointer transform ;
   transform = SetUpTransform< PixelType >( list , resample , vectorImage[ 0 ] ) ;
   if( !transform )
   {
      return EXIT_FAILURE ;
   }
   resample->SetTransform( transform ) ;
   resample->SetInterpolator( interpol ) ;
   if(list.numberOfThread)
   {
      resample->SetNumberOfThreads( list.numberOfThread ) ;
   }
   for( ::size_t idx = 0 ; idx < vectorImage.size() ; idx++ )
   {
     resample->SetInput( vectorImage[ idx ] ) ;
     resample->Update() ;
     vectorOutputImage.push_back( resample->GetOutput() ) ;
     vectorOutputImage[ idx ]->DisconnectPipeline() ;
   }
   //If necessary, transform gradient vectors with the loaded transformations
   CheckDWMRI< PixelType >( dico , transform , list ) ;
   typename itk::VectorImage< PixelType, 3 >::Pointer outputImage = itk::VectorImage< PixelType , 3 >::New() ;
   AddImage< PixelType >( outputImage , vectorOutputImage ) ;
   if( !list.space.compare( "RAS" ) && list.transformationFile.compare( "" ) )
   {
      RASLPS<PixelType>( outputImage) ;
   }
   outputImage->SetMetaDataDictionary( dico ) ;
   typedef itk::ImageFileWriter< typename itk::VectorImage< PixelType, 3 > > WriterType ;
   typename WriterType::Pointer writer = WriterType::New() ;
   writer->SetInput( outputImage ) ;
   writer->SetFileName( list.outputVolume.c_str() ) ;
   writer->UseCompressionOn() ;
   writer->Update() ;
   return EXIT_SUCCESS ;
}

} // end of anonymous namespace


int main( int argc , char * argv[] )
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
  list.deffield = deffield ;
  list.typeOfField = typeOfField ;
  list.defaultPixelValue = defaultPixelValue ;
  if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
    {
    std::cerr<<"Argument(s) having wrong size"<<std::endl ;
    return EXIT_FAILURE;
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
  return EXIT_FAILURE;
}
