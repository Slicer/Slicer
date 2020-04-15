/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DNearestCorrection_h
#define itkDiffusionTensor3DNearestCorrection_h

#include "itkUnaryFunctorImageFilter.h"
#include "vnl/vnl_math.h"
#include <itkMatrix.h>
#include "itkDiffusionTensor3DExtended.h"

namespace itk
{

/** \class DiffusionTensor3DNearestCorrectionFilter
 *
 * This filter is templated over the pixel type of the input image
 * and the pixel type of the output image.
 *
 * The filter will walk over all the pixels in the input image, and for
 * each one of them it will compute the nearest symmetric semi-definite matrix
 * with the Frobenius norm.
 *
 *
 * The filter expect both images to have the same dimension (e.g. both 2D,
 * or both 3D, or both ND).
 * The filter needs DiffusionTensor3D images to work
 *
 */
namespace Functor
{

template <class TInput, class TOutput>
class DiffusionTensor3DNearest
{
public:
  DiffusionTensor3DNearest() = default;
  ~DiffusionTensor3DNearest() = default;
  bool operator!=( const DiffusionTensor3DNearest & other ) const
  {
    return *this != other;
  }

  bool operator==( const DiffusionTensor3DNearest & other ) const
  {
    return !( *this != other );
  }

  inline DiffusionTensor3D<TOutput> operator()
    ( const DiffusionTensor3D<TInput> & tensorA )
  {
    DiffusionTensor3DExtended<double> tensorDouble( tensorA );
    Matrix<double, 3, 3> B;
    Matrix<double, 3, 3> A;
    Matrix<double, 3, 3> transpose;
    Matrix<double, 3, 3> H;
    Matrix<double, 3, 3> mat;
    A = tensorDouble.GetTensor2Matrix();
    transpose = A.GetTranspose();
    B = (A + transpose) / 2;
    transpose = B.GetTranspose();
    H = transpose * B;
    tensorDouble.SetTensorFromMatrix(H);

    typename DiffusionTensor3DExtended<double>::EigenValuesArrayType eigenValues;
    typename DiffusionTensor3DExtended<double>::EigenVectorsMatrixType eigenVectors;
    tensorDouble.ComputeEigenAnalysis( eigenValues, eigenVectors );
    for( int i = 0; i < 3; i++ )
      {
      mat[i][i] = sqrt(eigenValues[i]);
      }
    eigenVectors = eigenVectors.GetTranspose();
    H = eigenVectors * mat * eigenVectors.GetInverse();
    mat = (B + H) / 2;
    tensorDouble.SetTensorFromMatrix( mat );
    tensorDouble.ComputeEigenAnalysis( eigenValues, eigenVectors );  // sometimes very small negative eigenvalues
                                                                     // appear; we suppress them
    mat.Fill(0);
    for( int i = 0; i < 3; i++ )
      {
      mat[i][i] = ( eigenValues[i] <= 0 ? ITK_DIFFUSION_TENSOR_3D_ZERO : eigenValues[i] );
      }
    eigenVectors = eigenVectors.GetTranspose();
    tensorDouble.SetTensorFromMatrix<double>( eigenVectors * mat * eigenVectors.GetInverse() );

    DiffusionTensor3D<TOutput>        tensor;
    for( int i = 0; i < 6; i++ )
      {
      tensor[i] = ( TOutput ) tensorDouble[i];
      }
    return tensor;
  }

};
} // end of Functor namespace

template <class TInputImage, class TOutputImage>
class DiffusionTensor3DNearestCorrectionFilter :
  public
  UnaryFunctorImageFilter<TInputImage, TOutputImage,
                          Functor::DiffusionTensor3DNearest<
                            typename TInputImage::PixelType::ComponentType,
                            typename TOutputImage::PixelType::ComponentType> >
{
public:
  /** Standard class typedefs. */
  typedef DiffusionTensor3DNearestCorrectionFilter Self;
  typedef UnaryFunctorImageFilter<TInputImage, TOutputImage,
                                  Functor::DiffusionTensor3DNearest<typename TInputImage::PixelType,
                                                                    typename TOutputImage::PixelType> >  Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Explicitly deleted functions belong in the public interface */
  DiffusionTensor3DNearestCorrectionFilter( const Self & ) = delete;
  void operator=( const Self & ) = delete;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DNearestCorrectionFilter, UnaryFunctorImageFilter);

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( InputTensorTypeCheck,
                   ( Concept::SameType<DiffusionTensor3D<typename TInputImage::PixelType::ComponentType>,
                                       typename TInputImage::PixelType> ) );
  itkConceptMacro( OutputTensorTypeCheck,
                   ( Concept::SameType<DiffusionTensor3D<typename TOutputImage::PixelType::ComponentType>,
                                       typename TOutputImage::PixelType> ) );

  /** End concept checking */
#endif
protected:
  DiffusionTensor3DNearestCorrectionFilter() = default;
  ~DiffusionTensor3DNearestCorrectionFilter() override = default;
};

} // end namespace itk

#endif
