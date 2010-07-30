#ifndef __itkExponentialDeformationFieldImageFilter2_h
#define __itkExponentialDeformationFieldImageFilter2_h

#include "itkImageToImageFilter.h"
#include "itkDivideByConstantImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkOppositeImageFilter.h"
#include "itkWarpVectorImageFilter.h"
#include "itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction.h"
#include "itkAddImageFilter.h"

namespace itk
{
/** \class ExponentialDeformationFieldImageFilter2
  * \brief Computes a diffeomorphic deformation field as the Lie group
  * exponential of a vector field.
  *
  * ExponentialDeformationFieldImageFilter2 takes a 'smooth' vector field
  * as input and computes the deformation field that is its exponential.
  *
  * Given that both the input and output deformation field are represented as
  * discrete images with pixel type vector, the exponential will be only an
  * estimation and will probably not correspond to a perfect exponential.  The
  * precision of the exponential can be improved at the price of increasing the
  * computation time (number of iterations).
  *
  * The method used for computing the exponential deformation field is
  * an iterative scaling and squaring (cf Arsigny et al., "A
  * Log-Euclidean Framework for Statistics on Diffeomorphisms", MICCAI'06).
  *
  *    \f[
  *      exp(\Phi) = exp( \frac{\Phi}{2^N} )^{2^N}
  *    \f]
  *
  *
  * This filter expects both the input and output images to be of pixel type
  * Vector.
  *
  * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
  *
  * This implementation was taken from the Insight Journal paper:
  * http://hdl.handle.net/1926/510
  *
  * \ingroup ImageToImageFilter
  */
template< class TInputImage, class TOutputImage >
class ITK_EXPORT ExponentialDeformationFieldImageFilter2:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef ExponentialDeformationFieldImageFilter2         Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ExponentialDeformationFieldImageFilter2, ImageToImageFilter);

  /** Some convenient typedefs. */
  typedef TInputImage                            InputImageType;
  typedef typename InputImageType::Pointer       InputImagePointer;
  typedef typename InputImageType::ConstPointer  InputImageConstPointer;
  typedef typename InputImageType::PixelType     InputPixelType;
  typedef typename InputPixelType::RealValueType InputPixelRealValueType;

  typedef TOutputImage                        OutputImageType;
  typedef typename OutputImageType::Pointer   OutputImagePointer;
  typedef typename OutputImageType::PixelType OutputPixelType;

  /** Specify the maximum number of iteration. */
  itkSetMacro(MaximumNumberOfIterations, unsigned int);
  itkGetConstMacro(MaximumNumberOfIterations, unsigned int);

  /** If AutomaticNumberOfIterations is off, the number of iterations is
    * given by MaximumNumberOfIterations. If it is on, we try to get
    * the lowest good number (which may not be larger than
    * MaximumNumberOfIterations ) */
  itkSetMacro(AutomaticNumberOfIterations, bool);
  itkGetConstMacro(AutomaticNumberOfIterations, bool);
  itkBooleanMacro(AutomaticNumberOfIterations);

  /** If ComputeInverse is on, the filter will compute the exponential
    * of the opposite (minus) of the input vector field. The output deformation
    * fields computed with ComputeInverse set to on and off respectively
    * therefore represent spatial transformations that are inverses of
    * each other. */
  itkSetMacro(ComputeInverse, bool);
  itkGetConstMacro(ComputeInverse, bool);
  itkBooleanMacro(ComputeInverse);

  /** Image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(PixelDimension, unsigned int,
                      InputPixelType::Dimension);
  itkStaticConstMacro(OutputPixelDimension, unsigned int,
                      OutputPixelType::Dimension);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( OutputHasNumericTraitsCheck,
                   ( Concept::HasNumericTraits< typename OutputPixelType::ValueType > ) );
  itkConceptMacro( SameDimensionCheck1,
                   ( Concept::SameDimension< ImageDimension, OutputImageDimension > ) );
  itkConceptMacro( SameDimensionCheck2,
                   ( Concept::SameDimension< ImageDimension, PixelDimension > ) );
  itkConceptMacro( SameDimensionCheck3,
                   ( Concept::SameDimension< ImageDimension, OutputPixelDimension > ) );
  /** End concept checking */
#endif
protected:
  ExponentialDeformationFieldImageFilter2();
  virtual ~ExponentialDeformationFieldImageFilter2() {}

  void PrintSelf(std::ostream & os, Indent indent) const;

  /** GenerateData() */
  void GenerateData();

  typedef typename InputImageType::RegionType RegionType;

  typedef DivideByConstantImageFilter<
    InputImageType,
    InputPixelRealValueType, OutputImageType >         DivideByConstantType;

  typedef CastImageFilter<
    InputImageType, OutputImageType >                   CasterType;

  typedef OppositeImageFilter<
    InputImageType, OutputImageType >                   OppositerType;

  typedef WarpVectorImageFilter<
    OutputImageType,
    OutputImageType, OutputImageType >                  VectorWarperType;

  typedef VectorLinearInterpolateNearestNeighborExtrapolateImageFunction<
    OutputImageType, double >                            FieldInterpolatorType;

  typedef AddImageFilter<
    OutputImageType, OutputImageType, OutputImageType > AdderType;

  typedef typename DivideByConstantType::Pointer     DivideByConstantPointer;
  typedef typename CasterType::Pointer               CasterPointer;
  typedef typename OppositerType::Pointer            OppositerPointer;
  typedef typename VectorWarperType::Pointer         VectorWarperPointer;
  typedef typename FieldInterpolatorType::Pointer    FieldInterpolatorPointer;
  typedef typename FieldInterpolatorType::OutputType FieldInterpolatorOutputType;
  typedef typename AdderType::Pointer                AdderPointer;
private:
  ExponentialDeformationFieldImageFilter2(const Self &); // purposely not
                                                         // implemented
  void operator=(const Self &);                          // purposely not

  // implemented

  bool         m_AutomaticNumberOfIterations;
  unsigned int m_MaximumNumberOfIterations;

  bool m_ComputeInverse;

  DivideByConstantPointer m_Divider;
  CasterPointer           m_Caster;
  OppositerPointer        m_Oppositer;
  VectorWarperPointer     m_Warper;
  AdderPointer            m_Adder;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkExponentialDeformationFieldImageFilter2.txx"
#endif

#endif
