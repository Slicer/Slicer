
#ifndef itkNewOtsuThresholdImageCalculator_h
#define itkNewOtsuThresholdImageCalculator_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkNumericTraits.h"

namespace itk
{

/** \class NewOtsuThresholdImageCalculator
 * \brief Computes the Otsu's threshold for an image.
 *
 * This calculator computes the Otsu's threshold which separates an image
 * into foreground and background components. The method relies on a
 * histogram of image intensities. The basic idea is to maximize the
 * between-class variance.
 *
 * This class is templated over the input image type.
 *
 * \warning This method assumes that the input image consists of scalar pixel
 * types.
 *
 * \ingroup Operators
 */
template <class TInputImage>
class NewOtsuThresholdImageCalculator : public Object
{
public:
  /** Standard class typedefs. */
  typedef NewOtsuThresholdImageCalculator Self;
  typedef Object                          Superclass;
  typedef SmartPointer<Self>              Pointer;
  typedef SmartPointer<const Self>        ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(NewOtsuThresholdImageCalculator, Object);

  /** Type definition for the input image. */
  typedef TInputImage  ImageType;

  /** Pointer type for the image. */
  typedef typename TInputImage::Pointer  ImagePointer;

  /** Const Pointer type for the image. */
  typedef typename TInputImage::ConstPointer ImageConstPointer;

  /** Type definition for the input image pixel type. */
  typedef typename TInputImage::PixelType PixelType;

  /** Set the input image. */
  itkSetConstObjectMacro(Image,ImageType);

  /** Compute the Otsu's threshold for the input image. */
  void Compute();

  /** Return the Otsu's threshold value. */
  itkGetMacro(Threshold,PixelType);

  /** Set/Get the number of histogram bins. Default is 128. */
  itkSetClampMacro( NumberOfHistogramBins, unsigned long, 1,
                    NumericTraits<unsigned long>::max() );
  itkGetMacro( NumberOfHistogramBins, unsigned long );

  itkSetMacro(Omega, double);
  itkGetMacro(Omega,double);

protected:
  NewOtsuThresholdImageCalculator();
  ~NewOtsuThresholdImageCalculator() override  = default;
  void PrintSelf(std::ostream& os, Indent indent) const override;

private:
  NewOtsuThresholdImageCalculator(const Self&) = delete;
  void operator=(const Self&) = delete;

  PixelType            m_Threshold;
  unsigned long        m_NumberOfHistogramBins;
  double               m_Omega;
  ImageConstPointer    m_Image;

};

} /// end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNewOtsuThresholdImageCalculator.txx"
#endif

#endif
