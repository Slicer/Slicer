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
#include "itkDiffusionTensor3DNonRigidTransform.h"
#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.h"
#include "itkDiffusionTensor3DLinearInterpolateFunction.h"
#include "itkDiffusionTensor3DWindowedSincInterpolateImageFunction.h"
#include "itkDiffusionTensor3DBSplineInterpolateImageFunction.h"
#include <itkTransformFileReader.h>
#include "ResampleDTIVolumeCLP.h"
#include "itkDiffusionTensor3DZeroCorrection.h"
#include "itkDiffusionTensor3DAbsCorrection.h"
#include "itkDiffusionTensor3DNearestCorrection.h"
#include "itkDiffusionTensor3DRead.h"
#include "itkDiffusionTensor3DWrite.h"
#include "dtiprocessFiles/deformationfieldio.h"
#include "itkWarpTransform3D.h"
#include "itkTransformDeformationFieldFilter.h"
#include <itkVectorResampleImageFilter.h>
#include <itkBSplineDeformableTransform.h>
#include <itkThinPlateSplineKernelTransform.h>
#include <itkTransformFactory.h>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

#define RADIUS 3

// Structure of the command lign parameters
struct parameters
  {
  std::vector<double> outputImageSpacing;
  std::vector<double> outputImageSize;
  std::vector<float> outputImageOrigin;
  int numberOfThread;
  std::string interpolationType;
  std::string transformType;
  std::vector<double> directionMatrix;
  std::vector<double> transformMatrix;
  std::string inputVolume;
  std::string outputVolume;
  std::vector<float> rotationPoint;
  std::string transformationFile;
  bool inverseITKTransformation;
  std::string windowFunction;
  unsigned int splineOrder;
  std::string referenceVolume;
  bool space;
  bool centeredTransform;
  std::string ppd;
  std::string correction;
  std::string deffield;
  std::string typeOfField;
  double defaultPixelValue;
  std::string imageCenter;
  std::string transformsOrder;
  bool notbulk;
  bool noMeasurementFrame ;
  };

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

// What pixeltype is the image
void GetImageType( std::string fileName,
                   itk::ImageIOBase::IOPixelType & pixelType,
                   itk::ImageIOBase::IOComponentType & componentType )
{
  typedef itk::Image<unsigned char, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
    itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName( fileName.c_str() );
  imageReader->UpdateOutputInformation();
  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
}

// Select which interpolator will be used
template <class PixelType>
typename itk::DiffusionTensor3DInterpolateImageFunction<PixelType>
::Pointer
InterpolationTypeFct( std::string interpolationType,
                      std::string windowFunction,
                      unsigned int splineOrder,
                      int numberOfThreads
                      )
{
  typedef itk::DiffusionTensor3DInterpolateImageFunction<PixelType>
  InterpolatorType;
  typedef itk::DiffusionTensor3DNearestNeighborInterpolateFunction<PixelType>
  NearestNeighborhoodInterpolatorType;
  typedef itk::DiffusionTensor3DLinearInterpolateFunction<PixelType>
  LinearInterpolatorType;
  typedef itk::ConstantBoundaryCondition<itk::Image<PixelType, 3> >
  BoundaryConditionType;
  typedef itk::Function::HammingWindowFunction<RADIUS>
  HammingwindowFunctionType;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
  <PixelType,
   RADIUS,
   HammingwindowFunctionType,
   BoundaryConditionType> HammingWindowedSincInterpolateFunctionType;
  typedef itk::Function::CosineWindowFunction<RADIUS>
  CosinewindowFunctionType;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
  <PixelType,
   RADIUS,
   CosinewindowFunctionType,
   BoundaryConditionType> CosineWindowedSincInterpolateFunctionType;
  typedef itk::Function::WelchWindowFunction<RADIUS>
  WelchwindowFunctionType;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
  <PixelType,
   RADIUS,
   WelchwindowFunctionType,
   BoundaryConditionType> WelchWindowedSincInterpolateFunctionType;
  typedef itk::Function::LanczosWindowFunction<RADIUS>
  LanczoswindowFunctionType;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
  <PixelType,
   RADIUS,
   LanczoswindowFunctionType,
   BoundaryConditionType> LanczosWindowedSincInterpolateFunctionType;
  typedef itk::Function::BlackmanWindowFunction<RADIUS>
  BlackmanwindowFunctionType;
  typedef itk::DiffusionTensor3DWindowedSincInterpolateImageFunction
  <PixelType,
   RADIUS,
   BlackmanwindowFunctionType,
   BoundaryConditionType> BlackmanWindowedSincInterpolateFunctionType;
  typedef itk::DiffusionTensor3DBSplineInterpolateImageFunction<PixelType>
  BSplineInterpolateImageFunctionType;
  typedef typename InterpolatorType::Pointer InterpolatorTypePointer;
  InterpolatorTypePointer interpol;
  if( !interpolationType.compare( "nn" ) ) // nearest neighborhood
    {
    interpol = NearestNeighborhoodInterpolatorType::New();
    }
  else if( !interpolationType.compare( "linear" ) ) // linear
    {
    typename LinearInterpolatorType::Pointer linearInterpol = LinearInterpolatorType::New();
    linearInterpol->SetNumberOfThreads( numberOfThreads );
    interpol = linearInterpol;
    }
  else if( !interpolationType.compare( "ws" ) ) // windowed sinc
    {
    typedef itk::DiffusionTensor3DInterpolateImageFunctionReimplementation<PixelType>
    DiffusionTensor3DInterpolateImageFunctionReimplementation;
    typename DiffusionTensor3DInterpolateImageFunctionReimplementation::Pointer itkReimplementationInterpol;
    if( !windowFunction.compare( "h" ) ) // Hamming window
      {
      itkReimplementationInterpol = HammingWindowedSincInterpolateFunctionType::New();
      }
    else if( !windowFunction.compare( "c" ) ) // cosine window
      {
      itkReimplementationInterpol = CosineWindowedSincInterpolateFunctionType::New();
      }
    else if( !windowFunction.compare( "w" ) ) // Welch window
      {
      itkReimplementationInterpol = WelchWindowedSincInterpolateFunctionType::New();
      }
    else if( !windowFunction.compare( "l" ) ) // Lanczos window
      {
      itkReimplementationInterpol = LanczosWindowedSincInterpolateFunctionType::New();
      }
    else if( !windowFunction.compare( "b" ) ) // Blackman window
      {
      itkReimplementationInterpol = BlackmanWindowedSincInterpolateFunctionType::New();
      }
    itkReimplementationInterpol->SetNumberOfThreads( numberOfThreads );
    interpol = itkReimplementationInterpol;
    }
  else if( !interpolationType.compare( "bs" ) ) // BSpline interpolation
    {
    typename BSplineInterpolateImageFunctionType::Pointer
    bSplineInterpolator = BSplineInterpolateImageFunctionType::New();
    bSplineInterpolator->SetSplineOrder( splineOrder );
    bSplineInterpolator->SetNumberOfThreads( numberOfThreads );
    interpol = bSplineInterpolator;
    }
  return interpol;
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

template <class PixelType>
itk::Point<double>
ImageCenter( const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
             ::Pointer & image
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

template <class PixelType>
itk::Matrix<double, 4, 4>
ComputeTransformMatrix( const parameters & list,
                        const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
                        ::Pointer & image,
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
      center = ImageCenter<PixelType>( image );
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
    center[i] = list.rotationPoint[i];
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

template <class PixelType>
typename itk::DiffusionTensor3DAffineTransform<PixelType>::Pointer
FSOrPPD( const std::string & ppd, itk::Matrix<double, 4, 4> *matrix = nullptr )
{
  typedef itk::DiffusionTensor3DFSAffineTransform<PixelType>
  FSAffineTransformType;
  typedef itk::DiffusionTensor3DPPDAffineTransform<PixelType>
  PPDAffineTransformType;
  typename itk::DiffusionTensor3DAffineTransform<PixelType>::Pointer affine;
  if( !ppd.compare( "FS" ) )
    {
    typename FSAffineTransformType::Pointer affineFS = FSAffineTransformType::New();
    affine = affineFS;
    }
  // Preservation of principal direction
  else
    {
    typename PPDAffineTransformType::Pointer affineppd
      = PPDAffineTransformType::New();
    affine = affineppd;
    }
  if( matrix )
    {
    affine->SetMatrix4x4( *matrix );
    }
  return affine;
}

template <class PixelType>
typename itk::DiffusionTensor3DTransform<PixelType>::Pointer
SetUpTransform( const parameters & list,
                const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
                ::Pointer & image,
                const typename itk::DiffusionTensor3DNonRigidTransform<PixelType>::TransformType
                ::Pointer & nonRigidFile,
                bool precisionChecking,
                const itk::Point<double> & outputImageCenter
                )
{
  typedef itk::DiffusionTensor3DTransform<PixelType> TransformType;
  typedef itk::DiffusionTensor3DNonRigidTransform<PixelType>
  NonRigidTransformType;
  typedef typename TransformType::Pointer TransformTypePointer;
  itk::Matrix<double, 4, 4> transformMatrix4x4;
  typedef itk::DiffusionTensor3DRigidTransform<PixelType> RigidTransformType;
  TransformTypePointer transform;
  if( list.transformType.compare( "nr" ) ) // if rigid or affine transform
    {
    transformMatrix4x4 = ComputeTransformMatrix<PixelType>( list, image, outputImageCenter );
    // Rigid Transform (rotation+translation)
    if( !list.transformType.compare( "rt" ) )
      {
      typename RigidTransformType::Pointer rotation = RigidTransformType::New();
      rotation->SetPrecisionChecking( precisionChecking );
      rotation->SetMatrix4x4( transformMatrix4x4 );
      transform = rotation;
      }
    // Affine transform
    else if( !list.transformType.compare( "a" ) )
      {
      transform = FSOrPPD<PixelType>( list.ppd, &transformMatrix4x4 );
      }
    }
  else
    {
    typename NonRigidTransformType::Pointer nonRigid
      = NonRigidTransformType::New();
    nonRigid->SetTransform( nonRigidFile );
    typename itk::DiffusionTensor3DAffineTransform<PixelType>::Pointer affine;
    affine = FSOrPPD<PixelType>( list.ppd );
    nonRigid->SetAffineTransformType( affine );
    transform = nonRigid;
    }
  return transform;
}

// Set the transformation
template <class PixelType>
typename itk::DiffusionTensor3DTransform<PixelType>::Pointer
SetTransformAndOrder( parameters & list,
                      const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
                      ::Pointer & image,
                      typename itk::DiffusionTensor3DNonRigidTransform<PixelType>
                      ::TransformType::Pointer & transform,
                      const itk::Point<double> & outputImageCenter
                      )
{
  typedef itk::DiffusionTensor3DNonRigidTransform<PixelType>
  NonRigidTransformType;
  typedef itk::DiffusionTensor3DRigidTransform<PixelType> RigidTransformType;
  typedef itk::DiffusionTensor3DFSAffineTransform<PixelType>
  FSAffineTransformType;
  typename NonRigidTransformType::TransformType::Pointer nonRigidFile;
  bool precisionChecking = true;
  // Get transformation matrix from the given file
  if( list.transformationFile.compare( "" ) )
    {
    std::string transformClassName = transform->GetNameOfClass();
    list.transformMatrix.resize( 0 );
    list.rotationPoint.resize( 0 );
    typename itk::MatrixOffsetTransformBase<double, 3, 3>
    ::Pointer matrixOffsetTransform;
    if( transformClassName.find("AffineTransform") != std::string::npos ) // if affine transform
      {
      matrixOffsetTransform = static_cast<
          typename FSAffineTransformType::Superclass::AffineTransformType *>
        ( transform.GetPointer() );
      list.transformType.assign( "a" );
      SetListFromTransform<double>( matrixOffsetTransform, list );
      }
    else
      {
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
          precisionChecking = false;
          matrixOffsetTransform = static_cast<
              typename RigidTransformType::Rigid3DTransformType *>
            ( transform.GetPointer() );;
          SetListFromTransform<double>( matrixOffsetTransform, list );
          }
        else // if non-rigid
          {
          if ( transformClassName.find("Transform") != std::string::npos )
            { // if non rigid Transform loaded
            list.transformType.assign( "nr" );
            nonRigidFile = static_cast<
                typename NonRigidTransformType::TransformType *>
              ( transform.GetPointer() );
            }
          else // something else
            {
            std::cerr << "Transformation type not yet implemented for tensors"
                      << std::endl;
            return nullptr;
            }
          }
      }
    if( list.transformType.compare( "nr" ) ) // if rigid or affine transform
      {
      // if problem in the number of parameters
      if( list.transformMatrix.size() != 12 || list.rotationPoint.size() != 3 )
        {
        std::cerr << "Error in the file containing the transformation"
                  << std::endl;
        return nullptr;
        }
      }
    }
  return SetUpTransform<PixelType>( list, image, nonRigidFile, precisionChecking, outputImageCenter );
}

// Set the transformation
template <class PixelType>
typename itk::DiffusionTensor3DTransform<PixelType>::Pointer
SetTransform( parameters & list,
              const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
              ::Pointer & image,
              itk::TransformFileReader::Pointer & transformFile,
              const itk::Point<double> & outputImageCenter
              )
{
  typedef typename itk::DiffusionTensor3DNonRigidTransform<PixelType>
  ::TransformType TransformType;
  typename TransformType::Pointer transform;
  typename itk::DiffusionTensor3DTransform<PixelType>::Pointer tensorTransform;
  if( list.transformationFile.compare( "" ) )
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
  tensorTransform = SetTransformAndOrder<PixelType>( list, image, transform, outputImageCenter );
  if( list.transformationFile.compare( "" ) )
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
  return tensorTransform;
}

// Read the transform file and return the number of non-rigid transform.
// If the transform file contain a transform that the program does not
// handle, the function returns -1
template <class PixelType>
int ReadTransform( parameters & list,
                   const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>::Pointer & image,
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
      if( !SetTransform<PixelType>( list, image, transformFile, dummyOutputCenter ) )
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

// Set Output parameters
template <class PixelType>
itk::Matrix< double , 3 , 3 > SetOutputParameters( const parameters & list,
                          typename itk::DiffusionTensor3DResample<PixelType, PixelType>
                          ::Pointer & resampler,
                          const typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
                          ::Pointer & image
                          )
{
  typedef itk::Image<unsigned char, 3>                         ImageType;
  typedef itk::ImageFileReader<ImageType>                      ReaderType;
  typedef itk::DiffusionTensor3DResample<PixelType, PixelType> ResamplerType;
  typedef typename ReaderType::Pointer                         ReaderTypePointer;
  ReaderTypePointer readerReference;
  // is there a reference image to set the size, the orientation,
  // the spacing and the origin of the output image?
  if( list.referenceVolume.compare( "" ) )
    {
    readerReference = ReaderType::New();
    readerReference->SetFileName( list.referenceVolume.c_str() );
//    readerReference->Update() ;
    readerReference->UpdateOutputInformation();
    if( list.space ) // && list.transformationFile.compare( "" ) )
      {
      typename ReaderType::OutputImageType::PointType originReference;
      typename ReaderType::OutputImageType::DirectionType directionReference;
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
  resampler->SetOutputParametersFromImage( image ); // is probably useless but doesn't cost much
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
  if( VectorIsNul( list.outputImageSize) )
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
  resampler->SetOutputSize( m_Size );
  resampler->SetOutputOrigin( m_Origin );
  resampler->SetOutputDirection( m_Direction );
  return m_Direction ;
}

template <class PixelType>
void RASLPS( typename itk::Image<itk::DiffusionTensor3D<PixelType>, 3>::Pointer & image )
{
  typename itk::Image<PixelType, 3>::PointType m_Origin;
  typename itk::Image<PixelType, 3>::DirectionType m_Direction;
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

// Check all input parameters and transform the input image
// Template over the pixeltype of the input image
template <class PixelType>
int Do( parameters list )
{
  typedef itk::Image<itk::DiffusionTensor3D<PixelType>, 3>
  InputImageType;
  typename InputImageType::Pointer image;
  typedef itk::DiffusionTensor3DWrite<PixelType> WriterType;
  typedef typename WriterType::Pointer           WriterTypePointer;
  WriterTypePointer         writer = WriterType::New();
  itk::Matrix<double, 3, 3> measurementFrame;
  bool hasMeasurementFrame ;
  try
    {
    typedef itk::DiffusionTensor3DRead<PixelType> ReaderType;
    typedef typename ReaderType::Pointer          ReaderTypePointer;
    ReaderTypePointer reader = ReaderType::New();
    // Read input volume
    if( list.numberOfThread )
      {
      reader->SetNumberOfThreads( list.numberOfThread );
      }
    reader->Update( list.inputVolume.c_str() );
    image = reader->GetOutput();
    writer->SetMetaDataDictionary( reader->GetMetaDataDictionary() );
    measurementFrame = reader->GetMeasurementFrame();
    hasMeasurementFrame = reader->GetHasMeasurementFrame() ;
    if( !hasMeasurementFrame || list.noMeasurementFrame )
    {
      measurementFrame = reader->GetOutput()->GetDirection() ;
    }
    }
  catch( itk::ExceptionObject & Except )
    {
    std::cerr << "Reading input image: Exception caught!"
              << std::endl;
    std::cerr << Except << std::endl;
    return EXIT_FAILURE;
    }
  if( list.space )  // &&  list.transformationFile.compare( "" ) )
    {
    RASLPS<PixelType>( image );
    }
  typedef itk::DiffusionTensor3DInterpolateImageFunction<PixelType>
  InterpolatorType;
  typedef typename InterpolatorType::Pointer InterpolatorTypePointer;
  InterpolatorTypePointer interpol;
  // Select interpolation type
  interpol = InterpolationTypeFct<PixelType>( list.interpolationType,
                                              list.windowFunction,
                                              list.splineOrder,
                                              list.numberOfThread
                                              );

  // Select the transformation
  typedef itk::TransformFileReader::Pointer TransformReaderPointer;
  TransformReaderPointer transformFile;
  int                    nonRigidTransforms;
  nonRigidTransforms = ReadTransform<PixelType>( list, image, transformFile );
  if( nonRigidTransforms < 0 )  // The transform file contains a transform that is not handled by resampleDTI, it exits.
    {
    return EXIT_FAILURE;
    }
  // if the transform is in RAS space coordinate, add a transform to transform the image into that space
  typedef itk::DiffusionTensor3DTransform<PixelType>           TransformType;
  typedef typename TransformType::Pointer                      TransformTypePointer;
  typedef itk::DiffusionTensor3DResample<PixelType, PixelType> ResamplerType;
  typedef itk::DiffusionTensor3DNonRigidTransform<PixelType>   NonRigidTransformType;
  typedef typename ResamplerType::Pointer                      ResamplerTypePointer;
  typename InputImageType::Pointer dummyOutputImage = InputImageType::New();
    {                                                          // local for memory management
    ResamplerTypePointer tempResampler = ResamplerType::New(); // this resampler is just used to be able to use
                                                               // SetOutputParameters and get the origin, size,
                                                               // direction and spacing of the output image
    SetOutputParameters<PixelType>( list, tempResampler, image );
    dummyOutputImage->SetDirection( tempResampler->GetOutputDirection() );
    dummyOutputImage->SetSpacing( tempResampler->GetOutputSpacing() );
    dummyOutputImage->SetOrigin( tempResampler->GetOutputOrigin() );
    dummyOutputImage->SetRegions( tempResampler->GetOutputSize() );
    }
  itk::Point<double> outputImageCenter = ImageCenter<PixelType>( dummyOutputImage );
  // Check if displacement or h-field transform
  typename DeformationImageType::Pointer fieldPointer;
  if( list.deffield.compare( "" ) )
    {
    // set if the field is a displacement or a H- field
    DeformationFieldType dftype = HField;
    if( !list.typeOfField.compare( "displacement" ) )
      {
      dftype = Displacement;
      }
    // reads deformation field and if it is a h-field, it transforms it to a displacement field
    fieldPointer = readDeformationField( list.deffield, dftype );
    nonRigidTransforms++;
    }
  // If more than one transform or if hfield, add all transforms and compute the deformation field
  TransformTypePointer transform;
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
                                dummyOutputImage->GetOrigin(),
                                dummyOutputImage->GetSpacing(),
                                dummyOutputImage->GetLargestPossibleRegion().GetSize(),
                                dummyOutputImage->GetDirection()
                                );
      }
    else  // if no deformation field was loaded, we create an empty one
      {
      field = DeformationImageType::New();
      field->SetSpacing( dummyOutputImage->GetSpacing() );
      field->SetOrigin( dummyOutputImage->GetOrigin() );
      field->SetRegions( dummyOutputImage->GetLargestPossibleRegion().GetSize() );
      field->SetDirection( dummyOutputImage->GetDirection() );
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
      transform = SetTransform<PixelType>( list, image, transformFile, outputImageCenter );
      // check if there is a bspline transform and a bulk transform with it
      if( !list.notbulk && transform->GetTransform()->GetTransformTypeAsString() ==
          "BSplineDeformableTransform_double_3_3"  && transformFile->GetTransformList()->size() )                                                                            //
                                                                                                                                                                             // Check
                                                                                                                                                                             // if
                                                                                                                                                                             // transform
                                                                                                                                                                             // file
                                                                                                                                                                             // contains
                                                                                                                                                                             // a
                                                                                                                                                                             // BSpline
        {
        // transform = SetTransform< PixelType > ( list , image , transformFile , outputImageCenter ) ;
        // order=3 for the BSpline seems to be standard among tools in Slicer and BRAINTools
        if( std::string(transform->GetTransform()->GetNameOfClass()) == "BSplineDeformableTransform" )
          {
          typedef itk::BSplineDeformableTransform<double, 3, 3> BSplineDeformableTransformType;
          BSplineDeformableTransformType::Pointer BSplineTransform;
          BSplineTransform = static_cast<BSplineDeformableTransformType *>(transform->GetTransform().GetPointer() );
          typename TransformType::Pointer bulkTransform;
          bulkTransform = SetTransform<PixelType>( list, image, transformFile, outputImageCenter  );
          BSplineTransform->SetBulkTransform( bulkTransform->GetTransform() );
          }
        }
      if( list.numberOfThread )
        {
        transformDeformationFieldFilter->SetNumberOfThreads( list.numberOfThread );
        }
      transformDeformationFieldFilter->SetInput( field );
      transformDeformationFieldFilter->SetTransform( transform->GetTransform() );
      transformDeformationFieldFilter->Update();
      field = transformDeformationFieldFilter->GetOutput();
      field->DisconnectPipeline();
      }

    // Create the DTI transform
    warpTransform->SetDeformationField( field );
    typename NonRigidTransformType::Pointer nonRigid = NonRigidTransformType::New();
    nonRigid->SetTransform( warpTransform.GetPointer() );
    typename itk::DiffusionTensor3DAffineTransform<PixelType>::Pointer affine;
    affine = FSOrPPD<PixelType>( list.ppd );
    nonRigid->SetAffineTransformType( affine );
    transform = nonRigid;
    }
  // multiple rigid/affine transforms: concatenate them
  else if( list.transformationFile.compare( "" ) && transformFile->GetTransformList()->size() > 1 )
    {
    typedef itk::DiffusionTensor3DMatrix3x3Transform<PixelType> MatrixTransformType;
    itk::Matrix<double, 4, 4> composedMatrix;
    composedMatrix.SetIdentity();
    itk::Matrix<double, 4, 4> tempMatrix;
    itk::Matrix<double, 3, 3> matrix;
    itk::Vector<double, 3>    vector;
    composedMatrix.SetIdentity();

    do
      {
      transform = SetTransform<PixelType>( list, image, transformFile, outputImageCenter );
      typename MatrixTransformType::Pointer localTransform;
      std::string transformClassName;
      if ( transform.IsNotNull() )
        {
        transformClassName = transform->GetNameOfClass();
        }
      if( transformClassName.find("DiffusionTensor3D") == std::string::npos )  // should never happen, just for security
        {
        std::cerr << "An affine or rigid transform was not convertible to DiffusionTensor3DMatrix3x3Transform"
                  << std::endl;
        return EXIT_FAILURE;
        }
      localTransform = static_cast<MatrixTransformType *>(transform.GetPointer() );
      matrix = localTransform->GetMatrix3x3();
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

    // Finite Strain
    transform = FSOrPPD<PixelType>( list.ppd, &composedMatrix );
    }
  else
    {
    // only one transform, just load it
    transform = SetTransform<PixelType>( list, image, transformFile, outputImageCenter );
    }
  double defaultPixelValue = list.defaultPixelValue;
  // start transform

    {                                                 // local for memory management: the input image should not stay in
                                                      // memory after we exit this portion of code
    ResamplerTypePointer resampler = ResamplerType::New();
    resampler->SetInput( image );
    if( list.numberOfThread )
      {
      resampler->SetNumberOfThreads( list.numberOfThread );
      }
    resampler->SetInterpolator( interpol );
    itk::Matrix<double, 3, 3> outputImageDirection ;
    outputImageDirection = SetOutputParameters<PixelType>( list, resampler, image );
    if( !hasMeasurementFrame || list.noMeasurementFrame )
    {
      itk::Matrix<double, 3, 3> transposeOutputDirection ( outputImageDirection.GetTranspose() ) ;
      measurementFrame = transposeOutputDirection * measurementFrame ;
    }
    resampler->SetDefaultPixelValue( static_cast<PixelType>( defaultPixelValue ) );    // Could be set directly in the
                                                                                       // interpolator, but we keep
                                                                                       // ITKResampleImageFilter design
    transform->SetMeasurementFrame( measurementFrame );
    resampler->SetTransform( transform );
    // Compute the resampled image
    resampler->Update();
    writer->SetMeasurementFrame( resampler->GetOutputMeasurementFrame() );   // measurement frame set to Identity after
                                                                             // resampling
    image = resampler->GetOutput();
    }
  // Tensors Corrections
  if( !list.correction.compare( "zero" ) )
    {
    typedef itk::DiffusionTensor3DZeroCorrectionFilter<InputImageType, InputImageType> ZeroCorrection;
    typename ZeroCorrection::Pointer zeroFilter;
    zeroFilter = ZeroCorrection::New();
    zeroFilter->SetInput( image );
    if( list.numberOfThread )
      {
      zeroFilter->SetNumberOfThreads( list.numberOfThread );
      }
    zeroFilter->Update();
    image = zeroFilter->GetOutput();
    }
  else if( !list.correction.compare( "abs" ) )
    {
    typedef itk::DiffusionTensor3DAbsCorrectionFilter<InputImageType, InputImageType> AbsCorrection;
    typename AbsCorrection::Pointer absFilter;
    absFilter = AbsCorrection::New();
    absFilter->SetInput( image );
    if( list.numberOfThread )
      {
      absFilter->SetNumberOfThreads( list.numberOfThread );
      }
    absFilter->Update();
    image = absFilter->GetOutput();
    }
  else if( !list.correction.compare( "nearest" ) )
    {
    typedef itk::DiffusionTensor3DNearestCorrectionFilter<InputImageType, InputImageType> NearestCorrection;
    typename NearestCorrection::Pointer nearestFilter;
    nearestFilter = NearestCorrection::New();
    nearestFilter->SetInput( image );
    if( list.numberOfThread )
      {
      nearestFilter->SetNumberOfThreads( list.numberOfThread );
      }
    nearestFilter->Update();
    image = nearestFilter->GetOutput();
    }
  // Save result
  if( list.space )  // &&  list.transformationFile.compare( "" ) )
    {
    RASLPS<PixelType>( image );
    }
  if( list.numberOfThread )
    {
    writer->SetNumberOfThreads( list.numberOfThread );
    }
  writer->SetInput( image );
  // Save output image
  try
    {
    writer->Update( list.outputVolume.c_str() );
    }
  catch( itk::ExceptionObject & Except )
    {
    std::cerr << "Writing output image: Exception caught!"
              << std::endl;
    std::cerr << Except << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  parameters list;

  // Copy all the input parameters into a structure,
  // to pass everything to the template function
  list.outputImageSpacing = outputImageSpacing;
  list.outputImageSize = outputImageSize;
  list.outputImageOrigin = outputImageOrigin;
  list.numberOfThread = numberOfThread;
  list.interpolationType = interpolationType;
  list.transformType = transformType;
  list.directionMatrix = directionMatrix;
  list.transformMatrix = transformMatrix;
  list.inputVolume = inputVolume;
  list.outputVolume = outputVolume;
  list.rotationPoint = rotationPoint;
  list.transformationFile = transformationFile;
  list.inverseITKTransformation = inverseITKTransformation;
  list.windowFunction = windowFunction;
  list.splineOrder = splineOrder;
  list.referenceVolume = referenceVolume;
  list.space = space;
  list.centeredTransform = centeredTransform;
  list.ppd = ppd;
  list.correction = correction;
  list.deffield = deffield;
  list.typeOfField = typeOfField;
  list.defaultPixelValue = defaultPixelValue;
  list.imageCenter = imageCenter;
  list.transformsOrder = transformsOrder;
  list.notbulk = notbulk;
  list.noMeasurementFrame = noMeasurementFrame;
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
  typedef itk::ThinPlateSplineKernelTransform<float, 3> ThinPlateSplineTransformFloatType;
  typedef itk::ThinPlateSplineKernelTransform<double, 3> ThinPlateSplineTransformDoubleType;
  itk::TransformFactory<ThinPlateSplineTransformFloatType>::RegisterTransform();
  itk::TransformFactory<ThinPlateSplineTransformDoubleType>::RegisterTransform();

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;
  // Check the input image pixel type
  GetImageType( inputVolume, pixelType, componentType );

  switch( componentType )
    {
    case itk::ImageIOBase::FLOAT:
      return Do<float>( list );
      break;
    case itk::ImageIOBase::DOUBLE:
      return Do<double>( list );
      break;
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    default:
      std::cerr << "unknown component type" << std::endl;
      break;
    }
  return EXIT_SUCCESS;
}
