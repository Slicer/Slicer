#ifndef __itkVelocityFieldLieBracketFilter_h
#define __itkVelocityFieldLieBracketFilter_h

#include <itkImageToImageFilter.h>
#include <itkVectorCentralDifferenceImageFunction.h>
#include <itkImageRegionConstIteratorWithIndex.h>

namespace itk
{
/** \class VelocityFieldLieBracketFilter
 * \brief Compute the Lie bracket of two vector fields
 *  using the formula [v,u](p) = Jac(v)(p).u(p) − Jac(u)(p).v(p) (1)
 *
 * See M. Bossa, M. Hernandez and S.Olmos, "Contributions to 3D diffeomorphic atlas
 * estimation: Application to brain images", Proc. of MICCAI’07
 * and
 * T. Vercauteren, X. Pennec, A. Perchant and N. Ayache,
 * "Symmetric Log-Domain Diffeomorphic Registration: A Demons-based Approach",
 * Proc. of MICCAI 2008.
 * 
 * \note Most authors define the Lie bracket as the opposite of (1). Numerical simulations,
 * and personal communication with M. Bossa, showed the relevance of this definition.
 * Future research will aim at fully understanding the reason of this discrepancy.
 *
 * Velocity fields are represented as images whose pixel type are vector type
 * with N elements, where N is the dimension of the image. 
 * The vector type must support element access via operator[]. It is assumed 
 * that the vector elements behave like floating point scalars.
 *
 * This class is templated over the input field type and the output 
 * field type.
 * 
 * \warning This filter assumes that the input field type and velocity field type
 * both have the same number of dimensions.
 *
 * \author Florence Dru, INRIA and Tom Vercauteren, MKT
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT VelocityFieldLieBracketFilter :
   public ImageToImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef VelocityFieldLieBracketFilter                  Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>   Superclass;
  typedef SmartPointer<Self>                             Pointer;
  typedef SmartPointer<const Self>                       ConstPointer;

  /** Some convenient typedefs. */
  typedef TInputImage                                    InputFieldType;
  typedef typename InputFieldType::PixelType             InputFieldPixelType;
  typedef typename InputFieldType::Pointer               InputFieldPointer;
  typedef typename InputFieldType::ConstPointer          InputFieldConstPointer;
  typedef typename InputFieldType::RegionType            InputFieldRegionType;

  typedef TOutputImage                                   OutputFieldType;
  typedef typename OutputFieldType::PixelType            OutputFieldPixelType;
  typedef typename OutputFieldType::Pointer              OutputFieldPointer;
  typedef typename OutputFieldType::ConstPointer         OutputFieldConstPointer;
  typedef typename OutputFieldType::RegionType           OutputFieldRegionType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( VelocityFieldLieBracketFilter, ImageToImageFilter );

  /** Gradient calculator type. */
  typedef itk::VectorCentralDifferenceImageFunction<InputFieldType>
     InputFieldGradientCalculatorType;

  /** Gradient type. */
  typedef typename InputFieldGradientCalculatorType::OutputType
     InputFieldGradientType;

  /** VelocityFieldLieBracketFilter needs a larger input requested region than
   * the output requested region.  As such, VelocityFieldLieBracketFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(InvalidRequestedRegionError);
 
  /** ImageDimension constants */
  itkStaticConstMacro( InputFieldDimension, unsigned int,
                       TInputImage::ImageDimension);
  itkStaticConstMacro( OutputFieldDimension, unsigned int,
                       TOutputImage::ImageDimension);
  itkStaticConstMacro( InputFieldPixelDimension, unsigned int,
                       InputFieldPixelType::Dimension );
  itkStaticConstMacro( OutputFieldPixelDimension, unsigned int,
                       OutputFieldPixelType::Dimension );

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(SameDimensionCheck1,
                  (Concept::SameDimension<InputFieldDimension,OutputFieldDimension>));
  itkConceptMacro(SameDimensionCheck2,
                  (Concept::SameDimension<InputFieldDimension,InputFieldPixelDimension>));
  itkConceptMacro(SameDimensionCheck3,
                  (Concept::SameDimension<OutputFieldDimension,OutputFieldPixelDimension>));
  /** End concept checking */
#endif

protected:
   VelocityFieldLieBracketFilter();
  ~VelocityFieldLieBracketFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** VelocityFieldLieBracketFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData() routine
   * which is called for each processing thread. The output image data is
   * allocated automatically by the superclass prior to calling
   * ThreadedGenerateData().  ThreadedGenerateData can only write to the
   * portion of the output image specified by the parameter
   * "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData()  */
  void ThreadedGenerateData(const OutputFieldRegionType& outputRegionForThread,
                            int threadId );

  void BeforeThreadedGenerateData();
  
  /** Set right and left gradient calculators. */
  itkSetObjectMacro( RightGradientCalculator, InputFieldGradientCalculatorType );
  itkSetObjectMacro( LeftGradientCalculator,  InputFieldGradientCalculatorType );
  
  /** Get right and left gradient calculators. */
  itkGetObjectMacro( RightGradientCalculator, InputFieldGradientCalculatorType );
  itkGetObjectMacro( LeftGradientCalculator,  InputFieldGradientCalculatorType );

private:
  VelocityFieldLieBracketFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typename InputFieldGradientCalculatorType::Pointer m_RightGradientCalculator;
  typename InputFieldGradientCalculatorType::Pointer m_LeftGradientCalculator;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVelocityFieldLieBracketFilter.txx"
#endif

#endif
