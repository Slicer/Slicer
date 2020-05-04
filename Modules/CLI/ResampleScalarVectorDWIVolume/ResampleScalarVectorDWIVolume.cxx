/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// ITK includes
#include <itkBSplineDeformableTransform.h>
#include <itkCompositeTransform.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkRigid3DTransform.h>
#include <itkThinPlateSplineKernelTransform.h>
#include <itkTransformFileReader.h>
#include <itkVectorResampleImageFilter.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkConstantBoundaryCondition.h>
#include <itkTransformFactory.h>

// ResampleScalarVectorDWIVolume includes
#include "ResampleScalarVectorDWIVolumeCLP.h"

// ResampleDTIVolume includes
#include "dtiprocessFiles/deformationfieldio.h"
#include "itkTransformDeformationFieldFilter.h"
#include "itkWarpTransform3D.h"

// STD includes

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

#define RADIUS 3

struct parameters
  {
  int numberOfThread;
  std::string interpolationType;
  std::string transformType;
  std::vector<double> transformMatrix;
  std::string inputVolume;
  std::string outputVolume;
  std::string referenceVolume;
  std::vector<float> rotationPoint;
  std::string transformationFile;
  bool inverseITKTransformation;
  std::string windowFunction;
  unsigned int splineOrder;
  bool space;
  bool centeredTransform;
  std::vector<double> outputImageSpacing;
  std::vector<double> outputImageSize;
  std::vector<float> outputImageOrigin;
  std::vector<double> directionMatrix;
  std::string deffield;
  std::string typeOfField;
  double defaultPixelValue;
  std::string imageCenter;
  std::string transformsOrder;
  bool notbulk;
  };

// To check the image voxel type
void GetImageType( std::string fileName,
                   itk::ImageIOBase::IOPixelType & pixelType,
                   itk::ImageIOBase::IOComponentType & componentType
                   )
{
  typedef itk::Image<unsigned char, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader;
  imageReader = itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName( fileName.c_str() );
  imageReader->UpdateOutputInformation();
  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
}

// Copy rigid or affine transform to list.transformMatrix
template <class PixelType>
void
SetListFromTransform( const typename itk::MatrixOffsetTransformBase<PixelType, 3, 3>::Pointer & transform,
                      parameters & list
                      )
{
  for( int i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      list.transformMatrix.push_back( (double)transform->GetMatrix()[i][j] );
      }
    }
  for( int i = 0; i < 3; i++ )
    {
    list.transformMatrix.push_back( (double)transform->GetTranslation()[i] );
    list.rotationPoint.push_back( (double)transform->GetCenter()[i] );
    }
}

template <class ImageType>
itk::Point<double>
ImageCenter( const typename ImageType::Pointer & image
             )
{
  typename itk::Index<3> index;
  itk::Size<3>          sizeim = image->GetLargestPossibleRegion().GetSize();
  itk::Point<double, 3> point;
  itk::Point<double, 3> pointOpposite;
  itk::Point<double>    center;
  for( int i = 0; i < 3; i++ )
    {
    index[i] = 0;
    }
  image->TransformIndexToPhysicalPoint( index, point );
  for( int i = 0; i < 3; i++ )
    {
    index[i] = sizeim[i] - 1;
    }
  image->TransformIndexToPhysicalPoint( index, pointOpposite );
  for( int i = 0; i < 3; i++ )
    {
    center[i] = ( point[i] + pointOpposite[i] ) / 2;
    }
  return center;
}

template <class ImageType>
itk::Matrix<double, 4, 4>
ComputeTransformMatrix( const parameters & list,
                        const typename ImageType::Pointer & image,
                        const itk::Point<double> & outputImageCenter
                        )
{
  itk::Matrix<double, 4, 4> transformMatrix4x4;
  transformMatrix4x4.SetIdentity();
  itk::Point<double>  center;
  itk::Vector<double> translation;
  itk::Vector<double> offset;
  // if centered transform set, ignore the transformation center
  // set before and use the center of the image instead
  if( list.centeredTransform )
    {
    if( !list.imageCenter.compare( "input" ) )
      {
      center = ImageCenter<ImageType>( image );
      }
    else
      {
      center = outputImageCenter;
      }
    }
  else
    {
    for( int i = 0; i < 3; i++ )
      {
      center[i] = list.rotationPoint[i];
      }
    }
  // Set the transform matrix
  for( int i = 0; i < 3; i++ )
    {
    translation[i] = ( double ) list.transformMatrix[9 + i];
    offset[i] = translation[i] + center[i];
    for( int j = 0; j < 3; j++ )
      {
      transformMatrix4x4[i][j]
        = ( double ) list.transformMatrix[i * 3 + j];
      offset[i] -= transformMatrix4x4[i][j] * center[j];
      }
    // Compute the 4th column of the matrix
    transformMatrix4x4[i][3] = offset[i];
    }
  // If set, inverse the given transform.
  // The given transform therefore transform the input image into the output image
  if( list.inverseITKTransformation )
    {
    transformMatrix4x4 = transformMatrix4x4.GetInverse();
    }
/*  //If the transform is in RAS, transform it in LPS
  if( list.space  && !list.transformationFile.compare( "" )  )
  {
    itk::Matrix< double , 4 , 4 > ras ;
    ras.SetIdentity() ;
    ras[ 0 ][ 0 ] = -1 ;
    ras[ 1 ][ 1 ] = -1 ;
    transformMatrix4x4 = ras * transformMatrix4x4 * ras ;
}*/
  return transformMatrix4x4;
}

// Center or/and invert the affine/rigid transform if this option was selected
template <class ImageType>
typename itk::Transform<double, 3, 3>::Pointer
SetUpTransform( parameters & list,
                const typename ImageType::Pointer & image,
                typename itk::Transform<double, 3, 3>::Pointer transform,
                const itk::Point<double> & outputImageCenter
                )
{
  typedef itk::AffineTransform<double, 3>        AffineTransformType;
  typedef itk::MatrixOffsetTransformBase<double> RotationType;
  itk::Matrix<double, 3, 3> transformMatrix;
  itk::Vector<double, 3>    vec;
  if( list.transformType.compare( "nr" ) )  // if rigid or affine transform
    {
    itk::Matrix<double, 4, 4> transformMatrix4x4;
    transformMatrix4x4 = ComputeTransformMatrix<ImageType>( list, image, outputImageCenter );
    for( int i = 0; i < 3; i++ )
      {
      for( int j = 0; j < 3; j++ )
        {
        transformMatrix[i][j] = transformMatrix4x4[i][j];
        }
      vec[i] = transformMatrix4x4[i][3];
      }
    if( !list.transformType.compare( "rt" ) )  // Rotation around a selected point
      {
      try
        {
        typename RotationType::Pointer rotation = RotationType::New();
        rotation->SetMatrix( transformMatrix );
        rotation->SetTranslation( vec );
        transform = rotation;
        }
      catch( itk::ExceptionObject &exp )
        {
        std::string exception = exp.GetDescription();
        if( exception.find( "Attempting to set a non-orthogonal rotation matrix" ) != std::string::npos )
          {
          list.transformType = "a";
          std::cerr << "Non-orthogonal rotation matrix: uses affine transform" << std::endl;
          }
        else
          {
          return nullptr;
          }
        }
      }
    if( !list.transformType.compare( "a" ) )  // Affine transform
      {
      typename AffineTransformType::Pointer affine = AffineTransformType::New();
      affine->SetMatrix( transformMatrix );
      affine->SetTranslation( vec );
      transform = affine;
      }
    }
  return transform;
}

// ITK does not initialize TPS transforms properly when they are read from file.
// tpsTransform->ComputeWMatrix() has to be called after the transform is read but
//  before the transform is used, otherwise the application crashes.
void InitializeThinPlateSplineTransform(itk::Transform<double, 3, 3>::Pointer transform)
{
  std::string transformClassName = transform->GetNameOfClass();
  if (transformClassName == "ThinPlateSplineKernelTransform")
    {
    typedef itk::ThinPlateSplineKernelTransform< double, 3 > ThinPlateSplineTransformType;
    ThinPlateSplineTransformType* tpsTransform = static_cast<ThinPlateSplineTransformType*>(transform.GetPointer());
    tpsTransform->ComputeWMatrix();
    }
  else if (transformClassName == "CompositeTransform")
    {
    typedef itk::CompositeTransform< double, 3 > CompositeTransformType;
    CompositeTransformType* compositeTransform = static_cast<CompositeTransformType*>(transform.GetPointer());
    for (unsigned int i = 0; i < compositeTransform->GetNumberOfTransforms(); ++i)
      {
      InitializeThinPlateSplineTransform(compositeTransform->GetNthTransform(i));
      }
    }
}

// Set the transformation
template <class ImageType>
typename itk::Transform<double, 3, 3>::Pointer
SetTransformAndOrder( parameters & list,
                      const typename ImageType::Pointer & image,
                      typename itk::Transform<double, 3, 3>::Pointer transform,
                      const itk::Point<double> & outputImageCenter
                      )
{
  typedef itk::AffineTransform<double, 3>        AffineTransformType;
  if( list.transformationFile.compare( "" ) ) // Get transformation matrix from command line if no file given
    {
    std::string transformClassName = transform->GetNameOfClass();
    list.transformMatrix.resize( 0 );
    list.rotationPoint.resize( 0 );
    typename itk::MatrixOffsetTransformBase<double, 3, 3>
    ::Pointer matrixOffsetTransform;
    if( transformClassName.find("AffineTransform") != std::string::npos ) // if affine transform
      {
      matrixOffsetTransform = static_cast<AffineTransformType *>( transform.GetPointer() );
      list.transformType.assign( "a" );
      SetListFromTransform<double>( matrixOffsetTransform, list );
      }
    else
      {
        // Check if transform is not of type "Rigid3DTransform"
        if( transformClassName == "Rigid3DTransform" ||
            transformClassName == "Euler3DTransform" ||
            transformClassName == "CenteredEuler3DTransform" ||
            transformClassName == "QuaternionRigidTransform" ||
            transformClassName == "VersorTransform" ||
            transformClassName == "ScaleSkewVersor3DTransform" ||
            transformClassName == "ScaleVersor3DTransform" ||
            transformClassName == "Similarity3DTransform"
            ) // if rigid3D transform
          {
          list.transformType.assign( "rt" );
          matrixOffsetTransform = static_cast<itk::Rigid3DTransform<double> *>
            ( transform.GetPointer() );
          SetListFromTransform<double>( matrixOffsetTransform, list );
          }
        else // if non-rigid
          {

          // Workaround: prevent ITK from crashing when the transform contains TPS transform
          InitializeThinPlateSplineTransform(transform.GetPointer());

          if ( transformClassName.find("Transform") != std::string::npos )
            { // if non rigid Transform loaded
            list.transformType.assign( "nr" );
            }
          else               // something else
            {
            std::cerr << "Transformation type not yet implemented"
                      << std::endl;
            return nullptr;
            }
          }
      }
    if( list.transformType.compare( "nr" ) )      // if rigid or affine transform
      {
      // if problem in the number of parameters
      if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
        {
        std::cerr << "Error in the file containing the matrix transformation"
                  << std::endl;
        return nullptr;
        }
      }
    }
  return SetUpTransform<ImageType>( list, image, transform, outputImageCenter );
}

// Set the transformation
template <class ImageType>
typename itk::Transform<double, 3, 3>::Pointer
SetTransform( parameters & list,
              const typename ImageType::Pointer & image,
              itk::TransformFileReader::Pointer & transformFile,
              const itk::Point<double> & outputImageCenter
              )
{
  typedef itk::Transform<double, 3, 3> TransformType;
  typename TransformType::Pointer transform = nullptr;
  if( list.transformationFile.compare( "" ) ) // Get transformation matrix from command line if no file given
    {
    if( !list.transformsOrder.compare( "input-to-output" ) )
      {
      transform = static_cast<TransformType *>
        ( transformFile->GetTransformList()->back().GetPointer() );
      }
    else
      {
      transform = static_cast<TransformType *>
        ( transformFile->GetTransformList()->front().GetPointer() );
      }
    }
  transform = SetTransformAndOrder<ImageType>( list, image, transform, outputImageCenter );
  if( list.transformationFile.compare( "" ) ) // Get transformation matrix from command line if no file given
    {
    if( !list.transformsOrder.compare( "input-to-output" ) )
      {
      transformFile->GetTransformList()->pop_back();
      }
    else
      {
      transformFile->GetTransformList()->pop_front();
      }
    }
  return transform;
}

// Read the transform file and return the number of non-rigid transform.
// If the transform file contain a transform that the program does not
// handle, the function returns -1
template <class ImageType>
int ReadTransform( parameters & list,
                   const typename ImageType::Pointer & image,
                   itk::TransformFileReader::Pointer & transformFile
                   )
{
  int numberOfNonRigidTransform = 0;

  itk::Point<double> dummyOutputCenter;
  dummyOutputCenter.Fill( 0 );
  if( list.transformationFile.compare( "" ) )
    {
    transformFile = itk::TransformFileReader::New();
    transformFile->SetFileName( list.transformationFile.c_str() );
    transformFile->Update();

    // Check if any of the transform is not supported and counts the number of non-rigid transform
    do
      {
      if( !SetTransform<ImageType>( list, image, transformFile, dummyOutputCenter ) )
        {
        return -1;
        }
      if( !list.transformType.compare( "nr" ) )
        {
        numberOfNonRigidTransform++;
        }
      }
    while( transformFile->GetTransformList()->size() );

    transformFile->Update();
    return numberOfNonRigidTransform;
    }
  return 0;
}

// resamples field to output image size; local filter so that the memory is freed once it has run
void ResampleDeformationField( DeformationImageType::Pointer & field,
                               const itk::Point<double, 3> & origin,
                               const itk::Vector<double, 3> & spacing,
                               const itk::Size<3> & size,
                               const itk::Matrix<double, 3, 3> & direction
                               )
{
  // Check if the field does not already have the same properties as the output image:
  // It would save some time if we did not have to resample the field
  itk::Point<double, 3>     fieldOrigin;
  itk::Vector<double, 3>    fieldSpacing;
  itk::Size<3>              fieldSize;
  itk::Matrix<double, 3, 3> fieldDirection;
  fieldOrigin = field->GetOrigin();
  fieldSpacing = field->GetSpacing();
  fieldSize = field->GetLargestPossibleRegion().GetSize();
  fieldDirection = field->GetDirection();
  if(  fieldSize == size
       && fieldSpacing == spacing
       && fieldDirection == direction
       && fieldOrigin == origin
       )
    {
    return;
    }
  typedef itk::VectorLinearInterpolateImageFunction<DeformationImageType> VectorInterpolatorType;
  VectorInterpolatorType::Pointer linearVectorInterpolator = VectorInterpolatorType::New();
  typedef itk::VectorResampleImageFilter<DeformationImageType,
                                         DeformationImageType,
                                         double
                                         > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleFieldFilter = ResampleImageFilter::New();
  DeformationPixelType         defaultPixel;
  defaultPixel.Fill( 0.0 );
  resampleFieldFilter->SetDefaultPixelValue( defaultPixel );
  resampleFieldFilter->SetInput( field );
  resampleFieldFilter->SetInterpolator( linearVectorInterpolator );
  resampleFieldFilter->SetOutputDirection( direction );
  resampleFieldFilter->SetSize( size );
  resampleFieldFilter->SetOutputSpacing( spacing );
  resampleFieldFilter->SetOutputOrigin( origin );
  resampleFieldFilter->Update();
  field = resampleFieldFilter->GetOutput();
}

// Loads the transforms and merge them into only one transform
template <class ImageType>
itk::Transform<double, 3, 3>::Pointer
SetAllTransform( parameters & list,
                 typename itk::ResampleImageFilter<ImageType, ImageType>::Pointer resampler,
                 typename ImageType::Pointer image
                 )
{
  typedef itk::Transform<double, 3, 3>    TransformType;
  typedef itk::AffineTransform<double, 3> AffineTransformType;
  typename DeformationImageType::Pointer fieldPointer;
  typedef itk::TransformFileReader::Pointer TransformReaderPointer;
  TransformReaderPointer transformFile;
  int                    nonRigidTransforms = 0;
  nonRigidTransforms = ReadTransform<ImageType>( list, image, transformFile );
  if( nonRigidTransforms < 0 ) // The transform file contains a transform that is not handled by ResampleVolume2, it
                               // exits.
    {
    return nullptr;
    }
  if( list.deffield.compare( "" ) )
    {
    // set if the field is a displacement or a h- field
    DeformationFieldType dftype = HField;
    if( !list.typeOfField.compare( "displacement" ) )
      {
      dftype = Displacement;
      }
    // reads deformation field and if it is a h-field, it transforms it to a displacement field
    fieldPointer = readDeformationField( list.deffield, dftype );
    nonRigidTransforms++;
    }
  TransformType::Pointer transform;
  // typename ImageType::PointType originOutput ;
  // typename ImageType::SpacingType spacingOutput ;
  // typename ImageType::SizeType sizeOutput ;
  // typename ImageType::DirectionType directionOutput ;
  typename ImageType::Pointer dummyOutputImage = ImageType::New();
  dummyOutputImage->SetDirection( resampler->GetOutputDirection() );
  dummyOutputImage->SetSpacing( resampler->GetOutputSpacing() );
  dummyOutputImage->SetOrigin( resampler->GetOutputOrigin() );
  dummyOutputImage->SetRegions( resampler->GetSize() );
  itk::Point<double> outputImageCenter = ImageCenter<ImageType>( dummyOutputImage );
  // If more than one transform or if hfield, add all transforms and compute the deformation field
  if( ( list.transformationFile.compare( "" )
        && transformFile->GetTransformList()->size() > 1
        && nonRigidTransforms > 0
        )
      || list.deffield.compare( "" )
      )
    {
    // Create warp transform
    typedef itk::WarpTransform3D<double> WarpTransformType;
    typename WarpTransformType::Pointer warpTransform = WarpTransformType::New();
    typename DeformationImageType::Pointer field;
    if( list.deffield.compare( "" ) )
      {
      field = fieldPointer;
      // Resample the deformation field so that it has the same properties as the output image we want to compute
      ResampleDeformationField( field,
                                resampler->GetOutputOrigin(),
                                resampler->GetOutputSpacing(),
                                resampler->GetSize(),
                                resampler->GetOutputDirection()
                                );
      }
    else  // if no deformation field was loaded, we create an empty one
      {
      field = DeformationImageType::New();
      field->SetSpacing( resampler->GetOutputSpacing() );
      field->SetOrigin( resampler->GetOutputOrigin() );
      field->SetRegions( resampler->GetSize() );
      field->SetDirection( resampler->GetOutputDirection() );
      field->Allocate();
      DeformationPixelType vectorNull;
      vectorNull.Fill( 0.0 );
      field->FillBuffer( vectorNull );
      }
    // Compute the transformation field adding all the transforms together
    while( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() )
      {
      typedef itk::TransformDeformationFieldFilter<double, double, 3> itkTransformDeformationFieldFilterType;
      typename itkTransformDeformationFieldFilterType::Pointer transformDeformationFieldFilter =
        itkTransformDeformationFieldFilterType::New();
      transform = SetTransform<ImageType>( list, image, transformFile, outputImageCenter  );
      // check if there is a bspline transform and a bulk transform with it
      if( !list.notbulk && transform->GetTransformTypeAsString() == "BSplineDeformableTransform_double_3_3"  &&
          transformFile->GetTransformList()->size() )                                                                                                        //
                                                                                                                                                             // Check
                                                                                                                                                             // if
                                                                                                                                                             // transform
                                                                                                                                                             // file
                                                                                                                                                             // contains
                                                                                                                                                             // a
                                                                                                                                                             // BSpline
        {
        // order=3 for the BSpline seems to be standard among tools in Slicer3 and BRAINTools
        typedef itk::BSplineDeformableTransform<double, 3, 3> BSplineDeformableTransformType;
        BSplineDeformableTransformType::Pointer BSplineTransform;
        BSplineTransform = static_cast<BSplineDeformableTransformType *>(transform.GetPointer() );
        typename TransformType::Pointer bulkTransform;
        bulkTransform = SetTransform<ImageType>( list, image, transformFile, outputImageCenter );
        BSplineTransform->SetBulkTransform( bulkTransform );
        }
      if( list.numberOfThread )
        {
        transformDeformationFieldFilter->SetNumberOfThreads( list.numberOfThread );
        }
      transformDeformationFieldFilter->SetInput( field );
      transformDeformationFieldFilter->SetTransform( transform );
      transformDeformationFieldFilter->Update();
      field = transformDeformationFieldFilter->GetOutput();
      field->DisconnectPipeline();
      }

    // Create the transform
    warpTransform->SetDeformationField( field );
    transform = warpTransform;
    }
  // multiple rigid/affine transforms: concatenate them
  else if( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() > 1 )
    {
    typedef itk::MatrixOffsetTransformBase<double, 3, 3> MatrixTransformType;
    itk::Matrix<double, 4, 4> composedMatrix;
    composedMatrix.SetIdentity();
    itk::Matrix<double, 4, 4> tempMatrix;
    itk::Matrix<double, 3, 3> matrix;
    itk::Vector<double, 3>    vector;
    composedMatrix.SetIdentity();

    do
      {
      transform = SetTransform<ImageType>( list, image, transformFile, outputImageCenter );
      std::string transformClassName;
      if ( transform.IsNotNull() )
        {
        transformClassName = transform->GetNameOfClass();
        }
      // Check if transform is NOT of type "MatrixOffsetTransformBase<double, 3, 3>"
      // (itself typedef as "MatrixTransformType")
      if( !(
            transformClassName.find( "AffineTransform" ) != std::string::npos ||
            transformClassName == "MatrixOffsetTransformBase" ||
            transformClassName == "Rigid3DTransform" ||
            transformClassName == "Euler3DTransform" ||
            transformClassName == "CenteredEuler3DTransform" ||
            transformClassName == "QuaternionRigidTransform" ||
            transformClassName == "VersorTransform" ||
            transformClassName == "VersorRigid3DTransform" ||
            transformClassName == "ScaleSkewVersor3DTransform" ||
            transformClassName == "ScaleVersor3DTransform" ||
            transformClassName == "Similarity3DTransform" ||
            transformClassName == "ScaleTransform" ||
            transformClassName == "ScaleLogarithmicTransform"
            )
          )   // should never happen, just for security
        {
        std::cerr
        << "An affine or rigid transform was not convertible to itk::MatrixOffsetTransformBase< double , 3 , 3 >"
        << std::endl;
        return nullptr;
        }
      typename MatrixTransformType::Pointer localTransform;
      localTransform = static_cast<MatrixTransformType *>(transform.GetPointer() );
      matrix = localTransform->GetMatrix();
      vector = localTransform->GetTranslation();
      tempMatrix.SetIdentity();
      for( int i = 0; i < 3; i++ )
        {
        for( int j = 0; j < 3; j++ )
          {
          tempMatrix[i][j] = matrix[i][j];
          }
        tempMatrix[i][3] = vector[i];
        }
      tempMatrix *= composedMatrix;
      composedMatrix = tempMatrix;
      }
    while( transformFile->GetTransformList()->size() );

    typename AffineTransformType::Pointer affine = AffineTransformType::New();
    // copy 4x4 matrix into a 3x3 matrix and a vector ;
    for( int i = 0; i < 3; i++ )
      {
      for( int j = 0; j < 3; j++ )
        {
        matrix[i][j] = composedMatrix[i][j];
        }
      vector[i] = composedMatrix[i][3];
      }
    affine->SetMatrix( matrix );
    affine->SetTranslation( vector );
    transform = affine;
    }
  else
    {
    // only one transform, just load it
    transform = SetTransform<ImageType>( list, image, transformFile, outputImageCenter );
    }
  return transform;
}

// Write back the vector of images into a image vector
template <class PixelType>
int AddImage( typename itk::VectorImage<PixelType, 3>
              ::Pointer & imagePile,
              const std::vector<typename itk::Image<PixelType, 3>::Pointer> & vectorImage
              )
{
  typedef itk::Image<PixelType, 3> ImageType;
  imagePile->SetRegions( vectorImage.at( 0 )->GetLargestPossibleRegion().GetSize() );
  imagePile->SetOrigin( vectorImage.at( 0 )->GetOrigin() );
  imagePile->SetDirection( vectorImage.at( 0 )->GetDirection() );
  imagePile->SetSpacing( vectorImage.at( 0 )->GetSpacing() );
  imagePile->SetVectorLength( vectorImage.size() );
  imagePile->Allocate();
  typename itk::ImageRegionIterator<itk::VectorImage<PixelType, 3> > out( imagePile,
                                                                          imagePile->GetLargestPossibleRegion()
                                                                          );
  typedef typename itk::ImageRegionIterator<ImageType> IteratorImageType;
  std::vector<IteratorImageType> in;
  for( unsigned int i = 0; i < imagePile->GetVectorLength(); i++ )
    {
    IteratorImageType intemp( vectorImage.at( i ), vectorImage.at( i )->GetLargestPossibleRegion() );
    intemp.GoToBegin();
    in.push_back( intemp );
    }
  itk::VariableLengthVector<PixelType> value;
  value.SetSize( vectorImage.size() );
  for( out.GoToBegin(); !out.IsAtEnd(); ++out )
    {
    for( unsigned int i = 0; i < imagePile->GetVectorLength(); i++ )
      {
      value.SetElement( i, in.at( i ).Get() );
      ++in[i];
      }
    out.Set( value );
    }
  return EXIT_SUCCESS;
}

// Separate the vector image into a vector of images
template <class PixelType>
int SeparateImages( const typename itk::VectorImage<PixelType, 3>
                    ::Pointer & imagePile,
                    std::vector<typename itk::Image<PixelType, 3>::Pointer> & vectorImage
                    )
{
  typedef itk::Image<PixelType, 3>       ImageType;
  typedef itk::VectorImage<PixelType, 3> VectorImageType;
  typename itk::VectorImage<PixelType, 3>::SizeType size;
  typename itk::VectorImage<PixelType, 3>::DirectionType direction;
  typename itk::VectorImage<PixelType, 3>::PointType origin;
  typename itk::VectorImage<PixelType, 3>::SpacingType spacing;
  size = imagePile->GetLargestPossibleRegion().GetSize();
  direction = imagePile->GetDirection();
  origin = imagePile->GetOrigin();
  spacing = imagePile->GetSpacing();
  typename itk::ImageRegionIterator<VectorImageType> in( imagePile,
                                                         imagePile->GetLargestPossibleRegion() );
  typedef typename itk::ImageRegionIterator<ImageType> IteratorImageType;
  std::vector<IteratorImageType> out;
  for( unsigned int i = 0; i < imagePile->GetVectorLength(); i++ )
    {
    typename ImageType::Pointer imageTemp = ImageType::New();
    imageTemp->SetRegions( size );
    imageTemp->SetOrigin( origin );
    imageTemp->SetDirection( direction );
    imageTemp->SetSpacing( spacing );
    imageTemp->Allocate();
    vectorImage.push_back( imageTemp );
    IteratorImageType outtemp( imageTemp, imageTemp->GetLargestPossibleRegion() );
    outtemp.GoToBegin();
    out.push_back( outtemp );
    }
  for( in.GoToBegin(); !in.IsAtEnd(); ++in )
    {
    itk::VariableLengthVector<PixelType> value = in.Get();
    for( unsigned int i = 0; i < imagePile->GetVectorLength(); i++ )
      {
      out[i].Set( value[i] );
      ++out[i];
      }
    }
  return EXIT_SUCCESS;
}

// Verify if some input parameters are null
bool VectorIsNul( std::vector<double> vec )
{
  bool zero = true;

  for( ::size_t i = 0; i < vec.size(); i++ )
    {
    if( vec[i] != 0 )
      {
      zero = false;
      }
    }
  return zero;
}

// Set resampler's output parameters
template <class ImageType>
void SetOutputParameters( const parameters & list,
                          typename itk::ResampleImageFilter<ImageType, ImageType>
                          ::Pointer & resampler,
                          typename ImageType::Pointer & image
                          )
{
  typedef itk::ImageFileReader<ImageType>                FileReaderType;
  typedef itk::ResampleImageFilter<ImageType, ImageType> ResamplerType;
  typename FileReaderType::Pointer readerReference;
  // is there a reference image to set the size, the orientation,
  // the spacing and the origin of the output image
  if( list.referenceVolume.compare( "" ) )
    {
    readerReference = FileReaderType::New();
    readerReference->SetFileName(list.referenceVolume.c_str() );
    readerReference->Update();
    if( list.space )  // && list.transformationFile.compare( "" ) )
      {
      typename ImageType::PointType originReference;
      typename ImageType::DirectionType directionReference;
      originReference = readerReference->GetOutput()->GetOrigin();
      directionReference = readerReference->GetOutput()->GetDirection();
      originReference[0] = -originReference[0];
      originReference[1] = -originReference[1];
      itk::Matrix<double, 3, 3> ras;
      ras.SetIdentity();
      ras[0][0] = -1;
      ras[1][1] = -1;
      directionReference = ras * directionReference;
      readerReference->GetOutput()->SetOrigin( originReference );
      readerReference->GetOutput()->SetDirection( directionReference );
      }
    }
  resampler->SetOutputParametersFromImage( image );
  typename ResamplerType::OutputImageType::SpacingType m_Spacing;
  typename ResamplerType::OutputImageType::PointType m_Origin;
  typename ResamplerType::OutputImageType::DirectionType m_Direction;
  typename ResamplerType::OutputImageType::SizeType m_Size;
  if( VectorIsNul( list.outputImageSpacing ) )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Spacing = readerReference->GetOutput()->GetSpacing();
      }
    else
      {
      m_Spacing = image->GetSpacing();
      }
    }
  else
    {
    for( int i = 0; i < 3; i++ )
      {
      m_Spacing[i] = list.outputImageSpacing[i];
      }
    }
  if( VectorIsNul( list.outputImageSize ) )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Size
        = readerReference->GetOutput()->GetLargestPossibleRegion().GetSize();
      }
    else
      {
      m_Size = image->GetLargestPossibleRegion().GetSize();
      }
    }
  else
    {
    for( int i = 0; i < 3; i++ )
      {
      m_Size[i] = ( unsigned long ) list.outputImageSize[i];
      }
    }
  if( list.outputImageOrigin.size() == 0 )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Origin = readerReference->GetOutput()->GetOrigin();
      }
    else
      {
      m_Origin = image->GetOrigin();
      }
    }
  else
    {
    for( int i = 0; i < 3; i++ )
      {
      m_Origin[i] = list.outputImageOrigin[i];
      }
    }
  if( VectorIsNul( list.directionMatrix ) )
    {
    if( list.referenceVolume.compare( "" ) )
      {
      m_Direction = readerReference->GetOutput()->GetDirection();
      }
    else
      {
      m_Direction = image->GetDirection();
      }
    }
  else
    {
    for( int i = 0; i < 3; i++ )
      {
      for( int j = 0; j < 3; j++ )
        {
        m_Direction[i][j] = ( double ) list.directionMatrix[i * 3 + j];
        }
      }
    }
  resampler->SetOutputSpacing( m_Spacing );
  resampler->SetSize( m_Size );
  resampler->SetOutputOrigin( m_Origin );
  resampler->SetOutputDirection( m_Direction );
  resampler->SetDefaultPixelValue( static_cast<typename ImageType::PixelType>( list.defaultPixelValue ) );
}

// typedef to avoid a compilation issue with VS7
typedef itk::Transform<double, 3, 3>::Pointer Transform3DPointer;
// Compute the inverse transform
Transform3DPointer InverseTransform( const Transform3DPointer & transform )
{
  itk::Transform<double, 3, 3>::Pointer inverseTransform;
  typedef itk::AffineTransform<double, 3>        AffineTransformType;
  typedef itk::MatrixOffsetTransformBase<double> RotationType;
  try
    {
    std::string transformClassName;
    if ( transform.IsNotNull() )
      {
      transformClassName = transform->GetNameOfClass();
      }
    if( transformClassName.find( "AffineTransform" ) != std::string::npos )  // Rotation around a selected point
      {
      AffineTransformType::Pointer affine = static_cast<AffineTransformType *>( transform.GetPointer() );
      AffineTransformType::Pointer affinetemp = AffineTransformType::New();
      affine->GetInverse( affinetemp );
      inverseTransform = affinetemp;
      }
    else
      {
      if( transformClassName == "MatrixOffsetTransformBase" ||
          transformClassName == "Rigid3DTransform" ||
          transformClassName == "Euler3DTransform" ||
          transformClassName == "CenteredEuler3DTransform" ||
          transformClassName == "QuaternionRigidTransform" ||
          transformClassName == "VersorTransform" ||
          transformClassName == "VersorRigid3DTransform" ||
          transformClassName == "ScaleSkewVersor3DTransform" ||
          transformClassName == "ScaleVersor3DTransform" ||
          transformClassName == "Similarity3DTransform" ||
          transformClassName == "ScaleTransform" ||
          transformClassName == "ScaleLogarithmicTransform"
          ) // if rotation transform
        {
        RotationType::Pointer rigid = static_cast<RotationType *>( transform.GetPointer() );
        RotationType::Pointer rigidtemp = RotationType::New();
        rigid->GetInverse( rigidtemp );
        inverseTransform = rigidtemp;
        }
      else
        {
        inverseTransform = nullptr;
        }
      }
    }
  catch( ... )
    {
    std::cerr << "Exception Detected" << std::endl;
    inverseTransform = nullptr;
    }
  return inverseTransform;
}

// Read Measurement Frame and set it to identity if inverseTransform is not nullptr
itk::Matrix<double, 3, 3>
ReadMeasurementFrame( itk::MetaDataDictionary & dico, const Transform3DPointer & inverseTransform )
{
  itk::Matrix<double, 3, 3> measurementFrame;
  typedef std::vector<std::vector<double> >     DoubleVectorType;
  typedef itk::MetaDataObject<DoubleVectorType> MetaDataDoubleVectorType;
  itk::MetaDataDictionary::ConstIterator itr = dico.Begin();
  itk::MetaDataDictionary::ConstIterator end = dico.End();
  // We look for the measurement frame in the metadatadictionary
  while( itr != end )
    {
    itk::MetaDataObjectBase::Pointer  entry = itr->second;
    MetaDataDoubleVectorType::Pointer entryvalue
      = dynamic_cast<MetaDataDoubleVectorType *>( entry.GetPointer() );
    if( entryvalue )
      {
      int pos = itr->first.find( "NRRD_measurement frame" );
      if( pos != -1 )
        {
        DoubleVectorType tagvalue = entryvalue->GetMetaDataObjectValue();
        for( int i = 0; i < 3; i++ )
          {
          for( int j = 0; j < 3; j++ )
            {
            // we copy the measurement frame
            measurementFrame[i][j] = tagvalue.at( j ).at( i );
            tagvalue.at(j).at(i) = ( i == j ? 1 : 0 );
            }
          }
        // if the transform is invertible, we set the measurement frame to the identity matrix
        // because we are going to apply this measurement frame to the gradient vectors before transforming them
        if( inverseTransform )
          {
          entryvalue->SetMetaDataObjectValue( tagvalue );
          }
        }
      }
    ++itr;
    }

  return measurementFrame;
}

// Transform the gradient vectors
int TransformGradients( itk::MetaDataDictionary & dico,
                        const Transform3DPointer & inverseTransform,
                        const itk::Matrix<double, 3, 3> & measurementFrame
                        )
{
  typedef itk::MetaDataObject<std::string> MetaDataStringType;
  itk::MetaDataDictionary::ConstIterator itr = dico.Begin();
  itk::MetaDataDictionary::ConstIterator end = dico.End();
  bool                                   dtmri = false;
  while( itr != end )
    {
    itk::MetaDataObjectBase::Pointer entry = itr->second;
    MetaDataStringType::Pointer      entryvalue
      = dynamic_cast<MetaDataStringType *>( entry.GetPointer() );
    if( entryvalue )
      {
      // get the gradient directions
      int pos = itr->first.find( "DWMRI_gradient" );
      if( pos != -1 )
        {
        dtmri = true;
        if( inverseTransform )
          {
          std::string            tagvalue = entryvalue->GetMetaDataObjectValue();
          itk::Vector<double, 3> vec;
          itk::Vector<double, 3> transformedVector;
          std::istringstream     iss( tagvalue );
          iss >> vec[0] >> vec[1] >> vec[2];            // we copy the metavalue in an itk::vector
          if( iss.fail() )
            {
            iss.str( tagvalue );
            iss.clear();
            std::string trash;
            iss >> vec[0] >> trash >> vec[1] >> trash >> vec[2]; // in case the separator between the values is
                                                                 // something else than spaces
            if( iss.fail() )                                     // problem reading the gradient values
              {
              std::cerr << "Error reading a DWMRI gradient value" << std::endl;
              }
            }
          // gradient null
          if( vec.GetNorm() <= .00001 )
            {
            transformedVector = vec;
            }
          else
            {
            // We transform the gradient vector with the measurement frame and then apply the inverse transform
            transformedVector = inverseTransform->TransformVector( measurementFrame * vec );
            }
          std::ostringstream oss;
          // write the new gradient values (after transformation) in the metadatadictionary
          oss << transformedVector[0] << "  "
              << transformedVector[1] << "  "
              << transformedVector[2];
          entryvalue->SetMetaDataObjectValue( oss.str() );
          }
        else    // if the image is a DWMRI we exit the loop
          {
          break;
          }
        }
      }
    ++itr;
    }

  // print an error message if the transform is not invertible and if the image is a DWMRI
  if( !inverseTransform && dtmri )
    {
    std::cerr
    <<
    "The gradient transformation is not handle correctly with the current \
                    transformation.\nThe gradient direction of the output image is probably wrong"
    << std::endl;
    return 1;
    }
  return 0;
}

int CheckDWMRI( itk::MetaDataDictionary & dico,
                const Transform3DPointer & transform
                )
{
  // Inverse the transform if possible
  itk::Transform<double, 3, 3>::Pointer inverseTransform;
  inverseTransform = InverseTransform( transform );
  // if the transform is not invertible, we continue to see if the image is a DWMRI
  itk::Matrix<double, 3, 3> measurementFrame;
  measurementFrame = ReadMeasurementFrame( dico, inverseTransform );
  // even if the transform is not invertible, we still go through the metadatadictionary to check if the image is a
  // DWMRI
  // if the image is not, everything is fine. If the image is, we print an error message. The gradient direction of the
  // transform image is not going to be correct
  if( !TransformGradients( dico, inverseTransform, measurementFrame ) )
    {
    return 0;
    }
  else
    {
    return 1;
    }
}

template <class ImageType>
void RASLPS( typename ImageType::Pointer image )
{
  typename ImageType::PointType m_Origin;
  typename ImageType::DirectionType m_Direction;
  m_Origin = image->GetOrigin();
  m_Direction = image->GetDirection();
  m_Origin[0] = -m_Origin[0];
  m_Origin[1] = -m_Origin[1];
  itk::Matrix<double, 3, 3> ras;
  ras.SetIdentity();
  ras[0][0] = -1;
  ras[1][1] = -1;
  m_Direction = ras * m_Direction;
  image->SetOrigin( m_Origin );
  image->SetDirection( m_Direction );
}

// Check the selected interpolator. Creates and returns an object of that type
template <class ImageType>
typename itk::InterpolateImageFunction<ImageType, double>::Pointer
SetInterpolator( const parameters & list )
{
  typedef itk::ConstantBoundaryCondition<ImageType>                       BoundaryCondition;
  typedef itk::InterpolateImageFunction<ImageType, double>                InterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double> NearestNeighborInterpolateType;
  typedef itk::LinearInterpolateImageFunction<ImageType, double>          LinearInterpolateType;
  typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> BSplineInterpolateFunction;
  typename InterpolatorType::Pointer interpol;
  if( !list.interpolationType.compare( "linear" ) )
    {
    typename LinearInterpolateType::Pointer linearinterpolator = LinearInterpolateType::New();
    interpol = linearinterpolator;
    }
  else if( !list.interpolationType.compare( "nn" ) )
    {
    typename NearestNeighborInterpolateType::Pointer interpolator = NearestNeighborInterpolateType::New();
    interpol = interpolator;
    }
  else if( !list.interpolationType.compare( "ws" ) )
    {
    if( !list.windowFunction.compare( "h" ) )
      {
      typedef itk::Function::HammingWindowFunction<RADIUS> windowFunction;
      typedef itk::WindowedSincInterpolateImageFunction<ImageType,
                                                        RADIUS,
                                                        windowFunction,
                                                        BoundaryCondition,
                                                        double> WindowedSincInterpolateImageFunctionType;
      interpol = WindowedSincInterpolateImageFunctionType::New();
      }
    else if( !list.windowFunction.compare( "c" ) )
      {
      typedef itk::Function::CosineWindowFunction<RADIUS> windowFunction;
      typedef itk::WindowedSincInterpolateImageFunction<ImageType,
                                                        RADIUS,
                                                        windowFunction,
                                                        BoundaryCondition,
                                                        double> WindowedSincInterpolateImageFunctionType;
      interpol = WindowedSincInterpolateImageFunctionType::New();
      }
    else if( !list.windowFunction.compare( "w" ) )
      {
      typedef itk::Function::WelchWindowFunction<RADIUS> windowFunction;
      typedef itk::WindowedSincInterpolateImageFunction<ImageType,
                                                        RADIUS,
                                                        windowFunction,
                                                        BoundaryCondition,
                                                        double> WindowedSincInterpolateImageFunctionType;
      interpol = WindowedSincInterpolateImageFunctionType::New();
      }
    else if( !list.windowFunction.compare( "l" ) )
      {
      typedef itk::Function::LanczosWindowFunction<RADIUS> windowFunction;
      typedef itk::WindowedSincInterpolateImageFunction<ImageType,
                                                        RADIUS,
                                                        windowFunction,
                                                        BoundaryCondition,
                                                        double> WindowedSincInterpolateImageFunctionType;
      interpol = WindowedSincInterpolateImageFunctionType::New();
      }
    else if( !list.windowFunction.compare( "b" ) )
      {
      typedef itk::Function::BlackmanWindowFunction<RADIUS> windowFunction;
      typedef itk::WindowedSincInterpolateImageFunction<ImageType,
                                                        RADIUS,
                                                        windowFunction,
                                                        BoundaryCondition,
                                                        double> WindowedSincInterpolateImageFunctionType;
      interpol = WindowedSincInterpolateImageFunctionType::New();
      }
    }
  else if( !list.interpolationType.compare( "bs" ) )
    {
    typename BSplineInterpolateFunction::Pointer bSplineInterpolator = BSplineInterpolateFunction::New();
    bSplineInterpolator->SetSplineOrder( list.splineOrder );
    interpol = bSplineInterpolator;
    }
  return interpol;
}

template <class PixelType>
int Rotate( parameters & list )
{
  typedef itk::Image<PixelType, 3>                         ImageType;
  typedef itk::InterpolateImageFunction<ImageType, double> InterpolatorType;
  typedef itk::ResampleImageFilter<ImageType, ImageType>   ResampleType;
  typedef itk::Transform<double, 3, 3>                     TransformType;
  typedef itk::VectorImage<PixelType, 3>                   VectorImageType;
  typename ImageType::Pointer image;
  std::vector<typename ImageType::Pointer> vectorOfImage;
  itk::MetaDataDictionary                  dico;
  try
    {
    // open image file
    typename itk::ImageFileReader<VectorImageType>::Pointer reader;
    reader = itk::ImageFileReader<VectorImageType>::New();
    reader->SetFileName( list.inputVolume.c_str() );
    reader->Update();
    if( list.space )  // && list.transformationFile.compare( "" ) )
      {
      RASLPS<VectorImageType>( reader->GetOutput() );
      }
    // Save metadata dictionary
    dico = reader->GetOutput()->GetMetaDataDictionary();
    // Separate the vector image into a vector of images
    SeparateImages<PixelType>( reader->GetOutput(), vectorOfImage );
    }
  catch( itk::ExceptionObject &exception )
    {
    std::cerr << exception << std::endl;
    return EXIT_FAILURE;
    }
  // Set interpolator
  typename InterpolatorType::Pointer interpol;
  interpol = SetInterpolator<ImageType>( list );
  // Create resampler and initialize its output parameters
  typename ResampleType::Pointer resample = ResampleType::New();
  SetOutputParameters<ImageType>( list, resample, vectorOfImage[0] );
  TransformType::Pointer transform;
  // Load transforms and compute a merged transform
  transform = SetAllTransform<ImageType>( list, resample, vectorOfImage[0] );
  if( !transform )
    {
    return EXIT_FAILURE;
    }
  resample->SetTransform( transform );
  resample->SetInterpolator( interpol );
  std::vector<typename ImageType::Pointer> vectorOutputImage;
  // Resample all the images separately
  for( ::size_t idx = 0; idx < vectorOfImage.size(); idx++ )
    {
    resample->SetInput( vectorOfImage[idx] );
    resample->Update();
    vectorOutputImage.push_back( resample->GetOutput() );
    vectorOutputImage[idx]->DisconnectPipeline();
    }
  typename itk::VectorImage<PixelType, 3>::Pointer outputImage;
  outputImage = itk::VectorImage<PixelType, 3>::New();
  AddImage<PixelType>( outputImage, vectorOutputImage );
  vectorOutputImage.clear();
  // If necessary, transform gradient vectors with the loaded transformations
  int dwmriProblem = CheckDWMRI( dico, transform );
  if( list.space ) // && list.transformationFile.compare( "" ) )
    {
    RASLPS<VectorImageType>( outputImage );
    }
  outputImage->SetMetaDataDictionary( dico );
  // Save transformed image
  typedef itk::ImageFileWriter<VectorImageType> WriterType;
  try
    {
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput( outputImage );
    writer->SetFileName( list.outputVolume.c_str() );
    writer->UseCompressionOn();
    writer->Update();
    }
  catch( itk::ExceptionObject &exception )
    {
    std::cerr << exception << std::endl;
    return EXIT_FAILURE;
    }
  // If there was a problem while computing the transformed dwmri, exits with an error
  if( dwmriProblem )
    {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  parameters list;
  list.numberOfThread = numberOfThread;
  list.interpolationType = interpolationType;
  list.transformType = transformType;
  list.transformMatrix = transformMatrix;
  list.inputVolume = inputVolume;
  list.referenceVolume = referenceVolume;
  list.outputVolume = outputVolume;
  list.rotationPoint = rotationPoint;
  list.transformationFile = transformationFile;
  list.inverseITKTransformation = inverseITKTransformation;
  list.windowFunction = windowFunction;
  list.splineOrder = splineOrder;
  list.space = space;
  list.centeredTransform = centeredTransform;
  list.outputImageSpacing = outputImageSpacing;
  list.outputImageSize = outputImageSize;
  list.outputImageOrigin = outputImageOrigin;
  list.directionMatrix = directionMatrix;
  list.deffield = deffield;
  list.typeOfField = typeOfField;
  list.defaultPixelValue = defaultPixelValue;
  list.imageCenter = imageCenter;
  list.transformsOrder = transformsOrder;
  list.notbulk = notbulk;
  // verify if all the vector parameters have the good length
  if( list.outputImageSpacing.size() != 3 || list.outputImageSize.size() != 3
      || ( list.outputImageOrigin.size() != 3
           && list.outputImageOrigin.size() != 0 )
      || list.directionMatrix.size() != 9 || rotationPoint.size() != 3
      || list.transformMatrix.size() != 12 )
    {
    std::cerr << "Argument(s) having wrong size" << std::endl;
    return EXIT_FAILURE;
    }

  // Thin-plate spline transform is not registered in transform factory by default in recent
  // ITK versions, so we need to register it manually.
  typedef itk::ThinPlateSplineKernelTransform<float,3> ThinPlateSplineTransformFloatType;
  typedef itk::ThinPlateSplineKernelTransform<double,3> ThinPlateSplineTransformDoubleType;
  itk::TransformFactory<ThinPlateSplineTransformFloatType>::RegisterTransform();
  itk::TransformFactory<ThinPlateSplineTransformDoubleType>::RegisterTransform(); 

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;
  GetImageType( list.inputVolume, pixelType, componentType );

  // templated over the input image voxel type
  switch( componentType )
    {
    case itk::ImageIOBase::UCHAR:
      return Rotate<unsigned char>( list );
      break;
    case itk::ImageIOBase::CHAR:
      return Rotate<char>( list );
      break;
    case itk::ImageIOBase::USHORT:
      return Rotate<unsigned short>( list );
      break;
    case itk::ImageIOBase::SHORT:
      return Rotate<short>( list );
      break;
    case itk::ImageIOBase::UINT:
      return Rotate<unsigned int>( list );
      break;
    case itk::ImageIOBase::INT:
      return Rotate<int>( list );
      break;
    case itk::ImageIOBase::ULONG:
      return Rotate<unsigned long>( list );
      break;
    case itk::ImageIOBase::LONG:
      return Rotate<long>( list );
      break;
    case itk::ImageIOBase::FLOAT:
      return Rotate<float>( list );
      break;
    case itk::ImageIOBase::DOUBLE:
      return Rotate<double>( list );
      break;
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    default:
      std::cerr << "unknown component type" << std::endl;
      break;
    }
  return EXIT_FAILURE;
}
