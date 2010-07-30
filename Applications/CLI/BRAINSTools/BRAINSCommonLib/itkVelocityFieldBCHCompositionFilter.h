#ifndef __itkVelocityFieldBCHCompositionFilter_h
#define __itkVelocityFieldBCHCompositionFilter_h

#include <itkImageToImageFilter.h>
#include <itkNaryAddImageFilter.h>
#include "itkVelocityFieldLieBracketFilter.h"
#include <itkMultiplyByConstantImageFilter.h>

namespace itk
{
/** \class VelocityFieldBCHCompositionFilter
  * \brief Compute Baker-Campbell-Hausdorff formula on two vector fields.
  *
  * See M. Bossa, M. Hernandez and S.Olmos, "Contributions to 3D diffeomorphic
  *atlas
  * estimation: Application to brain images", Proc. of MICCAI’07
  * and
  * T. Vercauteren, X. Pennec, A. Perchant and N. Ayache,
  * "Symmetric Log-Domain Diffeomorphic Registration: A Demons-based Approach",
  * Proc. of MICCAI 2008.
  *
  * This class is templated over the input field type and the output
  * field type.
  *
  * Velocity fields are represented as images whose pixel type are vector type
  * with N elements, where N is the dimension of the image.
  * The vector type must support element access via operator[]. It is assumed
  * that the vector elements behave like floating point scalars.
  *
  * The number of approximation terms to used in the BCH approximation is set
  *via
  * SetNumberOfApproximationTerms method.
  *
  * \warning This filter assumes that the input field type and velocity field
  *type
  * both have the same number of dimensions.
  *
  * \author Florence Dru, INRIA and Tom Vercauteren, MKT
  */
template< class TInputImage, class TOutputImage >
class ITK_EXPORT VelocityFieldBCHCompositionFilter:
  public InPlaceImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef VelocityFieldBCHCompositionFilter               Self;
  typedef InPlaceImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Some convenient typedefs. */
  typedef TInputImage                           InputFieldType;
  typedef typename InputFieldType::PixelType    InputFieldPixelType;
  typedef typename InputFieldType::Pointer      InputFieldPointer;
  typedef typename InputFieldType::ConstPointer InputFieldConstPointer;

  typedef TOutputImage                           OutputFieldType;
  typedef typename OutputFieldType::PixelType    OutputFieldPixelType;
  typedef typename OutputFieldType::Pointer      OutputFieldPointer;
  typedef typename OutputFieldType::ConstPointer OutputFieldConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VelocityFieldBCHCompositionFilter, InPlaceImageFilter);

  /** Set/Get the NumberOfApproximationTerms used in the BCH approximation. */
  itkSetMacro(NumberOfApproximationTerms, unsigned int);
  itkGetConstMacro(NumberOfApproximationTerms, unsigned int);
protected:
  VelocityFieldBCHCompositionFilter();
  ~VelocityFieldBCHCompositionFilter() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

  /**
    * GenerateData()
    */
  void GenerateData();

  /** Adder type. */
  typedef NaryAddImageFilter< InputFieldType, InputFieldType > AdderType;
  typedef typename AdderType::Pointer                          AdderPointer;

  /** Lie bracket calculator type. */
  typedef VelocityFieldLieBracketFilter< InputFieldType, InputFieldType > LieBracketFilterType;
  typedef typename LieBracketFilterType::Pointer                          LieBracketFilterPointer;

  /** Multiplier type. */
  typedef MultiplyByConstantImageFilter< InputFieldType, double, InputFieldType > MultiplierType;
  typedef typename MultiplierType::Pointer                                        MultiplierPointer;

  /** Set/Get the adder. */
  itkSetObjectMacro(Adder, AdderType);
  itkGetObjectMacro(Adder, AdderType);

  /** Set/Get the multipliers. */
  itkSetObjectMacro(Multiplier, MultiplierType);
  itkGetObjectMacro(Multiplier, MultiplierType);
  itkSetObjectMacro(Multiplier2, MultiplierType);
  itkGetObjectMacro(Multiplier2, MultiplierType);

  /** Set/Get the Lie bracket filters. */
  itkSetObjectMacro(LieBracketFilter, LieBracketFilterType);
  itkGetObjectMacro(LieBracketFilter, LieBracketFilterType);
  itkSetObjectMacro(LieBracketFilter2, LieBracketFilterType);
  itkGetObjectMacro(LieBracketFilter2, LieBracketFilterType);

#if ( ITK_VERSION_MAJOR < 3 ) || ( ITK_VERSION_MAJOR == 3 && ITK_VERSION_MINOR < 13 )
  virtual void SetInPlace(const bool b)
  {
    // Work-around for http://www.itk.org/Bug/view.php?id=8672
    if ( b ) { itkWarningMacro("A more recent version of ITK is required for this filter to run inplace"); }
    this->Superclass::SetInPlace(false);
  }

#endif
private:
  // purposely not implemented
  VelocityFieldBCHCompositionFilter(const Self &);
  void operator=(const Self &);

  AdderPointer            m_Adder;
  LieBracketFilterPointer m_LieBracketFilter;
  LieBracketFilterPointer m_LieBracketFilter2;
  MultiplierPointer       m_Multiplier;
  MultiplierPointer       m_Multiplier2;
  unsigned int            m_NumberOfApproximationTerms;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkVelocityFieldBCHCompositionFilter.txx"
#endif

#endif
